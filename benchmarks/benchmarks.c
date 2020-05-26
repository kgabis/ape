#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define ARRAY_LEN(array) (int)(sizeof(array) / sizeof(array[0]))
#include "ape.h"

static bool execute_file(const char *filename, bool must_succeed);
static void *counted_malloc(size_t size);
static void counted_free(void *ptr);
static void print_ape_errors(ape_t *ape);

static int g_malloc_count;

#ifdef APE_BENCHMARKS_MAIN
int main(int argc, char *argv[]) {
    int tests_len = argc - 1;
    char **tests = argv + 1;
#else
    int benchmarks_main() {
        const char *tests[] = {
            "raytracer_profile.bn",
            "raytracer_profile_optimised.bn",
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
        clock_t start = clock();
        execute_file(test, true);
        clock_t end = clock();
        float seconds = (float)(end - start) / CLOCKS_PER_SEC;
        printf("%1.10g seconds\n", (double)seconds);
        assert(g_malloc_count == 0);
    }
    ape_set_memory_functions(malloc, free);
    return 0;
}

static bool execute_file(const char *filename, bool must_succeed) {
    ape_t *ape = ape_make();

    ape_set_gc_interval(ape, 10000);

    ape_program_t *program = ape_compile_file(ape, filename);
    if (!program || ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }
    ape_execute_program(ape, program);
    if (ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }
    ape_program_destroy(program);
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


static void print_ape_errors(ape_t *ape) {
    int count = ape_errors_count(ape);
    for (int i = 0; i < count; i++) {
        const ape_error_t *err = ape_get_error(ape, i);
        char *err_str = ape_error_serialize(err);
        puts(err_str);
        counted_free(err_str);
    }
}
