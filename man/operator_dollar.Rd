% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/utils.R
\name{\%s$\%}
\alias{\%s$\%}
\alias{operator_dollar}
\alias{oper_dollar}
\alias{\%stri$\%}
\title{C-Style Formatting with sprintf as a Binary Operator}
\usage{
e1 \%s$\% e2

e1 \%stri$\% e2
}
\arguments{
\item{e1}{a single format string, see \code{\link{sprintf}} for syntax}

\item{e2}{a list of arguments to be passed to \code{\link{sprintf}}
or a single atomic vector}
}
\value{
Returns a character vector.
}
\description{
Provides access to base R's \code{\link{sprintf}} in form of a binary
operator in a way similar to Python's \code{\%} overloaded for strings.
}
\details{
Vectorized over \code{e2}.

\code{e1 \%s$\% atomic_vector} is equivalent to
\code{e1 \%s$\% list(atomic_vector)}.
}
\examples{
"value='\%d'" \%s$\% 3
"value='\%d'" \%s$\% 1:3
"\%s='\%d'" \%s$\% list("value", 3)
"\%s='\%d'" \%s$\% list("value", 1:3)
"\%s='\%d'" \%s$\% list(c("a", "b", "c"), 1)
"\%s='\%d'" \%s$\% list(c("a", "b", "c"), 1:3)

}
