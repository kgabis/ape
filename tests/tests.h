#ifndef tests_h
#define tests_h

#include "collections.h"
#include "error.h"

void print_errors(ptrarray(error_t) *errors);
ptrarray(char)* get_lines(const char *code);

#ifdef APE_TESTS_MAIN
int main(void);
#else
int tests_main(void);
#endif

#endif /* tests_h */
