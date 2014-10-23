/* This file is part of the 'stringi' package for R.
 * Copyright (c) 2013-2014, Marek Gagolewski and Bartek Tartanus
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "stri_stringi.h"
#include "stri_container_utf8.h"
#include "stri_container_integer.h"
#include "stri_container_logical.h"
#include "stri_container_regex.h"
#include <deque>
#include <utility>
using namespace std;


/**
 * Split a string into parts.
 *
 * The pattern matches identify delimiters that separate the input into fields.
 * The input data between the matches becomes the fields themselves.
 *
 * @param str character vector
 * @param pattern character vector
 * @param n_max integer vector
 * @param opts_regex
 * @param tokens_only single logical value
 * @param simplify single logical value
 * 
 * @return list of character vectors  or character matrix
 *
 * @version 0.1-?? (Marek Gagolewski, 2013-06-21)
 *
 * @version 0.1-?? (Marek Gagolewski, 2013-07-10)
 *          BUGFIX: wrong behavior on empty str
 *
 * @version 0.1-24 (Marek Gagolewski, 2014-03-11)
 *          Added missing utext_close call to avoid memleaks
 * 
 * @version 0.3-1 (Marek Gagolewski, 2014-10-19)
 *          added tokens_only param
 * 
 * @version 0.3-1 (Marek Gagolewski, 2014-10-24)
 *          added simplify param
 */
SEXP stri_split_regex(SEXP str, SEXP pattern, SEXP n_max, SEXP omit_empty, 
                      SEXP tokens_only, SEXP simplify, SEXP opts_regex)
{
   str = stri_prepare_arg_string(str, "str");
   pattern = stri_prepare_arg_string(pattern, "pattern");
   n_max = stri_prepare_arg_integer(n_max, "n_max");
   omit_empty = stri_prepare_arg_logical(omit_empty, "omit_empty");
   bool tokens_only1 = stri__prepare_arg_logical_1_notNA(tokens_only, "tokens_only");
   bool simplify1 = stri__prepare_arg_logical_1_notNA(simplify, "simplify");
   R_len_t vectorize_length = stri__recycling_rule(true, 4,
      LENGTH(str), LENGTH(pattern), LENGTH(n_max), LENGTH(omit_empty));

   uint32_t pattern_flags = StriContainerRegexPattern::getRegexFlags(opts_regex);

   UText* str_text = NULL; // may potentially be slower, but definitely is more convenient!
   STRI__ERROR_HANDLER_BEGIN
   StriContainerUTF8      str_cont(str, vectorize_length);
   StriContainerInteger   n_max_cont(n_max, vectorize_length);
   StriContainerLogical   omit_empty_cont(omit_empty, vectorize_length);
   StriContainerRegexPattern pattern_cont(pattern, vectorize_length, pattern_flags);

   SEXP ret;
   STRI__PROTECT(ret = Rf_allocVector(VECSXP, vectorize_length));

   for (R_len_t i = pattern_cont.vectorize_init();
         i != pattern_cont.vectorize_end();
         i = pattern_cont.vectorize_next(i))
   {
      if (n_max_cont.isNA(i) || omit_empty_cont.isNA(i)) {
         SET_VECTOR_ELT(ret, i, stri__vector_NA_strings(1));
         continue;
      }

      int  n_max_cur        = n_max_cont.get(i);
      int  omit_empty_cur   = omit_empty_cont.get(i);

      STRI__CONTINUE_ON_EMPTY_OR_NA_STR_PATTERN(str_cont, pattern_cont,
         SET_VECTOR_ELT(ret, i, stri__vector_NA_strings(1));,
         SET_VECTOR_ELT(ret, i, stri__vector_empty_strings((omit_empty_cur || n_max_cur == 0)?0:1));)

      R_len_t     str_cur_n = str_cont.get(i).length();
      const char* str_cur_s = str_cont.get(i).c_str();

      if (n_max_cur >= INT_MAX-1)
         throw StriException(MSG__EXPECTED_SMALLER, "n_max");
      else if (n_max_cur < 0)
         n_max_cur = INT_MAX;
      else if (n_max_cur == 0) {
         SET_VECTOR_ELT(ret, i, Rf_allocVector(STRSXP, 0));
         continue;
      }
      else if (tokens_only1)
         n_max_cur++; // we need to do one split ahead here

      UErrorCode status = U_ZERO_ERROR;
      RegexMatcher *matcher = pattern_cont.getMatcher(i); // will be deleted automatically
      str_text = utext_openUTF8(str_text, str_cont.get(i).c_str(), str_cont.get(i).length(), &status);
      if (U_FAILURE(status)) throw StriException(status);

      matcher->reset(str_text);


      R_len_t k;
      deque< pair<R_len_t, R_len_t> > fields; // byte based-indices
      fields.push_back(pair<R_len_t, R_len_t>(0,0));

      for (k=1; k < n_max_cur && (int)matcher->find(); ) {
         R_len_t s1 = (R_len_t)matcher->start(status);
         R_len_t s2 = (R_len_t)matcher->end(status);
         if (U_FAILURE(status)) throw StriException(status);

         if (omit_empty_cur && fields.back().first == s1)
            fields.back().first = s2; // don't start new field
         else {
            fields.back().second = s1;
            fields.push_back(pair<R_len_t, R_len_t>(s2, s2)); // start new field here
            ++k; // another field
         }
      }
      fields.back().second = str_cur_n;
      if (omit_empty_cur && fields.back().first == fields.back().second)
         fields.pop_back();
         
      if (tokens_only1 && n_max_cur < INT_MAX) {
         n_max_cur--; // one split ahead could have been made, see above
         while (fields.size() > (size_t)n_max_cur)
            fields.pop_back(); // get rid of the remainder
      }

      SEXP ans;
      STRI__PROTECT(ans = Rf_allocVector(STRSXP, fields.size()));

      deque< pair<R_len_t, R_len_t> >::iterator iter = fields.begin();
      for (k = 0; iter != fields.end(); ++iter, ++k) {
         pair<R_len_t, R_len_t> curoccur = *iter;
         SET_STRING_ELT(ans, k,
            Rf_mkCharLenCE(str_cur_s+curoccur.first, curoccur.second-curoccur.first, CE_UTF8));
      }

      SET_VECTOR_ELT(ret, i, ans);
      STRI__UNPROTECT(1);
   }

   if (str_text) {
      utext_close(str_text);
      str_text = NULL;
   }
   
   if (simplify1) {
      ret = stri_list2matrix(ret, Rf_ScalarLogical(TRUE),
         stri__vector_NA_strings(1));
   }

   STRI__UNPROTECT_ALL
   return ret;
   STRI__ERROR_HANDLER_END({
      if (str_text) {
         utext_close(str_text);
         str_text = NULL;
      }
   })
}
