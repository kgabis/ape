#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
#endif

#include "test_vm.h"

#include "common.h"

#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "object.h"
#include "compiler.h"
#include "vm.h"
#include "gc.h"
#include "tests.h"

static object_t execute(const char *input, bool must_succeed);
static void test_number(object_t obj, double expected);
static void test_number_arithmentic(void);
static void test_boolean_expressions(void);
static void test_conditionals(void);
static void test_global_define(void);
static void test_string_expressions(void);
static void test_array_literals(void);
static void test_map_literals(void);
static void test_index_and_dot_expressions(void);
static void test_calling_functions_without_arguments(void);
static void test_calling_functions_with_bindings(void);
static void test_native_functions(void);
static void test_functions(void);
static void test_recursive_functions(void);
static void test_assign(void);
static void test_block_scopes(void);
static void test_while_loops(void);
static void test_foreach(void);
static void test_for_loops(void);
static void test_code_blocks(void);
static void test_errors(void);

void vm_test() {
    puts("### VM test");
    test_number_arithmentic();
    test_boolean_expressions();
    test_conditionals();
    test_global_define();
    test_string_expressions();
    test_array_literals();
    test_map_literals();
    test_index_and_dot_expressions();
    test_calling_functions_without_arguments();
    test_calling_functions_with_bindings();
    test_native_functions();
    test_functions();
    test_recursive_functions();
    test_assign();
    test_block_scopes();
    test_while_loops();
    test_foreach();
    test_for_loops();
    test_code_blocks();
    test_errors();
    puts("\tOK");
}

// INTERNAL
static object_t execute(const char *input, bool must_succeed) {
    bool ok = false;

    ape_config_t config;
    memset(&config, 0, sizeof(ape_config_t));

    config.repl_mode = true;

    gcmem_t *mem = gcmem_make(NULL);

    errors_t errors;
    errors_init(&errors);
    ptrarray(compiled_file_t) *files = ptrarray_make(NULL);
    global_store_t *gs = global_store_make(NULL, mem);
    compiler_t *comp = compiler_make(NULL, &config, mem, &errors, files, gs);

    compilation_result_t *comp_res = compiler_compile(comp, input);
    if (!comp_res || errors_has_errors(&errors)) {
        print_errors(&errors);
        assert(false); // can only fail on vm_run
    }

    errors_t errs;
    errors_init(&errs);

    global_store_t *store = global_store_make(NULL, mem);

    vm_t *vm = vm_make(NULL, NULL, mem, &errs, store);
    ok = vm_run(vm, comp_res, compiler_get_constants(comp));
    assert(vm->sp == 0);

    if (!ok || errors_has_errors(&errs)) {
        if (!must_succeed) {
            return object_make_null();
        }
        print_errors(vm->errors);
        assert(false);
    }

    if (!must_succeed) {
        assert(false);
    }

    object_t top = vm_get_last_popped(vm);

    return top;
}

static void test_number(object_t obj, double expected) {
    assert(object_get_type(obj) == OBJECT_NUMBER);
    double num = object_get_number(obj);
    assert(APE_DBLEQ(num, expected));
}

static void test_number_arithmentic() {
    struct {
        const char *input;
        double val;
    } tests[] = {
        {"1", 1},
        {"2", 2},
        {"1 + 2", 3},
        {"1 - 2", -1},
        {"1 * 2", 2},
        {"4 / 2", 2},
        {"10 % 2", 0},
        {"10 % 3", 1},
        {"4.4 % 2.2", 0},
        {"6.6 % 4.4", fmod(6.6, 4.4)},
        {"0 % 10", 0},
        {"1.1 + 2.2", 1.1 + 2.2},
        {"1.1 - 2.2", 1.1 - 2.2},
        {"10 * 2.2", 10.0 * 2.2},
        {"5 / 2", 5.0 / 2.0},
        {"50 / 2 * 2 + 10 - 5", 55},
        {"5 + 5 + 5 + 5 - 10", 10},
        {"2 * 2 * 2 * 2 * 2", 32},
        {"5 * 2 + 10", 20},
        {"5 + 2 * 10", 25},
        {"5 * (2 + 10)", 60},
        {"-5", -5},
        {"-10", -10},
        {"-50 + 100 + -50", 0},
        {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_boolean_expressions() {
    struct {
        const char *input;
        bool val;
    } tests[] = {
        {"true", true},
        {"false", false},
        {"1 < 2", true},
        {"1 > 2", false},
        {"1 < 1", false},
        {"1 > 1", false},
        {"1 <= 2", true},
        {"2 <= 2", true},
        {"3 <= 2", false},
        {"2 >= 1", true},
        {"2 >= 2", true},
        {"2 >= 3", false},
        {"1 == 1", true},
        {"1 != 1", false},
        {"1 == 2", false},
        {"1 != 2", true},
        {"true == true", true},
        {"false == false", true},
        {"true == false", false},
        {"true != false", true},
        {"false != true", true},
        {"(1 < 2) == true", true},
        {"(1 < 2) == false", false},
        {"(1 > 2) == true", false},
        {"(1 > 2) == false", true},
        {"!true", false},
        {"!false", true},
        {"!5", false},
        {"!!true", true},
        {"!!false", false},
        {"!!5", true},
        {"\"lorem\" == \"lorem\"", true},
        {"\"lorem\" == \"ipsum\"", false},
        {"\"lorem\" != \"ipsum\"", true},
        {"\"abcde\" < \"abcdf\"", true},
        {"true && true", true},
        {"false && false", false},
        {"true && false", false},
        {"false && true", false},
        {"true || false", true},
        {"false || true", true},
        {"true || true", true},
        {"false || false", false},
        {"false || false || true", true},
        {"true && false || true", true},
        {"false || true && false", false},
        {"null == null", true},
        {"null == false", true},
        {"!null == true", true},
        {"{} == {}", false},
        {"var a = {}; var b = {}; a == b", false},
        {"var a = {}; var b = a; a == b", true},
        {"{} == null", false},
        {"\"\" == null", false},
        {"1.1 < 1.2", true},
        {"1.1 > 1.2", false},
        {"3.14 > 2.9", true},
        {"3 == 3", true},
        {"{} == {}", false},
        {"[] != []", true},

    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        assert(object_get_type(obj) == OBJECT_BOOL);
        assert(object_get_bool(obj) == test.val);
    }
}

static void test_conditionals() {
    struct {
        const char *input;
        int val;
    } tests[] = {
        {"var x = 0; if (true) { x = 10; } x;", 10},
        {"var x = 0; if (true) { x = 10; } else { x = 20; } x", 10},
        {"var x = 0; if (false) { x = 10; } else { x = 20; } x", 20},
        {"var x = 0; if (1 < 2) { x = 10; } x", 10},
        {"var x = 0; if (1 < 2) { x = 10; } else { x = 20; } x ", 10},
        {"var x = 0; if (1 > 2) { x = 10; } else { x = 20; } x", 20},
        {"var x = 0; if (true) { const y = 1337; } x", 0},
        {"const x = 1; var y = -1; if (x == 0) { y = 0; } else if (x == 1) { y = 1; } y", 1},
        {"const x = 2; var y = -1; if (x == 0) { y = 0; } else if (x == 1) { y = 1; } else { y = 2; } y", 2},
        {"const x = 2; var y = -1; if (x == 0) { y = 0; } else if (x == 1) { y = 1; } else if (x == 2) { y = 2; } else { y = 3; } y", 2},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_global_define() {
    struct {
        const char *input;
        int val;
    } tests[] = {
        {"const one = 1; one", 1},
        {"const one = 1; const two = 2; one + two", 3},
        {"const one = 1; const two = one + one; one + two", 3},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_string_expressions() {
    struct {
        const char *input;
        const char *val;
    } tests[] = {
        {"\"monkey\"", "monkey"},
        {"\"lorem\\nipsum\"", "lorem\nipsum"},
        {"\"lorem\\tipsum\"", "lorem\tipsum"},
        {"\"mon\" + \"key\"", "monkey"},
        {"\"mon\" + \"key\" + \"banana\"", "monkeybanana"},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        assert(object_get_type(obj) == OBJECT_STRING);
        assert(APE_STREQ(object_get_string(obj), test.val));
    }
}

static void test_array_literals() {
    struct {
        const char *input;
        int elements_count;
        int elements[3];
    } tests[] = {
        {"[]", 0, {0}},
        {"[1, 2, 3]", 3, {1, 2, 3}},
        {"[1 + 2, 3 * 4, 5 + 6]", 3, {3, 12, 11}},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        assert(object_get_type(obj) == OBJECT_ARRAY);
        assert(object_get_array_length(obj) == test.elements_count);
        for (int j = 0; j < object_get_array_length(obj); j++) {
            object_t element = object_get_array_value_at(obj, j);
            assert(object_get_type(element) == OBJECT_NUMBER);
            double got = object_get_number(element);
            assert(APE_DBLEQ(got, test.elements[j]));
        }
    }
}

static void test_map_literals() {
    struct {
        const char *input;
        int count;
        int keys[2];
        int vals[2];
    } tests[] = {
        {"{}", 0, {0}, {0}},
        {"{1: 2, 2: 3}", 2, {1, 2}, {2, 3}},
        {"{2: 2 * 2, 6: 4 * 4}", 2, {2, 6}, {4, 16}},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        assert(object_get_type(obj) == OBJECT_MAP);
        int len = object_get_map_length(obj);
        assert(len == test.count);
        for (int j = 0; j < len; j++) {
            object_t key = object_get_map_key_at(obj, j);
            assert(object_get_type(key) == OBJECT_NUMBER);
            assert(APE_DBLEQ(object_get_number(key), test.keys[j]));

            object_t val = object_get_map_value_at(obj, j);
            assert(object_get_type(val) == OBJECT_NUMBER);
            assert(APE_DBLEQ(object_get_number(val), test.vals[j]));
        }
    }
}

static void test_index_and_dot_expressions() {
    struct {
        const char *input;
        bool is_null;
        int val;
    } tests[] = {
        {"{a: 1}.a", false, 1},
        {"[1, 2, 3][1]", false, 2},
        {"[1, 2, 3][0 + 2]", false, 3},
        {"[[1, 1, 1]][0][0]", false, 1},
        {"[][0]", true, 0},
        {"[1, 2, 3][99]", true, 0},
        {"[1][-1]", false, 1},
        {"{1: 1, 2: 2}[1]", false, 1},
        {"{1: 1, 2: 2}[2]", false, 2},
        {"{1: 1}[0]", true, 0},
        {"{}[0]", true, 0},
        {"{\"a\": 2}[\"a\"]", false, 2},
        {"{\"a\": 2}.a", false, 2},
        {"{\"a\": 2}.b", true, 0},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        if (test.is_null) {
            assert(object_get_type(obj) == OBJECT_NULL);
        } else {
            test_number(obj, test.val);
        }
    }
}

static void test_calling_functions_without_arguments() {
    struct {
        const char *input;
        bool is_null;
        int val;
    } tests[] = {
        {"fn test() { }; test()", true, 0},
        {"const fivePlusTen = fn() { return 5 + 10; }; fivePlusTen()", false, 15},
        {"const one = fn() { return 1; }; const two = fn() { return 2; }; one() + two()", false, 3},
        {"const a = fn() { return 1; }; const b = fn() { return a() + 1; }; const c = fn() { return b() + 1; }; c()", false, 3},
        {"const earlyExit = fn() { return 99; }; earlyExit()", false, 99},
        {"const earlyExit = fn() { return 99; return 100; }; earlyExit()", false, 99},
        {"const noReturn = fn() { }; noReturn()", true, 0},
        {"const noReturn = fn() { }; const noReturnTwo = fn() { noReturn(); }; noReturn(); noReturnTwo()", true, 0},
        {"const returnsOne = fn() { return 1; }; const returnsOneReturner = fn() { return returnsOne; }; returnsOneReturner()()", false, 1},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        if (test.is_null) {
            assert(object_get_type(obj) == OBJECT_NULL);
        } else {
            test_number(obj, test.val);
        }
    }
}

static void test_calling_functions_with_bindings() {
    struct {
        const char *input;
        int val;
    } tests[] = {
        {
            "\
            const make_one = fn() { const one = 1; return one; };\
            make_one();\
            ",
            1
        },
        {
            "\
            const oneAndTwo = fn() { const one = 1; const two = 2; return one + two; };\
            oneAndTwo();\
            ",
            3
        },
        {
            "\
            const oneAndTwo = fn() { const one = 1; const two = 2; return one + two; };\
            const threeAndFour = fn() { const three = 3; const four = 4; return three + four; };\
            oneAndTwo() + threeAndFour();\
            ",
            10
        },
        {
            "\
            const firstFoobar = fn() { const foobar = 50; return foobar; };\
            const secondFoobar = fn() { const foobar = 100; return foobar; };\
            firstFoobar() + secondFoobar();\
            ",
            150
        },
        {
            "\
            const globalSeed = 50;\
            const minusOne = fn() {\
                const num = 1;\
                return globalSeed - num;\
            };\
            const minusTwo = fn() {\
                const num = 2;\
                return globalSeed - num;\
            };\
            minusOne() + minusTwo();\
            ",
            97
        },
        {
            "\
            const identity = fn(a) { return a; };\
            identity(4);\
            ",
            4
        },
        {
            "\
            const sum = fn(a, b) { return a + b; };\
            sum(1, 2);\
            ",
            3
        },
        {
            "\
            const sum = fn(a, b) {\
                const c = a + b;\
                return c;\
            };\
            sum(1, 2);\
            ",
            3
        },
        {
            "\
            const sum = fn(a, b) {\
                const c = a + b;\
                return c;\
            };\
            sum(1, 2) + sum(3, 4);\
            ",
            10
        },
        {
            "\
            const sum = fn(a, b) {\
                const c = a + b;\
                return c;\
            };\
            const outer = fn() {\
                return sum(1, 2) + sum(3, 4);\
            };\
            outer();\
            ",
            10
        },
        {
            "\
            const globalNum = 10;\
            const sum = fn(a, b) {\
               const c = a + b;\
               return c + globalNum;\
            };\
            const outer = fn() {\
               return sum(1, 2) + sum(3, 4) + globalNum;\
            };\
            outer() + globalNum;\
            ",
            50
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_native_functions() {
    struct {
        const char *input;
        bool is_null;
        int val;
    } tests[] = {
        {"len(\"\")", false, 0},
        {"len(\"four\")", false, 4},
        {"len(\"hello world\")", false, 11},
        {"len([1, 2, 3])", false, 3},
        {"len([])", false, 0},
        {"first([1, 2, 3])", false, 1},
        {"first([])", true, 0},
        {"last([1, 2, 3])", false, 3},
        {"last([])", true, 0},
        {"rest([1, 2, 3])[1]", false, 3},
        {"rest([])", true, 0},
        {"var arr = []; append(arr, 1); arr[0]", false, 1},
        {"values({\"a\":1, \"b\": 2})[0]", false, 1},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        if (test.is_null) {
            assert(object_get_type(obj) == OBJECT_NULL);
        } else {
            test_number(obj, test.val);
        }
    }
}

static void test_functions() {
    struct {
        const char *input;
        int val;
    } tests[] = {
        {
            "\
                const newClosure = fn(a) {\
                    return fn() { return a; };\
                };\
                const function = newClosure(99);\
                function();\
            ",
            99
        },
        {
            "\
            const newAdderOuter = fn(a, b) {\
                const c = a + b;\
                return fn(d) {\
                    const e = d + c;\
                    return fn(f) { return e + f; };\
                };\
            };\
            const newAdderInner = newAdderOuter(1, 2);\
            const adder = newAdderInner(3);\
            adder(8);\
            ",
            14,
        },
        {
            "\
            const a = 1;\
            const newAdderOuter = fn(b) {\
                return fn(c) {\
                    return fn(d) { return a + b + c + d; };\
                };\
            };\
            const newAdderInner = newAdderOuter(2);\
            const adder = newAdderInner(3);\
            adder(8);\
            ",
            14,
        },
        {
            "\
            const newClosure = fn(a, b) {\
                const one = fn() { return a; };\
                const two = fn() { return b; };\
                return fn() { return one() + two(); };\
            };\
            const function = newClosure(9, 90);\
            function();\
            ",
            99,
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_recursive_functions() {
    struct {
        const char *input;
        int val;
    } tests[] = {
        {
            "\
            const countDown = fn(x) {\
                if (x == 0) {\
                    return 0;\
                } else {\
                    return countDown(x - 1);\
                }\
            };\
            countDown(1);\
            ",
            0,
        },
        {
            "\
            const countDown = fn(x) {\
                if (x == 0) {\
                    return 0;\
                } else {\
                    return countDown(x - 1);\
                }\
            };\
            const wrapper = fn() {\
                return countDown(1);\
            };\
            wrapper();\
            ",
            0,
        },
        {
            "\
            const wrapper = fn() {\
                const countDown = fn(x) {\
                    if (x == 0) {\
                        return 0;\
                    } else {\
                        return countDown(x - 1);\
                    }\
                };\
                return countDown(1);\
            };\
            wrapper();\
            ",
            0,
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_assign() {
    struct {
        const char *input;
        int val;
    } tests[] = {
        {
            "\
            var x = 123;\
            x = 124;\
            x;\
            ",
            124,
        },
        {
            "\
            var x = 5;\
            x += 6;\
            x;\
            ",
            11,
        },
        {
            "\
            var x = 5;\
            x -= 6;\
            x;\
            ",
            -1,
        },
        {
            "\
            var x = 5;\
            x *= 6;\
            x;\
            ",
            30,
        },
        {
            "\
            var x = 30;\
            x /= 6;\
            x;\
            ",
            5,
        },
        {
            "\
            const arr = [1, 2, 3];\
            arr[0] = 4;\
            arr[0];\
            ",
            4,
        },
        {
            "\
            const arr = [1, 2, 3];\
            arr[0] += 4;\
            arr[0];\
            ",
            5,
        },
        {
            "\
            const dict = {\"a\": 1, \"b\": 2};\
            dict[\"a\"] = 3;\
            dict[\"a\"];\
            ",
            3,
        },
        {
            "\
            const dict = {\"a\": 1, \"b\": 2};\
            dict[\"a\"] += 3;\
            dict[\"a\"];\
            ",
            4,
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_block_scopes() {
    struct {
        const char *input;
        int val;
    } tests[] = {
        {
            "\
            var x = 1;\
            if (true) { var y = 2; }\
            if (true) { var y = 3; x = y; }\
            x;\
            ",
            3,
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_while_loops() {
    struct {
        const char *input;
        int val;
    } tests[] = {
        {
            "\
            var x = 0;\
            while (x < 10) {\
                if (x < 10) {\
                    x++;\
                    continue;\
                }\
                x = 100;\
            }\
            x;\
            ",
            10
        },
        {
            "\
            var x = 0;\n\
            while (x < 10) {\n\
                x = x + 1;\n\
            }\n\
            x;\
            ",
            10,
        },
        {
            "\
            var x = 0;\
            var y = 0;\
            while (true) {\
                while (true) {\
                    x = x + 1;\
                    if (x > 100) {\
                        y = y + 1;\
                        break;\
                    }\
                }\
                if (y > 100) {\
                    break;\
                }\
            }\
            y;\
            ",
            101,
        },
        {
            "\
            const factorial = fn(num) {\
                var res = 1;\
                var i = 2;\
                while (true) {\
                    if (i > num) {\
                        break;\
                    }\
                    res = res * i;\
                    i = i + 1;\
                }\
                return res;\
            };\
            factorial(10);\
            ",
            3628800,
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_foreach() {
    struct {
        const char *input;
        int val;
    } tests[] = {
        {
            "\
            var res = 0;\
            for (item in range(0, 10)) {\
                res = res + 1;\
            }\
            res;\
            ",
            10,
        },
        {
            "\
            var res = 0;\
            for (item in range(0, 10)) {\
                if (res > 5) {\
                    break;\
                }\
                res++;\
            }\
            res;\
            ",
            6,
        },
        {
            "\
            var res = 1;\
            for (item in range(0, 10)) {\
                if (res > 5) {\
                    break;\
                } else {\
                    continue;\
                }\
                res++;\
            }\
            res;\
            ",
            1,
        },
        {
            "\
            var res = 0;\
            for (item in [0, 1, 2]) {\
                res = item;\
            }\
            res;\
            ",
            2,
        },
        {
            "\
            const arr = [0, 1, 2];\
            var res = 0;\
            for (item in arr) {\
                res = item;\
            }\
            res;\
            ",
            2,
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_for_loops() {
    struct {
        const char *input;
        int val;
    } tests[] = {
        {
            "var x = 0; for (var i = 0; i < 10; i++) { x++; } x",
            10,
        },
        {
            "var i = 5; for (i = 0; i < 10; i++) { } i",
            10,
        },
        {
            "var i = 5; for (;false;) { } i",
            5,
        },
        {
            "var i = 5; for (;;) { break; } i",
            5,
        },
        {
            "var i = 0; for (;i < 10; i += 10) { } i",
            10,
        },
        {
            "var i = 0; for (;i < 10;) { i++; } i",
            10,
        },
        {
            "var x = 0; for (var i = 0; i < 10; i++) { if (i%2) { continue; } x++; } x",
            5,
        }
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_code_blocks() {
        struct {
        const char *input;
        int val;
    } tests[] = {
        {
            "\
            fn test() {\
                var res = 0;\
                {\
                    res = 1;\
                }\
                return res;\
            }\
            test();\
            ",
            1,
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        object_t obj = execute(test.input, true);
        test_number(obj, test.val);
    }
}

static void test_errors() {
    struct {
        const char *input;
        int line;
        int column;
    } tests[] = {
        {"fn fun(x){return x;};fun()", 0, 24},
        {"fn(x){}()", 0, 7},
        {"1+1;\ncrash()", 1, 5},
        {"1;\n2;\nfn(x){return x[0];}(1)", 2, 14},
        {"1()", 0, 1},
        {"var x = 0; for (i in range(0, 10)) { if (i == 9) { x = i[\"a\"];}}", 0, 56},
        {"var arr = [1, 2, 3];\narr[4] = 5", 1, 3},
        {"var arr = [1, 2, 3];\narr[\"a\"] = 5", 1, 3},
    };

    ape_config_t config;
    memset(&config, 0, sizeof(ape_config_t));

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];

        bool ok = false;

        gcmem_t *mem = gcmem_make(NULL);

        ape_config_t *config = malloc(sizeof(ape_config_t));
        config->repl_mode = true;
        errors_t errs;
        errors_init(&errs);
        ptrarray(compiled_file_t) *files = ptrarray_make(NULL);
        global_store_t *gs = global_store_make(NULL, mem);
        compiler_t *comp = compiler_make(NULL, config, mem, &errs, files, gs);

        compilation_result_t *comp_res = compiler_compile(comp, test.input);
        if (!comp_res || errors_has_errors(&errs)) {
            print_errors(&errs);
            assert(false); // can only fail on vm_run
        }

        global_store_t *store = global_store_make(NULL, mem);
        
        vm_t *vm = vm_make(NULL, NULL, mem, &errs, store);
        ok = vm_run(vm, comp_res, compiler_get_constants(comp));

        assert(!ok);
        assert(errors_get_count(&errs) == 1);
        error_t *err = errors_get(&errs, 0);
        assert(err->type == ERROR_RUNTIME);
        assert(err->pos.line == test.line);
        assert(err->pos.column == test.column);
    }
}

#pragma GCC diagnostic pop
