#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
#endif
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wmissing-braces"

#include "test_lexer.h"

#include <assert.h>
#include <stdio.h>

#include "lexer.h"
#include "common.h"

static void test_lexing(void);
static void test_token_positions(void);

void lexer_test() {
    puts("### Lexer test");
    test_lexing();
    test_token_positions();
    puts("\tOK");
}

static void test_lexing() {
    const char *input = "\
    import test\
    const five = 5;\
    var ten = 10;\
    const add = fn(x, y) {\
    x.foo + y;\
    };\
    'abc'\
    null\
    +=\
    -=\
    /=\
    *=\
    %=\
    &=\
    |=\
    ^=\
    <<=\
    >>=\
    const result = add(five, ten);\
    !-/*%5;\
    5 < 10 > 5 <= >=;\
    if (5 < 10) {\
    return true;\
    } else if (x) {\
    } else {\n\
    // comment\n\
    return false;\
    }\
    10 == 10;\
    10 != 9;\
    && ||\
    \"foobar\"\
    \"foo bar\"//comment\n\
    \"foo \\\"bar\"\
    [1, 2];\
    {\"foo\": \"bar\"}\
    while (true) { break; }\
    for (item in foo) { }\
    for\
    continue\
    recover\
    ^\
    <<\
    >>\
    ";

    token_t expected_tokens[] = {
        {TOKEN_IMPORT, "import"},
        {TOKEN_IDENT, "test"},
        {TOKEN_CONST, "const"},
        {TOKEN_IDENT, "five"},
        {TOKEN_ASSIGN, "="},
        {TOKEN_NUMBER, "5"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_VAR, "var"},
        {TOKEN_IDENT, "ten"},
        {TOKEN_ASSIGN, "="},
        {TOKEN_NUMBER, "10"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_CONST, "const"},
        {TOKEN_IDENT, "add"},
        {TOKEN_ASSIGN, "="},
        {TOKEN_FUNCTION, "fn"},
        {TOKEN_LPAREN, "("},
        {TOKEN_IDENT, "x"},
        {TOKEN_COMMA, ","},
        {TOKEN_IDENT, "y"},
        {TOKEN_RPAREN, ")"},
        {TOKEN_LBRACE, "{"},
        {TOKEN_IDENT, "x"},
        {TOKEN_DOT, "."},
        {TOKEN_IDENT, "foo"},
        {TOKEN_PLUS, "+"},
        {TOKEN_IDENT, "y"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_RBRACE, "}"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_STRING, "abc"},
        {TOKEN_NULL, "null"},
        {TOKEN_PLUS_ASSIGN, "+="},
        {TOKEN_MINUS_ASSIGN, "-="},
        {TOKEN_SLASH_ASSIGN, "/="},
        {TOKEN_ASTERISK_ASSIGN, "*="},
        {TOKEN_PERCENT_ASSIGN, "%="},
        {TOKEN_BIT_AND_ASSIGN, "&="},
        {TOKEN_BIT_OR_ASSIGN, "|="},
        {TOKEN_BIT_XOR_ASSIGN, "^="},
        {TOKEN_LSHIFT_ASSIGN, "<<="},
        {TOKEN_RSHIFT_ASSIGN, ">>="},
        {TOKEN_CONST, "const"},
        {TOKEN_IDENT, "result"},
        {TOKEN_ASSIGN, "="},
        {TOKEN_IDENT, "add"},
        {TOKEN_LPAREN, "("},
        {TOKEN_IDENT, "five"},
        {TOKEN_COMMA, ","},
        {TOKEN_IDENT, "ten"},
        {TOKEN_RPAREN, ")"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_BANG, "!"},
        {TOKEN_MINUS, "-"},
        {TOKEN_SLASH, "/"},
        {TOKEN_ASTERISK, "*"},
        {TOKEN_PERCENT, "%"},
        {TOKEN_NUMBER, "5"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_NUMBER, "5"},
        {TOKEN_LT, "<"},
        {TOKEN_NUMBER, "10"},
        {TOKEN_GT, ">"},
        {TOKEN_NUMBER, "5"},
        {TOKEN_LTE, "<="},
        {TOKEN_GTE, ">="},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_IF, "if"},
        {TOKEN_LPAREN, "("},
        {TOKEN_NUMBER, "5"},
        {TOKEN_LT, "<"},
        {TOKEN_NUMBER, "10"},
        {TOKEN_RPAREN, ")"},
        {TOKEN_LBRACE, "{"},
        {TOKEN_RETURN, "return"},
        {TOKEN_TRUE, "true"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_RBRACE, "}"},
        {TOKEN_ELSE, "else"},
        {TOKEN_IF, "if"},
        {TOKEN_LPAREN, "("},
        {TOKEN_IDENT, "x"},
        {TOKEN_RPAREN, ")"},
        {TOKEN_LBRACE, "{"},
        {TOKEN_RBRACE, "}"},
        {TOKEN_ELSE, "else"},
        {TOKEN_LBRACE, "{"},
        {TOKEN_RETURN, "return"},
        {TOKEN_FALSE, "false"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_RBRACE, "}"},
        {TOKEN_NUMBER, "10"},
        {TOKEN_EQ, "=="},
        {TOKEN_NUMBER, "10"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_NUMBER, "10"},
        {TOKEN_NOT_EQ, "!="},
        {TOKEN_NUMBER, "9"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_AND, "&&"},
        {TOKEN_OR, "||"},
        {TOKEN_STRING, "foobar"},
        {TOKEN_STRING, "foo bar"},
        {TOKEN_STRING, "foo \\\"bar"},
        {TOKEN_LBRACKET, "["},
        {TOKEN_NUMBER, "1"},
        {TOKEN_COMMA, ","},
        {TOKEN_NUMBER, "2"},
        {TOKEN_RBRACKET, "]"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_LBRACE, "{"},
        {TOKEN_STRING, "foo"},
        {TOKEN_COLON, ":"},
        {TOKEN_STRING, "bar"},
        {TOKEN_RBRACE, "}"},
        {TOKEN_WHILE, "while"},
        {TOKEN_LPAREN, "("},
        {TOKEN_TRUE, "true"},
        {TOKEN_RPAREN, ")"},
        {TOKEN_LBRACE, "{"},
        {TOKEN_BREAK, "break"},
        {TOKEN_SEMICOLON, ";"},
        {TOKEN_RBRACE, "}"},
        {TOKEN_FOR, "for"},
        {TOKEN_LPAREN, "("},
        {TOKEN_IDENT, "item"},
        {TOKEN_IN, "in"},
        {TOKEN_IDENT, "foo"},
        {TOKEN_RPAREN, ")"},
        {TOKEN_LBRACE, "{"},
        {TOKEN_RBRACE, "}"},
        {TOKEN_FOR, "for"},
        {TOKEN_CONTINUE, "continue"},
        {TOKEN_RECOVER, "recover"},
        {TOKEN_BIT_XOR, "^"},
        {TOKEN_LSHIFT, "<<"},
        {TOKEN_RSHIFT, ">>"},
        {TOKEN_EOF, "EOF"},
    };

    lexer_t lexer;
    lexer_init(&lexer, NULL, NULL, input, NULL);

    for (int i = 0; i < APE_ARRAY_LEN(expected_tokens); i++) {
        token_t test_tok = expected_tokens[i];
        token_t tok = lexer_next_token_internal(&lexer);
        assert(tok.type == test_tok.type);
        assert(APE_STRNEQ(tok.literal, test_tok.literal, tok.len));
    }
}

static void test_token_positions() {
    const char *input = "\n\
var five = 5;\n\
    var add = fn(x, y) {\n\
        x + y;\n\
    }\n\
    ";

    token_t expected_tokens[] = {
        {TOKEN_VAR, "var", 0, NULL, 1, 0},
        {TOKEN_IDENT, "five", 0, NULL, 1, 4},
        {TOKEN_ASSIGN, "=", 0, NULL, 1, 9},
        {TOKEN_NUMBER, "5", 0, NULL, 1, 11},
        {TOKEN_SEMICOLON, ";", 0, NULL, 1, 12},
        {TOKEN_VAR, "var", 0, NULL, 2, 4},
        {TOKEN_IDENT, "add", 0, NULL, 2, 8},
        {TOKEN_ASSIGN, "=", 0, NULL, 2, 12},
        {TOKEN_FUNCTION, "fn", 0, NULL, 2, 14},
        {TOKEN_LPAREN, "(", 0, NULL, 2, 16},
        {TOKEN_IDENT, "x", 0, NULL, 2, 17},
        {TOKEN_COMMA, ",", 0, NULL, 2, 18},
        {TOKEN_IDENT, "y", 0, NULL, 2, 20},
        {TOKEN_RPAREN, ")", 0, NULL, 2, 21},
        {TOKEN_LBRACE, "{", 0, NULL, 2, 23},
        {TOKEN_IDENT, "x", 0, NULL, 3, 8},
        {TOKEN_PLUS, "+", 0, NULL, 3, 10},
        {TOKEN_IDENT, "y", 0, NULL, 3, 12},
        {TOKEN_SEMICOLON, ";", 0, NULL, 3, 13},
        {TOKEN_RBRACE, "}", 0, NULL, 4, 4},
        {TOKEN_EOF, "EOF", 0, NULL, 5, 4},
    };

    lexer_t lexer;
    lexer_init(&lexer, NULL, NULL, input, NULL);

    for (int i = 0; i < APE_ARRAY_LEN(expected_tokens); i++) {
        token_t test_tok = expected_tokens[i];
        token_t tok = lexer_next_token_internal(&lexer);
        assert(tok.type == test_tok.type);
        assert(APE_STRNEQ(tok.literal, test_tok.literal, tok.len));
        assert(tok.pos.line == test_tok.pos.line);
        assert(tok.pos.column == test_tok.pos.column);
    }
}

#pragma GCC diagnostic pop
