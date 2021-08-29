#include "tests.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "test_lexer.h"
#include "test_parser.h"
#include "test_code.h"
#include "test_vm.h"
#include "test_symbol_table.h"
#include "test_api.h"

#include "compiler.h"

#ifdef APE_TESTS_MAIN
int main() {
#else
int tests_main() {
#endif
#if 0
} // unconfuse xcode
#endif
    lexer_test();
    parser_test();
    code_test();
    symbol_table_test();
    vm_test();
    api_test();
    return 0;
}

void print_errors(errors_t *errors) {
    int count = errors_get_count(errors);
    for (int i = 0; i < count; i++) {
        const error_t *err = errors_get(errors, i);
        int line_num = err->pos.line;
        int col_num = err->pos.column;
        error_type_t type = err->type;
        const char *type_str = error_type_to_string(err->type);
        if (line_num >= 0 && err->pos.file && line_num < ptrarray_count(err->pos.file->lines)) {
            const char *line_str = ptrarray_get(err->pos.file->lines, line_num);
            puts(line_str);
        }
        if (type == ERROR_RUNTIME) {
            printf("%s ERROR: %s\n", type_str, err->message);
        } else {
            for (int j = 0; j < col_num; j++) {
                printf(" ");
            }
            printf("^\n");
            printf("%s ERROR on %d:%d: %s\n", type_str,
                   line_num, col_num, err->message);
        }
    }
}

ptrarray(char)* get_lines(const char *code) {
    ptrarray(char)* res = ptrarray_make(NULL);
    if (!code) {
        return res;
    }
    const char *line_start = code;
    const char *line_end = strchr(line_start, '\n');
    while (line_end != NULL) {
        long len = line_end - line_start;
        char *line = strndup(line_start, len);
        ptrarray_add(res, line);
        line_start = line_end + 1;
        line_end = strchr(line_start, '\n');
    }
    char *rest = strdup(line_start);
    ptrarray_add(res, rest);
    return res;
}
