#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
#endif
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "test_api.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ape.h"
#include "common.h"
#include "collections.h"

#include "tests.h"

typedef struct failing_alloc {
    int allocation_to_fail;
    int alloc_count;
    int total_count;
    bool has_failed;
    bool should_fail;
} failing_alloc_t;

static void test_repl(void);
static void test_program(void);
static void test_compiling(void);
static void test_fails(void);
static void test_calling_functions(void);
static void test_traceback(void);
static void test_various(void);
static void test_time_limit(void);
static void test_allocation_fails(void);

static void *failing_malloc(void *ctx, size_t size);
static void failing_free(void *ctx, void *ptr);

static void *counted_malloc(void *ctx, size_t size);
static void counted_free(void *ctx, void *ptr);

static char* read_file(const char * filename);
static void print_ape_errors(ape_t *ape);
static size_t stdout_write(void* context, const void *data, size_t size);

static ape_object_t external_fn_test(ape_t *ape, void *data, int argc, ape_object_t *args);
static ape_object_t square_array_fun(ape_t *ape, void *data, int argc, ape_object_t *args);
static ape_object_t make_test_dict_fun(ape_t *ape, void *data, int argc, ape_object_t *args);
static ape_object_t test_check_args_fun(ape_t *ape, void *data, int argc, ape_object_t *args);
static ape_object_t custom_error_fun(ape_t *ape, void *data, int argc, ape_object_t *args);
static ape_object_t add_fun(ape_t *ape, void *data, int argc, ape_object_t *args);
static ape_object_t fourtytwo_fun(ape_t *ape, void *data, int argc, ape_object_t *args);
static ape_object_t vec2_add_fun(ape_t *ape, void *data, int argc, ape_object_t *args);
static ape_object_t vec2_sub_fun(ape_t *ape, void *data, int argc, ape_object_t *args);

static int g_external_fn_test;
    
static char g_stdout_buf[1024];

void api_test() {
    puts("### API test");
    test_repl();
    test_program();
    test_compiling();
    test_fails();
    test_calling_functions();
    test_traceback();
    test_various();
    test_time_limit();
    test_allocation_fails();
    puts("\tOK");
}

// INTERNAL
static void test_repl() {
    const char *filename = "repl_inputs.ape";
    char *inputs = read_file(filename);
    assert(inputs);
    ptrarray(char) *lines = get_lines(inputs);

    int malloc_count = 0;
    ape_t *ape = ape_make_ex(counted_malloc, counted_free, &malloc_count);

    ape_set_repl_mode(ape, true);
    ape_set_stdout_write_function(ape, stdout_write, NULL);

    for (int i = 0; i < ptrarray_count(lines); i++) {
        const char *line = ptrarray_get(lines, i);
        if (strlen(line) == 0) {
            continue;
        }
        ape_execute(ape, line);
        if (ape_has_errors(ape)) {
            print_ape_errors(ape);
            assert(false);
        }
    }
    ape_destroy(ape);
    assert(malloc_count == 0);
    ptrarray_destroy_with_items(lines, free);
    free(inputs);
}

static void test_program() {
    g_external_fn_test = 0;
    int malloc_count = 0;

    ape_t *ape = ape_make_ex(counted_malloc, counted_free, &malloc_count);

    ape_set_stdout_write_function(ape, stdout_write, NULL);

    ape_set_native_function(ape, "external_fn_test", external_fn_test, &g_external_fn_test);
    ape_set_global_constant(ape, "test", ape_object_make_number(42));
    ape_set_global_constant(ape, "test_str", ape_object_make_stringf(ape, "%s %s", "lorem", "ipsum"));
    ape_set_native_function(ape, "square_array", square_array_fun, NULL);
    ape_set_native_function(ape, "make_test_dict", make_test_dict_fun, NULL);
    ape_set_native_function(ape, "test_check_args", test_check_args_fun, NULL);
    ape_set_native_function(ape, "vec2_add", vec2_add_fun, NULL);
    ape_set_native_function(ape, "vec2_sub", vec2_sub_fun, NULL);

    ape_execute_file(ape, "program.ape");
    if (ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }

    ape_object_t val = ape_get_object(ape, "val");
    int val_num = (int)ape_object_get_number(val);
    assert(val_num == 123);
    ape_destroy(ape);
    assert(malloc_count == 0);
    assert(g_external_fn_test == 42);
}

static void test_compiling() {
    g_external_fn_test = 0;
    int malloc_count = 0;
    char *code = read_file("program.ape");
    assert(code);

    ape_t *ape = ape_make_ex(counted_malloc, counted_free, &malloc_count);
    ape_set_stdout_write_function(ape, stdout_write, NULL);

    ape_set_native_function(ape, "external_fn_test", external_fn_test, &g_external_fn_test);
    ape_set_global_constant(ape, "test", ape_object_make_number(42));
    ape_set_global_constant(ape, "test_str", ape_object_make_stringf(ape, "%s %s", "lorem", "ipsum"));
    ape_set_native_function(ape, "square_array", square_array_fun, NULL);
    ape_set_native_function(ape, "make_test_dict", make_test_dict_fun, NULL);
    ape_set_native_function(ape, "test_check_args", test_check_args_fun, NULL);
    ape_set_native_function(ape, "vec2_add", vec2_add_fun, NULL);
    ape_set_native_function(ape, "vec2_sub", vec2_sub_fun, NULL);

    ape_program_t *program = ape_compile(ape, code);
    if (!program || ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }

    for (int i = 0; i < 1000; i++) {
        ape_execute_program(ape, program);
        if (ape_has_errors(ape)) {
            print_ape_errors(ape);
            assert(false);
        }

        ape_object_t val = ape_get_object(ape, "val");
        assert((int)ape_object_get_number(val) == 123);
    }

    ape_program_destroy(program);

    ape_destroy(ape);
    free(code);
    assert(malloc_count == 0);
    assert(g_external_fn_test == 42);
}

static void test_fails() {
    const char *filename = "fails.ape";
    char *fails = read_file(filename);
    assert(fails);
    ptrarray(char) *lines = get_lines(fails);
    for (int i = 0; i < ptrarray_count(lines); i++) {
        int malloc_count = 0;
        const char *line = ptrarray_get(lines, i);
        if (strlen(line) == 0) {
            continue;
        }
        
        ape_t *ape = ape_make_ex(counted_malloc, counted_free, &malloc_count);

        ape_execute(ape, line);
        if (!ape_has_errors(ape)) {
            assert(false);
        }

        ape_destroy(ape);

        assert(malloc_count == 0);
    }
    ptrarray_destroy_with_items(lines, free);
    free(fails);
}

static void test_calling_functions() {
    int malloc_count = 0;
    ape_t *ape = ape_make_ex(counted_malloc, counted_free, &malloc_count);

    ape_set_stdout_write_function(ape, stdout_write, NULL);

    ape_set_native_function(ape, "add", add_fun, NULL);
    ape_set_native_function(ape, "fourtytwo", fourtytwo_fun, NULL);

    ape_object_t res;

    res = ape_execute(ape, "fn test_calling_external() { var res = fourtytwo(); println(res);}");

    if (ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }

    res = ape_call(ape, "test_calling_external", 0, NULL);
    if (ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }

    const char *test_stack_corruption_code = "\n\
    fn check_map(m) {\n\
        return m.val == 0\n\
    }\n\
    fn test_stack_corruption() {\n\
        var tests = [{val: 0}, {val: 1}]\n\
        for (test in tests) {\n\
            assert(is_map(test))\n\
            if (check_map(test)) {\n\
                while (check_map(test)) {\n\
                    test.val -= 1\n\
                }\n\
            }\n\
        }\n\
    }\
    ";

    res = ape_execute(ape, test_stack_corruption_code);

    if (ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }

    res = ape_call(ape, "test_stack_corruption", 0, NULL);
    if (ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }

    assert(ape_object_get_type(res) == APE_OBJECT_NULL);
    assert(APE_STREQ(g_stdout_buf, "42\n"));

    res = ape_execute(ape, "fn test_calling(a, b) { var res = add(a, b); println(res); return res; }");
    if (ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }

    res = APE_CALL(ape, "test_calling", ape_object_make_number(21), ape_object_make_number(37));
    if (ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }

    assert(APE_DBLEQ(ape_object_get_number(res), 58));
    assert(APE_STREQ(g_stdout_buf, "58\n"));

    res = APE_CALL(ape, "println", ape_object_make_string(ape, "hello world"));
    if (ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }
    
    assert(APE_STREQ(g_stdout_buf, "hello world\n"));

    res = APE_CALL(ape, "len", ape_object_make_string(ape, "lorem"));
    if (ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }
    
    assert(ape_object_get_number(res) == strlen("lorem"));

    ape_destroy(ape);
    assert(malloc_count == 0);
}

static void test_traceback() {
    char *program = read_file("tracebacks.ape");
    assert(program);

    int malloc_count = 0;
    ape_t *ape = ape_make_ex(counted_malloc, counted_free, &malloc_count);

    ape_set_native_function(ape, "custom_error", custom_error_fun, NULL);

    ape_execute(ape, program);
    if (ape_has_errors(ape)) {
        print_ape_errors(ape);
        assert(false);
    }

    {
        ape_call(ape, "traceback", 0, NULL);
        if (!ape_has_errors(ape)) {
            assert(false);
        }

        const ape_error_t *err = ape_get_error(ape, 0);
        const ape_traceback_t *traceback = ape_error_get_traceback(err);

        struct {
            const char *name;
            int line;
            int column;
        } tests[] = {
            {"c", 2, 20},
            {"b", 6, 16},
            {"a", 10, 16},
            {"traceback", 13, 12},
        };

        assert(ape_traceback_get_depth(traceback) == APE_ARRAY_LEN(tests));

        for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
            typeof(tests[0]) test = tests[i];
            int line = ape_traceback_get_line_number(traceback, i) ;
            int col = ape_traceback_get_column_number(traceback, i);
            const char *fname = ape_traceback_get_function_name(traceback, i);
            assert(line == test.line);
            assert(col == test.column);
            assert(APE_STREQ(fname, test.name));
        }
    }

    {
        ape_call(ape, "traceback_native_function", 0, NULL);
        if (!ape_has_errors(ape)) {
            assert(false);
        }

        const ape_error_t *err = ape_get_error(ape, 0);
        const ape_traceback_t *traceback = ape_error_get_traceback(err);

        struct {
            const char *name;
            int line;
            int column;
        } tests[] = {
            {"len", -1, -1},
            {"c", 18, 11},
            {"b", 22, 9},
            {"a", 26, 9},
            {"traceback_native_function", 29, 12},
        };

        int len = ape_traceback_get_depth(traceback);
        assert(len == APE_ARRAY_LEN(tests));

        for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
            typeof(tests[0]) test = tests[i];
            int line = ape_traceback_get_line_number(traceback, i);
            int col = ape_traceback_get_column_number(traceback, i);
            const char *name = ape_traceback_get_function_name(traceback, i);
            assert(line == test.line);
            assert(col == test.column);
            assert(APE_STREQ(name, test.name));
        }
    }

    {
        ape_object_t res = ape_call(ape, "traceback_native_function_error", 0, NULL);
        if (ape_has_errors(ape)) {
            print_ape_errors(ape);
            assert(false);
        }

        assert(ape_object_get_type(res) == APE_OBJECT_ERROR);
        assert(APE_STREQ(ape_object_get_error_message(res), "Error"));
        
        const ape_traceback_t *traceback = ape_object_get_error_traceback(res);

        struct {
            const char *name;
            int line;
            int column;
        } tests[] = {
            {"custom_error", -1, -1},
            {"c", 35, 27},
            {"b", 39, 16},
            {"a", 43, 16},
            {"traceback_native_function_error", 46, 12},
        };

        assert(ape_traceback_get_depth(traceback) == APE_ARRAY_LEN(tests));

        for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
            typeof(tests[0]) test = tests[i];
            int line = ape_traceback_get_line_number(traceback, i);
            int col = ape_traceback_get_column_number(traceback, i);
            const char *name = ape_traceback_get_function_name(traceback, i);
            assert(line == test.line);
            assert(col == test.column);
            assert(APE_STREQ(name, test.name));
        }
    }

    ape_destroy(ape);
    assert(malloc_count == 0);

    free(program);
}

static void test_various() {
    double nan0 = ape_uint64_to_double(0x7ff8000000000000);
    double nan1 = ape_uint64_to_double(0xfff8000000000000);

    ape_object_t nan0_obj = ape_object_make_number(nan0);
    ape_object_t nan1_obj = ape_object_make_number(nan1);

    double nan0_retrieved = ape_object_get_number(nan0_obj);
    double nan1_retrieved = ape_object_get_number(nan1_obj);

    assert(isnan(nan0_retrieved));
    assert(ape_double_to_uint64(nan0_retrieved) == 0x7ff8000000000000);

    assert(isnan(nan1_retrieved));
    assert(ape_double_to_uint64(nan1_retrieved) == 0x7ff8000000000000);
}

static void test_time_limit() {
    const char *tests[] = {
        "while (true) {}",
        "fn(){ while (true) {}}()"
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        int malloc_count = 0;
        ape_t *ape = ape_make_ex(counted_malloc, counted_free, &malloc_count);
        bool limit_set = ape_set_timeout(ape, 1.0);
        if (!limit_set) {
            puts("CAN'T TEST MAX EXECUTION TIME");
            return;
        }

        ape_set_stdout_write_function(ape, stdout_write, NULL);

        ape_execute(ape, tests[i]);

        assert(ape_has_errors(ape));
        assert(ape_errors_count(ape) == 1);

        const ape_error_t *err = ape_get_error(ape, 0);

        assert(ape_error_get_type(err) == APE_ERROR_TIMEOUT);
        ape_destroy(ape);
        assert(malloc_count == 0);
    }
}

static void test_allocation_fails() {
    int n = 0;
    while (true) {
        failing_alloc_t failing_alloc = {
            .allocation_to_fail = n,
            .alloc_count = 0,
            .total_count = 0,
            .has_failed = false,
            .should_fail = true,
        };
        n++;

        ape_t *ape = ape_make_ex(failing_malloc, failing_free, &failing_alloc);
        if (!ape) {
            assert(failing_alloc.alloc_count == 0);
            continue;
        }

        ape_set_stdout_write_function(ape, stdout_write, NULL);

        ape_set_native_function(ape, "external_fn_test", external_fn_test, &g_external_fn_test);
        ape_set_global_constant(ape, "test", ape_object_make_number(42));
        ape_set_native_function(ape, "square_array", square_array_fun, NULL);
        ape_set_native_function(ape, "make_test_dict", make_test_dict_fun, NULL);
        ape_set_native_function(ape, "test_check_args", test_check_args_fun, NULL);
        ape_set_native_function(ape, "vec2_add", vec2_add_fun, NULL);
        ape_set_native_function(ape, "vec2_sub", vec2_sub_fun, NULL);

        ape_execute_file(ape, "program.ape");

        if (failing_alloc.has_failed) {
            assert(ape_has_errors(ape));
            const ape_error_t *err = ape_get_error(ape, 0);
            assert(ape_error_get_type(err) == APE_ERROR_ALLOCATION);

            failing_alloc.should_fail = false;

            ape_execute(ape, "println(\"hello world\")");
            assert(!ape_has_errors(ape));
        }

        ape_destroy(ape);

        if (failing_alloc.alloc_count != 0) {
            printf("Leak after failing allocation %d\n", n - 1);
            assert(false);
        }

        if (!failing_alloc.has_failed) {
            break;
        }
    }
}

static void *failing_malloc(void *ctx, size_t size) {
    failing_alloc_t *alloc = (failing_alloc_t*)ctx;
    if (alloc->should_fail && alloc->total_count >= alloc->allocation_to_fail) {
        alloc->has_failed = true;
        return NULL;
    }
    void *res = malloc(size);
    if (res != NULL) {
        alloc->total_count++;
        alloc->alloc_count++;
    }
    return res;
}

static void failing_free(void *ctx, void *ptr) {
    failing_alloc_t *alloc = (failing_alloc_t*)ctx;
    if (ptr != NULL) {
        alloc->alloc_count--;
    }
    free(ptr);
}

static void *counted_malloc(void *ctx, size_t size) {
    int *malloc_count = (int*)ctx;
    void *res = malloc(size);
    if (res != NULL) {
        (*malloc_count)++;
    }
    return res;
}

static void counted_free(void *ctx, void *ptr) {
    int *malloc_count = (int*)ctx;
    if (ptr != NULL) {
        (*malloc_count)--;
    }
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

static void print_ape_errors(ape_t *ape) {
    for (int i = 0; i < ape_errors_count(ape); i++) {
        const ape_error_t *err = ape_get_error(ape, i);
        char *err_str = ape_error_serialize(ape, err);
        puts(err_str);
        ape_free_allocated(ape, err_str);
    }
}

static size_t stdout_write(void* context, const void *data, size_t size) {
    memcpy(g_stdout_buf, data, size);
    g_stdout_buf[size] = '\0';
    return size;
}

static ape_object_t external_fn_test(ape_t *ape, void *data, int argc, ape_object_t *args) {
    int *test = (int*)data;
    *test = 42;
    return ape_object_make_null();
}

static ape_object_t square_array_fun(ape_t *ape, void *data, int argc, ape_object_t *args) {
    ape_object_t res = ape_object_make_array(ape);
    if (ape_object_get_type(res) == APE_OBJECT_NULL) {
        return ape_object_make_null();
    }
    for (int i = 0; i < argc; i++) {
        if (ape_object_get_type(args[i]) != APE_OBJECT_NUMBER) {
            ape_set_runtime_error(ape, "Invalid type passed to square_array");
            return ape_object_make_null();
        }
        double num = ape_object_get_number(args[i]);
        ape_object_t res_item = ape_object_make_number(num * num);
        ape_object_add_array_value(res, res_item);
    }
    return res;
}

static ape_object_t make_test_dict_fun(ape_t *ape, void *data, int argc, ape_object_t *args) {
    if (argc != 1) {
        ape_set_runtime_errorf(ape, "Invalid type passed to make_test_dict, got %d, expected 1", argc);
        return ape_object_make_null();
    }
    
    if (ape_object_get_type(args[0]) != APE_OBJECT_NUMBER) {
        const char *type_name = ape_object_get_type_string(args[0]);
        ape_set_runtime_errorf(ape, "Invalid type passed to make_test_dict, got %s", type_name);
        return ape_object_make_null();
    }

    int num_items = ape_object_get_number(args[0]);

    ape_object_t res = ape_object_make_map(ape);
    if (ape_object_get_type(res) == APE_OBJECT_NULL) {
        return ape_object_make_null();
    }
    for (int i = 0; i < num_items; i++) {
        char *key_name = ape_stringf(NULL, "%d", i);
        ape_object_t key = ape_object_make_string(ape, key_name);
        free(key_name);
        ape_object_t val = ape_object_make_number(i);
        ape_object_set_map_value_with_value_key(res, key, val);
    }
    return res;
}

static ape_object_t test_check_args_fun(ape_t *ape, void *data, int argc, ape_object_t *args) {
    if (!APE_CHECK_ARGS(ape, true, argc, args,
                  APE_OBJECT_NUMBER,
                  APE_OBJECT_ARRAY | APE_OBJECT_MAP,
                  APE_OBJECT_MAP,
                  APE_OBJECT_STRING,
                  APE_OBJECT_NUMBER | APE_OBJECT_BOOL,
                  APE_OBJECT_FUNCTION | APE_OBJECT_NATIVE_FUNCTION,
                  APE_OBJECT_ANY)) {
        return ape_object_make_null();
    }

    return ape_object_make_number(42);
}

static ape_object_t custom_error_fun(ape_t *ape, void *data, int argc, ape_object_t *args) {
    return ape_object_make_error(ape, "Error");
}

static ape_object_t add_fun(ape_t *ape, void *data, int argc, ape_object_t *args) {
    if (!APE_CHECK_ARGS(ape, true, argc, args,
                        APE_OBJECT_NUMBER,
                        APE_OBJECT_NUMBER)) {
        return ape_object_make_null();
    }

    double a = ape_object_get_number(args[0]);
    double b = ape_object_get_number(args[1]);
    return ape_object_make_number(a + b);
}

static ape_object_t fourtytwo_fun(ape_t *ape, void *data, int argc, ape_object_t *args) {
    if (!ape_check_args(ape, true, argc, args, 0, NULL)) {
        return ape_object_make_null();
    }

    return ape_object_make_number(42);
}

static ape_object_t vec2_add_fun(ape_t *ape, void *data, int argc, ape_object_t *args) {
    if (!APE_CHECK_ARGS(ape, true, argc, args, APE_OBJECT_MAP, APE_OBJECT_MAP)) {
        return ape_object_make_null();
    }
    double a_x = ape_object_get_map_number(args[0], "x");
    double a_y = ape_object_get_map_number(args[0], "y");

    double b_x = ape_object_get_map_number(args[1], "x");
    double b_y = ape_object_get_map_number(args[1], "y");

    ape_object_t res = ape_object_make_map(ape);
    if (ape_object_get_type(res) == APE_OBJECT_NULL) {
        return res;
    }
    ape_object_set_map_number(res, "x", a_x + b_x);
    ape_object_set_map_number(res, "y", a_y + b_y);
    return res;
}

static ape_object_t vec2_sub_fun(ape_t *ape, void *data, int argc, ape_object_t *args) {
    if (!APE_CHECK_ARGS(ape, true, argc, args, APE_OBJECT_MAP, APE_OBJECT_MAP)) {
        return ape_object_make_null();
    }
    double a_x = ape_object_get_map_number(args[0], "x");
    double a_y = ape_object_get_map_number(args[0], "y");

    double b_x = ape_object_get_map_number(args[1], "x");
    double b_y = ape_object_get_map_number(args[1], "y");

    ape_object_t res = ape_object_make_map(ape);
    ape_object_set_map_number(res, "x", a_x - b_x);
    ape_object_set_map_number(res, "y", a_y - b_y);
    return res;
}


#pragma GCC diagnostic pop
