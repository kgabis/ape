#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"

#include "test_compiler.h"

#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "parser.h"
#include "common.h"
#include "compiler.h"
#include "code.h"
#include "object.h"
#include "symbol_table.h"
#include "error.h"
#include "gc.h"
#include "tests.h"

typedef struct {
    object_type_t type;
    union {
        double num_val;
        uint8_t function[256];
        const char *str_val;
    };
} expected_constant_t;

static void test_compiler_scopes(void);
static void test_arithmetic(void);
static void test_boolean_expressions(void);
static void test_conditionals(void);
static void test_global_define(void);
static void test_string_expressions(void);
static void test_array_literals(void);
static void test_map_literals(void);
static void test_index_and_dot_operator(void);
static void test_functions(void);
static void test_function_calls(void);
static void test_compiler_scopes(void);
static void test_define_statement_scopes(void);
static void test_builtins(void);
static void test_functions_with_closures(void);
static void test_recursive_functions(void);
static void test_assignment(void);
static void test_while_loop(void);
static void test_foreach(void);
static void test_for_loop(void);
static void test_tricky_programs(void);

static compiler_t* compile(const char *input, bool must_succeed, compilation_result_t **out_res);
static void test_number_constants(compiler_t *comp, double *expected_constants, int expected_constants_count);
static void test_constants(compiler_t *comp, expected_constant_t *expected_constants, int expected_constants_count);
static void test_string_constants(compiler_t *comp, const char** expected_constants, int expected_constants_count);
static int test_operands(opcode_definition_t *def, uint8_t *actual, uint8_t *expected);
static void test_bytecode(uint8_t *bytecode, size_t bytecode_size, uint8_t *expected_bytecode);

void compiler_test() {
    puts("### Compiler test");
    test_compiler_scopes();
    test_arithmetic();
    test_boolean_expressions();
    test_conditionals();
    test_global_define();
    test_string_expressions();
    test_array_literals();
    test_map_literals();
    test_index_and_dot_operator();
    test_functions();
    test_function_calls();
    test_define_statement_scopes();
    test_builtins();
    test_functions_with_closures();
    test_recursive_functions();
    test_assignment();
    test_while_loop();
    test_foreach();
    test_for_loop();
    test_tricky_programs();
}

static void test_compiler_scopes() {
    gcmem_t *mem = gcmem_make();
    compiler_t *comp = compiler_make(NULL, mem, ptrarray_make());
    array_push(comp->src_positions_stack, &src_pos_zero);
    compilation_scope_t *scope = compiler_get_compilation_scope(comp);
    symbol_table_t *global_table = compiler_get_symbol_table(comp);
    assert(scope->outer == NULL);
    compiler_emit(comp, OPCODE_MUL, 0, NULL);
    compiler_push_compilation_scope(comp);
    assert(scope->outer->outer == NULL);
    assert(array_count(scope->bytecode) == 0);
    compiler_emit(comp, OPCODE_SUB, 0, NULL);
    assert(array_count(scope->bytecode) == 1);
    opcode_t opcode = compiler_last_opcode(comp);
    assert(opcode == OPCODE_SUB);
    symbol_table_t *current_table = compiler_get_symbol_table(comp);
    assert(current_table->outer == global_table);
    compiler_pop_compilation_scope(comp);
    current_table = compiler_get_symbol_table(comp);
    assert(current_table == global_table);
    assert(current_table->outer == NULL);
    assert(scope->outer == NULL);
    compiler_emit(comp, OPCODE_ADD, 0, NULL);
    assert(array_count(scope->bytecode) == 2);
    opcode = compiler_last_opcode(comp);
    assert(opcode == OPCODE_ADD);
}

static void test_arithmetic() {
    struct {
        const char *input;
        int expected_constants_count;
        double expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "1 + 2;",
            2, {1, 2},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_ADD, OPCODE_POP, OPCODE_NONE}
        },
        {
            "1; 2;",
            2, {1, 2},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_POP, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_POP, OPCODE_NONE}
        },
        {
            "1 - 2;",
            2, {1, 2},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_SUB, OPCODE_POP, OPCODE_NONE}
        },
        {
            "1 * 2;",
            2, {1, 2},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_MUL, OPCODE_POP, OPCODE_NONE}
        },
        {
            "2 / 1;",
            2, {2, 1},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_DIV, OPCODE_POP, OPCODE_NONE}
        },
        {
            "-1;",
            1, {1},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_MINUS, OPCODE_POP, OPCODE_NONE}
        },
        {
            "10 % 2;",
            2, {10, 2},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_MOD, OPCODE_POP, OPCODE_NONE}
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_number_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_boolean_expressions() {
    struct {
        const char *input;
        int expected_constants_count;
        double expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "true;",
            0, {0},
            {OPCODE_TRUE, OPCODE_POP, OPCODE_NONE}
        },
        {
            "false;",
            0, {0},
            {OPCODE_FALSE, OPCODE_POP, OPCODE_NONE}
        },
        {
            "1 > 2;",
            2, {1, 2},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_GREATER_THAN, OPCODE_POP, OPCODE_NONE}
        },
        {
            "1 < 2;",
            2, {2, 1},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_GREATER_THAN, OPCODE_POP, OPCODE_NONE}
        },
        {
            "1 >= 2;",
            2, {1, 2},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_GREATER_THAN_EQUAL, OPCODE_POP, OPCODE_NONE}
        },
        {
            "1 <= 2;",
            2, {2, 1},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_GREATER_THAN_EQUAL, OPCODE_POP, OPCODE_NONE}
        },
        {
            "1 == 2;",
            2, {1, 2},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_EQUAL, OPCODE_POP, OPCODE_NONE}
        },
        {
            "1 != 2;",
            2, {1, 2},
            {OPCODE_CONSTANT, 0x0, 0x0, OPCODE_CONSTANT, 0x0, 0x1, OPCODE_NOT_EQUAL, OPCODE_POP, OPCODE_NONE}
        },
        {
            "true == false;",
            0, {0},
            {OPCODE_TRUE, OPCODE_FALSE, OPCODE_EQUAL, OPCODE_POP, OPCODE_NONE}
        },
        {
            "true != false;",
            0, {0},
            {OPCODE_TRUE, OPCODE_FALSE, OPCODE_NOT_EQUAL, OPCODE_POP, OPCODE_NONE}
        },
        {
            "!true;",
            0, {0},
            {OPCODE_TRUE, OPCODE_BANG, OPCODE_POP, OPCODE_NONE}
        },
        {
            "true || false;",
            0, {0},
            {
                OPCODE_TRUE,
                OPCODE_DUP,
                OPCODE_JUMP_IF_TRUE, 0x0, 0x7,
                OPCODE_POP,
                OPCODE_FALSE,
                OPCODE_POP,
                OPCODE_NONE
            }
        },
        {
            "true && true;",
            0, {0},
            {
                OPCODE_TRUE,
                OPCODE_DUP,
                OPCODE_JUMP_IF_FALSE, 0x0, 0x7,
                OPCODE_POP,
                OPCODE_TRUE,
                OPCODE_POP,
                OPCODE_NONE
            }
        },
        {
            "true && false || true;",
            0, {0},
            {
                OPCODE_TRUE,
                OPCODE_DUP,
                OPCODE_JUMP_IF_FALSE, 0x0, 0x7,
                OPCODE_POP,
                OPCODE_FALSE,
                OPCODE_DUP,
                OPCODE_JUMP_IF_TRUE, 0x0, 0xd,
                OPCODE_POP,
                OPCODE_TRUE,
                OPCODE_POP,
                OPCODE_NONE
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_number_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_conditionals() {
    struct {
        const char *input;
        int expected_constants_count;
        double expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "var x = 0; if (true) { x = 10 }; 3333;",
            3, {0, 10, 3333},
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_TRUE,
                OPCODE_JUMP_IF_FALSE, 0, 21,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_DUP,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_POP,
                OPCODE_JUMP, 0, 21,
                OPCODE_CONSTANT, 0, 2,
                OPCODE_POP,
            }
        },
        {
            "var x = 0; if (true) { x = 10; } else { x = 20; }; 3333;",
            4, {0, 10, 20, 3333},
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_TRUE,
                OPCODE_JUMP_IF_FALSE, 0, 21,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_DUP,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_POP,
                OPCODE_JUMP, 0, 29,
                OPCODE_CONSTANT, 0, 2,
                OPCODE_DUP,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_POP,
                OPCODE_CONSTANT, 0, 3,
                OPCODE_POP,
            }
        },
        {
            "var x = 1; if (x == 0) { x = 0; } else if (x == 1) { x = 1; } else { x = 3; };",
            6, {1, 0, 0, 1, 1, 3},
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_EQUAL,
                OPCODE_JUMP_IF_FALSE, 0, 27,
                OPCODE_CONSTANT, 0, 2,
                OPCODE_DUP,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_POP,
                OPCODE_JUMP, 0, 56,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 3,
                OPCODE_EQUAL,
                OPCODE_JUMP_IF_FALSE, 0, 48,
                OPCODE_CONSTANT, 0, 4,
                OPCODE_DUP,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_POP,
                OPCODE_JUMP, 0, 56,
                OPCODE_CONSTANT, 0, 5,
                OPCODE_DUP,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_POP,
            }
        },
        
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_number_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_global_define() {
    struct {
        const char *input;
        int expected_constants_count;
        double expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "const one = 1; const two = 2;",
            2, {1, 2},
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_SET_GLOBAL, 0, 1,
            }
        },
        {
            "const one = 1; one;",
            1, {1},
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_POP,
            }
        },
        {
            "const one = 1; const two = one; two;",
            1, {1},
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_SET_GLOBAL, 0, 1,
                OPCODE_GET_GLOBAL, 0, 1,
                OPCODE_POP, 
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_number_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_string_expressions() {
    struct {
        const char *input;
        int expected_constants_count;
        const char* expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "\"monkey\";",
            1, {"monkey"},
            {
                OPCODE_CONSTANT, 0x0, 0x0,
                OPCODE_POP,
                OPCODE_NONE
            }
        },
        {
            "\"mon\" + \"key\";",
            2, {"mon", "key"},
            {
                OPCODE_CONSTANT, 0x0, 0x0,
                OPCODE_CONSTANT, 0x0, 0x1,
                OPCODE_ADD,
                OPCODE_POP,
                OPCODE_NONE
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_string_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_array_literals() {
    struct {
        const char *input;
        int expected_constants_count;
        double expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "[];",
            0, {0},
            {
                OPCODE_ARRAY, 0x0, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "[1, 2, 3];",
            3, {1, 2, 3},
            {

                OPCODE_CONSTANT, 0x0, 0x0,
                OPCODE_CONSTANT, 0x0, 0x1,
                OPCODE_CONSTANT, 0x0, 0x2,
                OPCODE_ARRAY, 0x0, 0x3,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "[1 + 2, 3 - 4, 5 * 6];",
            6, {1, 2, 3, 4, 5, 6},
            {
                OPCODE_CONSTANT, 0x0, 0x0,
                OPCODE_CONSTANT, 0x0, 0x1,
                OPCODE_ADD,
                OPCODE_CONSTANT, 0x0, 0x2,
                OPCODE_CONSTANT, 0x0, 0x3,
                OPCODE_SUB,
                OPCODE_CONSTANT, 0x0, 0x4,
                OPCODE_CONSTANT, 0x0, 0x5,
                OPCODE_MUL,
                OPCODE_ARRAY, 0x0, 0x3,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_number_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_map_literals() {
    struct {
        const char *input;
        int expected_constants_count;
        double expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "{};",
            0, {0},
            {
                OPCODE_MAP, 0x0, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "{1: 2, 3: 4, 5: 6};",
            6, {1, 2, 3, 4, 5, 6},
            {
                OPCODE_CONSTANT, 0x0, 0x0,
                OPCODE_CONSTANT, 0x0, 0x1,
                OPCODE_CONSTANT, 0x0, 0x2,
                OPCODE_CONSTANT, 0x0, 0x3,
                OPCODE_CONSTANT, 0x0, 0x4,
                OPCODE_CONSTANT, 0x0, 0x5,
                OPCODE_MAP, 0x0, 0x6,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "{1: 2 + 3, 4: 5 * 6};",
            6, {1, 2, 3, 4, 5, 6},
            {
                OPCODE_CONSTANT, 0x0, 0x0,
                OPCODE_CONSTANT, 0x0, 0x1,
                OPCODE_CONSTANT, 0x0, 0x2,
                OPCODE_ADD,
                OPCODE_CONSTANT, 0x0, 0x3,
                OPCODE_CONSTANT, 0x0, 0x4,
                OPCODE_CONSTANT, 0x0, 0x5,
                OPCODE_MUL,
                OPCODE_MAP, 0x0, 0x4,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_number_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_index_and_dot_operator() {
    struct {
        const char *input;
        int expected_constants_count;
        expected_constant_t expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "[1, 2, 3][1 + 1];",
            5,
            {
                { .type = OBJECT_NUMBER, .num_val = 1 },
                { .type = OBJECT_NUMBER, .num_val = 2 },
                { .type = OBJECT_NUMBER, .num_val = 3 },
                { .type = OBJECT_NUMBER, .num_val = 1 },
                { .type = OBJECT_NUMBER, .num_val = 1 },
            },
            {
                OPCODE_CONSTANT, 0x0, 0x0,
                OPCODE_CONSTANT, 0x0, 0x1,
                OPCODE_CONSTANT, 0x0, 0x2,
                OPCODE_ARRAY, 0x0, 0x3,
                OPCODE_CONSTANT, 0x0, 0x3,
                OPCODE_CONSTANT, 0x0, 0x4,
                OPCODE_ADD,
                OPCODE_GET_INDEX,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "{1: 2}[2 - 1];",
            4,
            {
                { .type = OBJECT_NUMBER, .num_val = 1 },
                { .type = OBJECT_NUMBER, .num_val = 2 },
                { .type = OBJECT_NUMBER, .num_val = 2 },
                { .type = OBJECT_NUMBER, .num_val = 1 },
            },
            {
                OPCODE_CONSTANT, 0x0, 0x0,
                OPCODE_CONSTANT, 0x0, 0x1,
                OPCODE_MAP, 0x0, 0x2,
                OPCODE_CONSTANT, 0x0, 0x2,
                OPCODE_CONSTANT, 0x0, 0x3,
                OPCODE_SUB,
                OPCODE_GET_INDEX,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "const obj = {}; obj.foo;",
            1,
            {
                { .type = OBJECT_STRING, .str_val = "foo" },
            },
            {
                OPCODE_MAP, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 0,
                OPCODE_GET_INDEX,
                OPCODE_POP,
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_functions() {
    struct {
        const char *input;
        int expected_constants_count;
        expected_constant_t expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "fn() { return 5 + 10; };",
            3,
            {
                { .type = OBJECT_NUMBER, .num_val = 5 },
                { .type = OBJECT_NUMBER, .num_val = 10 },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_CONSTANT, 0x0, 0x0,
                        OPCODE_CONSTANT, 0x0, 0x1,
                        OPCODE_ADD,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE
                    }
                },
            },
            {
                OPCODE_FUNCTION, 0x0, 0x2, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "1; fn() { return 2; };",
            3,
            {
                { .type = OBJECT_NUMBER, .num_val = 1 },
                { .type = OBJECT_NUMBER, .num_val = 2 },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_CONSTANT, 0x0, 0x1,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE
                    }
                },
            },
            {
                OPCODE_CONSTANT, 0x0, 0x0,
                OPCODE_POP,
                OPCODE_FUNCTION, 0x0, 0x2, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "fn() { };",
            1,
            {
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_RETURN,
                        OPCODE_NONE
                    }
                },
            },
            {
                OPCODE_FUNCTION, 0x0, 0x0, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);

        test_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_function_calls() {
    struct {
        const char *input;
        int expected_constants_count;
        expected_constant_t expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "fn() { return 24; }();",
            2,
            {
                { .type = OBJECT_NUMBER, .num_val = 24 },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_CONSTANT, 0x0, 0x0,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE
                    }
                },
            },
            {
                OPCODE_FUNCTION, 0x0, 0x1, 0x0,
                OPCODE_CALL, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "const noArg = fn() { return 24; }; noArg();",
            2,
            {
                { .type = OBJECT_NUMBER, .num_val = 24 },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_CONSTANT, 0x0, 0x0,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE
                    }
                },
            },
            {
                OPCODE_FUNCTION, 0, 1, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_CALL, 0,
                OPCODE_POP, 
            }
        },
        {
            "const oneArg = fn(a) { return a; }; oneArg(24);",
            2,
            {
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_GET_LOCAL, 0x0,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE
                    }
                },
                { .type = OBJECT_NUMBER, .num_val = 24 },
            },
            {
                OPCODE_FUNCTION, 0, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_CALL, 1,
                OPCODE_POP,
            }
        },
        {
            "const manyArg = fn(a, b, c) { return c; }; manyArg(24, 25, 26);",
            4,
            {
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_GET_LOCAL, 0x2,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE
                    }
                },
                { .type = OBJECT_NUMBER, .num_val = 24 },
                { .type = OBJECT_NUMBER, .num_val = 25 },
                { .type = OBJECT_NUMBER, .num_val = 26 },
            },
            {
                OPCODE_FUNCTION, 0, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_CONSTANT, 0, 2,
                OPCODE_CONSTANT, 0, 3,
                OPCODE_CALL, 3,
                OPCODE_POP,
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);

        test_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_define_statement_scopes() {
    struct {
        const char *input;
        int expected_constants_count;
        expected_constant_t expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "const num = 55; fn() { return num; };",
            2,
            {
                { .type = OBJECT_NUMBER, .num_val = 55 },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_GET_GLOBAL, 0x0, 0x0,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE
                    }
                },
            },
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_FUNCTION, 0, 1, 0,
                OPCODE_POP, 
            }
        },
        {
            "fn() { const num = 55; return num; };",
            2,
            {
                { .type = OBJECT_NUMBER, .num_val = 55 },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_CONSTANT, 0, 0,
                        OPCODE_SET_LOCAL, 0,
                        OPCODE_GET_LOCAL, 0,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE
                    }
                },
            },
            {
                OPCODE_FUNCTION, 0x0, 0x1, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "fn() { const a = 55; const b = 77; return a + b; };",
            3,
            {
                { .type = OBJECT_NUMBER, .num_val = 55 },
                { .type = OBJECT_NUMBER, .num_val = 77 },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_CONSTANT, 0, 0,
                        OPCODE_SET_LOCAL, 0,
                        OPCODE_CONSTANT, 0, 1,
                        OPCODE_SET_LOCAL, 1,
                        OPCODE_GET_LOCAL, 0,
                        OPCODE_GET_LOCAL, 1,
                        OPCODE_ADD,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE
                    }
                },
            },
            {
                OPCODE_FUNCTION, 0x0, 0x2, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);

        test_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_builtins() {
    struct {
        const char *input;
        int expected_constants_count;
        expected_constant_t expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "len([]); append([], 1);",
            1,
            {
                { .type = OBJECT_NUMBER, .num_val = 1 },
            },
            {
                OPCODE_GET_BUILTIN, 0, 0,
                OPCODE_ARRAY, 0, 0,
                OPCODE_CALL, 1,
                OPCODE_POP,
                OPCODE_GET_BUILTIN, 0, 5,
                OPCODE_ARRAY, 0, 0,
                OPCODE_CONSTANT, 0, 0,
                OPCODE_CALL, 2,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "fn() { return len([]); };",
            1,
            {
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_GET_BUILTIN, 0, 0,
                        OPCODE_ARRAY, 0, 0,
                        OPCODE_CALL, 1,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                },
            },
            {
                OPCODE_FUNCTION, 0x0, 0x0, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_functions_with_closures() {
    struct {
        const char *input;
        int expected_constants_count;
        expected_constant_t expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "\
            fn(a) {\
                return fn(b) {\
                    return a + b;\
                };\
            };",
            2,
            {
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_GET_FREE, 0x0,
                        OPCODE_GET_LOCAL, 0x0,
                        OPCODE_ADD,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_GET_LOCAL, 0x0,
                        OPCODE_FUNCTION, 0x0, 0x0, 0x1,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                }
            },
            {
                OPCODE_FUNCTION, 0x0, 0x1, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "\
            fn(a) {\
                return fn(b) {\
                    return fn(c) {\
                        return a + b + c;\
                    };\
                };\
            };",
            3,
            {
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_GET_FREE, 0x0,
                        OPCODE_GET_FREE, 0x1,
                        OPCODE_ADD,
                        OPCODE_GET_LOCAL, 0x0,
                        OPCODE_ADD,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_GET_FREE, 0x0,
                        OPCODE_GET_LOCAL, 0x0,
                        OPCODE_FUNCTION, 0x0, 0x0, 0x2,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_GET_LOCAL, 0x0,
                        OPCODE_FUNCTION, 0x0, 0x1, 0x1,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                }
            },
            {
                OPCODE_FUNCTION, 0x0, 0x2, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "\
            const global = 55;\
            fn() {\
                const a = 66;\
                return fn() {\
                    const b = 77;\
                    return fn() {\
                        const c = 88;\
                        return global + a + b + c;\
                    };\
                };\
            };\
            ",
            7,
            {
                { .type = OBJECT_NUMBER, .num_val = 55 },
                { .type = OBJECT_NUMBER, .num_val = 66 },
                { .type = OBJECT_NUMBER, .num_val = 77 },
                { .type = OBJECT_NUMBER, .num_val = 88 },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_CONSTANT, 0, 3,
                        OPCODE_SET_LOCAL, 0,
                        OPCODE_GET_GLOBAL, 0, 0,
                        OPCODE_GET_FREE, 0,
                        OPCODE_ADD,
                        OPCODE_GET_FREE, 1,
                        OPCODE_ADD,
                        OPCODE_GET_LOCAL, 0,
                        OPCODE_ADD,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_CONSTANT, 0, 2,
                        OPCODE_SET_LOCAL, 0,
                        OPCODE_GET_FREE, 0,
                        OPCODE_GET_LOCAL, 0,
                        OPCODE_FUNCTION, 0, 4, 2,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_CONSTANT, 0x0, 0x1,
                        OPCODE_SET_LOCAL, 0x0,
                        OPCODE_GET_LOCAL, 0x0,
                        OPCODE_FUNCTION, 0x0, 0x5, 0x1,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                }
            },
            {
                OPCODE_CONSTANT, 0x0, 0x0,
                OPCODE_SET_GLOBAL, 0x0, 0x0,
                OPCODE_FUNCTION, 0x0, 0x6, 0x0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_recursive_functions() {
    struct {
        const char *input;
        int expected_constants_count;
        expected_constant_t expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "const countDown = fn(x) { return countDown(x - 1); }; countDown(1);",
            3,
            {
                { .type = OBJECT_NUMBER, .num_val = 1 },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_CURRENT_FUNCTION,
                        OPCODE_GET_LOCAL, 0x0,
                        OPCODE_CONSTANT, 0x0, 0x0,
                        OPCODE_SUB,
                        OPCODE_CALL, 0x1,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                },
                { .type = OBJECT_NUMBER, .num_val = 1 },
            },
            {
                OPCODE_FUNCTION, 0, 1, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 2,
                OPCODE_CALL, 1,
                OPCODE_POP, 
            },
        },
        {
            "\
            const wrapper = fn() {\
                const countDown = fn(x) { return countDown(x - 1); };\
                return countDown(1);\
            };\
            wrapper();\
            ",
            4,
            {
                { .type = OBJECT_NUMBER, .num_val = 1 },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_CURRENT_FUNCTION,
                        OPCODE_GET_LOCAL, 0x0,
                        OPCODE_CONSTANT, 0x0, 0x0,
                        OPCODE_SUB,
                        OPCODE_CALL, 0x1,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                },
                { .type = OBJECT_NUMBER, .num_val = 1 },
                {
                    .type = OBJECT_FUNCTION,
                    .function = {
                        OPCODE_FUNCTION, 0, 1, 0,
                        OPCODE_SET_LOCAL, 0,
                        OPCODE_GET_LOCAL, 0,
                        OPCODE_CONSTANT, 0, 2,
                        OPCODE_CALL, 1,
                        OPCODE_RETURN_VALUE,
                        OPCODE_NONE,
                    }
                },
            },
            {
                OPCODE_FUNCTION, 0, 3, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_CALL, 0,
                OPCODE_POP,
                OPCODE_NONE,
            },
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_assignment() {
    struct {
        const char *input;
        int expected_constants_count;
        double expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "var x = 1; x = 2;",
            2, {1, 2},
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_DUP,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "const arr = [1, 2, 3]; arr[0] = 4;",
            5, {1, 2, 3, 4, 0},
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_CONSTANT, 0, 2,
                OPCODE_ARRAY, 0, 3,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 3,
                OPCODE_DUP,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 4,
                OPCODE_SET_INDEX,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
        {
            "const dict = {1: 2}; dict[1] = 3;",
            4, {1, 2, 3, 1},
            {
                OPCODE_CONSTANT, 0x0, 0x0,
                OPCODE_CONSTANT, 0x0, 0x1,
                OPCODE_MAP, 0x0, 0x2,
                OPCODE_SET_GLOBAL, 0x0, 0x0,
                OPCODE_CONSTANT, 0x0, 0x2,
                OPCODE_DUP,
                OPCODE_GET_GLOBAL, 0x0, 0x0,
                OPCODE_CONSTANT, 0x0, 0x3,
                OPCODE_SET_INDEX,
                OPCODE_POP,
                OPCODE_NONE,
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_number_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_while_loop() {
    struct {
        const char *input;
        int expected_constants_count;
        double expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "while (true) { }",
            0, {0},
            {
                OPCODE_TRUE,
                OPCODE_JUMP_IF_TRUE, 0x0, 0x7,
                OPCODE_JUMP, 0x0, 0xc,
                OPCODE_NULL,
                OPCODE_POP,
                OPCODE_JUMP, 0x0, 0x0,
                OPCODE_NONE,
            }
        },
        {
            "var x = 0; while (x < 10) { x = x + 1; }",
            3, {0, 10, 1},
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_GREATER_THAN,
                OPCODE_JUMP_IF_TRUE, 0, 19,
                OPCODE_JUMP, 0, 34,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 2,
                OPCODE_ADD,
                OPCODE_DUP,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_POP,
                OPCODE_JUMP, 0, 6,
                OPCODE_NONE,
            }
        },
        {
            "while (true) { break; }",
            0, {0},
            {
                OPCODE_TRUE,
                OPCODE_JUMP_IF_TRUE, 0x0, 0x7,
                OPCODE_JUMP, 0x0, 0xd,
                OPCODE_JUMP, 0x0, 0x4,
                OPCODE_JUMP, 0x0, 0x0,
                OPCODE_NONE,
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_number_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_foreach() {
    struct {
        const char *input;
        int expected_constants_count;
        double expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "for (item in [1, 2, 3]) { }",
            3, {1, 2, 3},
            {
                OPCODE_NUMBER, 0, 0, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 0,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_CONSTANT, 0, 2,
                OPCODE_ARRAY, 0, 3,
                OPCODE_SET_GLOBAL, 0, 1,
                OPCODE_JUMP, 0, 38,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_NUMBER, 0, 0, 0, 1,
                OPCODE_ADD,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 1,
                OPCODE_LEN,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_EQUAL,
                OPCODE_JUMP_IF_FALSE, 0, 52,
                OPCODE_JUMP, 0, 67,
                OPCODE_GET_GLOBAL, 0, 1,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_GET_VALUE_AT,
                OPCODE_SET_GLOBAL, 0, 2,
                OPCODE_NULL,
                OPCODE_POP,
                OPCODE_JUMP, 0, 26,
                OPCODE_NONE,
            }
        },
        {
            "for (item in [1, 2, 3]) { break; }",
            3, {1, 2, 3},
            {
                OPCODE_NUMBER, 0, 0, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 0,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_CONSTANT, 0, 2,
                OPCODE_ARRAY, 0, 3,
                OPCODE_SET_GLOBAL, 0, 1,
                OPCODE_JUMP, 0, 38,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_NUMBER, 0, 0, 0, 1,
                OPCODE_ADD,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 1,
                OPCODE_LEN,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_EQUAL,
                OPCODE_JUMP_IF_FALSE, 0, 52,
                OPCODE_JUMP, 0, 68,
                OPCODE_GET_GLOBAL, 0, 1,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_GET_VALUE_AT,
                OPCODE_SET_GLOBAL, 0, 2,
                OPCODE_JUMP, 0, 49,
                OPCODE_JUMP, 0, 26,
                OPCODE_NONE,
            }
        },
        {
            "const arr = [1, 2, 3]; for (item in arr) { println(item); }",
            3, {1, 2, 3},
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_CONSTANT, 0, 2,
                OPCODE_ARRAY, 0, 3,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_NUMBER, 0, 0, 0, 0,
                OPCODE_SET_GLOBAL, 0, 1,
                OPCODE_JUMP, 0, 38,
                OPCODE_GET_GLOBAL, 0, 1,
                OPCODE_NUMBER, 0, 0, 0, 1,
                OPCODE_ADD,
                OPCODE_SET_GLOBAL, 0, 1,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_LEN,
                OPCODE_GET_GLOBAL, 0, 1,
                OPCODE_EQUAL,
                OPCODE_JUMP_IF_FALSE, 0, 52,
                OPCODE_JUMP, 0, 74,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_GET_GLOBAL, 0, 1,
                OPCODE_GET_VALUE_AT,
                OPCODE_SET_GLOBAL, 0, 2,
                OPCODE_GET_BUILTIN, 0, 1,
                OPCODE_GET_GLOBAL, 0, 2,
                OPCODE_CALL, 1,
                OPCODE_POP,
                OPCODE_JUMP, 0, 26,
                OPCODE_NONE,
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_number_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_for_loop() {
    struct {
        const char *input;
        int expected_constants_count;
        double expected_constants[16];
        uint8_t expected_bytecode[256];
    } tests[] = {
        {
            "var x = 0; for (var i = 0; i < 10; i += 1) { x += 2; }",
            5, {0, 0, 1, 10, 2},
            {
                OPCODE_CONSTANT, 0, 0,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 1,
                OPCODE_SET_GLOBAL, 0, 1,
                OPCODE_JUMP, 0, 27,
                OPCODE_GET_GLOBAL, 0, 1,
                OPCODE_CONSTANT, 0, 2,
                OPCODE_ADD,
                OPCODE_DUP,
                OPCODE_SET_GLOBAL, 0, 1,
                OPCODE_POP,
                OPCODE_CONSTANT, 0, 3,
                OPCODE_GET_GLOBAL, 0, 1,
                OPCODE_GREATER_THAN,
                OPCODE_JUMP_IF_TRUE, 0, 40,
                OPCODE_JUMP, 0, 55,
                OPCODE_GET_GLOBAL, 0, 0,
                OPCODE_CONSTANT, 0, 4,
                OPCODE_ADD,
                OPCODE_DUP,
                OPCODE_SET_GLOBAL, 0, 0,
                OPCODE_POP,
                OPCODE_JUMP, 0, 15,
                OPCODE_NONE,
            }
        },
        {
            "for (;;) { break; }",
            0, {0},
            {
                OPCODE_JUMP, 0, 3,
                OPCODE_TRUE,
                OPCODE_JUMP_IF_TRUE, 0, 10,
                OPCODE_JUMP, 0, 16,
                OPCODE_JUMP, 0, 7,
                OPCODE_JUMP, 0, 3,
                OPCODE_NONE,
            }
        },

    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, true, &res);
        test_number_constants(comp, test.expected_constants, test.expected_constants_count);
        test_bytecode(res->bytecode, res->count, test.expected_bytecode);
    }
}

static void test_tricky_programs() {
    struct {
        const char *input;
        bool should_succeed;
    } tests[] = {
        {"const x = 0; x = 1;", false},
        {"var x = 0; x = 1;", true},
        {"const x = 0; const x = 1;", false},
        {"const x = 0; const fun = fn(x) { x = 1; };", false},
        {"const x = 0; const fun = fn(x) { const x = 0; };", false},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        compilation_result_t *res;
        compiler_t *comp = compile(test.input, test.should_succeed, &res);
        if (test.should_succeed) {
            assert(comp);
        } else {
            assert(comp == false);
        }
    }
}

// INTERNAL
static compiler_t* compile(const char *input, bool must_succeed, compilation_result_t **out_res) {
    *out_res = NULL;

    gcmem_t *mem = gcmem_make();

    ape_config_t *config = malloc(sizeof(ape_config_t));
    config->repl_mode = true;
    compiler_t *comp = compiler_make(config, mem, ptrarray_make());

    compilation_result_t *res = compiler_compile(comp, input);
    if (!res || ptrarray_count(comp->errors) > 0) {
        if (!must_succeed) {
            return NULL;
        }
        print_errors(comp->errors, input);
        assert(false);
    }
    if (!must_succeed) {
        assert(false);
    }
    *out_res = res;
    return comp;
}

static void test_number_constants(compiler_t *comp, double *expected_constants, int expected_constants_count) {
    assert(expected_constants_count == array_count(comp->constants));

    for (int i = 0; i < expected_constants_count; i++) {
        object_t *constant = array_get(comp->constants, i);
        double expected = expected_constants[i];
        double actual = object_get_number(*constant);
        assert(object_get_type(*constant) == OBJECT_NUMBER);
        assert(APE_DBLEQ(actual, expected));
    }
}

static void test_constants(compiler_t *comp, expected_constant_t *expected_constants, int expected_constants_count) {
    assert(expected_constants_count == array_count(comp->constants));

    for (int i = 0; i < expected_constants_count; i++) {
        object_t *constant = array_get(comp->constants, i);
        expected_constant_t expected = expected_constants[i];

        assert(object_get_type(*constant) == expected.type);
        switch (expected.type) {
            case OBJECT_NUMBER: {
                double num = object_get_number(*constant);
                assert(fabs(num - expected.num_val) < DBL_EPSILON);
                break;
            }
            case OBJECT_FUNCTION: {
                function_t *function = object_get_function(*constant);
                test_bytecode(function->comp_result->bytecode, function->comp_result->count, expected.function);
                break;
            }
            case OBJECT_STRING: {
                const char *str = object_get_string(*constant);
                assert(APE_STREQ(str, expected.str_val));
                break;
            }
            default:
                break;
        }
    }
}

static void test_string_constants(compiler_t *comp, const char** expected_constants, int expected_constants_count) {
    assert(expected_constants_count == array_count(comp->constants));

    for (int i = 0; i < expected_constants_count; i++) {
        object_t *constant = array_get(comp->constants, i);
        const char* expected = expected_constants[i];

        assert(object_get_type(*constant) == OBJECT_STRING);
        assert(APE_STREQ(object_get_string(*constant), expected));
    }
}

static int test_operands(opcode_definition_t *def, uint8_t *actual, uint8_t *expected) {
    int to_compare = 0;
    for (int i = 0; i < def->num_operands; i++) {
        to_compare += def->operand_widths[i];
    }

    for (int i = 0; i < to_compare; i++) {
        uint8_t actual_op = actual[i];
        uint8_t expected_op = expected[i];
        assert(actual_op == expected_op);
    }

    return to_compare;
}

static void test_bytecode(uint8_t *bytecode, size_t bytecode_size, uint8_t *expected_bytecode) {
    int i = 0;
    // Uncomment for debugging
//    puts("Expected");
//    for (i = 0; i < bytecode_size; i++) {
//        opcode_val_t expected = expected_bytecode[i];
//        opcode_definition_t *def = opcode_lookup(expected);
//        printf("%d %s, ", i, def->name);
//        int compared_bytes = test_operands(def, bytecode + i, expected_bytecode + i);
//        for (int j = 0; j < compared_bytes; j++) {
//            printf("%d, ", expected_bytecode[i + j + 1]);
//        }
//        printf("\n");
//        i += compared_bytes;
//    }
//    puts("Actual");
//    for (i = 0; i < bytecode_size; i++) {
//        opcode_val_t actual = bytecode[i];
//        opcode_definition_t *def = opcode_lookup(actual);
//        printf("OPCODE_%s, ", def->name);
//        int compared_bytes = test_operands(def, bytecode + i, expected_bytecode + i);
//        for (int j = 0; j < compared_bytes; j++) {
//            printf("%d, ", bytecode[i + j + 1]);
//        }
//        printf("\n");
//        i += compared_bytes;
//    }
    for (i = 0; i < (int)bytecode_size; i++) {
        opcode_val_t expected = expected_bytecode[i];
        opcode_val_t actual = bytecode[i];
        assert(expected == actual);
        opcode_definition_t *def = opcode_lookup(actual);
        int compared_bytes = test_operands(def, bytecode + i + 1, expected_bytecode + i + 1);
        i += compared_bytes;
    }
    assert(expected_bytecode[i] == OPCODE_NONE);
}

#pragma GCC diagnostic pop
