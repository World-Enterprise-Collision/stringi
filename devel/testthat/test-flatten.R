require(testthat)
context("test-flatten.R")

test_that("stri_flatten", {
    
    # basic tests (ASCII, border-line):
    expect_warning(stri_flatten(LETTERS, collapse = LETTERS))
    expect_identical(stri_flatten(c("", "", "", "", "")), "")
    expect_identical(stri_flatten(c(character(0))), "")
    expect_identical(stri_flatten(c(character(0)), collapse = "!"), "")
    expect_identical(stri_flatten(NA, NA), NA_character_)
    expect_identical(stri_flatten("a", NA), NA_character_)
    expect_identical(stri_flatten(NA, "a"), NA_character_)
    expect_identical(stri_flatten(c(NA, NA, NA)), NA_character_)
    expect_identical(stri_flatten(c("A", "", "B", "", "C")), "ABC")
    expect_identical(stri_flatten(c("A", "", "B", NA, "C")), NA_character_)
    expect_identical(stri_flatten(c(LETTERS)), paste(LETTERS, collapse = ""))
    expect_identical(stri_flatten("0987654321"), "0987654321")
    expect_identical(stri_flatten("ąĄ"), paste("ąĄ", collapse = ""))
    expect_identical(stri_flatten(1:1000, "test"), paste(1:1000, collapse = "test"))
    # collapse-separators
    expect_error(stri_flatten(LETTERS, collapse = character(0)))
    expect_identical(stri_flatten(letters, collapse = " "), paste(letters, collapse = " "))
    expect_identical(stri_flatten(letters, collapse = "#$"), paste(letters, collapse = "#$"))
    expect_identical(stri_flatten(letters, collapse = "ąĄ"), paste(letters, collapse = "ąĄ"))
    
    expect_identical(stri_flatten(c("A", "", "B", NA, "C"), na_empty = TRUE), "ABC")
    expect_identical(stri_flatten(c("A", "", "B", NA, "C"), collapse = ",", na_empty = TRUE), 
        "A,,B,,C")
    expect_identical(stri_flatten(c("A", "", "B", NA, "C"), na_empty = TRUE, omit_empty = TRUE), 
        "ABC")
    expect_identical(stri_flatten(c("A", "", "B", NA, "C"), collapse = ",", na_empty = TRUE, 
        omit_empty = TRUE), "A,B,C")
    expect_identical(stri_flatten(c(NA, "", "A", "", "B", NA, "C"), na_empty = TRUE), 
        "ABC")
    expect_identical(stri_flatten(c(NA, "", "A", "", "B", NA, "C"), collapse = ",", 
        na_empty = TRUE), ",,A,,B,,C")
    expect_identical(stri_flatten(c(NA, "", "A", "", "B", NA, "C"), na_empty = TRUE, 
        omit_empty = TRUE), "ABC")
    expect_identical(stri_flatten(c(NA, "", "A", "", "B", NA, "C"), collapse = ",", 
        na_empty = TRUE, omit_empty = TRUE), "A,B,C")
})
