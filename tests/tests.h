#ifndef tests_h
#define tests_h

#include "collections.h"
#include "errors.h"

void print_errors(errors_t *errors);
ptrarray(char)* get_lines(const char *code);

#ifdef APE_TESTS_MAIN
int main(void);
#else
int tests_main(void);
#endif

#endif /* tests_h */
