#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
#endif

#include "test_parser.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "common.h"
#include "errors.h"

#include "tests.h"

static ptrarray(statement_t)* parse(const char *code);
static void check_parser_errors(parser_t *parser);

static void test_define_statements(void);
static void test_return_statements(void);
static void test_string(void);
static void test_identifier_expression(void);
static void test_number_literal_expressions(void);
static void test_bool_literal_expressions(void);
static void test_string_literal_expressions(void);
static void test_null_literal_expression(void);
static void test_prefix_expressions(void);
static void test_number_literal(expression_t *expr, double expected);
static void test_infix_expressions(void);
static void test_operator_precedence(void);
static void test_expression(expression_t *expr, const char *value);
static void test_if_statement(void);
static void test_if_else_statement(void);
static void test_fn_literal_parsing(void);
static void test_call_expr_parsing(void);
static void test_array_literal_parsing(void);
static void test_index_expr_parsing(void);
static void test_dot_expr_parsing(void);
static void test_map_literal_parsing(void);
static void test_assign_parsing(void);
static void test_function_literal_with_name(void);
static void test_while_loop(void);
static void test_foreach_loop(void);
static void test_for_loop(void);
static void test_logical_expressions(void);
static void test_recover_statement(void);

void parser_test() {
    puts("### Parser test");
    test_define_statements();
    test_return_statements();
    test_string();
    test_identifier_expression();
    test_number_literal_expressions();
    test_bool_literal_expressions();
    test_string_literal_expressions();
    test_null_literal_expression();
    test_prefix_expressions();
    test_infix_expressions();
    test_operator_precedence();
    test_if_statement();
    test_if_else_statement();
    test_fn_literal_parsing();
    test_call_expr_parsing();
    test_array_literal_parsing();
    test_index_expr_parsing();
    test_dot_expr_parsing();
    test_map_literal_parsing();
    test_assign_parsing();
    test_function_literal_with_name();
    test_while_loop();
    test_foreach_loop();
    test_for_loop();
    test_logical_expressions();
    test_recover_statement();
    puts("\tOK");
}

// INTERNAL
static ptrarray(statement_t)* parse(const char *code) {
    ape_config_t *config = malloc(sizeof(ape_config_t));
    config->repl_mode = true;
    errors_t errs;
    errors_init(&errs);
    parser_t *parser = parser_make(NULL, config, &errs);

    ptrarray(statement_t)* statements = parser_parse_all(parser, code, NULL);

    if (!statements || errors_has_errors(&errs)) {
        print_errors(parser->errors);
        assert(false);
    }

    check_parser_errors(parser);

    return statements;
}

static void check_parser_errors(parser_t *parser) {
    int errors_len = errors_get_count(parser->errors);
    if (errors_len == 0) {
        return;
    }
    printf("Parser has %d errors\n", errors_len);
    for (int i = 0 ; i < errors_len; i++) {
        error_t *error = errors_get(parser->errors, i);
        printf("Parser error: %s\n", error->message);
    }
    assert(false);
}

static void test_define_statements() {
    struct {
        const char *input;
        const char *expected_ident;
        const char *expected_val;
        bool assignable;
    } tests[] = {
        {"const x = 5;", "x", "5", false},
        {"const y = true;", "y", "true", false},
        {"const foobar = 2 + 3;", "foobar", "(2 + 3)", false},
        {"var x = 5;", "x", "5", true},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];

        ptrarray(statement_t)* statements = parse(test.input);
        assert(statements);

        assert(ptrarray_count(statements) == 1);
        statement_t *stmt = ptrarray_get(statements, 0);

        assert(stmt);
        assert(stmt->type == STATEMENT_DEFINE);

        define_statement_t *def_stmt = &stmt->define;

        assert(APE_STREQ(def_stmt->name->value, test.expected_ident));
        assert(def_stmt->assignable == test.assignable);

        strbuf_t *buf = strbuf_make(NULL);
        expression_to_string(def_stmt->value, buf);
        const char *val_str = strbuf_get_string(buf);
        assert(APE_STREQ(val_str, test.expected_val));
        strbuf_destroy(buf);
    }
}

static void test_return_statements() {
    const char *input = "\
return 5;\
return 10;\
return 993322 \
return\
    ";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 4);

    for (int i = 0; i < ptrarray_count(statements); i++) {
        statement_t *stmt = ptrarray_get(statements, i);
        assert(stmt);

        assert(stmt->type == STATEMENT_RETURN_VALUE);
    }
}

static void test_string() {
    ptrarray(statement_t)* statements = ptrarray_make(NULL);

    expression_t *expr = expression_make_ident(NULL, ident_make(NULL, (token_t){
        .literal = "anotherVar",
        .len = strlen("anotherVar")
    }));
    statement_t *stmt = statement_make_define(NULL, ident_make(NULL, (token_t){
        .literal = "myVar",
        .len = strlen("myVar")
    }), expr, false);

    ptrarray_add(statements, stmt);

    char *program_string = statements_to_string(NULL, statements);
    assert(APE_STREQ(program_string, "const myVar = anotherVar"));
    free(program_string);

    ptrarray_destroy_with_items(statements, statement_destroy);
}

static void test_identifier_expression() {
    const char *input = "foobar;";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_EXPRESSION);
    assert(stmt->expression->type == EXPRESSION_IDENT);
    assert(APE_STREQ(stmt->expression->ident->value, "foobar"));
}

static void test_number_literal_expressions() {
    struct {
        const char *input;
        double expected;
    } tests[] = {
        {"5;", 5},
        {"0x5;", 5},
        {"0xa;", 10},
        {"0xbeef;", 0xbeef},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];

        ptrarray(statement_t)* statements = parse(test.input);
        assert(statements);

        assert(ptrarray_count(statements) == 1);

        statement_t *stmt = ptrarray_get(statements, 0);
        assert(stmt->type == STATEMENT_EXPRESSION);
        test_number_literal(stmt->expression, test.expected);
    }
}

static void test_bool_literal_expressions() {
    struct {
        const char *input;
        bool val;
    } bool_tests[] = {
        {"true;", true},
        {"false;", false},
    };

    for (int i = 0; i < APE_ARRAY_LEN(bool_tests); i++) {
        const char *input = bool_tests[i].input;
        bool val = bool_tests[i].val;

        ptrarray(statement_t)* statements = parse(input);
        assert(statements);

        assert(ptrarray_count(statements) == 1);

        statement_t *stmt = ptrarray_get(statements, 0);
        assert(stmt->type == STATEMENT_EXPRESSION);
        assert(stmt->expression->type == EXPRESSION_BOOL_LITERAL);
        assert(stmt->expression->bool_literal == val);
    }
}

static void test_string_literal_expressions() {
    struct {
        const char *input;
        const char *expected;
    } string_tests[] = {
        {"\"lorem ipsum\";", "lorem ipsum"},
    };

    for (int i = 0; i < APE_ARRAY_LEN(string_tests); i++) {
        const char *input = string_tests[i].input;
        const char *expected = string_tests[i].expected;

        ptrarray(statement_t)* statements = parse(input);
        assert(statements);

        assert(ptrarray_count(statements) == 1);

        statement_t *stmt = ptrarray_get(statements, 0);
        assert(stmt->type == STATEMENT_EXPRESSION);
        assert(stmt->expression->type == EXPRESSION_STRING_LITERAL);
        assert(APE_STREQ(stmt->expression->string_literal, expected));
    }
}

static void test_null_literal_expression() {
    const char *input = "null";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_EXPRESSION);
    assert(stmt->expression->type == EXPRESSION_NULL_LITERAL);
}

static void test_prefix_expressions() {
    struct {
        const char *input;
        operator_t op;
        double val;
    } prefix_tests[] = {
        {"!5;", OPERATOR_BANG, 5},
        {"-15;", OPERATOR_MINUS, 15},
    };

    for (int i = 0; i < APE_ARRAY_LEN(prefix_tests); i++) {
        const char *input = prefix_tests[i].input;
        operator_t op = prefix_tests[i].op;
        double val = prefix_tests[i].val;

        ptrarray(statement_t)* statements = parse(input);
        assert(statements);

        assert(ptrarray_count(statements) == 1);

        statement_t *stmt = ptrarray_get(statements, 0);
        assert(stmt->type == STATEMENT_EXPRESSION);
        assert(stmt->expression->type == EXPRESSION_PREFIX);
        assert(stmt->expression->prefix.op == op);
        test_number_literal(stmt->expression->prefix.right, val);
    }
}

static void test_number_literal(expression_t *expr, double expected) {
    assert(expr->type == EXPRESSION_NUMBER_LITERAL);
    assert(APE_DBLEQ(expr->number_literal, expected));
}

static void test_infix_expressions() {
    struct {
        const char *input;
        double left_val;
        operator_t op;
        double right_val;
    } infix_tests[] = {
        {"5 + 5;", 5, OPERATOR_PLUS, 5},
        {"5 - 5;", 5, OPERATOR_MINUS, 5},
        {"5 * 5;", 5, OPERATOR_ASTERISK, 5},
        {"5 / 5;", 5, OPERATOR_SLASH, 5},
        {"5 % 5;", 5, OPERATOR_MODULUS, 5},
        {"5 > 5;", 5, OPERATOR_GT, 5},
        {"5 < 5;", 5, OPERATOR_LT, 5},
        {"5 <= 5;", 5, OPERATOR_LTE, 5},
        {"5 >= 5;", 5, OPERATOR_GTE, 5},
        {"5 == 5;", 5, OPERATOR_EQ, 5},
        {"5 != 5;", 5, OPERATOR_NOT_EQ, 5},
    };

    for (int i = 0; i < APE_ARRAY_LEN(infix_tests); i++) {
        const char *input = infix_tests[i].input;
        operator_t op = infix_tests[i].op;
        double left_val = infix_tests[i].left_val;
        double right_val = infix_tests[i].right_val;

        ptrarray(statement_t)* statements = parse(input);
        assert(statements);

        assert(ptrarray_count(statements) == 1);

        statement_t *stmt = ptrarray_get(statements, 0);
        assert(stmt->type == STATEMENT_EXPRESSION);
        assert(stmt->expression->type == EXPRESSION_INFIX);
        assert(stmt->expression->infix.op == op);
        test_number_literal(stmt->expression->infix.left, left_val);
        test_number_literal(stmt->expression->infix.right, right_val);
    }
}

static void test_operator_precedence() {
    struct {
        const char *input;
        const char *expected;
    } precedence_tests[] = {
        {
            "add(b * c)",
            "add((b * c))",
        },
        {
            "-a * b",
            "((-a) * b)",
        },
        {
            "!-a",
            "(!(-a))",
        },
        {
            "a + b + c",
            "((a + b) + c)",
        },
        {
            "a + b - c",
            "((a + b) - c)",
        },
        {
            "a * b * c",
            "((a * b) * c)",
        },
        {
            "a * b / c",
            "((a * b) / c)",
        },
        {
            "a + b / c",
            "(a + (b / c))",
        },
        {
            "a + b * c + d / e - f",
            "(((a + (b * c)) + (d / e)) - f)",
        },
        {
            "3 + 4; -5 * 5",
            "(3 + 4)\n((-5) * 5)",
        },
        {
            "5 > 4 == 3 < 4",
            "((5 > 4) == (3 < 4))",
        },
        {
            "5 < 4 != 3 > 4",
            "((5 < 4) != (3 > 4))",
        },
        {
            "3 + 4 * 5 == 3 * 1 + 4 * 5",
            "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
        },
        {
            "true",
            "true",
        },
        {
            "false",
            "false",
        },
        {
            "3 > 5 == false",
            "((3 > 5) == false)",
        },
        {
            "3 < 5 == true",
            "((3 < 5) == true)",
        },
        {
            "1 + (2 + 3) + 4",
            "((1 + (2 + 3)) + 4)",
        },
        {
            "(5 + 5) * 2",
            "((5 + 5) * 2)",
        },
        {
            "2 / (5 + 5)",
            "(2 / (5 + 5))",
        },
        {
            "(5 + 5) * 2 * (5 + 5)",
            "(((5 + 5) * 2) * (5 + 5))",
        },
        {
            "-(5 + 5)",
            "(-(5 + 5))",
        },
        {
            "!(true == true)",
            "(!(true == true))",
        },
        {
            "a + add(b * c) + d",
            "((a + add((b * c))) + d)",
        },
        {
            "add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
            "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))",
        },
        {
            "add(a + b + c * d / f + g)",
            "add((((a + b) + ((c * d) / f)) + g))",
        },
        {
            "a * [1, 2, 3, 4][b * c] * d",
            "((a * ([1, 2, 3, 4][(b * c)])) * d)",
        },
        {
            "add(a * b[2], b[1], 2 * [1, 2][1])",
            "add((a * (b[2])), (b[1]), (2 * ([1, 2][1])))",
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(precedence_tests); i++) {
        const char *input = precedence_tests[i].input;
        const char *expected = precedence_tests[i].expected;

        ptrarray(statement_t)* statements = parse(input);
        assert(statements);

        char *actual = statements_to_string(NULL, statements);
        assert(APE_STREQ(actual, expected));
        free(actual);
    }
}

static void test_expression(expression_t *expr, const char *value) {
    strbuf_t *buf = strbuf_make(NULL);
    expression_to_string(expr, buf);
    const char *str = strbuf_get_string(buf);
    assert(APE_STREQ(str, value));
    strbuf_destroy(buf);
}

static void test_if_statement(void) {
    const char *input = "if (x < y) { x = 1; }";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_IF);

    if_statement_t *if_stmt = &stmt->if_statement;
    assert(ptrarray_count(if_stmt->cases) == 1);

    if_case_t *if_case = ptrarray_get(if_stmt->cases, 0);
    test_expression(if_case->test, "(x < y)");
    assert(ptrarray_count(if_case->consequence->statements) == 1);
    statement_t *elif_stmt = ptrarray_get(if_case->consequence->statements, 0);
    assert(elif_stmt->type == STATEMENT_EXPRESSION);
    test_expression(elif_stmt->expression, "x = 1");
}

static void test_if_else_statement() {
    const char *input = "if (x < y) { x = 1; } else if (x > y) { y = 1; } else if (x > y) { y2 = 1; } else { z = 1; }";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_IF);

    if_statement_t *if_stmt = &stmt->if_statement;

    assert(ptrarray_count(if_stmt->cases) == 3);

    if_case_t *if_case = ptrarray_get(if_stmt->cases, 0);
    test_expression(if_case->test, "(x < y)");
    assert(ptrarray_count(if_case->consequence->statements) == 1);
    statement_t *elif_stmt = ptrarray_get(if_case->consequence->statements, 0);
    assert(elif_stmt->type == STATEMENT_EXPRESSION);
    test_expression(elif_stmt->expression, "x = 1");

    if_case = ptrarray_get(if_stmt->cases, 1);
    test_expression(if_case->test, "(x > y)");
    assert(ptrarray_count(if_case->consequence->statements) == 1);
    elif_stmt = ptrarray_get(if_case->consequence->statements, 0);
    assert(elif_stmt->type == STATEMENT_EXPRESSION);
    test_expression(elif_stmt->expression, "y = 1");

    if_case = ptrarray_get(if_stmt->cases, 2);
    test_expression(if_case->test, "(x > y)");
    assert(ptrarray_count(if_case->consequence->statements) == 1);
    elif_stmt = ptrarray_get(if_case->consequence->statements, 0);
    assert(elif_stmt->type == STATEMENT_EXPRESSION);
    test_expression(elif_stmt->expression, "y2 = 1");

    assert(ptrarray_count(if_stmt->alternative->statements) == 1);
    statement_t *else_stmt = ptrarray_get(if_stmt->alternative->statements, 0);
    assert(else_stmt->type == STATEMENT_EXPRESSION);
    test_expression(else_stmt->expression, "z = 1");
}

static void test_fn_literal_parsing() {
    const char *input = "fn(x, y) { return x + y; };";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_EXPRESSION);
    assert(stmt->expression->type == EXPRESSION_FUNCTION_LITERAL);

    fn_literal_t *fn = &stmt->expression->fn_literal;

    assert(ptrarray_count(fn->params) == 2);
    ident_t *id_x = ptrarray_get(fn->params, 0);
    assert(APE_STREQ(id_x->value, "x"));

    ident_t *id_y = ptrarray_get(fn->params, 1);
    assert(APE_STREQ(id_y->value, "y"));

    code_block_t *body_stmt = fn->body;
    assert(body_stmt);
    strbuf_t *buf = strbuf_make(NULL);
    code_block_to_string(body_stmt, buf);
    const char *str = strbuf_get_string(buf);
    assert(APE_STREQ(str, "{ return (x + y)\n }"));
    strbuf_destroy(buf);
}

static void test_call_expr_parsing() {
    const char *input = "add(1, 2 * 3, 4 + 5);";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_EXPRESSION);
    assert(stmt->expression->type == EXPRESSION_CALL);

    call_expression_t *call_expr = &stmt->expression->call_expr;

    assert(call_expr->function->type == EXPRESSION_IDENT);
    assert(APE_STREQ(call_expr->function->ident->value, "add"));

    assert(ptrarray_count(call_expr->args) == 3);

#define TEST_ARGUMENT(n, str)\
do { \
    expression_t *arg_expr = ptrarray_get(call_expr->args, n);\
    strbuf_t *buf = strbuf_make(NULL);\
    expression_to_string(arg_expr, buf);\
    const char *buf_str = strbuf_get_string(buf);\
    assert(APE_STREQ(buf_str, str));\
    strbuf_destroy(buf);\
} while (0)

    TEST_ARGUMENT(0, "1");
    TEST_ARGUMENT(1, "(2 * 3)");
    TEST_ARGUMENT(2, "(4 + 5)");

#undef TEST_ARGUMENT
}

static void test_array_literal_parsing() {
    const char *input = "[1, 2 * 2, 3 + 3];";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_EXPRESSION);
    assert(stmt->expression->type == EXPRESSION_ARRAY_LITERAL);

    ptrarray(expression_t) *array = stmt->expression->array;

    assert(ptrarray_count(array) == 3);

#define TEST_ELEMENT(n, str)\
do { \
    expression_t *arr_expr = ptrarray_get(array, n);\
    strbuf_t *buf = strbuf_make(NULL);\
    expression_to_string(arr_expr, buf);\
    const char *buf_str = strbuf_get_string(buf);\
    assert(APE_STREQ(buf_str, str));\
    strbuf_destroy(buf);\
} while (0)

    TEST_ELEMENT(0, "1");
    TEST_ELEMENT(1, "(2 * 2)");
    TEST_ELEMENT(2, "(3 + 3)");

#undef TEST_ELEMENT
}

static void test_index_expr_parsing() {
    const char *input = "myArray[1 + 1];";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_EXPRESSION);
    assert(stmt->expression->type == EXPRESSION_INDEX);

    index_expression_t *index_expr = &stmt->expression->index_expr;

    assert(index_expr->left->type == EXPRESSION_IDENT);
    assert(APE_STREQ(index_expr->left->ident->value, "myArray"));

    strbuf_t *buf = strbuf_make(NULL);
    expression_to_string(index_expr->index, buf);
    const char *val_str = strbuf_get_string(buf);
    assert(APE_STREQ(val_str, "(1 + 1)"));
    strbuf_destroy(buf);
}

static void test_dot_expr_parsing() {
    const char *input = "myObj.foo;";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_EXPRESSION);
    assert(stmt->expression->type == EXPRESSION_INDEX);

    index_expression_t *index_expr = &stmt->expression->index_expr;

    assert(index_expr->left->type == EXPRESSION_IDENT);
    assert(APE_STREQ(index_expr->left->ident->value, "myObj"));

    strbuf_t *buf = strbuf_make(NULL);
    expression_to_string(index_expr->index, buf);
    const char *val_str = strbuf_get_string(buf);
    assert(APE_STREQ(val_str, "\"foo\""));
    strbuf_destroy(buf);
}

static void test_map_literal_parsing() {
    const char *input = "const dict = {\"one\": 1, \"two\": 2, \"three\": 3};";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_DEFINE);
    assert(stmt->define.value->type == EXPRESSION_MAP_LITERAL);

    map_literal_t *map = &stmt->define.value->map;

#define TEST_ELEMENT(n, key, val)\
do { \
    expression_t *key_expr = ptrarray_get(map->keys, n);\
    strbuf_t *buf = strbuf_make(NULL);\
    expression_to_string(key_expr, buf);\
    const char *buf_str = strbuf_get_string(buf);\
    assert(APE_STREQ(buf_str, key));\
    strbuf_destroy(buf);\
    expression_t *val_expr = ptrarray_get(map->values, n);\
    buf = strbuf_make(NULL);\
    expression_to_string(val_expr, buf);\
    buf_str = strbuf_get_string(buf);\
    assert(APE_STREQ(buf_str, val));\
    strbuf_destroy(buf);\
} while (0)

    TEST_ELEMENT(0, "\"one\"", "1");
    TEST_ELEMENT(1, "\"two\"", "2");
    TEST_ELEMENT(2, "\"three\"", "3");

#undef TEST_ELEMENT
}

static void test_assign_parsing() {
    const char *input = "const val = 1; val = 2;";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 2);

    statement_t *stmt = ptrarray_get(statements, 1);
    assert(stmt->type == STATEMENT_EXPRESSION);
    assert(stmt->expression->type == EXPRESSION_ASSIGN);
}

static void test_function_literal_with_name() {
    const char *input = "const myFunction = fn() { };";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_DEFINE);
    assert(stmt->define.value->type == EXPRESSION_FUNCTION_LITERAL);
    assert(APE_STREQ(stmt->define.value->fn_literal.name, "myFunction"));

    input = "fn myFunction() { };";

    statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_DEFINE);
    assert(stmt->define.value->type == EXPRESSION_FUNCTION_LITERAL);
    assert(APE_STREQ(stmt->define.value->fn_literal.name, "myFunction"));
}

static void test_while_loop() {
    const char *input = "while (true) { break; }";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_WHILE_LOOP);
    assert(stmt->while_loop.test->type == EXPRESSION_BOOL_LITERAL);

    code_block_t *body = stmt->while_loop.body;
    assert(ptrarray_count(body->statements) == 1);
    statement_t *break_stmt = ptrarray_get(body->statements, 0);
    assert(break_stmt->type == STATEMENT_BREAK);
}

static void test_foreach_loop() {
    const char *input = "for (item in foo) { continue; }";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_FOREACH);
    assert(APE_STREQ(stmt->foreach.iterator->value, "item"));

    assert(stmt->foreach.source->type == EXPRESSION_IDENT);
    assert(APE_STREQ(stmt->foreach.source->ident->value, "foo"));

    code_block_t *body = stmt->foreach.body;
    assert(ptrarray_count(body->statements) == 1);
    statement_t *cont_stmt = ptrarray_get(body->statements, 0);
    assert(cont_stmt->type == STATEMENT_CONTINUE);
}

static void test_for_loop() {
    const char *input = "for (var i = 0; i < 10; i++) { break; }";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_FOR_LOOP);

    assert(stmt->for_loop.init->type == STATEMENT_DEFINE);
    assert(APE_STREQ(stmt->for_loop.init->define.name->value, "i"));

    assert(stmt->for_loop.test->type == EXPRESSION_INFIX);

    assert(stmt->for_loop.update->type == EXPRESSION_ASSIGN);

    code_block_t *body = stmt->for_loop.body;
    assert(ptrarray_count(body->statements) == 1);
    statement_t *break_stmt = ptrarray_get(body->statements, 0);
    assert(break_stmt->type == STATEMENT_BREAK);
}

static void test_logical_expressions() {
    const char *input = "a && b;";
    
    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_EXPRESSION);
    assert(stmt->expression->type == EXPRESSION_LOGICAL);
    assert(stmt->expression->logical.op == OPERATOR_LOGICAL_AND);

    input = "a || b;";

    statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_EXPRESSION);
    assert(stmt->expression->type == EXPRESSION_LOGICAL);
    assert(stmt->expression->logical.op == OPERATOR_LOGICAL_OR);
}

static void test_recover_statement() {
    const char *input = "recover (e) { return null; }";

    ptrarray(statement_t)* statements = parse(input);
    assert(statements);

    assert(ptrarray_count(statements) == 1);

    statement_t *stmt = ptrarray_get(statements, 0);
    assert(stmt->type == STATEMENT_RECOVER);
    assert(APE_STREQ(stmt->recover.error_ident->value, "e"));

    assert(ptrarray_count(stmt->recover.body->statements) == 1);
}

#pragma GCC diagnostic pop
