#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"

#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define ARRAY_LEN(array) (int)(sizeof(array) / sizeof(array[0]))
#include "ape.h"

static bool execute_program(const char *code, bool must_succeed);
static void *counted_malloc(size_t size);
static void counted_free(void *ptr);
static char* read_file(const char * filename);
static void print_ape_errors(ape_t *ape, const char *code);

static int g_malloc_count;

#ifdef APE_BENCHMARKS_MAIN
int main(int argc, char *argv[]) {
    int tests_len = argc - 1;
    char **tests = argv + 1;
#else
int benchmarks_main() {
    const char *tests[] = {
        "primes.bn",
        "fibonacci.bn",
    };
    int tests_len = ARRAY_LEN(tests);
#endif
#if 0
} // unconfuse xcode
#endif

    ape_set_memory_functions(counted_malloc, counted_free);
    for (int i = 0; i < tests_len; i++) {
        const char *test = tests[i];
        g_malloc_count = 0;
        printf("Benchmarking %s: \n", test);
        char *program = read_file(test);
        assert(program);
        clock_t start = clock();
        execute_program(program, true);
        clock_t end = clock();
        float seconds = (float)(end - start) / CLOCKS_PER_SEC;
        printf("%1.10g seconds\n", (double)seconds);
        free(program);
        assert(g_malloc_count == 0);
    }
    ape_set_memory_functions(malloc, free);
    return 0;
}

static bool execute_program(const char *code, bool must_succeed) {
    ape_t *ape = ape_make();

    ape_set_gc_interval(ape, 100000);

    ape_execute(ape, code);
    if (ape_has_errors(ape)) {
        if (!must_succeed) {
            ape_destroy(ape);
            return true;
        }
        print_ape_errors(ape, code);
        assert(false);
    }
    if (!must_succeed) {
        assert(false);
    }
    ape_destroy(ape);
    return true;
}

static void *counted_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    void *res = malloc(size);
    g_malloc_count++;
    assert(res != NULL);
    return res;
}

static void counted_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    g_malloc_count--;
    free(ptr);
}

static char * read_file(const char * filename) {
    FILE *fp = fopen(filename, "r");
    size_t size_to_read = 0;
    size_t size_read = 0;
    long pos;
    char *file_contents;
    if (!fp) {
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    pos = ftell(fp);
    if (pos < 0) {
        fclose(fp);
        return NULL;
    }
    size_to_read = pos;
    rewind(fp);
    file_contents = (char*)malloc(sizeof(char) * (size_to_read + 1));
    if (!file_contents) {
        fclose(fp);
        return NULL;
    }
    size_read = fread(file_contents, 1, size_to_read, fp);
    if (size_read == 0 || ferror(fp)) {
        fclose(fp);
        free(file_contents);
        return NULL;
    }
    fclose(fp);
    file_contents[size_read] = '\0';
    return file_contents;
}

static void print_ape_errors(ape_t *ape, const char *code) {
    int count = ape_errors_count(ape);
    for (int i = 0; i < count; i++) {
        const ape_error_t *err = ape_get_error(ape, i);
        ape_error_type_t type = ape_error_get_type(err);
        const char *type_str = ape_error_get_type_string(err);
        int line = ape_error_get_line_number(err);
        int col = ape_error_get_column_number(err);
        if (type == APE_ERROR_RUNTIME) {
            printf("%s ERROR: %s\n", type_str, ape_error_get_message(err));
        } else {
            for (int j = 0; j < ape_error_get_column_number(err); j++) {
                printf(" ");
            }
            printf("^\n");
            printf("%s ERROR on %d:%d: %s\n", type_str,
                   line, col, ape_error_get_message(err));
        }
    }
}

#pragma GCC diagnostic pop
