/*
SPDX-License-Identifier: MIT

ape
https://github.com/kgabis/ape
Copyright (c) 2021 Krzysztof Gabis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#define APE_AMALGAMATED

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif /* _CRT_SECURE_NO_WARNINGS */
#endif /* _MSC_VER */

#include "ape.h"

//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------
//FILE_START:common.h
#ifndef common_h
#define common_h

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <float.h>

#if defined(__linux__)
    #define APE_LINUX
    #define APE_POSIX
#elif defined(_WIN32)
    #define APE_WINDOWS
#elif (defined(__APPLE__) && defined(__MACH__))
    #define APE_APPLE
    #define APE_POSIX
#elif defined(__EMSCRIPTEN__)
    #define APE_EMSCRIPTEN
#endif

#if defined(__unix__)
#include <unistd.h>
#if defined(_POSIX_VERSION)
    #define APE_POSIX
#endif
#endif

#if defined(APE_POSIX)
#include <sys/time.h>
#elif defined(APE_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(APE_EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

#ifndef APE_AMALGAMATED
#include "ape.h"
#endif

#define APE_STREQ(a, b) (strcmp((a), (b)) == 0)
#define APE_STRNEQ(a, b, n) (strncmp((a), (b), (n)) == 0)
#define APE_ARRAY_LEN(array) ((int)(sizeof(array) / sizeof(array[0])))
#define APE_DBLEQ(a, b) (fabs((a) - (b)) < DBL_EPSILON)

#ifdef APE_DEBUG
    #define APE_ASSERT(x) assert((x))
    #define APE_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
    #define APE_LOG(...) ape_log(APE_FILENAME, __LINE__, __VA_ARGS__)
#else
    #define APE_ASSERT(x) ((void)0)
    #define APE_LOG(...) ((void)0)
#endif

#ifdef APE_AMALGAMATED
    #define COLLECTIONS_AMALGAMATED
    #define APE_INTERNAL static
#else
    #define APE_INTERNAL
#endif

typedef struct compiled_file compiled_file_t;
typedef struct allocator allocator_t;

typedef struct src_pos {
    const compiled_file_t *file;
    int line;
    int column;
} src_pos_t;

typedef struct ape_config {
    struct {
        struct {
            ape_stdout_write_fn write;
            void *context;
        } write;
    } stdio;

    struct {
        struct {
            ape_read_file_fn read_file;
            void *context;
        } read_file;

        struct {
            ape_write_file_fn write_file;
            void *context;
        } write_file;
    } fileio;

    bool repl_mode; // allows redefinition of symbols

    double max_execution_time_ms;
    bool max_execution_time_set;
} ape_config_t;

typedef struct ape_timer {
#if defined(APE_POSIX)
    int64_t start_offset;
#elif defined(APE_WINDOWS)
    double pc_frequency;
#endif
    double start_time_ms;
} ape_timer_t;

#ifndef APE_AMALGAMATED
extern const src_pos_t src_pos_invalid;
extern const src_pos_t src_pos_zero;
#endif

APE_INTERNAL src_pos_t src_pos_make(const compiled_file_t *file, int line, int column);
APE_INTERNAL char *ape_stringf(allocator_t *alloc, const char *format, ...);
APE_INTERNAL void ape_log(const char *file, int line, const char *format, ...);
APE_INTERNAL char* ape_strndup(allocator_t *alloc, const char *string, size_t n);
APE_INTERNAL char* ape_strdup(allocator_t *alloc, const char *string);

APE_INTERNAL uint64_t ape_double_to_uint64(double val);
APE_INTERNAL double ape_uint64_to_double(uint64_t val);

APE_INTERNAL bool ape_timer_platform_supported(void);
APE_INTERNAL ape_timer_t ape_timer_start(void);
APE_INTERNAL double ape_timer_get_elapsed_ms(const ape_timer_t *timer);

#endif /* common_h */
//FILE_END
//FILE_START:collections.h
#ifndef collections_h
#define collections_h

#include <stdbool.h>
#include <stddef.h>

#ifdef COLLECTIONS_AMALGAMATED
#define COLLECTIONS_API static
#else
#define COLLECTIONS_API
#endif

//-----------------------------------------------------------------------------
// Collections
//-----------------------------------------------------------------------------

typedef unsigned long (*collections_hash_fn)(const void* val);
typedef bool (*collections_equals_fn)(const void *a, const void *b);

//-----------------------------------------------------------------------------
// Allocator
//-----------------------------------------------------------------------------

typedef void * (*allocator_malloc_fn)(void *ctx, size_t size);
typedef void (*allocator_free_fn)(void *ctx, void *ptr);

typedef struct allocator {
    allocator_malloc_fn malloc;
    allocator_free_fn free;
    void *ctx;
} allocator_t;

COLLECTIONS_API allocator_t allocator_make(allocator_malloc_fn malloc_fn, allocator_free_fn free_fn, void *ctx);
COLLECTIONS_API void* allocator_malloc(allocator_t *allocator, size_t size);
COLLECTIONS_API void  allocator_free(allocator_t *allocator, void *ptr);

//-----------------------------------------------------------------------------
// Dictionary
//-----------------------------------------------------------------------------

typedef struct dict_ dict_t_;

#define dict(TYPE) dict_t_

typedef void (*dict_item_destroy_fn)(void* item);
typedef void* (*dict_item_copy_fn)(void* item);

#define dict_make(alloc, copy_fn, destroy_fn) dict_make_(alloc, (dict_item_copy_fn)(copy_fn), (dict_item_destroy_fn)(destroy_fn))

COLLECTIONS_API dict_t_*     dict_make_(allocator_t *alloc, dict_item_copy_fn copy_fn, dict_item_destroy_fn destroy_fn);
COLLECTIONS_API void         dict_destroy(dict_t_ *dict);
COLLECTIONS_API void         dict_destroy_with_items(dict_t_ *dict);
COLLECTIONS_API dict_t_*     dict_copy_with_items(dict_t_ *dict);
COLLECTIONS_API bool         dict_set(dict_t_ *dict, const char *key, void *value);
COLLECTIONS_API void *       dict_get(const dict_t_ *dict, const char *key);
COLLECTIONS_API void *       dict_get_value_at(const dict_t_ *dict, unsigned int ix);
COLLECTIONS_API const char * dict_get_key_at(const dict_t_ *dict, unsigned int ix);
COLLECTIONS_API int          dict_count(const dict_t_ *dict);
COLLECTIONS_API bool         dict_remove(dict_t_ *dict, const char *key);

//-----------------------------------------------------------------------------
// Value dictionary
//-----------------------------------------------------------------------------

typedef struct valdict_ valdict_t_;

#define valdict(KEY_TYPE, VALUE_TYPE) valdict_t_

#define valdict_make(allocator, key_type, val_type) valdict_make_(allocator, sizeof(key_type), sizeof(val_type))

COLLECTIONS_API valdict_t_*  valdict_make_(allocator_t *alloc, size_t key_size, size_t val_size);
COLLECTIONS_API valdict_t_*  valdict_make_with_capacity(allocator_t *alloc, unsigned int min_capacity, size_t key_size, size_t val_size);
COLLECTIONS_API void         valdict_destroy(valdict_t_ *dict);
COLLECTIONS_API void         valdict_set_hash_function(valdict_t_ *dict, collections_hash_fn hash_fn);
COLLECTIONS_API void         valdict_set_equals_function(valdict_t_ *dict, collections_equals_fn equals_fn);
COLLECTIONS_API bool         valdict_set(valdict_t_ *dict, void *key, void *value);
COLLECTIONS_API void *       valdict_get(const valdict_t_ *dict, const void *key);
COLLECTIONS_API void *       valdict_get_key_at(const valdict_t_ *dict, unsigned int ix);
COLLECTIONS_API void *       valdict_get_value_at(const valdict_t_ *dict, unsigned int ix);
COLLECTIONS_API unsigned int valdict_get_capacity(const valdict_t_ *dict);
COLLECTIONS_API bool         valdict_set_value_at(const valdict_t_ *dict, unsigned int ix, const void *value);
COLLECTIONS_API int          valdict_count(const valdict_t_ *dict);
COLLECTIONS_API bool         valdict_remove(valdict_t_ *dict, void *key);
COLLECTIONS_API void         valdict_clear(valdict_t_ *dict);

//-----------------------------------------------------------------------------
// Pointer dictionary
//-----------------------------------------------------------------------------

typedef struct ptrdict_ ptrdict_t_;

#define ptrdict(KEY_TYPE, VALUE_TYPE) ptrdict_t_

COLLECTIONS_API ptrdict_t_* ptrdict_make(allocator_t *alloc);
COLLECTIONS_API void        ptrdict_destroy(ptrdict_t_ *dict);
COLLECTIONS_API void        ptrdict_set_hash_function(ptrdict_t_ *dict, collections_hash_fn hash_fn);
COLLECTIONS_API void        ptrdict_set_equals_function(ptrdict_t_ *dict, collections_equals_fn equals_fn);
COLLECTIONS_API bool        ptrdict_set(ptrdict_t_ *dict, void *key, void *value);
COLLECTIONS_API void *      ptrdict_get(const ptrdict_t_ *dict, const void *key);
COLLECTIONS_API void *      ptrdict_get_value_at(const ptrdict_t_ *dict, unsigned int ix);
COLLECTIONS_API void *      ptrdict_get_key_at(const ptrdict_t_ *dict, unsigned int ix);
COLLECTIONS_API int         ptrdict_count(const ptrdict_t_ *dict);
COLLECTIONS_API bool        ptrdict_remove(ptrdict_t_ *dict, void *key);

//-----------------------------------------------------------------------------
// Array
//-----------------------------------------------------------------------------

typedef struct array_ array_t_;

#define array(TYPE) array_t_

#define array_make(allocator, type) array_make_(allocator, sizeof(type))
typedef void (*array_item_deinit_fn)(void* item);
#define array_destroy_with_items(arr, fn) array_destroy_with_items_(arr, (array_item_deinit_fn)(fn))
#define array_clear_and_deinit_items(arr, fn) array_clear_and_deinit_items_(arr, (array_item_deinit_fn)(fn))

COLLECTIONS_API array_t_*    array_make_(allocator_t *alloc, size_t element_size);
COLLECTIONS_API array_t_*    array_make_with_capacity(allocator_t *alloc, unsigned int capacity, size_t element_size);
COLLECTIONS_API void         array_destroy(array_t_ *arr);
COLLECTIONS_API void         array_destroy_with_items_(array_t_ *arr, array_item_deinit_fn deinit_fn);
COLLECTIONS_API array_t_*    array_copy(const array_t_ *arr);
COLLECTIONS_API bool         array_add(array_t_ *arr, const void *value);
COLLECTIONS_API bool         array_addn(array_t_ *arr, const void *values, int n);
COLLECTIONS_API bool         array_add_array(array_t_ *dest, array_t_ *source);
COLLECTIONS_API bool         array_push(array_t_ *arr, const void *value);
COLLECTIONS_API bool         array_pop(array_t_ *arr, void *out_value);
COLLECTIONS_API void *       array_top(array_t_ *arr);
COLLECTIONS_API bool         array_set(array_t_ *arr, unsigned int ix, void *value);
COLLECTIONS_API bool         array_setn(array_t_ *arr, unsigned int ix, void *values, int n);
COLLECTIONS_API void *       array_get(array_t_ *arr, unsigned int ix);
COLLECTIONS_API const void * array_get_const(const array_t_ *arr, unsigned int ix);
COLLECTIONS_API void *       array_get_last(array_t_ *arr);
COLLECTIONS_API int          array_count(const array_t_ *arr);
COLLECTIONS_API unsigned int array_get_capacity(const array_t_ *arr);
COLLECTIONS_API bool         array_remove_at(array_t_ *arr, unsigned int ix);
COLLECTIONS_API bool         array_remove_item(array_t_ *arr, void *ptr);
COLLECTIONS_API void         array_clear(array_t_ *arr);
COLLECTIONS_API void         array_clear_and_deinit_items_(array_t_ *arr, array_item_deinit_fn deinit_fn);
COLLECTIONS_API void         array_lock_capacity(array_t_ *arr);
COLLECTIONS_API int          array_get_index(const array_t_ *arr, void *ptr);
COLLECTIONS_API bool         array_contains(const array_t_ *arr, void *ptr);
COLLECTIONS_API void*        array_data(array_t_ *arr); // might become invalidated by remove/add operations
COLLECTIONS_API const void*  array_const_data(const array_t_ *arr);
COLLECTIONS_API void         array_orphan_data(array_t_ *arr);
COLLECTIONS_API bool         array_reverse(array_t_ *arr);

//-----------------------------------------------------------------------------
// Pointer Array
//-----------------------------------------------------------------------------

typedef struct ptrarray_ ptrarray_t_;

#define ptrarray(TYPE) ptrarray_t_

typedef void (*ptrarray_item_destroy_fn)(void* item);
#define ptrarray_destroy_with_items(arr, fn) ptrarray_destroy_with_items_(arr, (ptrarray_item_destroy_fn)(fn))
#define ptrarray_clear_and_destroy_items(arr, fn) ptrarray_clear_and_destroy_items_(arr, (ptrarray_item_destroy_fn)(fn))

typedef void* (*ptrarray_item_copy_fn)(void* item);
#define ptrarray_copy_with_items(arr, copy_fn, destroy_fn) ptrarray_copy_with_items_(arr, (ptrarray_item_copy_fn)(copy_fn), (ptrarray_item_destroy_fn)(destroy_fn))

COLLECTIONS_API ptrarray_t_* ptrarray_make(allocator_t *alloc);
COLLECTIONS_API ptrarray_t_* ptrarray_make_with_capacity(allocator_t *alloc, unsigned int capacity);
COLLECTIONS_API void         ptrarray_destroy(ptrarray_t_ *arr);
COLLECTIONS_API void         ptrarray_destroy_with_items_(ptrarray_t_ *arr, ptrarray_item_destroy_fn destroy_fn);
COLLECTIONS_API ptrarray_t_* ptrarray_copy(ptrarray_t_ *arr);
COLLECTIONS_API ptrarray_t_* ptrarray_copy_with_items_(ptrarray_t_ *arr, ptrarray_item_copy_fn copy_fn, ptrarray_item_destroy_fn destroy_fn);
COLLECTIONS_API bool         ptrarray_add(ptrarray_t_ *arr, void *ptr);
COLLECTIONS_API bool         ptrarray_set(ptrarray_t_ *arr, unsigned int ix, void *ptr);
COLLECTIONS_API bool         ptrarray_add_array(ptrarray_t_ *dest, ptrarray_t_ *source);
COLLECTIONS_API void *       ptrarray_get(ptrarray_t_ *arr, unsigned int ix);
COLLECTIONS_API const void * ptrarray_get_const(const ptrarray_t_ *arr, unsigned int ix);
COLLECTIONS_API bool         ptrarray_push(ptrarray_t_ *arr, void *ptr);
COLLECTIONS_API void *       ptrarray_pop(ptrarray_t_ *arr);
COLLECTIONS_API void *       ptrarray_top(ptrarray_t_ *arr);
COLLECTIONS_API int          ptrarray_count(const ptrarray_t_ *arr);
COLLECTIONS_API bool         ptrarray_remove_at(ptrarray_t_ *arr, unsigned int ix);
COLLECTIONS_API bool         ptrarray_remove_item(ptrarray_t_ *arr, void *item);
COLLECTIONS_API void         ptrarray_clear(ptrarray_t_ *arr);
COLLECTIONS_API void         ptrarray_clear_and_destroy_items_(ptrarray_t_ *arr, ptrarray_item_destroy_fn destroy_fn);
COLLECTIONS_API void         ptrarray_lock_capacity(ptrarray_t_ *arr);
COLLECTIONS_API int          ptrarray_get_index(ptrarray_t_ *arr, void *ptr);
COLLECTIONS_API bool         ptrarray_contains(ptrarray_t_ *arr, void *ptr);
COLLECTIONS_API void *       ptrarray_get_addr(ptrarray_t_ *arr, unsigned int ix);
COLLECTIONS_API void*        ptrarray_data(ptrarray_t_ *arr); // might become invalidated by remove/add operations
COLLECTIONS_API void         ptrarray_reverse(ptrarray_t_ *arr);

//-----------------------------------------------------------------------------
// String buffer
//-----------------------------------------------------------------------------

typedef struct strbuf strbuf_t;

COLLECTIONS_API strbuf_t* strbuf_make(allocator_t *alloc);
COLLECTIONS_API strbuf_t* strbuf_make_with_capacity(allocator_t *alloc, unsigned int capacity);
COLLECTIONS_API void strbuf_destroy(strbuf_t *buf);
COLLECTIONS_API void strbuf_clear(strbuf_t *buf);
COLLECTIONS_API bool strbuf_append(strbuf_t *buf, const char *str);
COLLECTIONS_API bool strbuf_appendf(strbuf_t *buf, const char *fmt, ...)  __attribute__((format(printf, 2, 3)));
COLLECTIONS_API const char * strbuf_get_string(const strbuf_t *buf);
COLLECTIONS_API size_t strbuf_get_length(const strbuf_t *buf);
COLLECTIONS_API char * strbuf_get_string_and_destroy(strbuf_t *buf);
COLLECTIONS_API bool strbuf_failed(strbuf_t *buf);

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------

COLLECTIONS_API ptrarray(char)* kg_split_string(allocator_t *alloc, const char *str, const char *delimiter);
COLLECTIONS_API char* kg_join(allocator_t *alloc, ptrarray(char) *items, const char *with);
COLLECTIONS_API char* kg_canonicalise_path(allocator_t *alloc, const char *path);
COLLECTIONS_API bool  kg_is_path_absolute(const char *path);
COLLECTIONS_API bool  kg_streq(const char *a, const char *b);

#endif /* collections_h */
//FILE_END
//FILE_START:errors.h
#ifndef errors_h
#define errors_h

#ifndef APE_AMALGAMATED
#include "common.h"
#include "token.h"
#endif

#define ERRORS_MAX_COUNT 16
#define ERROR_MESSAGE_MAX_LENGTH 255

typedef struct traceback traceback_t;

typedef enum error_type {
    ERROR_NONE = 0,
    ERROR_PARSING,
    ERROR_COMPILATION,
    ERROR_RUNTIME,
    ERROR_TIMEOUT,
    ERROR_ALLOCATION,
    ERROR_USER,
} error_type_t;

typedef struct error {
    error_type_t type;
    char message[ERROR_MESSAGE_MAX_LENGTH];
    src_pos_t pos;
    traceback_t *traceback;
} error_t;

typedef struct errors {
    error_t errors[ERRORS_MAX_COUNT];
    int count;
} errors_t;

APE_INTERNAL void errors_init(errors_t *errors);
APE_INTERNAL void errors_deinit(errors_t *errors);

APE_INTERNAL void errors_add_error(errors_t *errors, error_type_t type, src_pos_t pos, const char *message);
APE_INTERNAL void errors_add_errorf(errors_t *errors, error_type_t type, src_pos_t pos, const char *format, ...) __attribute__ ((format (printf, 4, 5)));
APE_INTERNAL void errors_clear(errors_t *errors);
APE_INTERNAL int errors_get_count(const errors_t *errors);
APE_INTERNAL error_t *errors_get(errors_t *errors, int ix);
APE_INTERNAL const error_t *errors_getc(const errors_t *errors, int ix);
APE_INTERNAL const char *error_type_to_string(error_type_t type);
APE_INTERNAL error_t *errors_get_last_error(errors_t *errors);
APE_INTERNAL bool errors_has_errors(const errors_t *errors);

#endif /* errors_h */
//FILE_END
//FILE_START:token.h
#ifndef token_h
#define token_h

#ifndef APE_AMALGAMATED
#include "common.h"
#endif

typedef enum {
    TOKEN_INVALID = 0,
    TOKEN_EOF,

    // Operators
    TOKEN_ASSIGN,

    TOKEN_PLUS_ASSIGN,
    TOKEN_MINUS_ASSIGN,
    TOKEN_ASTERISK_ASSIGN,
    TOKEN_SLASH_ASSIGN,
    TOKEN_PERCENT_ASSIGN,
    TOKEN_BIT_AND_ASSIGN,
    TOKEN_BIT_OR_ASSIGN,
    TOKEN_BIT_XOR_ASSIGN,
    TOKEN_LSHIFT_ASSIGN,
    TOKEN_RSHIFT_ASSIGN,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_BANG,
    TOKEN_ASTERISK,
    TOKEN_SLASH,

    TOKEN_LT,
    TOKEN_LTE,
    TOKEN_GT,
    TOKEN_GTE,

    TOKEN_EQ,
    TOKEN_NOT_EQ,

    TOKEN_AND,
    TOKEN_OR,

    TOKEN_BIT_AND,
    TOKEN_BIT_OR,
    TOKEN_BIT_XOR,
    TOKEN_LSHIFT,
    TOKEN_RSHIFT,

    // Delimiters
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_DOT,
    TOKEN_PERCENT,

    // Keywords
    TOKEN_FUNCTION,
    TOKEN_CONST,
    TOKEN_VAR,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_RETURN,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_CONTINUE,
    TOKEN_NULL,
    TOKEN_IMPORT,
    TOKEN_RECOVER,

    // Identifiers and literals
    TOKEN_IDENT,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_TEMPLATE_STRING,

    TOKEN_TYPE_MAX
} token_type_t;

typedef struct token {
    token_type_t type;
    const char *literal;
    int len;
    src_pos_t pos;
} token_t;

APE_INTERNAL void token_init(token_t *tok, token_type_t type, const char *literal, int len); // no need to destroy
APE_INTERNAL char *token_duplicate_literal(allocator_t *alloc, const token_t *tok);
APE_INTERNAL const char *token_type_to_string(token_type_t type);

#endif /* token_h */
//FILE_END
//FILE_START:compiled_file.h
#ifndef compiled_file_h
#define compiled_file_h

#ifndef APE_AMALGAMATED
#include "common.h"
#include "collections.h"
#endif

typedef struct compiled_file compiled_file_t;

typedef struct compiled_file {
    allocator_t *alloc;
    char *dir_path;
    char *path;
    ptrarray(char*) *lines;
} compiled_file_t;

APE_INTERNAL compiled_file_t* compiled_file_make(allocator_t *alloc, const char *path);
APE_INTERNAL void compiled_file_destroy(compiled_file_t *file);

#endif /* compiled_file_h */
//FILE_END
//FILE_START:lexer.h
#ifndef lexer_h
#define lexer_h

#include <stdbool.h>
#include <stddef.h>

#ifndef APE_AMALGAMATED
#include "common.h"
#include "token.h"
#include "collections.h"
#include "compiled_file.h"
#endif

typedef struct errors errors_t;

typedef struct lexer {
    allocator_t *alloc;
    errors_t *errors;
    const char *input;
    int input_len;
    int position;
    int next_position;
    char ch;
    int line;
    int column;
    compiled_file_t *file;
    bool failed;
    bool continue_template_string;
    struct {
        int position;
        int next_position;
        char ch;
        int line;
        int column;
    } prev_token_state;
    token_t prev_token;
    token_t cur_token;
    token_t peek_token;
} lexer_t;

APE_INTERNAL bool lexer_init(lexer_t *lex, allocator_t *alloc, errors_t *errs, const char *input, compiled_file_t *file); // no need to deinit

APE_INTERNAL bool lexer_failed(lexer_t *lex);
APE_INTERNAL void lexer_continue_template_string(lexer_t *lex);
APE_INTERNAL bool lexer_cur_token_is(lexer_t *lex, token_type_t type);
APE_INTERNAL bool lexer_peek_token_is(lexer_t *lex, token_type_t type);
APE_INTERNAL bool lexer_next_token(lexer_t *lex);
APE_INTERNAL bool lexer_previous_token(lexer_t *lex);
APE_INTERNAL token_t lexer_next_token_internal(lexer_t *lex); // exposed here for tests
APE_INTERNAL bool lexer_expect_current(lexer_t *lex, token_type_t type);

#endif /* lexer_h */
//FILE_END
//FILE_START:ast.h
#ifndef ast_h
#define ast_h

#ifndef APE_AMALGAMATED
#include "common.h"
#include "collections.h"
#include "token.h"
#endif

typedef struct code_block {
    allocator_t *alloc;
    ptrarray(statement_t) *statements;
} code_block_t;

typedef struct expression expression_t;
typedef struct statement statement_t;

typedef struct map_literal {
    ptrarray(expression_t) *keys;
    ptrarray(expression_t) *values;
} map_literal_t;

typedef enum {
    OPERATOR_NONE,
    OPERATOR_ASSIGN,
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_BANG,
    OPERATOR_ASTERISK,
    OPERATOR_SLASH,
    OPERATOR_LT,
    OPERATOR_LTE,
    OPERATOR_GT,
    OPERATOR_GTE,
    OPERATOR_EQ,
    OPERATOR_NOT_EQ,
    OPERATOR_MODULUS,
    OPERATOR_LOGICAL_AND,
    OPERATOR_LOGICAL_OR,
    OPERATOR_BIT_AND,
    OPERATOR_BIT_OR,
    OPERATOR_BIT_XOR,
    OPERATOR_LSHIFT,
    OPERATOR_RSHIFT,
} operator_t;

typedef struct prefix {
    operator_t op;
    expression_t *right;
} prefix_expression_t;

typedef struct infix {
    operator_t op;
    expression_t *left;
    expression_t *right;
} infix_expression_t;

typedef struct if_case {
    allocator_t *alloc;
    expression_t *test;
    code_block_t *consequence;
} if_case_t;

typedef struct fn_literal {
    char *name;
    ptrarray(ident_t) *params;
    code_block_t *body;
} fn_literal_t;

typedef struct call_expression {
    expression_t *function;
    ptrarray(expression_t) *args;
} call_expression_t;

typedef struct index_expression {
    expression_t *left;
    expression_t *index;
} index_expression_t;

typedef struct assign_expression {
    expression_t *dest;
    expression_t *source;
} assign_expression_t;

typedef struct logical_expression {
    operator_t op;
    expression_t *left;
    expression_t *right;
} logical_expression_t;

typedef enum expression_type {
    EXPRESSION_NONE,
    EXPRESSION_IDENT,
    EXPRESSION_NUMBER_LITERAL,
    EXPRESSION_BOOL_LITERAL,
    EXPRESSION_STRING_LITERAL,
    EXPRESSION_NULL_LITERAL,
    EXPRESSION_ARRAY_LITERAL,
    EXPRESSION_MAP_LITERAL,
    EXPRESSION_PREFIX,
    EXPRESSION_INFIX,
    EXPRESSION_FUNCTION_LITERAL,
    EXPRESSION_CALL,
    EXPRESSION_INDEX,
    EXPRESSION_ASSIGN,
    EXPRESSION_LOGICAL,
} expression_type_t;

typedef struct ident {
    allocator_t *alloc;
    char *value;
    src_pos_t pos;
} ident_t;

typedef struct expression {
    allocator_t *alloc;
    expression_type_t type;
    union {
        ident_t *ident;
        double number_literal;
        bool bool_literal;
        char *string_literal;
        ptrarray(expression_t) *array;
        map_literal_t map;
        prefix_expression_t prefix;
        infix_expression_t infix;
        fn_literal_t fn_literal;
        call_expression_t call_expr;
        index_expression_t index_expr;
        assign_expression_t assign;
        logical_expression_t logical;
    };
    src_pos_t pos;
} expression_t;

typedef enum statement_type {
    STATEMENT_NONE,
    STATEMENT_DEFINE,
    STATEMENT_IF,
    STATEMENT_RETURN_VALUE,
    STATEMENT_EXPRESSION,
    STATEMENT_WHILE_LOOP,
    STATEMENT_BREAK,
    STATEMENT_CONTINUE,
    STATEMENT_FOREACH,
    STATEMENT_FOR_LOOP,
    STATEMENT_BLOCK,
    STATEMENT_IMPORT,
    STATEMENT_RECOVER,
} statement_type_t;

typedef struct define_statement {
    ident_t *name;
    expression_t *value;
    bool assignable;
} define_statement_t;

typedef struct if_statement {
    ptrarray(if_case_t) *cases;
    code_block_t *alternative;
} if_statement_t;

typedef struct while_loop_statement {
    expression_t *test;
    code_block_t *body;
} while_loop_statement_t;

typedef struct foreach_statement {
    ident_t *iterator;
    expression_t *source;
    code_block_t *body;
} foreach_statement_t;

typedef struct for_loop_statement {
    statement_t *init;
    expression_t *test;
    expression_t *update;
    code_block_t *body;
} for_loop_statement_t;

typedef struct import_statement {
    char *path;
} import_statement_t;

typedef struct recover_statement {
    ident_t *error_ident;
    code_block_t *body;
} recover_statement_t;

typedef struct statement {
    allocator_t *alloc;
    statement_type_t type;
    union {
        define_statement_t define;
        if_statement_t if_statement;
        expression_t *return_value;
        expression_t *expression;
        while_loop_statement_t while_loop;
        foreach_statement_t foreach;
        for_loop_statement_t for_loop;
        code_block_t *block;
        import_statement_t import;
        recover_statement_t recover;
    };
    src_pos_t pos;
} statement_t;

APE_INTERNAL char* statements_to_string(allocator_t *alloc, ptrarray(statement_t) *statements);

APE_INTERNAL statement_t* statement_make_define(allocator_t *alloc, ident_t *name, expression_t *value, bool assignable);
APE_INTERNAL statement_t* statement_make_if(allocator_t *alloc, ptrarray(if_case_t) *cases, code_block_t *alternative);
APE_INTERNAL statement_t* statement_make_return(allocator_t *alloc, expression_t *value);
APE_INTERNAL statement_t* statement_make_expression(allocator_t *alloc, expression_t *value);
APE_INTERNAL statement_t* statement_make_while_loop(allocator_t *alloc, expression_t *test, code_block_t *body);
APE_INTERNAL statement_t* statement_make_break(allocator_t *alloc);
APE_INTERNAL statement_t* statement_make_foreach(allocator_t *alloc, ident_t *iterator, expression_t *source, code_block_t *body);
APE_INTERNAL statement_t* statement_make_for_loop(allocator_t *alloc, statement_t *init, expression_t *test, expression_t *update, code_block_t *body);
APE_INTERNAL statement_t* statement_make_continue(allocator_t *alloc);
APE_INTERNAL statement_t* statement_make_block(allocator_t *alloc, code_block_t *block);
APE_INTERNAL statement_t* statement_make_import(allocator_t *alloc, char *path);
APE_INTERNAL statement_t* statement_make_recover(allocator_t *alloc, ident_t *error_ident, code_block_t *body);

APE_INTERNAL void statement_destroy(statement_t *stmt);

APE_INTERNAL statement_t* statement_copy(const statement_t *stmt);

APE_INTERNAL code_block_t* code_block_make(allocator_t *alloc, ptrarray(statement_t) *statements);
APE_INTERNAL void code_block_destroy(code_block_t *stmt);
APE_INTERNAL code_block_t* code_block_copy(code_block_t *block);

APE_INTERNAL expression_t* expression_make_ident(allocator_t *alloc, ident_t *ident);
APE_INTERNAL expression_t* expression_make_number_literal(allocator_t *alloc, double val);
APE_INTERNAL expression_t* expression_make_bool_literal(allocator_t *alloc, bool val);
APE_INTERNAL expression_t* expression_make_string_literal(allocator_t *alloc, char *value);
APE_INTERNAL expression_t* expression_make_null_literal(allocator_t *alloc);
APE_INTERNAL expression_t* expression_make_array_literal(allocator_t *alloc, ptrarray(expression_t) *values);
APE_INTERNAL expression_t* expression_make_map_literal(allocator_t *alloc, ptrarray(expression_t) *keys, ptrarray(expression_t) *values);
APE_INTERNAL expression_t* expression_make_prefix(allocator_t *alloc, operator_t op, expression_t *right);
APE_INTERNAL expression_t* expression_make_infix(allocator_t *alloc, operator_t op, expression_t *left, expression_t *right);
APE_INTERNAL expression_t* expression_make_fn_literal(allocator_t *alloc, ptrarray(ident_t) *params, code_block_t *body);
APE_INTERNAL expression_t* expression_make_call(allocator_t *alloc, expression_t *function, ptrarray(expression_t) *args);
APE_INTERNAL expression_t* expression_make_index(allocator_t *alloc, expression_t *left, expression_t *index);
APE_INTERNAL expression_t* expression_make_assign(allocator_t *alloc, expression_t *dest, expression_t *source);
APE_INTERNAL expression_t* expression_make_logical(allocator_t *alloc, operator_t op, expression_t *left, expression_t *right);

APE_INTERNAL void expression_destroy(expression_t *expr);

APE_INTERNAL expression_t* expression_copy(expression_t *expr);

APE_INTERNAL void statement_to_string(const statement_t *stmt, strbuf_t *buf);
APE_INTERNAL void expression_to_string(expression_t *expr, strbuf_t *buf);

APE_INTERNAL void code_block_to_string(const code_block_t *stmt, strbuf_t *buf);
APE_INTERNAL const char* operator_to_string(operator_t op);

APE_INTERNAL const char *expression_type_to_string(expression_type_t type);

APE_INTERNAL ident_t* ident_make(allocator_t *alloc, token_t tok);
APE_INTERNAL ident_t* ident_copy(ident_t *ident);
APE_INTERNAL void ident_destroy(ident_t *ident);

APE_INTERNAL if_case_t *if_case_make(allocator_t *alloc, expression_t *test, code_block_t *consequence);
APE_INTERNAL void if_case_destroy(if_case_t *cond);
APE_INTERNAL if_case_t* if_case_copy(if_case_t *cond);

#endif /* ast_h */
//FILE_END
//FILE_START:parser.h
#ifndef parser_h
#define parser_h

#ifndef APE_AMALGAMATED
#include "common.h"
#include "lexer.h"
#include "token.h"
#include "ast.h"
#include "collections.h"
#endif

typedef struct parser parser_t;
typedef struct errors errors_t;

typedef expression_t* (*prefix_parse_fn)(parser_t *p);
typedef expression_t* (*infix_parse_fn)(parser_t *p, expression_t *expr);

typedef struct parser {
    allocator_t *alloc;
    const ape_config_t *config;
    lexer_t lexer;
    errors_t *errors;
    
    prefix_parse_fn prefix_parse_fns[TOKEN_TYPE_MAX];
    infix_parse_fn infix_parse_fns[TOKEN_TYPE_MAX];

    int depth;
} parser_t;

APE_INTERNAL parser_t* parser_make(allocator_t *alloc, const ape_config_t *config, errors_t *errors);
APE_INTERNAL void parser_destroy(parser_t *parser);

APE_INTERNAL ptrarray(statement_t)* parser_parse_all(parser_t *parser,  const char *input, compiled_file_t *file);

#endif /* parser_h */
//FILE_END
//FILE_START:object.h
#ifndef object_h
#define object_h

#include <stdint.h>

#ifndef APE_AMALGAMATED
#include "common.h"
#include "collections.h"
#include "ast.h"
#endif

typedef struct compilation_result compilation_result_t;
typedef struct traceback traceback_t;
typedef struct vm vm_t;
typedef struct gcmem gcmem_t;

#define OBJECT_STRING_BUF_SIZE 32

typedef enum {
    OBJECT_NONE      = 0,
    OBJECT_ERROR     = 1 << 0,
    OBJECT_NUMBER    = 1 << 1,
    OBJECT_BOOL      = 1 << 2,
    OBJECT_STRING    = 1 << 3,
    OBJECT_NULL      = 1 << 4,
    OBJECT_NATIVE_FUNCTION = 1 << 5,
    OBJECT_ARRAY     = 1 << 6,
    OBJECT_MAP       = 1 << 7,
    OBJECT_FUNCTION  = 1 << 8,
    OBJECT_EXTERNAL  = 1 << 9,
    OBJECT_FREED     = 1 << 10,
    OBJECT_ANY       = 0xffff,
} object_type_t;

typedef struct object {
    union {
        uint64_t handle;
        double number;
    };
} object_t;

typedef struct function {
    union {
        object_t *free_vals_allocated;
        object_t free_vals_buf[2];
    };
    union {
        char *name;
        const char *const_name;
    };
    compilation_result_t *comp_result;
    int num_locals;
    int num_args;
    int free_vals_count;
    bool owns_data;
} function_t;

#define NATIVE_FN_MAX_DATA_LEN 24
typedef object_t (*native_fn)(vm_t *vm, void *data, int argc, object_t *args);

typedef struct native_function {
    char *name;
    native_fn fn;
    uint8_t data[NATIVE_FN_MAX_DATA_LEN];
    int data_len;
} native_function_t;

typedef void  (*external_data_destroy_fn)(void* data);
typedef void* (*external_data_copy_fn)(void* data);

typedef struct external_data {
    void *data;
    external_data_destroy_fn data_destroy_fn;
    external_data_copy_fn    data_copy_fn;
} external_data_t;

typedef struct object_error {
    char *message;
    traceback_t *traceback;
} object_error_t;

typedef struct object_string {
    union {
        char *value_allocated;
        char value_buf[OBJECT_STRING_BUF_SIZE];
    };
    unsigned long hash;
    bool is_allocated;
} object_string_t;

typedef struct object_data {
    gcmem_t *mem;
    union {
        object_string_t string;
        object_error_t error;
        array(object_t) *array;
        valdict(object_t, object_t) *map;
        function_t function;
        native_function_t native_function;
        external_data_t external;
    };
    bool gcmark;
    object_type_t type;
} object_data_t;

APE_INTERNAL object_t object_make_from_data(object_type_t type, object_data_t *data);
APE_INTERNAL object_t object_make_number(double val);
APE_INTERNAL object_t object_make_bool(bool val);
APE_INTERNAL object_t object_make_null(void);
APE_INTERNAL object_t object_make_string(gcmem_t *mem, const char *string);
APE_INTERNAL object_t object_make_string_no_copy(gcmem_t *mem, char *string);
APE_INTERNAL object_t object_make_stringf(gcmem_t *mem, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
APE_INTERNAL object_t object_make_native_function(gcmem_t *mem, const char *name, native_fn fn, void *data, int data_len);
APE_INTERNAL object_t object_make_array(gcmem_t *mem);
APE_INTERNAL object_t object_make_array_with_capacity(gcmem_t *mem, unsigned capacity);
APE_INTERNAL object_t object_make_map(gcmem_t *mem);
APE_INTERNAL object_t object_make_map_with_capacity(gcmem_t *mem, unsigned capacity);
APE_INTERNAL object_t object_make_error(gcmem_t *mem, const char *message);
APE_INTERNAL object_t object_make_error_no_copy(gcmem_t *mem, char *message);
APE_INTERNAL object_t object_make_errorf(gcmem_t *mem, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
APE_INTERNAL object_t object_make_function(gcmem_t *mem, const char *name, compilation_result_t *comp_res,
                                           bool owns_data, int num_locals, int num_args,
                                           int free_vals_count);
APE_INTERNAL object_t object_make_external(gcmem_t *mem, void *data);

APE_INTERNAL void object_deinit(object_t obj);
APE_INTERNAL void object_data_deinit(object_data_t *obj);

APE_INTERNAL bool        object_is_allocated(object_t obj);
APE_INTERNAL gcmem_t*    object_get_mem(object_t obj);
APE_INTERNAL bool        object_is_hashable(object_t obj);
APE_INTERNAL void        object_to_string(object_t obj, strbuf_t *buf, bool quote_str);
APE_INTERNAL const char* object_get_type_name(const object_type_t type);
APE_INTERNAL char*       object_get_type_union_name(allocator_t *alloc, const object_type_t type);
APE_INTERNAL char*       object_serialize(allocator_t *alloc, object_t object);
APE_INTERNAL object_t    object_deep_copy(gcmem_t *mem, object_t object);
APE_INTERNAL object_t    object_copy(gcmem_t *mem, object_t obj);
APE_INTERNAL double      object_compare(object_t a, object_t b);
APE_INTERNAL bool        object_equals(object_t a, object_t b);

APE_INTERNAL object_data_t* object_get_allocated_data(object_t object);

APE_INTERNAL bool           object_get_bool(object_t obj);
APE_INTERNAL double         object_get_number(object_t obj);
APE_INTERNAL function_t*    object_get_function(object_t obj);
APE_INTERNAL const char*    object_get_string(object_t obj);
APE_INTERNAL native_function_t* object_get_native_function(object_t obj);
APE_INTERNAL object_type_t  object_get_type(object_t obj);

APE_INTERNAL bool object_is_numeric(object_t obj);
APE_INTERNAL bool object_is_null(object_t obj);
APE_INTERNAL bool object_is_callable(object_t obj);

APE_INTERNAL const char* object_get_function_name(object_t obj);
APE_INTERNAL object_t    object_get_function_free_val(object_t obj, int ix);
APE_INTERNAL void        object_set_function_free_val(object_t obj, int ix, object_t val);
APE_INTERNAL object_t*   object_get_function_free_vals(object_t obj);

APE_INTERNAL const char*  object_get_error_message(object_t obj);
APE_INTERNAL void         object_set_error_traceback(object_t obj, traceback_t *traceback);
APE_INTERNAL traceback_t* object_get_error_traceback(object_t obj);

APE_INTERNAL external_data_t* object_get_external_data(object_t object);
APE_INTERNAL bool object_set_external_destroy_function(object_t object, external_data_destroy_fn destroy_fn);
APE_INTERNAL bool object_set_external_data(object_t object, void *data);
APE_INTERNAL bool object_set_external_copy_function(object_t object, external_data_copy_fn copy_fn);

APE_INTERNAL object_t object_get_array_value_at(object_t array, int ix);
APE_INTERNAL bool     object_set_array_value_at(object_t obj, int ix, object_t val);
APE_INTERNAL bool     object_add_array_value(object_t array, object_t val);
APE_INTERNAL int      object_get_array_length(object_t array);
APE_INTERNAL bool     object_remove_array_value_at(object_t array, int ix);

APE_INTERNAL int      object_get_map_length(object_t obj);
APE_INTERNAL object_t object_get_map_key_at(object_t obj, int ix);
APE_INTERNAL object_t object_get_map_value_at(object_t obj, int ix);
APE_INTERNAL bool     object_set_map_value_at(object_t obj, int ix, object_t val);
APE_INTERNAL object_t object_get_kv_pair_at(gcmem_t *mem, object_t obj, int ix);
APE_INTERNAL bool     object_set_map_value(object_t obj, object_t key, object_t val);
APE_INTERNAL object_t object_get_map_value(object_t obj, object_t key);
APE_INTERNAL bool     object_map_has_key(object_t obj, object_t key);

#endif /* object_h */
//FILE_END
//FILE_START:global_store.h
#ifndef global_store_h
#define global_store_h

#ifndef APE_AMALGAMATED
#include "common.h"
#include "collections.h"
#include "object.h"
#endif

typedef struct symbol symbol_t;
typedef struct gcmem gcmem_t;
typedef struct global_store global_store_t;

APE_INTERNAL global_store_t *global_store_make(allocator_t *alloc, gcmem_t *mem);
APE_INTERNAL void global_store_destroy(global_store_t *store);
APE_INTERNAL const symbol_t* global_store_get_symbol(global_store_t *store, const char *name);
APE_INTERNAL object_t global_store_get_object(global_store_t *store, const char *name);
APE_INTERNAL bool global_store_set(global_store_t *store, const char *name, object_t object);
APE_INTERNAL object_t global_store_get_object_at(global_store_t *store, int ix, bool *out_ok);
APE_INTERNAL bool global_store_set_object_at(global_store_t *store, int ix, object_t object);
APE_INTERNAL object_t *global_store_get_object_data(global_store_t *store);
APE_INTERNAL int global_store_get_object_count(global_store_t *store);

#endif /* global_store_h */
//FILE_END
//FILE_START:symbol_table.h
#ifndef symbol_table_h
#define symbol_table_h

#ifndef APE_AMALGAMATED
#include "common.h"
#include "token.h"
#include "collections.h"
#endif

typedef struct global_store global_store_t;

typedef enum symbol_type {
    SYMBOL_NONE = 0,
    SYMBOL_MODULE_GLOBAL,
    SYMBOL_LOCAL,
    SYMBOL_APE_GLOBAL,
    SYMBOL_FREE,
    SYMBOL_FUNCTION,
    SYMBOL_THIS,
} symbol_type_t;

typedef struct symbol {
    allocator_t *alloc;
    symbol_type_t type;
    char *name;
    int index;
    bool assignable;
} symbol_t;

typedef struct block_scope {
    allocator_t *alloc;
    dict(symbol_t) *store;
    int offset;
    int num_definitions;
} block_scope_t;

typedef struct symbol_table {
    allocator_t *alloc;
    struct symbol_table *outer;
    global_store_t *global_store;
    ptrarray(block_scope_t) *block_scopes;
    ptrarray(symbol_t) *free_symbols;
    ptrarray(symbol_t) *module_global_symbols;
    int max_num_definitions;
    int module_global_offset;
} symbol_table_t;

APE_INTERNAL symbol_t *symbol_make(allocator_t *alloc, const char *name, symbol_type_t type, int index, bool assignable);
APE_INTERNAL void symbol_destroy(symbol_t *symbol);
APE_INTERNAL symbol_t* symbol_copy(symbol_t *symbol);

APE_INTERNAL symbol_table_t *symbol_table_make(allocator_t *alloc, symbol_table_t *outer, global_store_t *global_store, int module_global_offset);
APE_INTERNAL void symbol_table_destroy(symbol_table_t *st);
APE_INTERNAL symbol_table_t* symbol_table_copy(symbol_table_t *st);
APE_INTERNAL bool symbol_table_add_module_symbol(symbol_table_t *st, symbol_t *symbol);
APE_INTERNAL const symbol_t *symbol_table_define(symbol_table_t *st, const char *name, bool assignable);
APE_INTERNAL const symbol_t *symbol_table_define_free(symbol_table_t *st, const symbol_t *original);
APE_INTERNAL const symbol_t *symbol_table_define_function_name(symbol_table_t *st, const char *name, bool assignable);
APE_INTERNAL const symbol_t *symbol_table_define_this(symbol_table_t *st);

APE_INTERNAL const symbol_t *symbol_table_resolve(symbol_table_t *st, const char *name);

APE_INTERNAL bool symbol_table_symbol_is_defined(symbol_table_t *st, const char *name);
APE_INTERNAL bool symbol_table_push_block_scope(symbol_table_t *table);
APE_INTERNAL void symbol_table_pop_block_scope(symbol_table_t *table);
APE_INTERNAL block_scope_t* symbol_table_get_block_scope(symbol_table_t *table);

APE_INTERNAL bool symbol_table_is_module_global_scope(symbol_table_t *table);
APE_INTERNAL bool symbol_table_is_top_block_scope(symbol_table_t *table);
APE_INTERNAL bool symbol_table_is_top_global_scope(symbol_table_t *table);

APE_INTERNAL int symbol_table_get_module_global_symbol_count(const symbol_table_t *table);
APE_INTERNAL const symbol_t * symbol_table_get_module_global_symbol_at(const symbol_table_t *table, int ix);


#endif /* symbol_table_h */
//FILE_END
//FILE_START:code.h
#ifndef code_h
#define code_h

#include <stdint.h>

#ifndef APE_AMALGAMATED
#include "common.h"
#include "collections.h"
#endif

typedef uint8_t opcode_t;

typedef enum opcode_val {
    OPCODE_NONE = 0,
    OPCODE_CONSTANT,
    OPCODE_ADD,
    OPCODE_POP,
    OPCODE_SUB,
    OPCODE_MUL,
    OPCODE_DIV,
    OPCODE_MOD,
    OPCODE_TRUE,
    OPCODE_FALSE,
    OPCODE_COMPARE,
    OPCODE_EQUAL,
    OPCODE_NOT_EQUAL,
    OPCODE_GREATER_THAN,
    OPCODE_GREATER_THAN_EQUAL,
    OPCODE_MINUS,
    OPCODE_BANG,
    OPCODE_JUMP,
    OPCODE_JUMP_IF_FALSE,
    OPCODE_JUMP_IF_TRUE,
    OPCODE_NULL,
    OPCODE_GET_MODULE_GLOBAL,
    OPCODE_SET_MODULE_GLOBAL,
    OPCODE_DEFINE_MODULE_GLOBAL,
    OPCODE_ARRAY,
    OPCODE_MAP_START,
    OPCODE_MAP_END,
    OPCODE_GET_THIS,
    OPCODE_GET_INDEX,
    OPCODE_SET_INDEX,
    OPCODE_GET_VALUE_AT,
    OPCODE_CALL,
    OPCODE_RETURN_VALUE,
    OPCODE_RETURN,
    OPCODE_GET_LOCAL,
    OPCODE_DEFINE_LOCAL,
    OPCODE_SET_LOCAL,
    OPCODE_GET_APE_GLOBAL,
    OPCODE_FUNCTION,
    OPCODE_GET_FREE,
    OPCODE_SET_FREE,
    OPCODE_CURRENT_FUNCTION,
    OPCODE_DUP,
    OPCODE_NUMBER,
    OPCODE_LEN,
    OPCODE_SET_RECOVER,
    OPCODE_OR,
    OPCODE_XOR,
    OPCODE_AND,
    OPCODE_LSHIFT,
    OPCODE_RSHIFT,
    OPCODE_MAX,
} opcode_val_t;

typedef struct opcode_definition {
    const char *name;
    int num_operands;
    int operand_widths[2];
} opcode_definition_t;

APE_INTERNAL opcode_definition_t* opcode_lookup(opcode_t op);
APE_INTERNAL const char *opcode_get_name(opcode_t op);
APE_INTERNAL int code_make(opcode_t op, int operands_count, uint64_t *operands, array(uint8_t) *res);
APE_INTERNAL void code_to_string(uint8_t *code, src_pos_t *source_positions, size_t code_size, strbuf_t *res);
APE_INTERNAL bool code_read_operands(opcode_definition_t *def, uint8_t *instr, uint64_t out_operands[2]);

#endif /* code_h */
//FILE_END
//FILE_START:compilation_scope.h
#ifndef compilation_scope_h
#define compilation_scope_h

#ifndef APE_AMALGAMATED
#include "symbol_table.h"
#include "code.h"
#include "gc.h"
#endif

typedef struct compilation_result {
    allocator_t *alloc;
    uint8_t *bytecode;
    src_pos_t *src_positions;
    int count;
} compilation_result_t;

typedef struct compilation_scope {
    allocator_t *alloc;
    struct compilation_scope *outer;
    array(uint8_t) *bytecode;
    array(src_pos_t) *src_positions;
    array(int) *break_ip_stack;
    array(int) *continue_ip_stack;
    opcode_t last_opcode;
} compilation_scope_t;

APE_INTERNAL compilation_scope_t* compilation_scope_make(allocator_t *alloc, compilation_scope_t *outer);
APE_INTERNAL void compilation_scope_destroy(compilation_scope_t *scope);
APE_INTERNAL compilation_result_t *compilation_scope_orphan_result(compilation_scope_t *scope);

APE_INTERNAL compilation_result_t* compilation_result_make(allocator_t *alloc, uint8_t *bytecode, src_pos_t *src_positions, int count);
APE_INTERNAL void compilation_result_destroy(compilation_result_t* res);

#endif /* compilation_scope_h */
//FILE_END
//FILE_START:optimisation.h
#ifndef optimisation_h
#define optimisation_h


#ifndef APE_AMALGAMATED
#include "common.h"
#include "parser.h"
#endif

APE_INTERNAL expression_t* optimise_expression(expression_t* expr);

#endif /* optimisation_h */
//FILE_END
//FILE_START:compiler.h
#ifndef compiler_h
#define compiler_h

#ifndef APE_AMALGAMATED
#include "collections.h"
#include "common.h"
#include "errors.h"
#include "parser.h"
#include "code.h"
#include "token.h"
#include "compilation_scope.h"
#include "compiled_file.h"
#endif

typedef struct ape_config ape_config_t;
typedef struct gcmem gcmem_t;
typedef struct symbol_table symbol_table_t;

typedef struct compiler compiler_t;
typedef struct compilation_result compilation_result_t;
typedef struct compiled_file compiled_file_t;

APE_INTERNAL compiler_t *compiler_make(allocator_t *alloc,
                                       const ape_config_t *config,
                                       gcmem_t *mem, errors_t *errors,
                                       ptrarray(compiled_file_t) *files,
                                       global_store_t *global_store);
APE_INTERNAL void compiler_destroy(compiler_t *comp);
APE_INTERNAL compilation_result_t* compiler_compile(compiler_t *comp, const char *code);
APE_INTERNAL compilation_result_t* compiler_compile_file(compiler_t *comp, const char *path);
APE_INTERNAL symbol_table_t* compiler_get_symbol_table(compiler_t *comp);
APE_INTERNAL void compiler_set_symbol_table(compiler_t *comp, symbol_table_t *table);
APE_INTERNAL array(object_t)* compiler_get_constants(const compiler_t *comp);

#endif /* compiler_h */
//FILE_END
//FILE_START:gc.h
#ifndef gc_h
#define gc_h

#ifndef APE_AMALGAMATED
#include "common.h"
#include "collections.h"
#include "object.h"
#endif

typedef struct object_data object_data_t;
typedef struct env env_t;

#define GCMEM_POOL_SIZE 2048
#define GCMEM_POOLS_NUM 2
#define GCMEM_SWEEP_INTERVAL 128

typedef struct object_data_pool {
    object_data_t *data[GCMEM_POOL_SIZE];
    int count;
} object_data_pool_t;

typedef struct gcmem {
    allocator_t *alloc;
    int allocations_since_sweep;

    ptrarray(object_data_t) *objects;
    ptrarray(object_data_t) *objects_back;

    array(object_t) *objects_not_gced;

    object_data_pool_t data_only_pool;
    object_data_pool_t pools[GCMEM_POOLS_NUM];
} gcmem_t;

APE_INTERNAL gcmem_t *gcmem_make(allocator_t *alloc);
APE_INTERNAL void gcmem_destroy(gcmem_t *mem);

APE_INTERNAL object_data_t* gcmem_alloc_object_data(gcmem_t *mem, object_type_t type);
APE_INTERNAL object_data_t* gcmem_get_object_data_from_pool(gcmem_t *mem, object_type_t type);

APE_INTERNAL void gc_unmark_all(gcmem_t *mem);
APE_INTERNAL void gc_mark_objects(object_t *objects, int count);
APE_INTERNAL void gc_mark_object(object_t object);
APE_INTERNAL void gc_sweep(gcmem_t *mem);

APE_INTERNAL bool gc_disable_on_object(object_t obj);
APE_INTERNAL void gc_enable_on_object(object_t obj);

APE_INTERNAL int gc_should_sweep(gcmem_t *mem);

#endif /* gc_h */
//FILE_END
//FILE_START:builtins.h
#ifndef builtins_h
#define builtins_h

#include <stdint.h>

#ifndef APE_AMALGAMATED
#include "common.h"
#include "object.h"
#endif

typedef struct vm vm_t;

APE_INTERNAL int builtins_count(void);
APE_INTERNAL native_fn builtins_get_fn(int ix);
APE_INTERNAL const char* builtins_get_name(int ix);

#endif /* builtins_h */
//FILE_END
//FILE_START:traceback.h
#ifndef traceback_h
#define traceback_h

#ifndef APE_AMALGAMATED
#include "common.h"
#include "collections.h"
#endif

typedef struct vm vm_t;

typedef struct traceback_item {
    char *function_name;
    src_pos_t pos;
} traceback_item_t;

typedef struct traceback {
    allocator_t *alloc;
    array(traceback_item_t)* items;
} traceback_t;

APE_INTERNAL traceback_t* traceback_make(allocator_t *alloc);
APE_INTERNAL void traceback_destroy(traceback_t *traceback);
APE_INTERNAL bool traceback_append(traceback_t *traceback, const char *function_name, src_pos_t pos);
APE_INTERNAL bool traceback_append_from_vm(traceback_t *traceback, vm_t *vm);
APE_INTERNAL bool traceback_to_string(const traceback_t *traceback, strbuf_t *buf);
APE_INTERNAL const char* traceback_item_get_line(traceback_item_t *item);
APE_INTERNAL const char* traceback_item_get_filepath(traceback_item_t *item);

#endif /* traceback_h */
//FILE_END
//FILE_START:frame.h
#ifndef frame_h
#define frame_h

#include <stdint.h>

#ifndef APE_AMALGAMATED
#include "common.h"
#include "object.h"
#include "collections.h"
#include "code.h"
#endif

typedef struct {
    object_t function;
    int ip;
    int base_pointer;
    const src_pos_t *src_positions;
    uint8_t *bytecode;
    int src_ip;
    int bytecode_size;
    int recover_ip;
    bool is_recovering;
} frame_t;

APE_INTERNAL bool frame_init(frame_t* frame, object_t function, int base_pointer);

APE_INTERNAL opcode_val_t frame_read_opcode(frame_t* frame);
APE_INTERNAL uint64_t frame_read_uint64(frame_t* frame);
APE_INTERNAL uint16_t frame_read_uint16(frame_t* frame);
APE_INTERNAL uint8_t frame_read_uint8(frame_t* frame);
APE_INTERNAL src_pos_t frame_src_position(const frame_t *frame);

#endif /* frame_h */
//FILE_END
//FILE_START:vm.h
#ifndef vm_h
#define vm_h

#ifndef APE_AMALGAMATED
#include "ape.h"
#include "common.h"
#include "ast.h"
#include "object.h"
#include "frame.h"
#include "errors.h"
#include "global_store.h"
#endif

#define VM_STACK_SIZE 2048
#define VM_MAX_GLOBALS 2048
#define VM_MAX_FRAMES 2048
#define VM_THIS_STACK_SIZE 2048

typedef struct ape_config ape_config_t;
typedef struct compilation_result compilation_result_t;

typedef struct vm {
    allocator_t *alloc;
    const ape_config_t *config;
    gcmem_t *mem;
    errors_t *errors;
    global_store_t *global_store;
    object_t globals[VM_MAX_GLOBALS];
    int globals_count;
    object_t stack[VM_STACK_SIZE];
    int sp;
    object_t this_stack[VM_THIS_STACK_SIZE];
    int this_sp;
    frame_t frames[VM_MAX_FRAMES];
    int frames_count;
    object_t last_popped;
    frame_t *current_frame;
    bool running;
    object_t operator_oveload_keys[OPCODE_MAX];
} vm_t;

APE_INTERNAL vm_t* vm_make(allocator_t *alloc, const ape_config_t *config, gcmem_t *mem, errors_t *errors, global_store_t *global_store); // config can be null (for internal testing purposes)
APE_INTERNAL void  vm_destroy(vm_t *vm);

APE_INTERNAL void vm_reset(vm_t *vm);

APE_INTERNAL bool vm_run(vm_t *vm, compilation_result_t *comp_res, array(object_t) *constants);
APE_INTERNAL object_t vm_call(vm_t *vm, array(object_t) *constants, object_t callee, int argc, object_t *args);
APE_INTERNAL bool vm_execute_function(vm_t *vm, object_t function, array(object_t) *constants);

APE_INTERNAL object_t vm_get_last_popped(vm_t *vm);
APE_INTERNAL bool vm_has_errors(vm_t *vm);

APE_INTERNAL bool vm_set_global(vm_t *vm, int ix, object_t val);
APE_INTERNAL object_t vm_get_global(vm_t *vm, int ix);

#endif /* vm_h */
//FILE_END

//-----------------------------------------------------------------------------
// C files
//-----------------------------------------------------------------------------
//FILE_START:common.c
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef APE_AMALGAMATED
#include "common.h"
#include "collections.h"
#endif

APE_INTERNAL const src_pos_t src_pos_invalid = { NULL, -1, -1 };
APE_INTERNAL const src_pos_t src_pos_zero = { NULL, 0, 0 };

APE_INTERNAL src_pos_t src_pos_make(const compiled_file_t *file, int line, int column) {
    return (src_pos_t) {
        .file = file,
        .line = line,
        .column = column,
    };
}

char *ape_stringf(allocator_t *alloc, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int to_write = vsnprintf(NULL, 0, format, args);
    va_end(args);
    va_start(args, format);
    char *res = (char*)allocator_malloc(alloc, to_write + 1);
    if (!res) {
        return NULL;
    }
    int written = vsprintf(res, format, args);
    (void)written;
    APE_ASSERT(written == to_write);
    va_end(args);
    return res;
}

void ape_log(const char *file, int line, const char *format, ...) {
    char msg[4096];
    int written = snprintf(msg, APE_ARRAY_LEN(msg), "%s:%d: ", file, line);
    (void)written;
    va_list args;
    va_start(args, format);
    int written_msg = vsnprintf(msg + written, APE_ARRAY_LEN(msg) - written, format, args);
    (void)written_msg;
    va_end(args);

    APE_ASSERT(written_msg <= (APE_ARRAY_LEN(msg) - written));

    printf("%s", msg);
}

char* ape_strndup(allocator_t *alloc, const char *string, size_t n) {
    char *output_string = (char*)allocator_malloc(alloc, n + 1);
    if (!output_string) {
        return NULL;
    }
    output_string[n] = '\0';
    memcpy(output_string, string, n);
    return output_string;
}

char* ape_strdup(allocator_t *alloc, const char *string) {
    if (!string) {
        return NULL;
    }
    return ape_strndup(alloc, string, strlen(string));
}

uint64_t ape_double_to_uint64(double val) {
    union {
        uint64_t val_uint64;
        double val_double;
    } temp = {
        .val_double = val
    };
    return temp.val_uint64;
}

double ape_uint64_to_double(uint64_t val) {
    union {
        uint64_t val_uint64;
        double val_double;
    } temp = {
        .val_uint64 = val
    };
    return temp.val_double;
}

bool ape_timer_platform_supported() {
#if defined(APE_POSIX) || defined(APE_EMSCRIPTEN) || defined(APE_WINDOWS)
    return true;
#else
    return false;
#endif
}

ape_timer_t ape_timer_start() {
    ape_timer_t timer;
    memset(&timer, 0, sizeof(ape_timer_t));
#if defined(APE_POSIX)
    // At some point it should be replaced with more accurate per-platform timers
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    timer.start_offset = start_time.tv_sec;
    timer.start_time_ms = start_time.tv_usec / 1000.0;
#elif defined(APE_WINDOWS)
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li); // not sure what to do if it fails
    timer.pc_frequency = (double)(li.QuadPart) / 1000.0;
    QueryPerformanceCounter(&li);
    timer.start_time_ms = li.QuadPart / timer.pc_frequency;
#elif defined(APE_EMSCRIPTEN)
    timer.start_time_ms = emscripten_get_now();
#endif
    return timer;
}

double ape_timer_get_elapsed_ms(const ape_timer_t *timer) {
#if defined(APE_POSIX)
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    int time_s = (int)((int64_t)current_time.tv_sec - timer->start_offset);
    double current_time_ms = (time_s * 1000) + (current_time.tv_usec / 1000.0);
    return current_time_ms - timer->start_time_ms;
#elif defined(APE_WINDOWS)
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    double current_time_ms = li.QuadPart / timer->pc_frequency;
    return current_time_ms - timer->start_time_ms;
#elif defined(APE_EMSCRIPTEN)
    double current_time_ms = emscripten_get_now();
    return current_time_ms - timer->start_time_ms;
#else
    return 0;
#endif
}
//FILE_END
//FILE_START:collections.c
#ifndef COLLECTIONS_AMALGAMATED
#include "collections.h"
#endif

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#ifdef COLLECTIONS_DEBUG
#include <assert.h>
#define COLLECTIONS_ASSERT(x) assert(x)
#else
#define COLLECTIONS_ASSERT(x)
#endif

//-----------------------------------------------------------------------------
// Collections
//-----------------------------------------------------------------------------
static char* collections_strndup(allocator_t *alloc, const char *string, size_t n);
static char* collections_strdup(allocator_t *alloc, const char *string);
static unsigned long collections_hash(const void *ptr, size_t len); /* djb2 */
static unsigned int upper_power_of_two(unsigned int v);

static char* collections_strndup(allocator_t *alloc, const char *string, size_t n) {
    char *output_string = (char*)allocator_malloc(alloc, n + 1);
    if (!output_string) {
        return NULL;
    }
    output_string[n] = '\0';
    memcpy(output_string, string, n);
    return output_string;
}

static char* collections_strdup(allocator_t *alloc, const char *string) {
    return collections_strndup(alloc, string, strlen(string));
}

static unsigned long collections_hash(const void *ptr, size_t len) { /* djb2 */
    const uint8_t *ptr_u8 = (const uint8_t*)ptr;
    unsigned long hash = 5381;
    for (size_t i = 0; i < len; i++) {
        uint8_t val = ptr_u8[i];
        hash = ((hash << 5) + hash) + val;
    }
    return hash;
}

static unsigned int upper_power_of_two(unsigned int v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

//-----------------------------------------------------------------------------
// Allocator
//-----------------------------------------------------------------------------

allocator_t allocator_make(allocator_malloc_fn malloc_fn, allocator_free_fn free_fn, void *ctx) {
    allocator_t alloc;
    alloc.malloc = malloc_fn;
    alloc.free = free_fn;
    alloc.ctx = ctx;
    return alloc;
}

void* allocator_malloc(allocator_t *allocator, size_t size) {
    if (!allocator || !allocator->malloc) {
        return malloc(size);
    }
    return allocator->malloc(allocator->ctx, size);
}

void allocator_free(allocator_t *allocator, void *ptr) {
    if (!allocator || !allocator->free) {
        free(ptr);
        return;
    }
    allocator->free(allocator->ctx, ptr);
}

//-----------------------------------------------------------------------------
// Dictionary
//-----------------------------------------------------------------------------

#define DICT_INVALID_IX UINT_MAX
#define DICT_INITIAL_SIZE 32

typedef struct dict_ {
    allocator_t *alloc;
    unsigned int *cells;
    unsigned long *hashes;
    char **keys;
    void **values;
    unsigned int *cell_ixs;
    unsigned int count;
    unsigned int item_capacity;
    unsigned int cell_capacity;
    dict_item_copy_fn copy_fn;
    dict_item_destroy_fn destroy_fn;
} dict_t_;

// Private declarations
static bool dict_init(dict_t_ *dict, allocator_t *alloc, unsigned int initial_capacity, dict_item_copy_fn copy_fn, dict_item_destroy_fn destroy_fn);
static void dict_deinit(dict_t_ *dict, bool free_keys);
static unsigned int dict_get_cell_ix(const dict_t_ *dict,
                                     const char *key,
                                     unsigned long hash,
                                     bool *out_found);
static unsigned long hash_string(const char *str);
static bool dict_grow_and_rehash(dict_t_ *dict);
static bool dict_set_internal(dict_t_ *dict, const char *ckey, char *mkey, void *value);

// Public
dict_t_* dict_make_(allocator_t *alloc, dict_item_copy_fn copy_fn, dict_item_destroy_fn destroy_fn) {
    dict_t_ *dict = allocator_malloc(alloc, sizeof(dict_t_));
    if (dict == NULL) {
        return NULL;
    }
    bool ok = dict_init(dict, alloc, DICT_INITIAL_SIZE, copy_fn, destroy_fn);
    if (!ok) {
        allocator_free(alloc, dict);
        return NULL;
    }
    return dict;
}

void dict_destroy(dict_t_ *dict) {
    if (!dict) {
        return;
    }
    allocator_t *alloc = dict->alloc;
    dict_deinit(dict, true);
    allocator_free(alloc, dict);
}

void dict_destroy_with_items(dict_t_ *dict) {
    if (!dict) {
        return;
    }

    if (dict->destroy_fn) {
        for (unsigned int i = 0; i < dict->count; i++) {
            dict->destroy_fn(dict->values[i]);
        }
    }

    dict_destroy(dict);
}

dict_t_* dict_copy_with_items(dict_t_ *dict) {
    if (!dict->copy_fn || !dict->destroy_fn) {
        return false;
    }

    dict_t_ *dict_copy = dict_make_(dict->alloc, dict->copy_fn, dict->destroy_fn);
    if (!dict_copy) {
        return NULL;
    }
    for (int i = 0; i < dict_count(dict); i++) {
        const char *key = dict_get_key_at(dict, i);
        void *item = dict_get_value_at(dict, i);
        void *item_copy = dict_copy->copy_fn(item);
        if (item && !item_copy) {
            dict_destroy_with_items(dict_copy);
            return NULL;
        }
        bool ok = dict_set(dict_copy, key, item_copy);
        if (!ok) {
            dict_copy->destroy_fn(item_copy);
            dict_destroy_with_items(dict_copy);
            return NULL;
        }
    }
    return dict_copy;
}

bool dict_set(dict_t_ *dict, const char *key, void *value) {
    return dict_set_internal(dict, key, NULL, value);
}

void* dict_get(const dict_t_ *dict, const char *key) {
    unsigned long hash = hash_string(key);
    bool found = false;
    unsigned long cell_ix = dict_get_cell_ix(dict, key, hash, &found);
    if (found == false) {
        return NULL;
    }
    unsigned int item_ix = dict->cells[cell_ix];
    return dict->values[item_ix];
}

void *dict_get_value_at(const dict_t_ *dict, unsigned int ix) {
    if (ix >= dict->count) {
        return NULL;
    }
    return dict->values[ix];
}

const char *dict_get_key_at(const dict_t_ *dict, unsigned int ix) {
    if (ix >= dict->count) {
        return NULL;
    }
    return dict->keys[ix];
}

int dict_count(const dict_t_ *dict) {
    if (!dict) {
        return 0;
    }
    return dict->count;
}

bool dict_remove(dict_t_ *dict, const char *key) {
    unsigned long hash = hash_string(key);
    bool found = false;
    unsigned int cell = dict_get_cell_ix(dict, key, hash, &found);
    if (!found) {
        return false;
    }

    unsigned int item_ix = dict->cells[cell];
    allocator_free(dict->alloc, dict->keys[item_ix]);
    unsigned int last_item_ix = dict->count - 1;
    if (item_ix < last_item_ix) {
        dict->keys[item_ix] = dict->keys[last_item_ix];
        dict->values[item_ix] = dict->values[last_item_ix];
        dict->cell_ixs[item_ix] = dict->cell_ixs[last_item_ix];
        dict->hashes[item_ix] = dict->hashes[last_item_ix];
        dict->cells[dict->cell_ixs[item_ix]] = item_ix;
    }
    dict->count--;

    unsigned int i = cell;
    unsigned int j = i;
    for (unsigned int x = 0; x < (dict->cell_capacity - 1); x++) {
        j = (j + 1) & (dict->cell_capacity - 1);
        if (dict->cells[j] == DICT_INVALID_IX) {
            break;
        }
        unsigned int k = dict->hashes[dict->cells[j]] & (dict->cell_capacity - 1);
        if ((j > i && (k <= i || k > j))
            || (j < i && (k <= i && k > j))) {
            dict->cell_ixs[dict->cells[j]] = i;
            dict->cells[i] = dict->cells[j];
            i = j;
        }
    }
    dict->cells[i] = DICT_INVALID_IX;
    return true;
}

// Private definitions
static bool dict_init(dict_t_ *dict, allocator_t *alloc, unsigned int initial_capacity, dict_item_copy_fn copy_fn, dict_item_destroy_fn destroy_fn) {
    dict->alloc = alloc;
    dict->cells = NULL;
    dict->keys = NULL;
    dict->values = NULL;
    dict->cell_ixs = NULL;
    dict->hashes = NULL;

    dict->count = 0;
    dict->cell_capacity = initial_capacity;
    dict->item_capacity = (unsigned int)(initial_capacity * 0.7f);
    dict->copy_fn = copy_fn;
    dict->destroy_fn = destroy_fn;

    dict->cells = allocator_malloc(alloc, dict->cell_capacity * sizeof(*dict->cells));
    dict->keys = allocator_malloc(alloc, dict->item_capacity * sizeof(*dict->keys));
    dict->values = allocator_malloc(alloc, dict->item_capacity * sizeof(*dict->values));
    dict->cell_ixs = allocator_malloc(alloc, dict->item_capacity * sizeof(*dict->cell_ixs));
    dict->hashes = allocator_malloc(alloc, dict->item_capacity * sizeof(*dict->hashes));
    if (dict->cells == NULL
     || dict->keys == NULL
     || dict->values == NULL
     || dict->cell_ixs == NULL
     || dict->hashes == NULL) {
        goto error;
    }
    for (unsigned int i = 0; i < dict->cell_capacity; i++) {
        dict->cells[i] = DICT_INVALID_IX;
    }
    return true;
error:
    allocator_free(dict->alloc, dict->cells);
    allocator_free(dict->alloc, dict->keys);
    allocator_free(dict->alloc, dict->values);
    allocator_free(dict->alloc, dict->cell_ixs);
    allocator_free(dict->alloc, dict->hashes);
    return false;
}

static void dict_deinit(dict_t_ *dict, bool free_keys) {
    if (free_keys) {
        for (unsigned int i = 0; i < dict->count; i++) {
            allocator_free(dict->alloc, dict->keys[i]);
        }
    }
    dict->count = 0;
    dict->item_capacity = 0;
    dict->cell_capacity = 0;

    allocator_free(dict->alloc, dict->cells);
    allocator_free(dict->alloc, dict->keys);
    allocator_free(dict->alloc, dict->values);
    allocator_free(dict->alloc, dict->cell_ixs);
    allocator_free(dict->alloc, dict->hashes);

    dict->cells = NULL;
    dict->keys = NULL;
    dict->values = NULL;
    dict->cell_ixs = NULL;
    dict->hashes = NULL;
}

static unsigned int dict_get_cell_ix(const dict_t_ *dict,
                                     const char *key,
                                     unsigned long hash,
                                     bool *out_found)
{
    *out_found = false;
    unsigned int cell_ix = hash & (dict->cell_capacity - 1);
    for (unsigned int i = 0; i < dict->cell_capacity; i++) {
        unsigned int ix = (cell_ix + i) & (dict->cell_capacity - 1);
        unsigned int cell = dict->cells[ix];
        if (cell == DICT_INVALID_IX) {
            return ix;
        }
        unsigned long hash_to_check = dict->hashes[cell];
        if (hash != hash_to_check) {
            continue;
        }
        const char *key_to_check = dict->keys[cell];
        if (strcmp(key, key_to_check) == 0) {
            *out_found = true;
            return ix;
        }
    }
    return DICT_INVALID_IX;
}

static unsigned long hash_string(const char *str) { /* djb2 */
    unsigned long hash = 5381;
    uint8_t c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

static bool dict_grow_and_rehash(dict_t_ *dict) {
    dict_t_ new_dict;
    bool ok = dict_init(&new_dict, dict->alloc, dict->cell_capacity * 2, dict->copy_fn, dict->destroy_fn);
    if (!ok) {
        return false;
    }
    for (unsigned int i = 0; i < dict->count; i++) {
        char *key = dict->keys[i];
        void *value = dict->values[i];
        ok = dict_set_internal(&new_dict, key, key, value);
        if (!ok) {
            dict_deinit(&new_dict, false);
            return false;
        }
    }
    dict_deinit(dict, false);
    *dict = new_dict;
    return true;
}

static bool dict_set_internal(dict_t_ *dict, const char *ckey, char *mkey, void *value) {
    unsigned long hash = hash_string(ckey);
    bool found = false;
    unsigned int cell_ix = dict_get_cell_ix(dict, ckey, hash, &found);
    if (found) {
        unsigned int item_ix = dict->cells[cell_ix];
        dict->values[item_ix] = value;
        return true;
    }
    if (dict->count >= dict->item_capacity) {
        bool ok = dict_grow_and_rehash(dict);
        if (!ok) {
            return false;
        }
        cell_ix = dict_get_cell_ix(dict, ckey, hash, &found);
    }

    if (mkey) {
        dict->keys[dict->count] = mkey;
    } else {
        char *key_copy = collections_strdup(dict->alloc, ckey);
        if (!key_copy) {
            return false;
        }
        dict->keys[dict->count] = key_copy;
    }
    dict->cells[cell_ix] = dict->count;
    dict->values[dict->count] = value;
    dict->cell_ixs[dict->count] = cell_ix;
    dict->hashes[dict->count] = hash;
    dict->count++;
    return true;
}

//-----------------------------------------------------------------------------
// Value dictionary
//-----------------------------------------------------------------------------
#define VALDICT_INVALID_IX UINT_MAX

typedef struct valdict_ {
    allocator_t *alloc;
    size_t key_size;
    size_t val_size;
    unsigned int *cells;
    unsigned long *hashes;
    void *keys;
    void *values;
    unsigned int *cell_ixs;
    unsigned int count;
    unsigned int item_capacity;
    unsigned int cell_capacity;
    collections_hash_fn _hash_key;
    collections_equals_fn _keys_equals;
} valdict_t_;

// Private declarations
static bool valdict_init(valdict_t_ *dict, allocator_t *alloc, size_t key_size, size_t val_size, unsigned int initial_capacity);
static void valdict_deinit(valdict_t_ *dict);
static unsigned int valdict_get_cell_ix(const valdict_t_ *dict,
                                        const void *key,
                                        unsigned long hash,
                                        bool *out_found);
static bool valdict_grow_and_rehash(valdict_t_ *dict);
static bool valdict_set_key_at(valdict_t_ *dict, unsigned int ix, void *key);
static bool valdict_keys_are_equal(const valdict_t_ *dict, const void *a, const void *b);
static unsigned long valdict_hash_key(const valdict_t_ *dict, const void *key);

// Public
valdict_t_* valdict_make_(allocator_t *alloc, size_t key_size, size_t val_size) {
    return valdict_make_with_capacity(alloc, DICT_INITIAL_SIZE, key_size, val_size);
}

valdict_t_* valdict_make_with_capacity(allocator_t *alloc, unsigned int min_capacity, size_t key_size, size_t val_size) {
    unsigned int capacity = upper_power_of_two(min_capacity * 2);

    valdict_t_ *dict = allocator_malloc(alloc, sizeof(valdict_t_));
    if (!dict) {
        return NULL;
    }
    bool ok = valdict_init(dict, alloc, key_size, val_size, capacity);
    if (!ok) {
        allocator_free(alloc, dict);
        return NULL;
    }
    return dict;
}

void valdict_destroy(valdict_t_ *dict) {
    if (!dict) {
        return;
    }
    allocator_t *alloc = dict->alloc;
    valdict_deinit(dict);
    allocator_free(alloc, dict);
}

void  valdict_set_hash_function(valdict_t_ *dict, collections_hash_fn hash_fn) {
    dict->_hash_key = hash_fn;
}

void valdict_set_equals_function(valdict_t_ *dict, collections_equals_fn equals_fn) {
    dict->_keys_equals = equals_fn;
}

bool valdict_set(valdict_t_ *dict, void *key, void *value) {
    unsigned long hash = valdict_hash_key(dict, key);
    bool found = false;
    unsigned int cell_ix = valdict_get_cell_ix(dict, key, hash, &found);
    if (found) {
        unsigned int item_ix = dict->cells[cell_ix];
        valdict_set_value_at(dict, item_ix, value);
        return true;
    }
    if (dict->count >= dict->item_capacity) {
        bool ok = valdict_grow_and_rehash(dict);
        if (!ok) {
            return false;
        }
        cell_ix = valdict_get_cell_ix(dict, key, hash, &found);
    }
    unsigned int last_ix = dict->count;
    dict->count++;
    dict->cells[cell_ix] = last_ix;
    valdict_set_key_at(dict, last_ix, key);
    valdict_set_value_at(dict, last_ix, value);
    dict->cell_ixs[last_ix] = cell_ix;
    dict->hashes[last_ix] = hash;
    return true;
}

void* valdict_get(const valdict_t_ *dict, const void *key) {
    unsigned long hash = valdict_hash_key(dict, key);
    bool found = false;
    unsigned long cell_ix = valdict_get_cell_ix(dict, key, hash, &found);
    if (!found) {
        return NULL;
    }
    unsigned int item_ix = dict->cells[cell_ix];
    return valdict_get_value_at(dict, item_ix);
}

void* valdict_get_key_at(const valdict_t_ *dict, unsigned int ix) {
    if (ix >= dict->count) {
        return NULL;
    }
    return (char*)dict->keys + (dict->key_size * ix);
}

void *valdict_get_value_at(const valdict_t_ *dict, unsigned int ix) {
    if (ix >= dict->count) {
        return NULL;
    }
    return (char*)dict->values + (dict->val_size * ix);
}

unsigned int valdict_get_capacity(const valdict_t_ *dict) {
    return dict->item_capacity;
}

bool valdict_set_value_at(const valdict_t_ *dict, unsigned int ix, const void *value) {
    if (ix >= dict->count) {
        return false;
    }
    size_t offset = ix * dict->val_size;
    memcpy((char*)dict->values + offset, value, dict->val_size);
    return true;
}

int valdict_count(const valdict_t_ *dict) {
    if (!dict) {
        return 0;
    }
    return dict->count;
}

bool valdict_remove(valdict_t_ *dict, void *key) {
    unsigned long hash = valdict_hash_key(dict, key);
    bool found = false;
    unsigned int cell = valdict_get_cell_ix(dict, key, hash, &found);
    if (!found) {
        return false;
    }

    unsigned int item_ix = dict->cells[cell];
    unsigned int last_item_ix = dict->count - 1;
    if (item_ix < last_item_ix) {
        void *last_key = valdict_get_key_at(dict, last_item_ix);
        valdict_set_key_at(dict, item_ix, last_key);
        void *last_value = valdict_get_key_at(dict, last_item_ix);
        valdict_set_value_at(dict, item_ix, last_value);
        dict->cell_ixs[item_ix] = dict->cell_ixs[last_item_ix];
        dict->hashes[item_ix] = dict->hashes[last_item_ix];
        dict->cells[dict->cell_ixs[item_ix]] = item_ix;
    }
    dict->count--;

    unsigned int i = cell;
    unsigned int j = i;
    for (unsigned int x = 0; x < (dict->cell_capacity - 1); x++) {
        j = (j + 1) & (dict->cell_capacity - 1);
        if (dict->cells[j] == VALDICT_INVALID_IX) {
            break;
        }
        unsigned int k = dict->hashes[dict->cells[j]] & (dict->cell_capacity - 1);
        if ((j > i && (k <= i || k > j))
            || (j < i && (k <= i && k > j))) {
            dict->cell_ixs[dict->cells[j]] = i;
            dict->cells[i] = dict->cells[j];
            i = j;
        }
    }
    dict->cells[i] = VALDICT_INVALID_IX;
    return true;
}

void valdict_clear(valdict_t_ *dict) {
    dict->count = 0;
    for (unsigned int i = 0; i < dict->cell_capacity; i++) {
        dict->cells[i] = VALDICT_INVALID_IX;
    }
}

// Private definitions
static bool valdict_init(valdict_t_ *dict, allocator_t *alloc, size_t key_size, size_t val_size, unsigned int initial_capacity) {
    dict->alloc = alloc;
    dict->key_size = key_size;
    dict->val_size = val_size;
    dict->cells = NULL;
    dict->keys = NULL;
    dict->values = NULL;
    dict->cell_ixs = NULL;
    dict->hashes = NULL;

    dict->count = 0;
    dict->cell_capacity = initial_capacity;
    dict->item_capacity = (unsigned int)(initial_capacity * 0.7f);

    dict->_keys_equals = NULL;
    dict->_hash_key = NULL;

    dict->cells = allocator_malloc(dict->alloc, dict->cell_capacity * sizeof(*dict->cells));
    dict->keys = allocator_malloc(dict->alloc, dict->item_capacity * key_size);
    dict->values = allocator_malloc(dict->alloc, dict->item_capacity * val_size);
    dict->cell_ixs = allocator_malloc(dict->alloc, dict->item_capacity * sizeof(*dict->cell_ixs));
    dict->hashes = allocator_malloc(dict->alloc, dict->item_capacity * sizeof(*dict->hashes));
    if (dict->cells == NULL
     || dict->keys == NULL
     || dict->values == NULL
     || dict->cell_ixs == NULL
     || dict->hashes == NULL) {
        goto error;
    }
    for (unsigned int i = 0; i < dict->cell_capacity; i++) {
        dict->cells[i] = VALDICT_INVALID_IX;
    }
    return true;
error:
    allocator_free(dict->alloc, dict->cells);
    allocator_free(dict->alloc, dict->keys);
    allocator_free(dict->alloc, dict->values);
    allocator_free(dict->alloc, dict->cell_ixs);
    allocator_free(dict->alloc, dict->hashes);
    return false;
}

static void valdict_deinit(valdict_t_ *dict) {
    dict->key_size = 0;
    dict->val_size = 0;
    dict->count = 0;
    dict->item_capacity = 0;
    dict->cell_capacity = 0;

    allocator_free(dict->alloc, dict->cells);
    allocator_free(dict->alloc, dict->keys);
    allocator_free(dict->alloc, dict->values);
    allocator_free(dict->alloc, dict->cell_ixs);
    allocator_free(dict->alloc, dict->hashes);

    dict->cells = NULL;
    dict->keys = NULL;
    dict->values = NULL;
    dict->cell_ixs = NULL;
    dict->hashes = NULL;
}

static unsigned int valdict_get_cell_ix(const valdict_t_ *dict,
                                        const void *key,
                                        unsigned long hash,
                                        bool *out_found)
{
    *out_found = false;
    unsigned int cell_ix = hash & (dict->cell_capacity - 1);
    for (unsigned int i = 0; i < dict->cell_capacity; i++) {
        unsigned int ix = (cell_ix + i) & (dict->cell_capacity - 1);
        unsigned int cell = dict->cells[ix];
        if (cell == VALDICT_INVALID_IX) {
            return ix;
        }
        unsigned long hash_to_check = dict->hashes[cell];
        if (hash != hash_to_check) {
            continue;
        }
        void *key_to_check = valdict_get_key_at(dict, cell);
        bool are_equal = valdict_keys_are_equal(dict, key, key_to_check);
        if (are_equal) {
            *out_found = true;
            return ix;
        }
    }
    return VALDICT_INVALID_IX;
}

static bool valdict_grow_and_rehash(valdict_t_ *dict) {
    valdict_t_ new_dict;
    unsigned new_capacity = dict->cell_capacity == 0 ? DICT_INITIAL_SIZE : dict->cell_capacity * 2;
    bool ok = valdict_init(&new_dict, dict->alloc, dict->key_size, dict->val_size, new_capacity);
    if (!ok) {
        return false;
    }
    new_dict._keys_equals = dict->_keys_equals;
    new_dict._hash_key = dict->_hash_key;
    for (unsigned int i = 0; i < dict->count; i++) {
        char *key = valdict_get_key_at(dict, i);
        void *value = valdict_get_value_at(dict, i);
        ok = valdict_set(&new_dict, key, value);
        if (!ok) {
            valdict_deinit(&new_dict);
            return false;
        }
    }
    valdict_deinit(dict);
    *dict = new_dict;
    return true;
}

static bool valdict_set_key_at(valdict_t_ *dict, unsigned int ix, void *key) {
    if (ix >= dict->count) {
        return false;
    }
    size_t offset = ix * dict->key_size;
    memcpy((char*)dict->keys + offset, key, dict->key_size);
    return true;
}

static bool valdict_keys_are_equal(const valdict_t_ *dict, const void *a, const void *b) {
    if (dict->_keys_equals) {
        return dict->_keys_equals(a, b);
    } else {
        return memcmp(a, b, dict->key_size) == 0;
    }
}

static unsigned long valdict_hash_key(const valdict_t_ *dict, const void *key) {
    if (dict->_hash_key) {
        return dict->_hash_key(key);
    } else {
        return collections_hash(key, dict->key_size);
    }
}

//-----------------------------------------------------------------------------
// Pointer dictionary
//-----------------------------------------------------------------------------
typedef struct ptrdict_ {
    allocator_t *alloc;
    valdict_t_ vd;
} ptrdict_t_;

// Public
ptrdict_t_* ptrdict_make(allocator_t *alloc) {
    ptrdict_t_ *dict = allocator_malloc(alloc, sizeof(ptrdict_t_));
    if (!dict) {
        return NULL;
    }
    dict->alloc = alloc;
    bool ok = valdict_init(&dict->vd, alloc, sizeof(void*), sizeof(void*), DICT_INITIAL_SIZE);
    if (!ok) {
        allocator_free(alloc, dict);
        return NULL;
    }
    return dict;
}

void ptrdict_destroy(ptrdict_t_ *dict) {
    if (dict == NULL) {
        return;
    }
    valdict_deinit(&dict->vd);
    allocator_free(dict->alloc, dict);
}

void  ptrdict_set_hash_function(ptrdict_t_ *dict, collections_hash_fn hash_fn) {
    valdict_set_hash_function(&dict->vd, hash_fn);
}

void ptrdict_set_equals_function(ptrdict_t_ *dict, collections_equals_fn equals_fn) {
    valdict_set_equals_function(&dict->vd, equals_fn);
}

bool ptrdict_set(ptrdict_t_ *dict, void *key, void *value) {
    return valdict_set(&dict->vd, &key, &value);
}

void* ptrdict_get(const ptrdict_t_ *dict, const void *key) {
    void* res = valdict_get(&dict->vd, &key);
    if (!res) {
        return NULL;
    }
    return *(void**)res;
}

void* ptrdict_get_value_at(const ptrdict_t_ *dict, unsigned int ix) {
    void* res = valdict_get_value_at(&dict->vd, ix);
    if (!res) {
        return NULL;
    }
    return *(void**)res;
}

void* ptrdict_get_key_at(const ptrdict_t_ *dict, unsigned int ix) {
    void* res = valdict_get_key_at(&dict->vd, ix);
    if (!res) {
        return NULL;
    }
    return *(void**)res;
}

int ptrdict_count(const ptrdict_t_ *dict) {
    return valdict_count(&dict->vd);
}

bool ptrdict_remove(ptrdict_t_ *dict, void *key) {
    return valdict_remove(&dict->vd, &key);
}

//-----------------------------------------------------------------------------
// Array
//-----------------------------------------------------------------------------

typedef struct array_ {
    allocator_t *alloc;
    unsigned char *data;
    unsigned char *data_allocated;
    unsigned int count;
    unsigned int capacity;
    size_t element_size;
    bool lock_capacity;
} array_t_;

static bool array_init_with_capacity(array_t_ *arr, allocator_t *alloc, unsigned int capacity, size_t element_size);
static void array_deinit(array_t_ *arr);

array_t_* array_make_(allocator_t *alloc, size_t element_size) {
    return array_make_with_capacity(alloc, 32, element_size);
}

array_t_* array_make_with_capacity(allocator_t *alloc, unsigned int capacity, size_t element_size) {
    array_t_ *arr = allocator_malloc(alloc, sizeof(array_t_));
    if (!arr) {
        return NULL;
    }

    bool ok = array_init_with_capacity(arr, alloc, capacity, element_size);
    if (!ok) {
        allocator_free(alloc, arr);
        return NULL;
    }
    return arr;
}

void array_destroy(array_t_ *arr) {
    if (!arr) {
        return;
    }
    allocator_t *alloc = arr->alloc;
    array_deinit(arr);
    allocator_free(alloc, arr);
}

void array_destroy_with_items_(array_t_ *arr, array_item_deinit_fn deinit_fn) {
    for (int i = 0; i < array_count(arr); i++) {
        void *item = array_get(arr, i);
        deinit_fn(item);
    }
    array_destroy(arr);
}

array_t_* array_copy(const array_t_ *arr) {
    array_t_ *copy = allocator_malloc(arr->alloc, sizeof(array_t_));
    if (!copy) {
        return NULL;
    }
    copy->alloc = arr->alloc;
    copy->capacity = arr->capacity;
    copy->count = arr->count;
    copy->element_size = arr->element_size;
    copy->lock_capacity = arr->lock_capacity;
    if (arr->data_allocated) {
        copy->data_allocated = allocator_malloc(arr->alloc, arr->capacity * arr->element_size);
        if (!copy->data_allocated) {
            allocator_free(arr->alloc, copy);
            return NULL;
        }
        copy->data = copy->data_allocated;
        memcpy(copy->data_allocated, arr->data, arr->capacity * arr->element_size);
    } else {
        copy->data_allocated = NULL;
        copy->data = NULL;
    }

    return copy;
}

bool array_add(array_t_ *arr, const void *value) {
    if (arr->count >= arr->capacity) {
        COLLECTIONS_ASSERT(!arr->lock_capacity);
        if (arr->lock_capacity) {
            return false;
        }
        unsigned int new_capacity = arr->capacity > 0 ? arr->capacity * 2 : 1;
        unsigned char *new_data = allocator_malloc(arr->alloc, new_capacity * arr->element_size);
        if (!new_data) {
            return false;
        }
        memcpy(new_data, arr->data, arr->count * arr->element_size);
        allocator_free(arr->alloc, arr->data_allocated);
        arr->data_allocated = new_data;
        arr->data = arr->data_allocated;
        arr->capacity = new_capacity;
    }
    if (value) {
        memcpy(arr->data + (arr->count * arr->element_size), value, arr->element_size);
    }
    arr->count++;
    return true;
}

bool array_addn(array_t_ *arr, const void *values, int n) {
    for (int i = 0; i < n; i++) {
        const uint8_t *value = NULL;
        if (values) {
            value = (const uint8_t*)values + (i * arr->element_size);
        }
        bool ok = array_add(arr, value);
        if (!ok) {
            return false;
        }
    }
    return true;
}

bool array_add_array(array_t_ *dest, array_t_ *source) {
    COLLECTIONS_ASSERT(dest->element_size == source->element_size);
    if (dest->element_size != source->element_size) {
        return false;
    }
    int dest_before_count = array_count(dest);
    for (int i = 0; i < array_count(source); i++) {
        void *item = array_get(source, i);
        bool ok = array_add(dest, item);
        if (!ok) {
            dest->count = dest_before_count;
            return false;
        }
    }
    return true;
}

bool array_push(array_t_ *arr, const void *value) {
    return array_add(arr, value);
}

bool array_pop(array_t_ *arr, void *out_value) {
    if (arr->count <= 0) {
        return false;
    }
    if (out_value) {
        void *res = array_get(arr, arr->count - 1);
        memcpy(out_value, res, arr->element_size);
    }
    array_remove_at(arr, arr->count - 1);
    return true;
}

void* array_top(array_t_ *arr) {
    if (arr->count <= 0) {
        return NULL;
    }
    return array_get(arr, arr->count - 1);
}

bool array_set(array_t_ *arr, unsigned int ix, void *value) {
    if (ix >= arr->count) {
        COLLECTIONS_ASSERT(false);
        return false;
    }
    size_t offset = ix * arr->element_size;
    memmove(arr->data + offset, value, arr->element_size);
    return true;
}

bool array_setn(array_t_ *arr, unsigned int ix, void *values, int n) {
    for (int i = 0; i < n; i++) {
        int dest_ix = ix + i;
        unsigned char *value = (unsigned char*)values + (i * arr->element_size);
        if (dest_ix < array_count(arr)) {
            bool ok = array_set(arr, dest_ix, value);
            if (!ok) {
                return false;
            }
        } else {
            bool ok = array_add(arr, value);
            if (!ok) {
                return false;
            }
        }
    }
    return true;
}

void * array_get(array_t_ *arr, unsigned int ix) {
    if (ix >= arr->count) {
        COLLECTIONS_ASSERT(false);
        return NULL;
    }
    size_t offset = ix * arr->element_size;
    return arr->data + offset;
}

const void * array_get_const(const array_t_ *arr, unsigned int ix) {
    if (ix >= arr->count) {
        COLLECTIONS_ASSERT(false);
        return NULL;
    }
    size_t offset = ix * arr->element_size;
    return arr->data + offset;
}

void * array_get_last(array_t_ *arr) {
    if (arr->count <= 0) {
        return NULL;
    }
    return array_get(arr, arr->count - 1);
}

int array_count(const array_t_ *arr) {
    if (!arr) {
        return 0;
    }
    return arr->count;
}

unsigned int array_get_capacity(const array_t_ *arr) {
    return arr->capacity;
}

bool array_remove_at(array_t_ *arr, unsigned int ix) {
    if (ix >= arr->count) {
        return false;
    }
    if (ix == 0) {
        arr->data += arr->element_size;
        arr->capacity--;
        arr->count--;
        return true;
    }
    if (ix == (arr->count - 1)) {
        arr->count--;
        return true;
    }
    size_t to_move_bytes = (arr->count - 1 - ix) * arr->element_size;
    void *dest = arr->data + (ix * arr->element_size);
    void *src = arr->data + ((ix + 1) * arr->element_size);
    memmove(dest, src, to_move_bytes);
    arr->count--;
    return true;
}

bool array_remove_item(array_t_ *arr, void *ptr) {
    int ix = array_get_index(arr, ptr);
    if (ix < 0) {
        return false;
    }
    return array_remove_at(arr, ix);
}

void array_clear(array_t_ *arr) {
    arr->count = 0;
}

void array_clear_and_deinit_items_(array_t_ *arr, array_item_deinit_fn deinit_fn) {
    for (int i = 0; i < array_count(arr); i++) {
        void *item = array_get(arr, i);
        deinit_fn(item);
    }
    arr->count = 0;
}

void array_lock_capacity(array_t_ *arr) {
    arr->lock_capacity = true;
}

int array_get_index(const array_t_ *arr, void *ptr) {
    for (int i = 0; i < array_count(arr); i++) {
        if (array_get_const(arr, i) == ptr) {
            return i;
        }
    }
    return -1;
}

bool array_contains(const array_t_ *arr, void *ptr) {
    return array_get_index(arr, ptr) >= 0;
}

void* array_data(array_t_ *arr) {
    return arr->data;
}

const void* array_const_data(const array_t_ *arr) {
    return arr->data;
}

void array_orphan_data(array_t_ *arr) {
    array_init_with_capacity(arr, arr->alloc, 0, arr->element_size);
}

bool array_reverse(array_t_ *arr) {
    int count = array_count(arr);
    if (count < 2) {
        return true;
    }
    void *temp = allocator_malloc(arr->alloc, arr->element_size);
    if (temp) {
        return false;
    }
    for (int a_ix = 0; a_ix < (count / 2); a_ix++) {
        int b_ix = count - a_ix - 1;
        void *a = array_get(arr, a_ix);
        void *b = array_get(arr, b_ix);
        memcpy(temp, a, arr->element_size);
        array_set(arr, a_ix, b); // no need for check because it will be within range
        array_set(arr, b_ix, temp);
    }
    allocator_free(arr->alloc, temp);
    return true;
}

static bool array_init_with_capacity(array_t_ *arr, allocator_t *alloc, unsigned int capacity, size_t element_size) {
    arr->alloc = alloc;
    if (capacity > 0) {
        arr->data_allocated = allocator_malloc(arr->alloc, capacity * element_size);
        arr->data = arr->data_allocated;
        if (!arr->data_allocated) {
            return false;
        }
    } else {
        arr->data_allocated = NULL;
        arr->data = NULL;
    }
    arr->capacity = capacity;
    arr->count = 0;
    arr->element_size = element_size;
    arr->lock_capacity = false;
    return true;
}

static void array_deinit(array_t_ *arr) {
    allocator_free(arr->alloc, arr->data_allocated);
}

//-----------------------------------------------------------------------------
// Pointer Array
//-----------------------------------------------------------------------------

typedef struct ptrarray_ {
    allocator_t *alloc;
    array_t_ arr;
} ptrarray_t_;

ptrarray_t_* ptrarray_make(allocator_t *alloc) {
    return ptrarray_make_with_capacity(alloc, 0);
}

ptrarray_t_* ptrarray_make_with_capacity(allocator_t *alloc, unsigned int capacity) {
    ptrarray_t_ *ptrarr = allocator_malloc(alloc, sizeof(ptrarray_t_));
    if (!ptrarr) {
        return NULL;
    }
    ptrarr->alloc = alloc;
    bool ok = array_init_with_capacity(&ptrarr->arr, alloc, capacity, sizeof(void*));
    if (!ok) {
        allocator_free(alloc, ptrarr);
        return NULL;
    }
    return ptrarr;
}

void ptrarray_destroy(ptrarray_t_ *arr) {
    if (!arr) {
        return;
    }
    array_deinit(&arr->arr);
    allocator_free(arr->alloc, arr);
}

void ptrarray_destroy_with_items_(ptrarray_t_ *arr, ptrarray_item_destroy_fn destroy_fn) { // todo: destroy and copy in make fn
    if (arr == NULL) {
        return;
    }

    if (destroy_fn) {
        ptrarray_clear_and_destroy_items_(arr, destroy_fn);
    }

    ptrarray_destroy(arr);
}

ptrarray_t_* ptrarray_copy(ptrarray_t_ *arr) {
    ptrarray_t_ *arr_copy = ptrarray_make_with_capacity(arr->alloc, arr->arr.capacity);
    if (!arr_copy) {
        return NULL;
    }
    for (int i = 0; i < ptrarray_count(arr); i++) {
        void *item = ptrarray_get(arr, i);
        bool ok = ptrarray_add(arr_copy, item);
        if (!ok) {
            ptrarray_destroy(arr_copy);
            return NULL;
        }
    }
    return arr_copy;
}

ptrarray_t_* ptrarray_copy_with_items_(ptrarray_t_ *arr, ptrarray_item_copy_fn copy_fn, ptrarray_item_destroy_fn destroy_fn) {
    ptrarray_t_ *arr_copy = ptrarray_make_with_capacity(arr->alloc, arr->arr.capacity);
    if (!arr_copy) {
        return NULL;
    }
    for (int i = 0; i < ptrarray_count(arr); i++) {
        void *item = ptrarray_get(arr, i);
        void *item_copy = copy_fn(item);
        if (item && !item_copy) {
            goto err;
        }
        bool ok = ptrarray_add(arr_copy, item_copy);
        if (!ok) {
            goto err;
        }
    }
    return arr_copy;
err:
    ptrarray_destroy_with_items_(arr_copy, destroy_fn);
    return NULL;
}

bool ptrarray_add(ptrarray_t_ *arr, void *ptr) {
    return array_add(&arr->arr, &ptr);
}

bool ptrarray_set(ptrarray_t_ *arr, unsigned int ix, void *ptr) {
    return array_set(&arr->arr, ix, &ptr);
}

bool ptrarray_add_array(ptrarray_t_ *dest, ptrarray_t_ *source) {
    return array_add_array(&dest->arr, &source->arr);
}

void * ptrarray_get(ptrarray_t_ *arr, unsigned int ix) {
    void* res = array_get(&arr->arr, ix);
    if (!res) {
        return NULL;
    }
    return *(void**)res;
}

const void * ptrarray_get_const(const ptrarray_t_ *arr, unsigned int ix) {
    const void* res = array_get_const(&arr->arr, ix);
    if (!res) {
        return NULL;
    }
    return *(void * const *)res;
}

bool ptrarray_push(ptrarray_t_ *arr, void *ptr) {
    return ptrarray_add(arr, ptr);
}


void *ptrarray_pop(ptrarray_t_ *arr) {
    int ix = ptrarray_count(arr) - 1;
    void *res = ptrarray_get(arr, ix);
    ptrarray_remove_at(arr, ix);
    return res;
}

void *ptrarray_top(ptrarray_t_ *arr) {
    int count = ptrarray_count(arr);
    if (count == 0) {
        return NULL;
    }
    return ptrarray_get(arr, count - 1);
}

int ptrarray_count(const ptrarray_t_ *arr) {
    if (!arr) {
        return 0;
    }
    return array_count(&arr->arr);
}

bool ptrarray_remove_at(ptrarray_t_ *arr, unsigned int ix) {
    return array_remove_at(&arr->arr, ix);
}

bool ptrarray_remove_item(ptrarray_t_ *arr, void *item) {
    for (int i = 0; i < ptrarray_count(arr); i++) {
        if (item == ptrarray_get(arr, i)) {
            ptrarray_remove_at(arr, i);
            return true;
        }
    }
    COLLECTIONS_ASSERT(false);
    return false;
}

void ptrarray_clear(ptrarray_t_ *arr) {
    array_clear(&arr->arr);
}

void ptrarray_clear_and_destroy_items_(ptrarray_t_ *arr, ptrarray_item_destroy_fn destroy_fn) {
    for (int i = 0; i < ptrarray_count(arr); i++) {
        void *item = ptrarray_get(arr, i);
        destroy_fn(item);
    }
    ptrarray_clear(arr);
}

void ptrarray_lock_capacity(ptrarray_t_ *arr) {
    array_lock_capacity(&arr->arr);
}

int ptrarray_get_index(ptrarray_t_ *arr, void *ptr) {
    for (int i = 0; i < ptrarray_count(arr); i++) {
        if (ptrarray_get(arr, i) == ptr) {
            return i;
        }
    }
    return -1;
}

bool ptrarray_contains(ptrarray_t_ *arr, void *item) {
    return ptrarray_get_index(arr, item) >= 0;
}

void * ptrarray_get_addr(ptrarray_t_ *arr, unsigned int ix) {
    void* res = array_get(&arr->arr, ix);
    if (res == NULL) {
        return NULL;
    }
    return res;
}

void* ptrarray_data(ptrarray_t_ *arr) {
    return array_data(&arr->arr);
}

void ptrarray_reverse(ptrarray_t_ *arr) {
    array_reverse(&arr->arr);
}

//-----------------------------------------------------------------------------
// String buffer
//-----------------------------------------------------------------------------

typedef struct strbuf {
    allocator_t *alloc;
    bool failed;
    char *data;
    size_t capacity;
    size_t len;
} strbuf_t;

static bool strbuf_grow(strbuf_t *buf, size_t new_capacity);

strbuf_t* strbuf_make(allocator_t *alloc) {
    return strbuf_make_with_capacity(alloc, 1);
}

strbuf_t* strbuf_make_with_capacity(allocator_t *alloc, unsigned int capacity) {
    strbuf_t *buf = allocator_malloc(alloc, sizeof(strbuf_t));
    if (buf == NULL) {
        return NULL;
    }
    memset(buf, 0, sizeof(strbuf_t));
    buf->alloc = alloc;
    buf->failed = false;
    buf->data = allocator_malloc(alloc, capacity);
    if (buf->data == NULL) {
        allocator_free(alloc, buf);
        return NULL;
    }
    buf->capacity = capacity;
    buf->len = 0;
    buf->data[0] = '\0';
    return buf;
}

void strbuf_destroy(strbuf_t *buf) {
    if (buf == NULL) {
        return;
    }
    allocator_free(buf->alloc, buf->data);
    allocator_free(buf->alloc, buf);
}

void strbuf_clear(strbuf_t *buf) {
    if (buf->failed) {
        return;
    }
    buf->len = 0;
    buf->data[0] = '\0';
}

bool strbuf_append(strbuf_t *buf, const char *str) {
    if (buf->failed) {
        return false;
    }
    size_t str_len = strlen(str);
    if (str_len == 0) {
        return true;
    }
    size_t required_capacity = buf->len + str_len + 1;
    if (required_capacity > buf->capacity) {
        bool ok = strbuf_grow(buf, required_capacity * 2);
        if (!ok) {
            return false;
        }
    }
    memcpy(buf->data + buf->len, str, str_len);
    buf->len = buf->len + str_len;
    buf->data[buf->len] = '\0';
    return true;
}

bool strbuf_appendf(strbuf_t *buf, const char *fmt, ...) {
    if (buf->failed) {
        return false;
    }
    va_list args;
    va_start(args, fmt);
    int to_write = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (to_write == 0) {
        return true;
    }
    size_t required_capacity = buf->len + to_write + 1;
    if (required_capacity > buf->capacity) {
        bool ok = strbuf_grow(buf, required_capacity * 2);
        if (!ok) {
            return false;
        }
    }
    va_start(args, fmt);
    int written = vsprintf(buf->data + buf->len, fmt, args);
    (void)written;
    va_end(args);
    if (written != to_write) {
        return false;
    }
    buf->len = buf->len + to_write;
    buf->data[buf->len] = '\0';
    return true;
}

const char * strbuf_get_string(const strbuf_t *buf) {
    if (buf->failed) {
        return NULL;
    }
    return buf->data;
}

size_t strbuf_get_length(const strbuf_t *buf) {
    if (buf->failed) {
        return 0;
    }
    return buf->len;
}

char * strbuf_get_string_and_destroy(strbuf_t *buf) {
    if (buf->failed) {
        strbuf_destroy(buf);
        return NULL;
    }
    char *res = buf->data;
    buf->data = NULL;
    strbuf_destroy(buf);
    return res;
}

bool strbuf_failed(strbuf_t *buf) {
    return buf->failed;
}

static bool strbuf_grow(strbuf_t *buf, size_t new_capacity) {
    char *new_data = allocator_malloc(buf->alloc, new_capacity);
    if (new_data == NULL) {
        buf->failed = true;
        return false;
    }
    memcpy(new_data, buf->data, buf->len);
    new_data[buf->len] = '\0';
    allocator_free(buf->alloc, buf->data);
    buf->data = new_data;
    buf->capacity = new_capacity;
    return true;
}

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------

ptrarray(char)* kg_split_string(allocator_t *alloc, const char *str, const char *delimiter) {
    ptrarray(char)* res = ptrarray_make(alloc);
    char *rest = NULL;
    if (!str) {
        return res;
    }
    const char *line_start = str;
    const char *line_end = strstr(line_start, delimiter);
    while (line_end != NULL) {
        long len = line_end - line_start;
        char *line = collections_strndup(alloc, line_start, len);
        if (!line) {
            goto err;
        }
        bool ok = ptrarray_add(res, line);
        if (!ok) {
            goto err;
        }
        line_start = line_end + 1;
        line_end = strstr(line_start, delimiter);
    }
    rest = collections_strdup(alloc, line_start);
    if (!rest) {
        goto err;
    }
    bool ok = ptrarray_add(res, rest);
    if (!ok) {
        goto err;
    }
    return res;
err:
    allocator_free(alloc, rest);
    if (res) {
        for (int i = 0; i < ptrarray_count(res); i++) {
            char *line = ptrarray_get(res, i);
            allocator_free(alloc, line);
        }
    }
    ptrarray_destroy(res);
    return NULL;
}

char* kg_join(allocator_t *alloc, ptrarray(char) *items, const char *with) {
    strbuf_t *res = strbuf_make(alloc);
    if (!res) {
        return NULL;
    }
    for (int i = 0; i < ptrarray_count(items); i++) {
        char *item = ptrarray_get(items, i);
        strbuf_append(res, item);
        if (i < (ptrarray_count(items) - 1)) {
            strbuf_append(res, with);
        }
    }
    return strbuf_get_string_and_destroy(res);
}

char* kg_canonicalise_path(allocator_t *alloc, const char *path) {
    if (!strchr(path, '/') || (!strstr(path, "/../") && !strstr(path, "./"))) {
        return collections_strdup(alloc, path);
    }

    ptrarray(char) *split = kg_split_string(alloc, path, "/");
    if (!split) {
        return NULL;
    }

    for (int i = 0; i < ptrarray_count(split) - 1; i++) {
        char *item = ptrarray_get(split, i);
        char *next_item = ptrarray_get(split, i + 1);
        if (kg_streq(item, ".")) {
            allocator_free(alloc, item);
            ptrarray_remove_at(split, i);
            i = -1;
            continue;
        }
        if (kg_streq(next_item, "..")) {
            allocator_free(alloc, item);
            allocator_free(alloc, next_item);
            ptrarray_remove_at(split, i);
            ptrarray_remove_at(split, i);
            i = -1;
            continue;
        }
    }

    char *joined = kg_join(alloc, split, "/");
    for (int i = 0; i < ptrarray_count(split); i++) {
        void *item = ptrarray_get(split, i);
        allocator_free(alloc, item);
    }
    ptrarray_destroy(split);
    return joined;
}

bool kg_is_path_absolute(const char *path) {
    return path[0] == '/';
}

bool kg_streq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}
//FILE_END
//FILE_START:errors.c
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef APE_AMALGAMATED
#include "errors.h"
#include "traceback.h"
#endif

void errors_init(errors_t *errors) {
    memset(errors, 0, sizeof(errors_t));
    errors->count = 0;
}

void errors_deinit(errors_t *errors) {
    errors_clear(errors);
}

void errors_add_error(errors_t *errors, error_type_t type, src_pos_t pos, const char *message) {
    if (errors->count >= ERRORS_MAX_COUNT) {
        return;
    }
    error_t err;
    memset(&err, 0, sizeof(error_t));
    err.type = type;
    int len = (int)strlen(message);
    int to_copy = len;
    if (to_copy >= (ERROR_MESSAGE_MAX_LENGTH - 1)) {
        to_copy = ERROR_MESSAGE_MAX_LENGTH - 1;
    }
    memcpy(err.message, message, to_copy);
    err.message[to_copy] = '\0';
    err.pos = pos;
    err.traceback = NULL;
    errors->errors[errors->count] = err;
    errors->count++;
}

void errors_add_errorf(errors_t *errors, error_type_t type, src_pos_t pos, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int to_write = vsnprintf(NULL, 0, format, args);
    (void)to_write;
    va_end(args);
    va_start(args, format);
    char res[ERROR_MESSAGE_MAX_LENGTH];
    int written = vsnprintf(res, ERROR_MESSAGE_MAX_LENGTH, format, args);
    (void)written;
    APE_ASSERT(to_write == written);
    va_end(args);
    errors_add_error(errors, type, pos, res);
}

void errors_clear(errors_t *errors) {
    for (int i = 0; i < errors_get_count(errors); i++) {
        error_t *error = errors_get(errors, i);
        if (error->traceback) {
            traceback_destroy(error->traceback);
        }
    }
    errors->count = 0;
}

int errors_get_count(const errors_t *errors) {
    return errors->count;
}

error_t *errors_get(errors_t *errors, int ix) {
    if (ix >= errors->count) {
        return NULL;
    }
    return &errors->errors[ix];
}

const error_t *errors_getc(const errors_t *errors, int ix) {
    if (ix >= errors->count) {
        return NULL;
    }
    return &errors->errors[ix];
}

const char *error_type_to_string(error_type_t type) {
    switch (type) {
        case ERROR_PARSING:     return "PARSING";
        case ERROR_COMPILATION: return "COMPILATION";
        case ERROR_RUNTIME:     return "RUNTIME";
        case ERROR_TIMEOUT:     return "TIMEOUT";
        case ERROR_ALLOCATION:  return "ALLOCATION";
        case ERROR_USER:        return "USER";
        default:                return "INVALID";
    }
}

error_t *errors_get_last_error(errors_t *errors) {
    if (errors->count <= 0) {
        return NULL;
    }
    return &errors->errors[errors->count - 1];
}

bool errors_has_errors(const errors_t *errors) {
    return errors_get_count(errors) > 0;
}
//FILE_END
//FILE_START:token.c
#include <string.h>
#include <stdlib.h>

#ifndef APE_AMALGAMATED
#include "token.h"
#endif

static const char *g_type_names[] = {
    "ILLEGAL",
    "EOF",
    "=",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    "&=",
    "|=",
    "^=",
    "<<=",
    ">>=",
    "+",
    "-",
    "!",
    "*",
    "/",
    "<",
    "<=",
    ">",
    ">=",
    "==",
    "!=",
    "&&",
    "||",
    "&",
    "|",
    "^",
    "<<",
    ">>",
    ",",
    ";",
    ":",
    "(",
    ")",
    "{",
    "}",
    "[",
    "]",
    ".",
    "%",
    "FUNCTION",
    "CONST",
    "VAR",
    "TRUE",
    "FALSE",
    "IF",
    "ELSE",
    "RETURN",
    "WHILE",
    "BREAK",
    "FOR",
    "IN",
    "CONTINUE",
    "NULL",
    "IMPORT",
    "RECOVER",
    "IDENT",
    "NUMBER",
    "STRING",
    "TEMPLATE_STRING",
};

void token_init(token_t *tok, token_type_t type, const char *literal, int len) {
    tok->type = type;
    tok->literal = literal;
    tok->len = len;
}

char* token_duplicate_literal(allocator_t *alloc, const token_t *tok) {
    return ape_strndup(alloc, tok->literal, tok->len);
}

const char* token_type_to_string(token_type_t type) {
    return g_type_names[type];
}
//FILE_END
//FILE_START:compiled_file.c
#ifndef APE_AMALGAMATED
#include "compiled_file.h"
#endif

compiled_file_t* compiled_file_make(allocator_t *alloc, const char *path) {
    compiled_file_t *file = allocator_malloc(alloc, sizeof(compiled_file_t));
    if (!file) {
        return NULL;
    }
    memset(file, 0, sizeof(compiled_file_t));
    file->alloc = alloc;
    const char *last_slash_pos = strrchr(path, '/');
    if (last_slash_pos) {
        size_t len = last_slash_pos - path + 1;
        file->dir_path = ape_strndup(alloc, path, len);
    } else {
        file->dir_path = ape_strdup(alloc, "");
    }
    if (!file->dir_path) {
        goto error;
    }
    file->path = ape_strdup(alloc, path);
    if (!file->path) {
        goto error;
    }
    file->lines = ptrarray_make(alloc);
    if (!file->lines) {
        goto error;
    }
    return file;
error:
    compiled_file_destroy(file);
    return NULL;
}

void compiled_file_destroy(compiled_file_t *file) {
    if (!file) {
        return;
    }
    for (int i = 0; i < ptrarray_count(file->lines); i++) {
        void *item = ptrarray_get(file->lines, i);
        allocator_free(file->alloc, item);
    }
    ptrarray_destroy(file->lines);
    allocator_free(file->alloc, file->dir_path);
    allocator_free(file->alloc, file->path);
    allocator_free(file->alloc, file);
}
//FILE_END
//FILE_START:lexer.c
#include <stdlib.h>
#include <string.h>

#ifndef APE_AMALGAMATED
#include "lexer.h"
#include "collections.h"
#include "compiler.h"
#endif

static bool read_char(lexer_t *lex);
static char peek_char(lexer_t *lex);
static bool is_letter(char ch);
static bool is_digit(char ch);
static bool is_one_of(char ch, const char* allowed, int allowed_len);
static const char* read_identifier(lexer_t *lex, int *out_len);
static const char* read_number(lexer_t *lex, int *out_len);
static const char* read_string(lexer_t *lex, char delimiter, bool is_template, bool *out_template_found, int *out_len);
static token_type_t lookup_identifier(const char *ident, int len);
static void skip_whitespace(lexer_t *lex);
static bool add_line(lexer_t *lex, int offset);

bool lexer_init(lexer_t *lex, allocator_t *alloc, errors_t *errs, const char *input, compiled_file_t *file) {
    lex->alloc = alloc;
    lex->errors = errs;
    lex->input = input;
    lex->input_len = (int)strlen(input);
    lex->position = 0;
    lex->next_position = 0;
    lex->ch = '\0';
    if (file) {
        lex->line = ptrarray_count(file->lines);
    } else {
        lex->line = 0;
    }
    lex->column = -1;
    lex->file = file;
    bool ok = add_line(lex, 0);
    if (!ok) {
        return false;
    }
    ok = read_char(lex);
    if (!ok) {
        return false;
    }
    lex->failed = false;
    lex->continue_template_string = false;

    memset(&lex->prev_token_state, 0, sizeof(lex->prev_token_state));
    token_init(&lex->prev_token, TOKEN_INVALID, NULL, 0);
    token_init(&lex->cur_token, TOKEN_INVALID, NULL, 0);
    token_init(&lex->peek_token, TOKEN_INVALID, NULL, 0);

    return true;
}

bool lexer_failed(lexer_t *lex) {
    return lex->failed;
}

void lexer_continue_template_string(lexer_t *lex) {
    lex->continue_template_string = true;
}

bool lexer_cur_token_is(lexer_t *lex, token_type_t type) {
    return lex->cur_token.type == type;
}

bool lexer_peek_token_is(lexer_t *lex, token_type_t type) {
    return lex->peek_token.type == type;
}

bool lexer_next_token(lexer_t *lex) {
    lex->prev_token = lex->cur_token;
    lex->cur_token = lex->peek_token;
    lex->peek_token = lexer_next_token_internal(lex);
    return !lex->failed;
}

bool lexer_previous_token(lexer_t *lex) {
    if (lex->prev_token.type == TOKEN_INVALID) {
        return false;
    }

    lex->peek_token = lex->cur_token;
    lex->cur_token = lex->prev_token;
    token_init(&lex->prev_token, TOKEN_INVALID, NULL, 0);

    lex->ch = lex->prev_token_state.ch;
    lex->column = lex->prev_token_state.column;
    lex->line = lex->prev_token_state.line;
    lex->position = lex->prev_token_state.position;
    lex->next_position = lex->prev_token_state.next_position;

    return true;
}

token_t lexer_next_token_internal(lexer_t *lex) {
    lex->prev_token_state.ch = lex->ch;
    lex->prev_token_state.column = lex->column;
    lex->prev_token_state.line = lex->line;
    lex->prev_token_state.position = lex->position;
    lex->prev_token_state.next_position = lex->next_position;

    while (true) {
        if (!lex->continue_template_string) {
            skip_whitespace(lex);
        }

        token_t out_tok;
        out_tok.type = TOKEN_INVALID;
        out_tok.literal = lex->input + lex->position;
        out_tok.len = 1;
        out_tok.pos = src_pos_make(lex->file, lex->line, lex->column);

        char c = lex->continue_template_string ? '`' : lex->ch;

        switch (c) {
            case '\0': token_init(&out_tok, TOKEN_EOF, "EOF", 3); break;
            case '=': {
                if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_EQ, "==", 2);
                    read_char(lex);
                } else {
                    token_init(&out_tok, TOKEN_ASSIGN, "=", 1);
                }
                break;
            }
            case '&': {
                if (peek_char(lex) == '&') {
                    token_init(&out_tok, TOKEN_AND, "&&", 2);
                    read_char(lex);
                } else if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_BIT_AND_ASSIGN, "&=", 2);
                    read_char(lex);
                } else {
                    token_init(&out_tok, TOKEN_BIT_AND, "&", 1);
                }
                break;
            }
            case '|': {
                if (peek_char(lex) == '|') {
                    token_init(&out_tok, TOKEN_OR, "||", 2);
                    read_char(lex);
                } else if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_BIT_OR_ASSIGN, "|=", 2);
                    read_char(lex);
                } else {
                    token_init(&out_tok, TOKEN_BIT_OR, "|", 1);
                }
                break;
            }
            case '^': {
                if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_BIT_XOR_ASSIGN, "^=", 2);
                    read_char(lex);
                } else {
                    token_init(&out_tok, TOKEN_BIT_XOR, "^", 1); break;
                }
                break;
            }
            case '+': {
                if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_PLUS_ASSIGN, "+=", 2);
                    read_char(lex);
                } else {
                    token_init(&out_tok, TOKEN_PLUS, "+", 1); break;
                }
                break;
            }
            case '-': {
                if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_MINUS_ASSIGN, "-=", 2);
                    read_char(lex);
                } else {
                    token_init(&out_tok, TOKEN_MINUS, "-", 1); break;
                }
                break;
            }
            case '!': {
                if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_NOT_EQ, "!=", 2);
                    read_char(lex);
                } else {
                    token_init(&out_tok, TOKEN_BANG, "!", 1);
                }
                break;
            }
            case '*': {
                if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_ASTERISK_ASSIGN, "*=", 2);
                    read_char(lex);
                } else {
                    token_init(&out_tok, TOKEN_ASTERISK, "*", 1); break;
                }
                break;
            }
            case '/': {
                if (peek_char(lex) == '/') {
                    read_char(lex);
                    while (lex->ch != '\n' && lex->ch != '\0') {
                        read_char(lex);
                    }
                    continue;
                } else if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_SLASH_ASSIGN, "/=", 2);
                    read_char(lex);
                } else {
                    token_init(&out_tok, TOKEN_SLASH, "/", 1); break;
                }
                break;
            }
            case '<': {
                if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_LTE, "<=", 2);
                    read_char(lex);
                } else if (peek_char(lex) == '<') {
                    read_char(lex);
                    if (peek_char(lex) == '=') {
                        token_init(&out_tok, TOKEN_LSHIFT_ASSIGN, "<<=", 3);
                        read_char(lex);
                    } else {
                        token_init(&out_tok, TOKEN_LSHIFT, "<<", 2);
                    }
                } else {
                    token_init(&out_tok, TOKEN_LT, "<", 1); break;
                }
                break;
            }
            case '>': {
                if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_GTE, ">=", 2);
                    read_char(lex);
                } else if (peek_char(lex) == '>') {
                    read_char(lex);
                    if (peek_char(lex) == '=') {
                        token_init(&out_tok, TOKEN_RSHIFT_ASSIGN, ">>=", 3);
                        read_char(lex);
                    } else {
                        token_init(&out_tok, TOKEN_RSHIFT, ">>", 2);
                    }
                } else {
                    token_init(&out_tok, TOKEN_GT, ">", 1);
                }
                break;
            }
            case ',': token_init(&out_tok, TOKEN_COMMA, ",", 1); break;
            case ';': token_init(&out_tok, TOKEN_SEMICOLON, ";", 1); break;
            case ':': token_init(&out_tok, TOKEN_COLON, ":", 1); break;
            case '(': token_init(&out_tok, TOKEN_LPAREN, "(", 1); break;
            case ')': token_init(&out_tok, TOKEN_RPAREN, ")", 1); break;
            case '{': token_init(&out_tok, TOKEN_LBRACE, "{", 1); break;
            case '}': token_init(&out_tok, TOKEN_RBRACE, "}", 1); break;
            case '[': token_init(&out_tok, TOKEN_LBRACKET, "[", 1); break;
            case ']': token_init(&out_tok, TOKEN_RBRACKET, "]", 1); break;
            case '.': token_init(&out_tok, TOKEN_DOT, ".", 1); break;
            case '%': {
                if (peek_char(lex) == '=') {
                    token_init(&out_tok, TOKEN_PERCENT_ASSIGN, "%=", 2);
                    read_char(lex);
                } else {
                    token_init(&out_tok, TOKEN_PERCENT, "%", 1); break;
                }
                break;
            }
            case '"': {
                read_char(lex);
                int len;
                const char *str = read_string(lex, '"', false, NULL, &len);
                if (str) {
                    token_init(&out_tok, TOKEN_STRING, str, len);
                } else {
                    token_init(&out_tok, TOKEN_INVALID, NULL, 0);
                }
                break;
            }
            case '\'': {
                read_char(lex);
                int len;
                const char *str = read_string(lex, '\'', false, NULL, &len);
                if (str) {
                    token_init(&out_tok, TOKEN_STRING, str, len);
                } else {
                    token_init(&out_tok, TOKEN_INVALID, NULL, 0);
                }
                break;
            }
            case '`': {
                if (!lex->continue_template_string) {
                    read_char(lex);
                }
                int len;
                bool template_found = false;
                const char *str = read_string(lex, '`', true, &template_found, &len);
                if (str) {
                    if (template_found) {
                        token_init(&out_tok, TOKEN_TEMPLATE_STRING, str, len);
                    } else {
                        token_init(&out_tok, TOKEN_STRING, str, len);
                    }
                } else {
                    token_init(&out_tok, TOKEN_INVALID, NULL, 0);
                }
                break;
            }
            default: {
                if (is_letter(lex->ch)) {
                    int ident_len = 0;
                    const char *ident = read_identifier(lex, &ident_len);
                    token_type_t type = lookup_identifier(ident, ident_len);
                    token_init(&out_tok, type, ident, ident_len);
                    return out_tok;
                } else if (is_digit(lex->ch)) {
                    int number_len = 0;
                    const char *number = read_number(lex, &number_len);
                    token_init(&out_tok, TOKEN_NUMBER, number, number_len);
                    return out_tok;
                }
                break;
            }
        }
        read_char(lex);
        if (lexer_failed(lex)) {
            token_init(&out_tok, TOKEN_INVALID, NULL, 0);
        }
        lex->continue_template_string = false;
        return out_tok;
    }
}

bool lexer_expect_current(lexer_t *lex, token_type_t type) {
    if (lexer_failed(lex)) {
        return false;
    }

    if (!lexer_cur_token_is(lex, type)) {
        const char *expected_type_str = token_type_to_string(type);
        const char *actual_type_str = token_type_to_string(lex->cur_token.type);
        errors_add_errorf(lex->errors, ERROR_PARSING, lex->cur_token.pos,
                          "Expected current token to be \"%s\", got \"%s\" instead",
                          expected_type_str, actual_type_str);
        return false;
    }
    return true;
}

// INTERNAL

static bool read_char(lexer_t *lex) {
    if (lex->next_position >= lex->input_len) {
        lex->ch = '\0';
    } else {
        lex->ch = lex->input[lex->next_position];
    }
    lex->position = lex->next_position;
    lex->next_position++;
    
    if (lex->ch == '\n') {
        lex->line++;
        lex->column = -1;
        bool ok = add_line(lex, lex->next_position);
        if (!ok) {
            lex->failed = true;
            return false;
        }
    } else {
        lex->column++;
    }
    return true;
}

static char peek_char(lexer_t *lex) {
    if (lex->next_position >= lex->input_len) {
        return '\0';
    } else {
        return lex->input[lex->next_position];
    }
}

static bool is_letter(char ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_';
}

static bool is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

static bool is_one_of(char ch, const char* allowed, int allowed_len) {
    for (int i = 0; i < allowed_len; i++) {
        if (ch == allowed[i]) {
            return true;
        }
    }
    return false;
}

static const char* read_identifier(lexer_t *lex, int *out_len) {
    int position = lex->position;
    int len = 0;
    while (is_digit(lex->ch) || is_letter(lex->ch) || lex->ch == ':') {
        if (lex->ch == ':') {
            if (peek_char(lex) != ':') {
                goto end;
            }
            read_char(lex);
        }
        read_char(lex);
    }
end:
    len = lex->position - position;
    *out_len = len;
    return lex->input + position;
}

static const char* read_number(lexer_t *lex, int *out_len) {
    char allowed[] = ".xXaAbBcCdDeEfF";
    int position = lex->position;
    while (is_digit(lex->ch) || is_one_of(lex->ch, allowed, APE_ARRAY_LEN(allowed) - 1)) {
        read_char(lex);
    }
    int len = lex->position - position;
    *out_len = len;
    return lex->input + position;
}

static const char* read_string(lexer_t *lex, char delimiter, bool is_template, bool *out_template_found, int *out_len) {
    *out_len = 0;

    bool escaped = false;
    int position = lex->position;

    while (true) {
        if (lex->ch == '\0') {
            return NULL;
        }
        if (lex->ch == delimiter && !escaped) {
            break;
        }
        if (is_template && !escaped && lex->ch == '$' && peek_char(lex) == '{') {
            *out_template_found = true;
            break;
        }
        escaped = false;
        if (lex->ch == '\\') {
            escaped = true;
        }
        read_char(lex);
    }
    int len = lex->position - position;
    *out_len = len;
    return lex->input + position;
}

static token_type_t lookup_identifier(const char *ident, int len) {
    struct {
        const char *value;
        int len;
        token_type_t type;
    } keywords[] = {
        {"fn", 2, TOKEN_FUNCTION},
        {"const", 5, TOKEN_CONST},
        {"var", 3, TOKEN_VAR},
        {"true", 4, TOKEN_TRUE},
        {"false", 5, TOKEN_FALSE},
        {"if", 2, TOKEN_IF},
        {"else", 4, TOKEN_ELSE},
        {"return", 6, TOKEN_RETURN},
        {"while", 5, TOKEN_WHILE},
        {"break", 5, TOKEN_BREAK},
        {"for", 3, TOKEN_FOR},
        {"in", 2, TOKEN_IN},
        {"continue", 8, TOKEN_CONTINUE},
        {"null", 4, TOKEN_NULL},
        {"import", 6, TOKEN_IMPORT},
        {"recover", 7, TOKEN_RECOVER},
    };

    for (int i = 0; i < APE_ARRAY_LEN(keywords); i++) {
        if (keywords[i].len == len && APE_STRNEQ(ident, keywords[i].value, len)) {
            return keywords[i].type;
        }
    }
    
    return TOKEN_IDENT;
}

static void skip_whitespace(lexer_t *lex) {
    char ch = lex->ch;
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        read_char(lex);
        ch = lex->ch;
    }
}

static bool add_line(lexer_t *lex, int offset) {
    if (!lex->file) {
        return true;
    }

    if (lex->line < ptrarray_count(lex->file->lines)) {
        return true;
    }

    const char *line_start = lex->input + offset;
    const char *new_line_ptr = strchr(line_start, '\n');
    char *line = NULL;
    if (!new_line_ptr) {
        line = ape_strdup(lex->alloc, line_start);
    } else {
        size_t line_len = new_line_ptr - line_start;
        line = ape_strndup(lex->alloc, line_start, line_len);
    }
    if (!line) {
        lex->failed = true;
        return false;
    }
    bool ok = ptrarray_add(lex->file->lines, line);
    if (!ok) {
        lex->failed = true;
        allocator_free(lex->alloc, line);
        return false;
    }
    return true;
}
//FILE_END
//FILE_START:ast.c
#include <stdlib.h>
#include <string.h>

#ifndef APE_AMALGAMATED
#include "ast.h"
#include "common.h"
#endif

static expression_t* expression_make(allocator_t *alloc, expression_type_t type);
static statement_t* statement_make(allocator_t *alloc, statement_type_t type);

expression_t* expression_make_ident(allocator_t *alloc, ident_t *ident) {
    expression_t *res = expression_make(alloc, EXPRESSION_IDENT);
    if (!res) {
        return NULL;
    }
    res->ident = ident;
    return res;
}

expression_t* expression_make_number_literal(allocator_t *alloc, double val) {
    expression_t *res = expression_make(alloc, EXPRESSION_NUMBER_LITERAL);
    if (!res) {
        return NULL;
    }
    res->number_literal = val;
    return res;
}

expression_t* expression_make_bool_literal(allocator_t *alloc, bool val) {
    expression_t *res = expression_make(alloc, EXPRESSION_BOOL_LITERAL);
    if (!res) {
        return NULL;
    }
    res->bool_literal = val;
    return res;
}

expression_t* expression_make_string_literal(allocator_t *alloc, char *value) {
    expression_t *res = expression_make(alloc, EXPRESSION_STRING_LITERAL);
    if (!res) {
        return NULL;
    }
    res->string_literal = value;
    return res;
}

expression_t* expression_make_null_literal(allocator_t *alloc) {
    expression_t *res = expression_make(alloc, EXPRESSION_NULL_LITERAL);
    if (!res) {
        return NULL;
    }
    return res;
}

expression_t* expression_make_array_literal(allocator_t *alloc, ptrarray(expression_t) *values) {
    expression_t *res = expression_make(alloc, EXPRESSION_ARRAY_LITERAL);
    if (!res) {
        return NULL;
    }
    res->array = values;
    return res;
}

expression_t* expression_make_map_literal(allocator_t *alloc, ptrarray(expression_t) *keys, ptrarray(expression_t) *values) {
    expression_t *res = expression_make(alloc, EXPRESSION_MAP_LITERAL);
    if (!res) {
        return NULL;
    }
    res->map.keys = keys;
    res->map.values = values;
    return res;
}

expression_t* expression_make_prefix(allocator_t *alloc, operator_t op, expression_t *right) {
    expression_t *res = expression_make(alloc, EXPRESSION_PREFIX);
    if (!res) {
        return NULL;
    }
    res->prefix.op = op;
    res->prefix.right = right;
    return res;
}

expression_t* expression_make_infix(allocator_t *alloc, operator_t op, expression_t *left, expression_t *right) {
    expression_t *res = expression_make(alloc, EXPRESSION_INFIX);
    if (!res) {
        return NULL;
    }
    res->infix.op = op;
    res->infix.left = left;
    res->infix.right = right;
    return res;
}

expression_t* expression_make_fn_literal(allocator_t *alloc, ptrarray(ident_t) *params, code_block_t *body) {
    expression_t *res = expression_make(alloc, EXPRESSION_FUNCTION_LITERAL);
    if (!res) {
        return NULL;
    }
    res->fn_literal.name = NULL;
    res->fn_literal.params = params;
    res->fn_literal.body = body;
    return res;
}

expression_t* expression_make_call(allocator_t *alloc, expression_t *function, ptrarray(expression_t) *args) {
    expression_t *res = expression_make(alloc, EXPRESSION_CALL);
    if (!res) {
        return NULL;
    }
    res->call_expr.function = function;
    res->call_expr.args = args;
    return res;
}

expression_t* expression_make_index(allocator_t *alloc, expression_t *left, expression_t *index) {
    expression_t *res = expression_make(alloc, EXPRESSION_INDEX);
    if (!res) {
        return NULL;
    }
    res->index_expr.left = left;
    res->index_expr.index = index;
    return res;
}

expression_t* expression_make_assign(allocator_t *alloc, expression_t *dest, expression_t *source) {
    expression_t *res = expression_make(alloc, EXPRESSION_ASSIGN);
    if (!res) {
        return NULL;
    }
    res->assign.dest = dest;
    res->assign.source = source;
    return res;
}

expression_t* expression_make_logical(allocator_t *alloc, operator_t op, expression_t *left, expression_t *right) {
    expression_t *res = expression_make(alloc, EXPRESSION_LOGICAL);
    if (!res) {
        return NULL;
    }
    res->logical.op = op;
    res->logical.left = left;
    res->logical.right = right;
    return res;
}

void expression_destroy(expression_t *expr) {
    if (!expr) {
        return;
    }

    switch (expr->type) {
        case EXPRESSION_NONE: {
            APE_ASSERT(false);
            break;
        }
        case EXPRESSION_IDENT: {
            ident_destroy(expr->ident);
            break;
        }
        case EXPRESSION_NUMBER_LITERAL:
        case EXPRESSION_BOOL_LITERAL: {
            break;
        }
        case EXPRESSION_STRING_LITERAL: {
            allocator_free(expr->alloc, expr->string_literal);
            break;
        }
        case EXPRESSION_NULL_LITERAL: {
            break;
        }
        case EXPRESSION_ARRAY_LITERAL: {
            ptrarray_destroy_with_items(expr->array, expression_destroy);
            break;
        }
        case EXPRESSION_MAP_LITERAL: {
            ptrarray_destroy_with_items(expr->map.keys, expression_destroy);
            ptrarray_destroy_with_items(expr->map.values, expression_destroy);
            break;
        }
        case EXPRESSION_PREFIX: {
            expression_destroy(expr->prefix.right);
            break;
        }
        case EXPRESSION_INFIX: {
            expression_destroy(expr->infix.left);
            expression_destroy(expr->infix.right);
            break;
        }
        case EXPRESSION_FUNCTION_LITERAL: {
            fn_literal_t *fn = &expr->fn_literal;
            allocator_free(expr->alloc, fn->name);
            ptrarray_destroy_with_items(fn->params, ident_destroy);
            code_block_destroy(fn->body);
            break;
        }
        case EXPRESSION_CALL: {
            ptrarray_destroy_with_items(expr->call_expr.args, expression_destroy);
            expression_destroy(expr->call_expr.function);
            break;
        }
        case EXPRESSION_INDEX: {
            expression_destroy(expr->index_expr.left);
            expression_destroy(expr->index_expr.index);
            break;
        }
        case EXPRESSION_ASSIGN: {
            expression_destroy(expr->assign.dest);
            expression_destroy(expr->assign.source);
            break;
        }
        case EXPRESSION_LOGICAL: {
            expression_destroy(expr->logical.left);
            expression_destroy(expr->logical.right);
            break;
        }
    }
    allocator_free(expr->alloc, expr);

}

expression_t* expression_copy(expression_t *expr) {
    if (!expr) {
        return NULL;
    }
    expression_t *res = NULL;
    switch (expr->type) {
        case EXPRESSION_NONE: {
            APE_ASSERT(false);
            break;
        }
        case EXPRESSION_IDENT: {
            ident_t *ident = ident_copy(expr->ident);
            if (!ident) {
                return NULL;
            }
            res = expression_make_ident(expr->alloc, ident);
            if (!res) {
                ident_destroy(ident);
                return NULL;
            }
            break;
        }
        case EXPRESSION_NUMBER_LITERAL: {
            res = expression_make_number_literal(expr->alloc, expr->number_literal);
            break;
        }
        case EXPRESSION_BOOL_LITERAL: {
            res = expression_make_bool_literal(expr->alloc, expr->bool_literal);
            break;
        }
        case EXPRESSION_STRING_LITERAL: {
            char *string_copy = ape_strdup(expr->alloc, expr->string_literal);
            if (!string_copy) {
                return NULL;
            }
            res = expression_make_string_literal(expr->alloc, string_copy);
            if (!res) {
                allocator_free(expr->alloc, string_copy);
                return NULL;
            }
            break;
        }
        case EXPRESSION_NULL_LITERAL: {
            res = expression_make_null_literal(expr->alloc);
            break;
        }
        case EXPRESSION_ARRAY_LITERAL: {
            ptrarray(expression_t) *values_copy = ptrarray_copy_with_items(expr->array, expression_copy, expression_destroy);
            if (!values_copy) {
                return NULL;
            }
            res = expression_make_array_literal(expr->alloc, values_copy);
            if (!res) {
                ptrarray_destroy_with_items(values_copy, expression_destroy);
                return NULL;
            }
            break;
        }
        case EXPRESSION_MAP_LITERAL: {
            ptrarray(expression_t) *keys_copy = ptrarray_copy_with_items(expr->map.keys, expression_copy, expression_destroy);
            ptrarray(expression_t) *values_copy = ptrarray_copy_with_items(expr->map.values, expression_copy, expression_destroy);
            if (!keys_copy || !values_copy) {
                ptrarray_destroy_with_items(keys_copy, expression_destroy);
                ptrarray_destroy_with_items(values_copy, expression_destroy);
                return NULL;
            }
            res = expression_make_map_literal(expr->alloc, keys_copy, values_copy);
            if (!res) {
                ptrarray_destroy_with_items(keys_copy, expression_destroy);
                ptrarray_destroy_with_items(values_copy, expression_destroy);
                return NULL;
            }
            break;
        }
        case EXPRESSION_PREFIX: {
            expression_t *right_copy = expression_copy(expr->prefix.right);
            if (!right_copy) {
                return NULL;
            }
            res = expression_make_prefix(expr->alloc, expr->prefix.op, right_copy);
            if (!res) {
                expression_destroy(right_copy);
                return NULL;
            }
            break;
        }
        case EXPRESSION_INFIX: {
            expression_t *left_copy = expression_copy(expr->infix.left);
            expression_t *right_copy = expression_copy(expr->infix.right);
            if (!left_copy || !right_copy) {
                expression_destroy(left_copy);
                expression_destroy(right_copy);
                return NULL;
            }
            res = expression_make_infix(expr->alloc, expr->infix.op, left_copy, right_copy);
            if (!res) {
                expression_destroy(left_copy);
                expression_destroy(right_copy);
                return NULL;
            }
            break;
        }
        case EXPRESSION_FUNCTION_LITERAL: {
            ptrarray(ident_t) *params_copy = ptrarray_copy_with_items(expr->fn_literal.params, ident_copy, ident_destroy);
            code_block_t *body_copy = code_block_copy(expr->fn_literal.body);
            char *name_copy =  ape_strdup(expr->alloc, expr->fn_literal.name);
            if (!params_copy || !body_copy) {
                ptrarray_destroy_with_items(params_copy, ident_destroy);
                code_block_destroy(body_copy);
                allocator_free(expr->alloc, name_copy);
                return NULL;
            }
            res = expression_make_fn_literal(expr->alloc, params_copy, body_copy);
            if (!res) {
                ptrarray_destroy_with_items(params_copy, ident_destroy);
                code_block_destroy(body_copy);
                allocator_free(expr->alloc, name_copy);
                return NULL;
            }
            res->fn_literal.name = name_copy;
            break;
        }
        case EXPRESSION_CALL: {
            expression_t *function_copy = expression_copy(expr->call_expr.function);
            ptrarray(expression_t) *args_copy = ptrarray_copy_with_items(expr->call_expr.args, expression_copy, expression_destroy);
            if (!function_copy || !args_copy) {
                expression_destroy(function_copy);
                ptrarray_destroy_with_items(expr->call_expr.args, expression_destroy);
                return NULL;
            }
            res = expression_make_call(expr->alloc, function_copy, args_copy);
            if (!res) {
                expression_destroy(function_copy);
                ptrarray_destroy_with_items(expr->call_expr.args, expression_destroy);
                return NULL;
            }
            break;
        }
        case EXPRESSION_INDEX: {
            expression_t *left_copy = expression_copy(expr->index_expr.left);
            expression_t *index_copy = expression_copy(expr->index_expr.index);
            if (!left_copy || !index_copy) {
                expression_destroy(left_copy);
                expression_destroy(index_copy);
                return NULL;
            }
            res = expression_make_index(expr->alloc, left_copy, index_copy);
            if (!res) {
                expression_destroy(left_copy);
                expression_destroy(index_copy);
                return NULL;
            }
            break;
        }
        case EXPRESSION_ASSIGN: {
            expression_t *dest_copy = expression_copy(expr->assign.dest);
            expression_t *source_copy = expression_copy(expr->assign.source);
            if (!dest_copy || !source_copy) {
                expression_destroy(dest_copy);
                expression_destroy(source_copy);
                return NULL;
            }
            res = expression_make_assign(expr->alloc, dest_copy, source_copy);
            if (!res) {
                expression_destroy(dest_copy);
                expression_destroy(source_copy);
                return NULL;
            }
            break;
        }
        case EXPRESSION_LOGICAL: {
            expression_t *left_copy = expression_copy(expr->logical.left);
            expression_t *right_copy = expression_copy(expr->logical.right);
            if (!left_copy || !right_copy) {
                expression_destroy(left_copy);
                expression_destroy(right_copy);
                return NULL;
            }
            res = expression_make_logical(expr->alloc, expr->logical.op, left_copy, right_copy);
            if (!res) {
                expression_destroy(left_copy);
                expression_destroy(right_copy);
                return NULL;
            }
            break;
        }
    }
    if (!res) {
        return NULL;
    }
    res->pos = expr->pos;
    return res;
}

statement_t* statement_make_define(allocator_t *alloc, ident_t *name, expression_t *value, bool assignable) {
    statement_t *res = statement_make(alloc, STATEMENT_DEFINE);
    if (!res) {
        return NULL;
    }
    res->define.name = name;
    res->define.value = value;
    res->define.assignable = assignable;
    return res;
}

statement_t* statement_make_if(allocator_t *alloc, ptrarray(if_case_t) *cases, code_block_t *alternative) {
    statement_t *res = statement_make(alloc, STATEMENT_IF);
    if (!res) {
        return NULL;
    }
    res->if_statement.cases = cases;
    res->if_statement.alternative = alternative;
    return res;
}

statement_t* statement_make_return(allocator_t *alloc, expression_t *value) {
    statement_t *res = statement_make(alloc, STATEMENT_RETURN_VALUE);
    if (!res) {
        return NULL;
    }
    res->return_value = value;
    return res;
}

statement_t* statement_make_expression(allocator_t *alloc, expression_t *value) {
    statement_t *res = statement_make(alloc, STATEMENT_EXPRESSION);
    if (!res) {
        return NULL;
    }
    res->expression = value;
    return res;
}

statement_t* statement_make_while_loop(allocator_t *alloc, expression_t *test, code_block_t *body) {
    statement_t *res = statement_make(alloc, STATEMENT_WHILE_LOOP);
    if (!res) {
        return NULL;
    }
    res->while_loop.test = test;
    res->while_loop.body = body;
    return res;
}

statement_t* statement_make_break(allocator_t *alloc) {
    statement_t *res = statement_make(alloc, STATEMENT_BREAK);
    if (!res) {
        return NULL;
    }
    return res;
}

statement_t* statement_make_foreach(allocator_t *alloc, ident_t *iterator, expression_t *source, code_block_t *body) {
    statement_t *res = statement_make(alloc, STATEMENT_FOREACH);
    if (!res) {
        return NULL;
    }
    res->foreach.iterator = iterator;
    res->foreach.source = source;
    res->foreach.body = body;
    return res;
}

statement_t* statement_make_for_loop(allocator_t *alloc, statement_t *init, expression_t *test, expression_t *update, code_block_t *body) {
    statement_t *res = statement_make(alloc, STATEMENT_FOR_LOOP);
    if (!res) {
        return NULL;
    }
    res->for_loop.init = init;
    res->for_loop.test = test;
    res->for_loop.update = update;
    res->for_loop.body = body;
    return res;
}

statement_t* statement_make_continue(allocator_t *alloc) {
    statement_t *res = statement_make(alloc, STATEMENT_CONTINUE);
    if (!res) {
        return NULL;
    }
    return res;
}

statement_t* statement_make_block(allocator_t *alloc, code_block_t *block) {
    statement_t *res = statement_make(alloc, STATEMENT_BLOCK);
    if (!res) {
        return NULL;
    }
    res->block = block;
    return res;
}

statement_t* statement_make_import(allocator_t *alloc, char *path) {
    statement_t *res = statement_make(alloc, STATEMENT_IMPORT);
    if (!res) {
        return NULL;
    }
    res->import.path = path;
    return res;
}

statement_t* statement_make_recover(allocator_t *alloc, ident_t *error_ident, code_block_t *body) {
    statement_t *res = statement_make(alloc, STATEMENT_RECOVER);
    if (!res) {
        return NULL;
    }
    res->recover.error_ident = error_ident;
    res->recover.body = body;
    return res;
}

void statement_destroy(statement_t *stmt) {
    if (!stmt) {
        return;
    }
    switch (stmt->type) {
        case STATEMENT_NONE: {
            APE_ASSERT(false);
            break;
        }
        case STATEMENT_DEFINE: {
            ident_destroy(stmt->define.name);
            expression_destroy(stmt->define.value);
            break;
        }
        case STATEMENT_IF: {
            ptrarray_destroy_with_items(stmt->if_statement.cases, if_case_destroy);
            code_block_destroy(stmt->if_statement.alternative);
            break;
        }
        case STATEMENT_RETURN_VALUE: {
            expression_destroy(stmt->return_value);
            break;
        }
        case STATEMENT_EXPRESSION: {
            expression_destroy(stmt->expression);
            break;
        }
        case STATEMENT_WHILE_LOOP: {
            expression_destroy(stmt->while_loop.test);
            code_block_destroy(stmt->while_loop.body);
            break;
        }
        case STATEMENT_BREAK: {
            break;
        }
        case STATEMENT_CONTINUE: {
            break;
        }
        case STATEMENT_FOREACH: {
            ident_destroy(stmt->foreach.iterator);
            expression_destroy(stmt->foreach.source);
            code_block_destroy(stmt->foreach.body);
            break;
        }
        case STATEMENT_FOR_LOOP: {
            statement_destroy(stmt->for_loop.init);
            expression_destroy(stmt->for_loop.test);
            expression_destroy(stmt->for_loop.update);
            code_block_destroy(stmt->for_loop.body);
            break;
        }
        case STATEMENT_BLOCK: {
            code_block_destroy(stmt->block);
            break;
        }
        case STATEMENT_IMPORT: {
            allocator_free(stmt->alloc, stmt->import.path);
            break;
        }
        case STATEMENT_RECOVER: {
            code_block_destroy(stmt->recover.body);
            ident_destroy(stmt->recover.error_ident);
            break;
        }
    }
    allocator_free(stmt->alloc, stmt);
}

statement_t* statement_copy(const statement_t *stmt) {
    if (!stmt) {
        return NULL;
    }
    statement_t *res = NULL;
    switch (stmt->type) {
        case STATEMENT_NONE: {
            APE_ASSERT(false);
            break;
        }
        case STATEMENT_DEFINE: {
            expression_t *value_copy = expression_copy(stmt->define.value);
            if (!value_copy) {
                return NULL;
            }
            res = statement_make_define(stmt->alloc, ident_copy(stmt->define.name), value_copy, stmt->define.assignable);
            if (!res) {
                expression_destroy(value_copy);
                return NULL;
            }
            break;
        }
        case STATEMENT_IF: {
            ptrarray(if_case_t) *cases_copy = ptrarray_copy_with_items(stmt->if_statement.cases, if_case_copy, if_case_destroy);
            code_block_t *alternative_copy = code_block_copy(stmt->if_statement.alternative);
            if (!cases_copy || !alternative_copy) {
                ptrarray_destroy_with_items(cases_copy, if_case_destroy);
                code_block_destroy(alternative_copy);
                return NULL;
            }
            res = statement_make_if(stmt->alloc, cases_copy, alternative_copy);
            if (res) {
                ptrarray_destroy_with_items(cases_copy, if_case_destroy);
                code_block_destroy(alternative_copy);
                return NULL;
            }
            break;
        }
        case STATEMENT_RETURN_VALUE: {
            expression_t *value_copy = expression_copy(stmt->return_value);
            if (!value_copy) {
                return NULL;
            }
            res = statement_make_return(stmt->alloc, value_copy);
            if (!res) {
                expression_destroy(value_copy);
                return NULL;
            }
            break;
        }
        case STATEMENT_EXPRESSION: {
            expression_t *value_copy = expression_copy(stmt->expression);
            if (!value_copy) {
                return NULL;
            }
            res = statement_make_expression(stmt->alloc, value_copy);
            if (!res) {
                expression_destroy(value_copy);
                return NULL;
            }
            break;
        }
        case STATEMENT_WHILE_LOOP: {
            expression_t *test_copy = expression_copy(stmt->while_loop.test);
            code_block_t *body_copy = code_block_copy(stmt->while_loop.body);
            if (!test_copy || !body_copy) {
                expression_destroy(test_copy);
                code_block_destroy(body_copy);
                return NULL;
            }
            res = statement_make_while_loop(stmt->alloc, test_copy, body_copy);
            if (!res) {
                expression_destroy(test_copy);
                code_block_destroy(body_copy);
                return NULL;
            }
            break;
        }
        case STATEMENT_BREAK: {
            res = statement_make_break(stmt->alloc);
            break;
        }
        case STATEMENT_CONTINUE: {
            res = statement_make_continue(stmt->alloc);
            break;
        }
        case STATEMENT_FOREACH: {
            expression_t *source_copy = expression_copy(stmt->foreach.source);
            code_block_t *body_copy = code_block_copy(stmt->foreach.body);
            if (!source_copy || !body_copy) {
                expression_destroy(source_copy);
                code_block_destroy(body_copy);
                return NULL;
            }
            res = statement_make_foreach(stmt->alloc, ident_copy(stmt->foreach.iterator), source_copy, body_copy);
            if (!res) {
                expression_destroy(source_copy);
                code_block_destroy(body_copy);
                return NULL;
            }
            break;
        }
        case STATEMENT_FOR_LOOP: {
            statement_t *init_copy = statement_copy(stmt->for_loop.init);
            expression_t *test_copy = expression_copy(stmt->for_loop.test);
            expression_t *update_copy = expression_copy(stmt->for_loop.update);
            code_block_t *body_copy = code_block_copy(stmt->for_loop.body);
            if (!init_copy || !test_copy || !update_copy || !body_copy) {
                statement_destroy(init_copy);
                expression_destroy(test_copy);
                expression_destroy(update_copy);
                code_block_destroy(body_copy);
                return NULL;
            }
            res = statement_make_for_loop(stmt->alloc, init_copy, test_copy, update_copy, body_copy);
            if (!res) {
                statement_destroy(init_copy);
                expression_destroy(test_copy);
                expression_destroy(update_copy);
                code_block_destroy(body_copy);
                return NULL;
            }
            break;
        }
        case STATEMENT_BLOCK: {
            code_block_t *block_copy = code_block_copy(stmt->block);
            if (!block_copy) {
                return NULL;
            }
            res = statement_make_block(stmt->alloc, block_copy);
            if (!res) {
                code_block_destroy(block_copy);
                return NULL;
            }
            break;
        }
        case STATEMENT_IMPORT: {
            char *path_copy = ape_strdup(stmt->alloc, stmt->import.path);
            if (!path_copy) {
                return NULL;
            }
            res = statement_make_import(stmt->alloc, path_copy);
            if (!res) {
                allocator_free(stmt->alloc, path_copy);
                return NULL;
            }
            break;
        }
        case STATEMENT_RECOVER: {
            code_block_t *body_copy = code_block_copy(stmt->recover.body);
            ident_t *error_ident_copy = ident_copy(stmt->recover.error_ident);
            if (!body_copy || !error_ident_copy) {
                code_block_destroy(body_copy);
                ident_destroy(error_ident_copy);
                return NULL;
            }
            res = statement_make_recover(stmt->alloc, error_ident_copy, body_copy);
            if (!res) {
                code_block_destroy(body_copy);
                ident_destroy(error_ident_copy);
                return NULL;
            }
            break;
        }
    }
    if (!res) {
        return NULL;
    }
    res->pos = stmt->pos;
    return res;
}

code_block_t* code_block_make(allocator_t *alloc, ptrarray(statement_t) *statements) {
    code_block_t *block = allocator_malloc(alloc, sizeof(code_block_t));
    if (!block) {
        return NULL;
    }
    block->alloc = alloc;
    block->statements = statements;
    return block;
}

void code_block_destroy(code_block_t *block) {
    if (!block) {
        return;
    }
    ptrarray_destroy_with_items(block->statements, statement_destroy);
    allocator_free(block->alloc, block);
}

code_block_t* code_block_copy(code_block_t *block) {
    if (!block) {
        return NULL;
    }
    ptrarray(statement_t) *statements_copy = ptrarray_copy_with_items(block->statements, statement_copy, statement_destroy);
    if (!statements_copy) {
        return NULL;
    }
    code_block_t* res = code_block_make(block->alloc, statements_copy);
    if (!res) {
        ptrarray_destroy_with_items(statements_copy, statement_destroy);
        return NULL;
    }
    return res;
}

char* statements_to_string(allocator_t *alloc, ptrarray(statement_t) *statements) {
    strbuf_t *buf = strbuf_make(alloc);
    if (!buf) {
        return NULL;
    }
    int count =  ptrarray_count(statements);
    for (int i = 0; i < count; i++) {
        const statement_t *stmt = ptrarray_get(statements, i);
        statement_to_string(stmt, buf);
        if (i < (count - 1)) {
            strbuf_append(buf, "\n");
        }
    }
    return strbuf_get_string_and_destroy(buf);
}

void statement_to_string(const statement_t *stmt, strbuf_t *buf) {
    switch (stmt->type) {
        case STATEMENT_DEFINE: {
            const define_statement_t *def_stmt = &stmt->define;
            if (stmt->define.assignable) {
                strbuf_append(buf, "var ");
            } else {
                strbuf_append(buf, "const ");
            }
            strbuf_append(buf, def_stmt->name->value);
            strbuf_append(buf, " = ");

            if (def_stmt->value) {
                expression_to_string(def_stmt->value, buf);
            }

            break;
        }
        case STATEMENT_IF: {
            if_case_t *if_case = ptrarray_get(stmt->if_statement.cases, 0);
            strbuf_append(buf, "if (");
            expression_to_string(if_case->test, buf);
            strbuf_append(buf, ") ");
            code_block_to_string(if_case->consequence, buf);
            for (int i = 1; i < ptrarray_count(stmt->if_statement.cases); i++) {
                if_case_t *elif_case = ptrarray_get(stmt->if_statement.cases, i);
                strbuf_append(buf, " elif (");
                expression_to_string(elif_case->test, buf);
                strbuf_append(buf, ") ");
                code_block_to_string(elif_case->consequence, buf);
            }
            if (stmt->if_statement.alternative) {
                strbuf_append(buf, " else ");
                code_block_to_string(stmt->if_statement.alternative, buf);
            }
            break;
        }
        case STATEMENT_RETURN_VALUE: {
            strbuf_append(buf, "return ");
            if (stmt->return_value) {
                expression_to_string(stmt->return_value, buf);
            }
            break;
        }
        case STATEMENT_EXPRESSION: {
            if (stmt->expression) {
                expression_to_string(stmt->expression, buf);
            }
            break;
        }
        case STATEMENT_WHILE_LOOP: {
            strbuf_append(buf, "while (");
            expression_to_string(stmt->while_loop.test, buf);
            strbuf_append(buf, ")");
            code_block_to_string(stmt->while_loop.body, buf);
            break;
        }
        case STATEMENT_FOR_LOOP: {
            strbuf_append(buf, "for (");
            if (stmt->for_loop.init) {
                statement_to_string(stmt->for_loop.init, buf);
                strbuf_append(buf, " ");
            } else {
                strbuf_append(buf, ";");
            }
            if (stmt->for_loop.test) {
                expression_to_string(stmt->for_loop.test, buf);
                strbuf_append(buf, "; ");
            } else {
                strbuf_append(buf, ";");
            }
            if (stmt->for_loop.update) {
                expression_to_string(stmt->for_loop.test, buf);
            }
            strbuf_append(buf, ")");
            code_block_to_string(stmt->for_loop.body, buf);
            break;
        }
        case STATEMENT_FOREACH: {
            strbuf_append(buf, "for (");
            strbuf_appendf(buf, "%s", stmt->foreach.iterator->value);
            strbuf_append(buf, " in ");
            expression_to_string(stmt->foreach.source, buf);
            strbuf_append(buf, ")");
            code_block_to_string(stmt->foreach.body, buf);
            break;
        }
        case STATEMENT_BLOCK: {
            code_block_to_string(stmt->block, buf);
            break;
        }
        case STATEMENT_BREAK: {
            strbuf_append(buf, "break");
            break;
        }
        case STATEMENT_CONTINUE: {
            strbuf_append(buf, "continue");
            break;
        }
        case STATEMENT_IMPORT: {
            strbuf_appendf(buf, "import \"%s\"", stmt->import.path);
            break;
        }
        case STATEMENT_NONE: {
            strbuf_append(buf, "STATEMENT_NONE");
            break;
        }
        case STATEMENT_RECOVER: {
            strbuf_appendf(buf, "recover (%s)", stmt->recover.error_ident->value);
            code_block_to_string(stmt->recover.body, buf);
            break;
        }
    }
}

void expression_to_string(expression_t *expr, strbuf_t *buf) {
    switch (expr->type) {
        case EXPRESSION_IDENT: {
            strbuf_append(buf, expr->ident->value);
            break;
        }
        case EXPRESSION_NUMBER_LITERAL: {
            strbuf_appendf(buf, "%1.17g", expr->number_literal);
            break;
        }
        case EXPRESSION_BOOL_LITERAL: {
            strbuf_appendf(buf, "%s", expr->bool_literal ? "true" : "false");
            break;
        }
        case EXPRESSION_STRING_LITERAL: {
            strbuf_appendf(buf, "\"%s\"", expr->string_literal);
            break;
        }
        case EXPRESSION_NULL_LITERAL: {
            strbuf_append(buf, "null");
            break;
        }
        case EXPRESSION_ARRAY_LITERAL: {
            strbuf_append(buf, "[");
            for (int i = 0; i < ptrarray_count(expr->array); i++) {
                expression_t *arr_expr = ptrarray_get(expr->array, i);
                expression_to_string(arr_expr, buf);
                if (i < (ptrarray_count(expr->array) - 1)) {
                    strbuf_append(buf, ", ");
                }
            }
            strbuf_append(buf, "]");
            break;
        }
        case EXPRESSION_MAP_LITERAL: {
            map_literal_t *map = &expr->map;

            strbuf_append(buf, "{");
            for (int i = 0; i < ptrarray_count(map->keys); i++) {
                expression_t *key_expr = ptrarray_get(map->keys, i);
                expression_t *val_expr = ptrarray_get(map->values, i);

                expression_to_string(key_expr, buf);
                strbuf_append(buf, " : ");
                expression_to_string(val_expr, buf);

                if (i < (ptrarray_count(map->keys) - 1)) {
                    strbuf_append(buf, ", ");
                }
            }
            strbuf_append(buf, "}");
            break;
        }
        case EXPRESSION_PREFIX: {
            strbuf_append(buf, "(");
            strbuf_append(buf, operator_to_string(expr->infix.op));
            expression_to_string(expr->prefix.right, buf);
            strbuf_append(buf, ")");
            break;
        }
        case EXPRESSION_INFIX: {
            strbuf_append(buf, "(");
            expression_to_string(expr->infix.left, buf);
            strbuf_append(buf, " ");
            strbuf_append(buf, operator_to_string(expr->infix.op));
            strbuf_append(buf, " ");
            expression_to_string(expr->infix.right, buf);
            strbuf_append(buf, ")");
            break;
        }
        case EXPRESSION_FUNCTION_LITERAL: {
            fn_literal_t *fn = &expr->fn_literal;
            
            strbuf_append(buf, "fn");

            strbuf_append(buf, "(");
            for (int i = 0; i < ptrarray_count(fn->params); i++) {
                ident_t *param = ptrarray_get(fn->params, i);
                strbuf_append(buf, param->value);
                if (i < (ptrarray_count(fn->params) - 1)) {
                    strbuf_append(buf, ", ");
                }
            }
            strbuf_append(buf, ") ");

            code_block_to_string(fn->body, buf);

            break;
        }
        case EXPRESSION_CALL: {
            call_expression_t *call_expr = &expr->call_expr;

            expression_to_string(call_expr->function, buf);

            strbuf_append(buf, "(");
            for (int i = 0; i < ptrarray_count(call_expr->args); i++) {
                expression_t *arg = ptrarray_get(call_expr->args, i);
                expression_to_string(arg, buf);
                if (i < (ptrarray_count(call_expr->args) - 1)) {
                    strbuf_append(buf, ", ");
                }
            }
            strbuf_append(buf, ")");

            break;
        }
        case EXPRESSION_INDEX: {
            strbuf_append(buf, "(");
            expression_to_string(expr->index_expr.left, buf);
            strbuf_append(buf, "[");
            expression_to_string(expr->index_expr.index, buf);
            strbuf_append(buf, "])");
            break;
        }
        case EXPRESSION_ASSIGN: {
            expression_to_string(expr->assign.dest, buf);
            strbuf_append(buf, " = ");
            expression_to_string(expr->assign.source, buf);
            break;
        }
        case EXPRESSION_LOGICAL: {
            expression_to_string(expr->logical.left, buf);
            strbuf_append(buf, " ");
            strbuf_append(buf, operator_to_string(expr->infix.op));
            strbuf_append(buf, " ");
            expression_to_string(expr->logical.right, buf);
            break;
        }
        case EXPRESSION_NONE: {
            strbuf_append(buf, "EXPRESSION_NONE");
            break;
        }
    }
}

void code_block_to_string(const code_block_t *stmt, strbuf_t *buf) {
    strbuf_append(buf, "{ ");
    for (int i = 0; i < ptrarray_count(stmt->statements); i++) {
        statement_t *istmt = ptrarray_get(stmt->statements, i);
        statement_to_string(istmt, buf);
        strbuf_append(buf, "\n");
    }
    strbuf_append(buf, " }");
}

const char* operator_to_string(operator_t op) {
    switch (op) {
        case OPERATOR_NONE:        return "OPERATOR_NONE";
        case OPERATOR_ASSIGN:      return "=";
        case OPERATOR_PLUS:        return "+";
        case OPERATOR_MINUS:       return "-";
        case OPERATOR_BANG:        return "!";
        case OPERATOR_ASTERISK:    return "*";
        case OPERATOR_SLASH:       return "/";
        case OPERATOR_LT:          return "<";
        case OPERATOR_GT:          return ">";
        case OPERATOR_EQ:          return "==";
        case OPERATOR_NOT_EQ:      return "!=";
        case OPERATOR_MODULUS:     return "%";
        case OPERATOR_LOGICAL_AND: return "&&";
        case OPERATOR_LOGICAL_OR:  return "||";
        case OPERATOR_BIT_AND:     return "&";
        case OPERATOR_BIT_OR:      return "|";
        case OPERATOR_BIT_XOR:     return "^";
        case OPERATOR_LSHIFT:      return "<<";
        case OPERATOR_RSHIFT:      return ">>";
        default:                   return "OPERATOR_UNKNOWN";
    }
}

const char *expression_type_to_string(expression_type_t type) {
    switch (type) {
        case EXPRESSION_NONE:             return "NONE";
        case EXPRESSION_IDENT:            return "IDENT";
        case EXPRESSION_NUMBER_LITERAL:   return "INT_LITERAL";
        case EXPRESSION_BOOL_LITERAL:     return "BOOL_LITERAL";
        case EXPRESSION_STRING_LITERAL:   return "STRING_LITERAL";
        case EXPRESSION_ARRAY_LITERAL:    return "ARRAY_LITERAL";
        case EXPRESSION_MAP_LITERAL:      return "MAP_LITERAL";
        case EXPRESSION_PREFIX:           return "PREFIX";
        case EXPRESSION_INFIX:            return "INFIX";
        case EXPRESSION_FUNCTION_LITERAL: return "FN_LITERAL";
        case EXPRESSION_CALL:             return "CALL";
        case EXPRESSION_INDEX:            return "INDEX";
        case EXPRESSION_ASSIGN:           return "ASSIGN";
        case EXPRESSION_LOGICAL:          return "LOGICAL";
        default:                          return "UNKNOWN";
    }
}

ident_t* ident_make(allocator_t *alloc, token_t tok) {
    ident_t *res = allocator_malloc(alloc, sizeof(ident_t));
    if (!res) {
        return NULL;
    }
    res->alloc = alloc;
    res->value = token_duplicate_literal(alloc, &tok);
    if (!res->value) {
        allocator_free(alloc, res);
        return NULL;
    }
    res->pos = tok.pos;
    return res;
}

ident_t* ident_copy(ident_t *ident) {
    ident_t *res = allocator_malloc(ident->alloc, sizeof(ident_t));
    if (!res) {
        return NULL;
    }
    res->alloc = ident->alloc;
    res->value = ape_strdup(ident->alloc, ident->value);
    if (!res->value) {
        allocator_free(ident->alloc, res);
        return NULL;
    }
    res->pos = ident->pos;
    return res;
}

void ident_destroy(ident_t *ident) {
    if (!ident) {
        return;
    }
    allocator_free(ident->alloc, ident->value);
    ident->value = NULL;
    ident->pos = src_pos_invalid;
    allocator_free(ident->alloc, ident);
}

if_case_t *if_case_make(allocator_t *alloc, expression_t *test, code_block_t *consequence) {
    if_case_t *res = allocator_malloc(alloc, sizeof(if_case_t));
    if (!res) {
        return NULL;
    }
    res->alloc = alloc;
    res->test = test;
    res->consequence = consequence;
    return res;
}

void if_case_destroy(if_case_t *cond) {
    if (!cond) {
        return;
    }
    expression_destroy(cond->test);
    code_block_destroy(cond->consequence);
    allocator_free(cond->alloc, cond);
}

if_case_t* if_case_copy(if_case_t *if_case) {
    if (!if_case) {
        return NULL;
    }
    expression_t *test_copy = NULL;
    code_block_t *consequence_copy = NULL;
    if_case_t *if_case_copy = NULL;

    test_copy = expression_copy(if_case->test);
    if (!test_copy) {
        goto err;
    }
    consequence_copy = code_block_copy(if_case->consequence);
    if (!test_copy || !consequence_copy) {
        goto err;
    }
    if_case_copy = if_case_make(if_case->alloc, test_copy, consequence_copy);
    if (!if_case_copy) {
        goto err;
    }
    return if_case_copy;
err:
    expression_destroy(test_copy);
    code_block_destroy(consequence_copy);
    if_case_destroy(if_case_copy);
    return NULL;
}

// INTERNAL
static expression_t *expression_make(allocator_t *alloc, expression_type_t type) {
    expression_t *res = allocator_malloc(alloc, sizeof(expression_t));
    if (!res) {
        return NULL;
    }
    res->alloc = alloc;
    res->type = type;
    res->pos = src_pos_invalid;
    return res;
}

static statement_t* statement_make(allocator_t *alloc, statement_type_t type) {
    statement_t *res = allocator_malloc(alloc, sizeof(statement_t));
    if (!res) {
        return NULL;
    }
    res->alloc = alloc;
    res->type = type;
    res->pos = src_pos_invalid;
    return res;
}
//FILE_END
//FILE_START:parser.c
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#ifndef APE_AMALGAMATED
#include "parser.h"
#include "errors.h"
#endif

typedef enum precedence {
    PRECEDENCE_LOWEST = 0,
    PRECEDENCE_ASSIGN,      // a = b
    PRECEDENCE_LOGICAL_OR,  // ||
    PRECEDENCE_LOGICAL_AND, // &&
    PRECEDENCE_BIT_OR,      // |
    PRECEDENCE_BIT_XOR,     // ^
    PRECEDENCE_BIT_AND,     // &
    PRECEDENCE_EQUALS,      // == !=
    PRECEDENCE_LESSGREATER, // >, >=, <, <=
    PRECEDENCE_SHIFT,       // << >>
    PRECEDENCE_SUM,         // + -
    PRECEDENCE_PRODUCT,     // * / %
    PRECEDENCE_PREFIX,      // -X or !X
    PRECEDENCE_CALL,        // myFunction(X)
    PRECEDENCE_INDEX,       // arr[x]
    PRECEDENCE_DOT,         // obj.foo
    PRECEDENCE_HIGHEST
} precedence_t;

static statement_t* parse_statement(parser_t *p);
static statement_t* parse_define_statement(parser_t *p);
static statement_t* parse_if_statement(parser_t *p);
static statement_t* parse_return_statement(parser_t *p);
static statement_t* parse_expression_statement(parser_t *p);
static statement_t* parse_while_loop_statement(parser_t *p);
static statement_t* parse_break_statement(parser_t *p);
static statement_t* parse_continue_statement(parser_t *p);
static statement_t* parse_for_loop_statement(parser_t *p);
static statement_t* parse_foreach(parser_t *p);
static statement_t* parse_classic_for_loop(parser_t *p);
static statement_t* parse_function_statement(parser_t *p);
static statement_t* parse_block_statement(parser_t *p);
static statement_t* parse_import_statement(parser_t *p);
static statement_t* parse_recover_statement(parser_t *p);

static code_block_t* parse_code_block(parser_t *p);

static expression_t* parse_expression(parser_t *p, precedence_t prec);
static expression_t* parse_identifier(parser_t *p);
static expression_t* parse_number_literal(parser_t *p);
static expression_t* parse_bool_literal(parser_t *p);
static expression_t* parse_string_literal(parser_t *p);
static expression_t* parse_template_string_literal(parser_t *p);
static expression_t* parse_null_literal(parser_t *p);
static expression_t* parse_array_literal(parser_t *p);
static expression_t* parse_map_literal(parser_t *p);
static expression_t* parse_prefix_expression(parser_t *p);
static expression_t* parse_infix_expression(parser_t *p, expression_t *left);
static expression_t* parse_grouped_expression(parser_t *p);
static expression_t* parse_function_literal(parser_t *p);
static bool parse_function_parameters(parser_t *p, ptrarray(ident_t) *out_params);
static expression_t* parse_call_expression(parser_t *p, expression_t *left);
static ptrarray(expression_t)* parse_expression_list(parser_t *p, token_type_t start_token, token_type_t end_token, bool trailing_comma_allowed);
static expression_t* parse_index_expression(parser_t *p, expression_t *left);
static expression_t* parse_dot_expression(parser_t *p, expression_t *left);
static expression_t* parse_assign_expression(parser_t *p, expression_t *left);
static expression_t* parse_logical_expression(parser_t *p, expression_t *left);

static precedence_t get_precedence(token_type_t tk);
static operator_t token_to_operator(token_type_t tk);

static char escape_char(const char c);
static char* process_and_copy_string(allocator_t *alloc, const char *input, size_t len);
static expression_t* wrap_expression_in_function_call(allocator_t *alloc, expression_t *expr, const char *function_name);

parser_t* parser_make(allocator_t *alloc, const ape_config_t *config, errors_t *errors) {
    parser_t *parser = allocator_malloc(alloc, sizeof(parser_t));
    if (!parser) {
        return NULL;
    }
    memset(parser, 0, sizeof(parser_t));

    parser->alloc = alloc;
    parser->config = config;
    parser->errors = errors;

    parser->prefix_parse_fns[TOKEN_IDENT] = parse_identifier;
    parser->prefix_parse_fns[TOKEN_NUMBER] = parse_number_literal;
    parser->prefix_parse_fns[TOKEN_TRUE] = parse_bool_literal;
    parser->prefix_parse_fns[TOKEN_FALSE] = parse_bool_literal;
    parser->prefix_parse_fns[TOKEN_STRING] = parse_string_literal;
    parser->prefix_parse_fns[TOKEN_TEMPLATE_STRING] = parse_template_string_literal;
    parser->prefix_parse_fns[TOKEN_NULL] = parse_null_literal;
    parser->prefix_parse_fns[TOKEN_BANG] = parse_prefix_expression;
    parser->prefix_parse_fns[TOKEN_MINUS] = parse_prefix_expression;
    parser->prefix_parse_fns[TOKEN_LPAREN] = parse_grouped_expression;
    parser->prefix_parse_fns[TOKEN_FUNCTION] = parse_function_literal;
    parser->prefix_parse_fns[TOKEN_LBRACKET] = parse_array_literal;
    parser->prefix_parse_fns[TOKEN_LBRACE] = parse_map_literal;

    parser->infix_parse_fns[TOKEN_PLUS] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_MINUS] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_SLASH] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_ASTERISK] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_PERCENT] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_EQ] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_NOT_EQ] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_LT] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_LTE] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_GT] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_GTE] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_LPAREN] = parse_call_expression;
    parser->infix_parse_fns[TOKEN_LBRACKET] = parse_index_expression;
    parser->infix_parse_fns[TOKEN_ASSIGN] = parse_assign_expression;
    parser->infix_parse_fns[TOKEN_PLUS_ASSIGN] = parse_assign_expression;
    parser->infix_parse_fns[TOKEN_MINUS_ASSIGN] = parse_assign_expression;
    parser->infix_parse_fns[TOKEN_SLASH_ASSIGN] = parse_assign_expression;
    parser->infix_parse_fns[TOKEN_ASTERISK_ASSIGN] = parse_assign_expression;
    parser->infix_parse_fns[TOKEN_PERCENT_ASSIGN] = parse_assign_expression;
    parser->infix_parse_fns[TOKEN_BIT_AND_ASSIGN] = parse_assign_expression;
    parser->infix_parse_fns[TOKEN_BIT_OR_ASSIGN] = parse_assign_expression;
    parser->infix_parse_fns[TOKEN_BIT_XOR_ASSIGN] = parse_assign_expression;
    parser->infix_parse_fns[TOKEN_LSHIFT_ASSIGN] = parse_assign_expression;
    parser->infix_parse_fns[TOKEN_RSHIFT_ASSIGN] = parse_assign_expression;
    parser->infix_parse_fns[TOKEN_DOT] = parse_dot_expression;
    parser->infix_parse_fns[TOKEN_AND] = parse_logical_expression;
    parser->infix_parse_fns[TOKEN_OR] = parse_logical_expression;
    parser->infix_parse_fns[TOKEN_BIT_AND] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_BIT_OR] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_BIT_XOR] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_LSHIFT] = parse_infix_expression;
    parser->infix_parse_fns[TOKEN_RSHIFT] = parse_infix_expression;

    parser->depth = 0;

    return parser;
}

void parser_destroy(parser_t *parser) {
    if (!parser) {
        return;
    }
    allocator_free(parser->alloc, parser);
}

ptrarray(statement_t)* parser_parse_all(parser_t *parser, const char *input, compiled_file_t *file) {
    parser->depth = 0;

    bool ok = lexer_init(&parser->lexer, parser->alloc, parser->errors, input, file);
    if (!ok) {
        return NULL;
    }

    lexer_next_token(&parser->lexer);
    lexer_next_token(&parser->lexer);

    ptrarray(statement_t)* statements = ptrarray_make(parser->alloc);
    if (!statements) {
        return NULL;
    }

    while (!lexer_cur_token_is(&parser->lexer, TOKEN_EOF)) {
        if (lexer_cur_token_is(&parser->lexer, TOKEN_SEMICOLON)) {
            lexer_next_token(&parser->lexer);
            continue;
        }
        statement_t *stmt = parse_statement(parser);
        if (!stmt) {
            goto err;
        }
        bool ok = ptrarray_add(statements, stmt);
        if (!ok) {
            statement_destroy(stmt);
            goto err;
        }
    }

    if (errors_get_count(parser->errors) > 0) {
        goto err;
    }

    return statements;
err:
    ptrarray_destroy_with_items(statements, statement_destroy);
    return NULL;
}

// INTERNAL
static statement_t* parse_statement(parser_t *p) {
    src_pos_t pos = p->lexer.cur_token.pos;

    statement_t *res = NULL;
    switch (p->lexer.cur_token.type) {
        case TOKEN_VAR:
        case TOKEN_CONST: {
            res = parse_define_statement(p);
            break;
        }
        case TOKEN_IF: {
            res = parse_if_statement(p);
            break;
        }
        case TOKEN_RETURN: {
            res = parse_return_statement(p);
            break;
        }
        case TOKEN_WHILE: {
            res = parse_while_loop_statement(p);
            break;
        }
        case TOKEN_BREAK: {
            res = parse_break_statement(p);
            break;
        }
        case TOKEN_FOR: {
            res = parse_for_loop_statement(p);
            break;
        }
        case TOKEN_FUNCTION: {
            if (lexer_peek_token_is(&p->lexer, TOKEN_IDENT)) {
                res = parse_function_statement(p);
            } else {
                res = parse_expression_statement(p);
            }
            break;
        }
        case TOKEN_LBRACE: {
            if (p->config->repl_mode && p->depth == 0) {
                res = parse_expression_statement(p);
            } else {
                res = parse_block_statement(p);
            }
            break;
        }
        case TOKEN_CONTINUE: {
            res = parse_continue_statement(p);
            break;
        }
        case TOKEN_IMPORT: {
            res = parse_import_statement(p);
            break;
        }
        case TOKEN_RECOVER: {
            res = parse_recover_statement(p);
            break;
        }
        default: {
            res = parse_expression_statement(p);
            break;
        }
    }
    if (res) {
        res->pos = pos;
    }
    return res;
}

static statement_t* parse_define_statement(parser_t *p) {
    ident_t *name_ident = NULL;
    expression_t *value = NULL;

    bool assignable = lexer_cur_token_is(&p->lexer, TOKEN_VAR);

    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_IDENT)) {
        goto err;
    }

    name_ident = ident_make(p->alloc, p->lexer.cur_token);
    if (!name_ident) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_ASSIGN)) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    value = parse_expression(p, PRECEDENCE_LOWEST);
    if (!value) {
        goto err;
    }

    if (value->type == EXPRESSION_FUNCTION_LITERAL) {
        value->fn_literal.name = ape_strdup(p->alloc, name_ident->value);
        if (!value->fn_literal.name) {
            goto err;
        }
    }

    statement_t *res = statement_make_define(p->alloc, name_ident, value, assignable);
    if (!res) {
        goto err;
    }
    return res;
err:
    expression_destroy(value);
    ident_destroy(name_ident);
    return NULL;
}

static statement_t* parse_if_statement(parser_t *p) {
    ptrarray(if_case_t) *cases = NULL;
    code_block_t *alternative = NULL;

    cases = ptrarray_make(p->alloc);
    if (!cases) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_LPAREN)) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    if_case_t *cond = if_case_make(p->alloc, NULL, NULL);
    if (!cond) {
        goto err;
    }

    bool ok = ptrarray_add(cases, cond);
    if (!ok) {
        if_case_destroy(cond);
        goto err;
    }

    cond->test = parse_expression(p, PRECEDENCE_LOWEST);
    if (!cond->test) {
        goto err;
    }

    if (!lexer_expect_current(&p->lexer, TOKEN_RPAREN)) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    cond->consequence = parse_code_block(p);
    if (!cond->consequence) {
        goto err;
    }

    while (lexer_cur_token_is(&p->lexer, TOKEN_ELSE)) {
        lexer_next_token(&p->lexer);

        if (lexer_cur_token_is(&p->lexer, TOKEN_IF)) {
            lexer_next_token(&p->lexer);

            if (!lexer_expect_current(&p->lexer, TOKEN_LPAREN)) {
                goto err;
            }

            lexer_next_token(&p->lexer);

            if_case_t *elif = if_case_make(p->alloc, NULL, NULL);
            if (!elif) {
                goto err;
            }

            ok = ptrarray_add(cases, elif);
            if (!ok) {
                if_case_destroy(elif);
                goto err;
            }

            elif->test = parse_expression(p, PRECEDENCE_LOWEST);
            if (!elif->test) {
                goto err;
            }

            if (!lexer_expect_current(&p->lexer, TOKEN_RPAREN)) {
                goto err;
            }

            lexer_next_token(&p->lexer);

            elif->consequence = parse_code_block(p);
            if (!elif->consequence) {
                goto err;
            }
        } else {
            alternative = parse_code_block(p);
            if (!alternative) {
                goto err;
            }
        }
    }

    statement_t *res = statement_make_if(p->alloc, cases, alternative);
    if (!res) {
        goto err;
    }
    return res;
err:
    ptrarray_destroy_with_items(cases, if_case_destroy);
    code_block_destroy(alternative);
    return NULL;
}

static statement_t* parse_return_statement(parser_t *p) {
    expression_t *expr = NULL;

    lexer_next_token(&p->lexer);

    if (!lexer_cur_token_is(&p->lexer, TOKEN_SEMICOLON) && !lexer_cur_token_is(&p->lexer, TOKEN_RBRACE) && !lexer_cur_token_is(&p->lexer, TOKEN_EOF)) {
        expr = parse_expression(p, PRECEDENCE_LOWEST);
        if (!expr) {
            return NULL;
        }
    }

    statement_t *res = statement_make_return(p->alloc, expr);
    if (!res) {
        expression_destroy(expr);
        return NULL;
    }
    return res;
}

static statement_t* parse_expression_statement(parser_t *p) {
    expression_t *expr = parse_expression(p, PRECEDENCE_LOWEST);
    if (!expr) {
        return NULL;
    }

    if (expr && (!p->config->repl_mode || p->depth > 0)) {
        if (expr->type != EXPRESSION_ASSIGN && expr->type != EXPRESSION_CALL) {
            errors_add_errorf(p->errors, ERROR_PARSING, expr->pos,
                              "Only assignments and function calls can be expression statements");
            expression_destroy(expr);
            return NULL;
        }
    }

    statement_t *res = statement_make_expression(p->alloc, expr);
    if (!res) {
        expression_destroy(expr);
        return NULL;
    }
    return res;
}

static statement_t* parse_while_loop_statement(parser_t *p) {
    expression_t *test = NULL;
    code_block_t *body = NULL;

    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_LPAREN)) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    test = parse_expression(p, PRECEDENCE_LOWEST);
    if (!test) {
        goto err;
    }

    if (!lexer_expect_current(&p->lexer, TOKEN_RPAREN)) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    body = parse_code_block(p);
    if (!body) {
        goto err;
    }

    statement_t *res = statement_make_while_loop(p->alloc, test, body);
    if (!res) {
        goto err;
    }
    return res;
err:
    code_block_destroy(body);
    expression_destroy(test);
    return NULL;
}

static statement_t* parse_break_statement(parser_t *p) {
    lexer_next_token(&p->lexer);
    return statement_make_break(p->alloc);
}

static statement_t* parse_continue_statement(parser_t *p) {
    lexer_next_token(&p->lexer);
    return statement_make_continue(p->alloc);
}

static statement_t* parse_block_statement(parser_t *p) {
    code_block_t *block = parse_code_block(p);
    if (!block) {
        return NULL;
    }
    statement_t *res = statement_make_block(p->alloc, block);
    if (!res) {
        code_block_destroy(block);
        return NULL;
    }
    return res;
}

static statement_t* parse_import_statement(parser_t *p) {
    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_STRING)) {
        return NULL;
    }

    char *processed_name = process_and_copy_string(p->alloc, p->lexer.cur_token.literal, p->lexer.cur_token.len);
    if (!processed_name) {
        errors_add_error(p->errors, ERROR_PARSING, p->lexer.cur_token.pos, "Error when parsing module name");
        return NULL;
    }
    lexer_next_token(&p->lexer);

    statement_t *res = statement_make_import(p->alloc, processed_name);
    if (!res) {
        allocator_free(p->alloc, processed_name);
        return NULL;
    }
    return res;
}

static statement_t* parse_recover_statement(parser_t *p) {
    ident_t *error_ident = NULL;
    code_block_t *body = NULL;

    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_LPAREN)) {
        return NULL;
    }
    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_IDENT)) {
        return NULL;
    }

    error_ident = ident_make(p->alloc, p->lexer.cur_token);
    if (!error_ident) {
        return NULL;
    }

    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_RPAREN)) {
        goto err;
    }
    lexer_next_token(&p->lexer);

    body = parse_code_block(p);
    if (!body) {
        goto err;
    }

    statement_t *res = statement_make_recover(p->alloc, error_ident, body);
    if (!res) {
        goto err;
    }
    return res;
err:
    code_block_destroy(body);
    ident_destroy(error_ident);
    return NULL;

}

static statement_t* parse_for_loop_statement(parser_t *p) {
    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_LPAREN)) {
        return NULL;
    }

    lexer_next_token(&p->lexer);

    if (lexer_cur_token_is(&p->lexer, TOKEN_IDENT) && lexer_peek_token_is(&p->lexer, TOKEN_IN)) {
        return parse_foreach(p);
    } else {
        return parse_classic_for_loop(p);
    }
}

static statement_t* parse_foreach(parser_t *p) {
    expression_t *source = NULL;
    code_block_t *body = NULL;
    ident_t *iterator_ident = NULL;

    iterator_ident = ident_make(p->alloc, p->lexer.cur_token);
    if (!iterator_ident) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_IN)) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    source = parse_expression(p, PRECEDENCE_LOWEST);
    if (!source) {
        goto err;
    }

    if (!lexer_expect_current(&p->lexer, TOKEN_RPAREN)) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    body = parse_code_block(p);
    if (!body) {
        goto err;
    }

    statement_t *res = statement_make_foreach(p->alloc, iterator_ident, source, body);
    if (!res) {
        goto err;
    }
    return res;
err:
    code_block_destroy(body);
    ident_destroy(iterator_ident);
    expression_destroy(source);
    return NULL;
}

static statement_t* parse_classic_for_loop(parser_t *p) {
    statement_t *init = NULL;
    expression_t *test = NULL;
    expression_t *update = NULL;
    code_block_t *body = NULL;

    if (!lexer_cur_token_is(&p->lexer, TOKEN_SEMICOLON)) {
        init = parse_statement(p);
        if (!init) {
            goto err;
        }
        if (init->type != STATEMENT_DEFINE && init->type != STATEMENT_EXPRESSION) {
            errors_add_errorf(p->errors, ERROR_PARSING, init->pos,
                              "for loop's init clause should be a define statement or an expression");
            goto err;
        }
        if (!lexer_expect_current(&p->lexer, TOKEN_SEMICOLON)) {
            goto err;
        }
    }

    lexer_next_token(&p->lexer);

    if (!lexer_cur_token_is(&p->lexer, TOKEN_SEMICOLON)) {
        test = parse_expression(p, PRECEDENCE_LOWEST);
        if (!test) {
            goto err;
        }
        if (!lexer_expect_current(&p->lexer, TOKEN_SEMICOLON)) {
            goto err;
        }
    }

    lexer_next_token(&p->lexer);

    if (!lexer_cur_token_is(&p->lexer, TOKEN_RPAREN)) {
        update = parse_expression(p, PRECEDENCE_LOWEST);
        if (!update) {
            goto err;
        }
        if (!lexer_expect_current(&p->lexer, TOKEN_RPAREN)) {
            goto err;
        }
    }

    lexer_next_token(&p->lexer);

    body = parse_code_block(p);
    if (!body) {
        goto err;
    }

    statement_t *res = statement_make_for_loop(p->alloc, init, test, update, body);
    if (!res) {
        goto err;
    }

    return res;
err:
    statement_destroy(init);
    expression_destroy(test);
    expression_destroy(update);
    code_block_destroy(body);
    return NULL;
}

static statement_t* parse_function_statement(parser_t *p) {
    ident_t *name_ident = NULL;
    expression_t* value = NULL;

    src_pos_t pos = p->lexer.cur_token.pos;

    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_IDENT)) {
        goto err;
    }

    name_ident = ident_make(p->alloc, p->lexer.cur_token);
    if (!name_ident) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    value = parse_function_literal(p);
    if (!value) {
        goto err;
    }

    value->pos = pos;
    value->fn_literal.name = ape_strdup(p->alloc, name_ident->value);

    if (!value->fn_literal.name) {
        goto err;
    }

    statement_t *res = statement_make_define(p->alloc, name_ident, value, false);
    if (!res) {
        goto err;
    }
    return res;

err:
    expression_destroy(value);
    ident_destroy(name_ident);
    return NULL;
}

static code_block_t* parse_code_block(parser_t *p) {
    if (!lexer_expect_current(&p->lexer, TOKEN_LBRACE)) {
        return NULL;
    }

    lexer_next_token(&p->lexer);
    p->depth++;

    ptrarray(statement_t)* statements = ptrarray_make(p->alloc);
    if (!statements) {
        goto err;
    }

    while (!lexer_cur_token_is(&p->lexer, TOKEN_RBRACE)) {
        if (lexer_cur_token_is(&p->lexer, TOKEN_EOF)) {
            errors_add_error(p->errors, ERROR_PARSING, p->lexer.cur_token.pos, "Unexpected EOF");
            goto err;
        }
        if (lexer_cur_token_is(&p->lexer, TOKEN_SEMICOLON)) {
            lexer_next_token(&p->lexer);
            continue;
        }
        statement_t *stmt = parse_statement(p);
        if (!stmt) {
            goto err;
        }
        bool ok = ptrarray_add(statements, stmt);
        if (!ok) {
            statement_destroy(stmt);
            goto err;
        }
    }

    lexer_next_token(&p->lexer);

    p->depth--;
    
    code_block_t *res = code_block_make(p->alloc, statements);
    if (!res) {
        goto err;
    }
    return res;

err:
    p->depth--;
    ptrarray_destroy_with_items(statements, statement_destroy);
    return NULL;
}

static expression_t* parse_expression(parser_t *p, precedence_t prec) {
    src_pos_t pos = p->lexer.cur_token.pos;

    if (p->lexer.cur_token.type == TOKEN_INVALID) {
        errors_add_error(p->errors, ERROR_PARSING, p->lexer.cur_token.pos, "Illegal token");
        return NULL;
    }

    prefix_parse_fn prefix = p->prefix_parse_fns[p->lexer.cur_token.type];
    if (!prefix) {
        char *literal = token_duplicate_literal(p->alloc, &p->lexer.cur_token);
        errors_add_errorf(p->errors, ERROR_PARSING, p->lexer.cur_token.pos,
                                  "No prefix parse function for \"%s\" found", literal);
        allocator_free(p->alloc, literal);
        return NULL;
    }

    expression_t *left_expr = prefix(p);
    if (!left_expr) {
        return NULL;
    }
    left_expr->pos = pos;

    while (!lexer_cur_token_is(&p->lexer, TOKEN_SEMICOLON) && prec < get_precedence(p->lexer.cur_token.type)) {
        infix_parse_fn infix = p->infix_parse_fns[p->lexer.cur_token.type];
        if (!infix) {
            return left_expr;
        }
        pos = p->lexer.cur_token.pos;
        expression_t *new_left_expr = infix(p, left_expr);
        if (!new_left_expr) {
            expression_destroy(left_expr);
            return NULL;
        }
        new_left_expr->pos = pos;
        left_expr = new_left_expr;
    }

    return left_expr;
}

static expression_t* parse_identifier(parser_t *p) {
    ident_t *ident = ident_make(p->alloc, p->lexer.cur_token);
    if (!ident) {
        return NULL;
    }
    expression_t *res = expression_make_ident(p->alloc, ident);
    if (!res) {
        ident_destroy(ident);
        return NULL;
    }
    lexer_next_token(&p->lexer);
    return res;
}

static expression_t* parse_number_literal(parser_t *p) {
    char *end;
    double number = 0;
    errno = 0;
    number = strtod(p->lexer.cur_token.literal, &end);
    long parsed_len = end - p->lexer.cur_token.literal;
    if (errno || parsed_len != p->lexer.cur_token.len) {
        char *literal = token_duplicate_literal(p->alloc, &p->lexer.cur_token);
        errors_add_errorf(p->errors, ERROR_PARSING, p->lexer.cur_token.pos,
                          "Parsing number literal \"%s\" failed", literal);
        allocator_free(p->alloc, literal);
        return NULL;
    }
    lexer_next_token(&p->lexer);
    return expression_make_number_literal(p->alloc, number);
}

static expression_t* parse_bool_literal(parser_t *p) {
    expression_t *res = expression_make_bool_literal(p->alloc, p->lexer.cur_token.type == TOKEN_TRUE);
    lexer_next_token(&p->lexer);
    return res;
}

static expression_t* parse_string_literal(parser_t *p) {
    char *processed_literal = process_and_copy_string(p->alloc, p->lexer.cur_token.literal, p->lexer.cur_token.len);
    if (!processed_literal) {
        errors_add_error(p->errors, ERROR_PARSING, p->lexer.cur_token.pos, "Error when parsing string literal");
        return NULL;
    }
    lexer_next_token(&p->lexer);
    expression_t *res = expression_make_string_literal(p->alloc, processed_literal);
    if (!res) {
        allocator_free(p->alloc, processed_literal);
        return NULL;
    }
    return res;
}

static expression_t* parse_template_string_literal(parser_t *p) {
    char *processed_literal = NULL;
    expression_t *left_string_expr = NULL;
    expression_t *template_expr = NULL;
    expression_t *to_str_call_expr = NULL;
    expression_t *left_add_expr = NULL;
    expression_t *right_expr = NULL;
    expression_t *right_add_expr = NULL;

    processed_literal = process_and_copy_string(p->alloc, p->lexer.cur_token.literal, p->lexer.cur_token.len);
    if (!processed_literal) {
        errors_add_error(p->errors, ERROR_PARSING, p->lexer.cur_token.pos, "Error when parsing string literal");
        return NULL;
    }
    lexer_next_token(&p->lexer);

    if (!lexer_expect_current(&p->lexer, TOKEN_LBRACE)) {
        goto err;
    }
    lexer_next_token(&p->lexer);

    src_pos_t pos = p->lexer.cur_token.pos;

    left_string_expr = expression_make_string_literal(p->alloc, processed_literal);
    if (!left_string_expr) {
        goto err;
    }
    left_string_expr->pos = pos;
    processed_literal = NULL;

    pos = p->lexer.cur_token.pos;
    template_expr = parse_expression(p, PRECEDENCE_LOWEST);
    if (!template_expr) {
        goto err;
    }
    template_expr->pos = pos;

    to_str_call_expr = wrap_expression_in_function_call(p->alloc, template_expr, "to_str");
    if (!to_str_call_expr) {
        goto err;
    }
    template_expr = NULL;

    left_add_expr = expression_make_infix(p->alloc, OPERATOR_PLUS, left_string_expr, to_str_call_expr);
    if (!left_add_expr) {
        goto err;
    }
    left_add_expr->pos = pos;
    left_string_expr = NULL;
    to_str_call_expr = NULL;

    if (!lexer_expect_current(&p->lexer, TOKEN_RBRACE)) {
        goto err;
    }
    lexer_previous_token(&p->lexer);
    lexer_continue_template_string(&p->lexer);
    lexer_next_token(&p->lexer);
    lexer_next_token(&p->lexer);

    pos = p->lexer.cur_token.pos;

    right_expr = parse_expression(p, PRECEDENCE_HIGHEST);
    if (!right_expr) {
        goto err;
    }
    right_expr->pos = pos;

    right_add_expr = expression_make_infix(p->alloc, OPERATOR_PLUS, left_add_expr, right_expr);
    if (!right_add_expr) {
        goto err;
    }
    right_add_expr->pos = pos;
    left_add_expr = NULL;
    right_expr = NULL;

    return right_add_expr;
err:
    expression_destroy(right_add_expr);
    expression_destroy(right_expr);
    expression_destroy(left_add_expr);
    expression_destroy(to_str_call_expr);
    expression_destroy(template_expr);
    expression_destroy(left_string_expr);
    allocator_free(p->alloc, processed_literal);
    return NULL;
}

static expression_t* parse_null_literal(parser_t *p) {
    lexer_next_token(&p->lexer);
    return expression_make_null_literal(p->alloc);
}

static expression_t* parse_array_literal(parser_t *p) {
    ptrarray(expression_t) *array = parse_expression_list(p, TOKEN_LBRACKET, TOKEN_RBRACKET, true);
    if (!array) {
        return NULL;
    }
    expression_t *res = expression_make_array_literal(p->alloc, array);
    if (!res) {
        ptrarray_destroy_with_items(array, expression_destroy);
        return NULL;
    }
    return res;
}

static expression_t* parse_map_literal(parser_t *p) {
    ptrarray(expression_t) *keys = ptrarray_make(p->alloc);
    ptrarray(expression_t) *values = ptrarray_make(p->alloc);

    if (!keys || !values) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    while (!lexer_cur_token_is(&p->lexer, TOKEN_RBRACE)) {
        expression_t *key = NULL;
        if (lexer_cur_token_is(&p->lexer, TOKEN_IDENT)) {
            char *str = token_duplicate_literal(p->alloc, &p->lexer.cur_token);
            key = expression_make_string_literal(p->alloc, str);
            if (!key) {
                allocator_free(p->alloc, str);
                goto err;
            }
            key->pos = p->lexer.cur_token.pos;
            lexer_next_token(&p->lexer);
        } else {
            key = parse_expression(p, PRECEDENCE_LOWEST);
            if (!key) {
                goto err;
            }
            switch (key->type) {
                case EXPRESSION_STRING_LITERAL:
                case EXPRESSION_NUMBER_LITERAL:
                case EXPRESSION_BOOL_LITERAL: {
                    break;
                }
                default: {
                    errors_add_errorf(p->errors, ERROR_PARSING, key->pos, "Invalid map literal key type");
                    expression_destroy(key);
                    goto err;
                }
            }
        }

        bool ok = ptrarray_add(keys, key);
        if (!ok) {
            expression_destroy(key);
            goto err;
        }

        if (!lexer_expect_current(&p->lexer, TOKEN_COLON)) {
            goto err;
        }

        lexer_next_token(&p->lexer);

        expression_t *value = parse_expression(p, PRECEDENCE_LOWEST);
        if (!value) {
            goto err;
        }
        ok = ptrarray_add(values, value);
        if (!ok) {
            expression_destroy(value);
            goto err;
        }

        if (lexer_cur_token_is(&p->lexer, TOKEN_RBRACE)) {
            break;
        }

        if (!lexer_expect_current(&p->lexer, TOKEN_COMMA)) {
            goto err;
        }

        lexer_next_token(&p->lexer);
    }

    lexer_next_token(&p->lexer);

    expression_t *res = expression_make_map_literal(p->alloc, keys, values);
    if (!res) {
        goto err;
    }
    return res;
err:
    ptrarray_destroy_with_items(keys, expression_destroy);
    ptrarray_destroy_with_items(values, expression_destroy);
    return NULL;
}

static expression_t* parse_prefix_expression(parser_t *p) {
    operator_t op = token_to_operator(p->lexer.cur_token.type);
    lexer_next_token(&p->lexer);
    expression_t *right = parse_expression(p, PRECEDENCE_PREFIX);
    if (!right) {
        return NULL;
    }
    expression_t *res = expression_make_prefix(p->alloc, op, right);
    if (!res) {
        expression_destroy(right);
        return NULL;
    }
    return res;
}

static expression_t* parse_infix_expression(parser_t *p, expression_t *left) {
    operator_t op = token_to_operator(p->lexer.cur_token.type);
    precedence_t prec = get_precedence(p->lexer.cur_token.type);
    lexer_next_token(&p->lexer);
    expression_t *right = parse_expression(p, prec);
    if (!right) {
        return NULL;
    }
    expression_t *res = expression_make_infix(p->alloc, op, left, right);
    if (!res) {
        expression_destroy(right);
        return NULL;
    }
    return res;
}

static expression_t* parse_grouped_expression(parser_t *p) {
    lexer_next_token(&p->lexer);
    expression_t *expr = parse_expression(p, PRECEDENCE_LOWEST);
    if (!expr || !lexer_expect_current(&p->lexer, TOKEN_RPAREN)) {
        expression_destroy(expr);
        return NULL;
    }
    lexer_next_token(&p->lexer);
    return expr;
}

static expression_t* parse_function_literal(parser_t *p) {
    p->depth += 1;
    ptrarray(ident) *params = NULL;
    code_block_t *body = NULL;

    if (lexer_cur_token_is(&p->lexer, TOKEN_FUNCTION)) {
        lexer_next_token(&p->lexer);
    }

    params = ptrarray_make(p->alloc);

    bool ok = parse_function_parameters(p, params);

    if (!ok) {
        goto err;
    }

    body = parse_code_block(p);
    if (!body) {
        goto err;
    }

    expression_t *res = expression_make_fn_literal(p->alloc, params, body);
    if (!res) {
        goto err;
    }

    p->depth -= 1;

    return res;
err:
    code_block_destroy(body);
    ptrarray_destroy_with_items(params, ident_destroy);
    p->depth -= 1;
    return NULL;
}

static bool parse_function_parameters(parser_t *p, ptrarray(ident_t) *out_params) {
    if (!lexer_expect_current(&p->lexer, TOKEN_LPAREN)) {
        return false;
    }

    lexer_next_token(&p->lexer);

    if (lexer_cur_token_is(&p->lexer, TOKEN_RPAREN)) {
        lexer_next_token(&p->lexer);
        return true;
    }

    if (!lexer_expect_current(&p->lexer, TOKEN_IDENT)) {
        return false;
    }

    ident_t *ident = ident_make(p->alloc, p->lexer.cur_token);
    if (!ident) {
        return false;
    }
    
    bool ok = ptrarray_add(out_params, ident);
    if (!ok) {
        ident_destroy(ident);
        return false;
    }

    lexer_next_token(&p->lexer);

    while (lexer_cur_token_is(&p->lexer, TOKEN_COMMA)) {
        lexer_next_token(&p->lexer);

        if (!lexer_expect_current(&p->lexer, TOKEN_IDENT)) {
            return false;
        }

        ident_t *ident = ident_make(p->alloc, p->lexer.cur_token);
        if (!ident) {
            return false;
        }
        bool ok = ptrarray_add(out_params, ident);
        if (!ok) {
            ident_destroy(ident);
            return false;
        }

        lexer_next_token(&p->lexer);
    }

    if (!lexer_expect_current(&p->lexer, TOKEN_RPAREN)) {
        return false;
    }

    lexer_next_token(&p->lexer);

    return true;
}

static expression_t* parse_call_expression(parser_t *p, expression_t *left) {
    expression_t *function = left;
    ptrarray(expression_t) *args = parse_expression_list(p, TOKEN_LPAREN, TOKEN_RPAREN, false);
    if (!args) {
        return NULL;
    }
    expression_t *res = expression_make_call(p->alloc, function, args);
    if (!res) {
        ptrarray_destroy_with_items(args, expression_destroy);
        return NULL;
    }
    return res;
}

static ptrarray(expression_t)* parse_expression_list(parser_t *p, token_type_t start_token, token_type_t end_token, bool trailing_comma_allowed) {
    if (!lexer_expect_current(&p->lexer, start_token)) {
        return NULL;
    }

    lexer_next_token(&p->lexer);

    ptrarray(expression_t)* res = ptrarray_make(p->alloc);

    if (lexer_cur_token_is(&p->lexer, end_token)) {
        lexer_next_token(&p->lexer);
        return res;
    }

    expression_t *arg_expr = parse_expression(p, PRECEDENCE_LOWEST);
    if (!arg_expr) {
        goto err;
    }
    bool ok = ptrarray_add(res, arg_expr);
    if (!ok) {
        expression_destroy(arg_expr);
        goto err;
    }

    while (lexer_cur_token_is(&p->lexer, TOKEN_COMMA)) {
        lexer_next_token(&p->lexer);

        if (trailing_comma_allowed && lexer_cur_token_is(&p->lexer, end_token)) {
            break;
        }

        arg_expr = parse_expression(p, PRECEDENCE_LOWEST);
        if (!arg_expr) {
            goto err;
        }

        bool ok = ptrarray_add(res, arg_expr);
        if (!ok) {
            expression_destroy(arg_expr);
            goto err;
        }
    }

    if (!lexer_expect_current(&p->lexer, end_token)) {
        goto err;
    }

    lexer_next_token(&p->lexer);

    return res;
err:
    ptrarray_destroy_with_items(res, expression_destroy);
    return NULL;
}

static expression_t* parse_index_expression(parser_t *p, expression_t *left) {
    lexer_next_token(&p->lexer);

    expression_t *index = parse_expression(p, PRECEDENCE_LOWEST);
    if (!index) {
        return NULL;
    }

    if (!lexer_expect_current(&p->lexer, TOKEN_RBRACKET)) {
        expression_destroy(index);
        return NULL;
    }

    lexer_next_token(&p->lexer);

    expression_t *res = expression_make_index(p->alloc, left, index);
    if (!res) {
        expression_destroy(index);
        return NULL;
    }

    return res;
}

static expression_t* parse_assign_expression(parser_t *p, expression_t *left) {
    expression_t *source = NULL;
    token_type_t assign_type = p->lexer.cur_token.type;

    lexer_next_token(&p->lexer);

    source = parse_expression(p, PRECEDENCE_LOWEST);
    if (!source) {
        goto err;
    }

    switch (assign_type) {
        case TOKEN_PLUS_ASSIGN:
        case TOKEN_MINUS_ASSIGN:
        case TOKEN_SLASH_ASSIGN:
        case TOKEN_ASTERISK_ASSIGN:
        case TOKEN_PERCENT_ASSIGN:
        case TOKEN_BIT_AND_ASSIGN:
        case TOKEN_BIT_OR_ASSIGN:
        case TOKEN_BIT_XOR_ASSIGN:
        case TOKEN_LSHIFT_ASSIGN:
        case TOKEN_RSHIFT_ASSIGN:
        {
            operator_t op = token_to_operator(assign_type);
            expression_t *left_copy = expression_copy(left);
            if (!left_copy) {
                goto err;
            }
            src_pos_t pos = source->pos;
            expression_t *new_source = expression_make_infix(p->alloc, op, left_copy, source);
            if (!new_source) {
                expression_destroy(left_copy);
                goto err;
            }
            new_source->pos = pos;
            source = new_source;
            break;
        }
        case TOKEN_ASSIGN: break;
        default: APE_ASSERT(false); break;
    }

    expression_t *res = expression_make_assign(p->alloc, left, source);
    if (!res) {
        goto err;
    }
    return res;
err:
    expression_destroy(source);
    return NULL;
}

static expression_t* parse_logical_expression(parser_t *p, expression_t *left) {
    operator_t op = token_to_operator(p->lexer.cur_token.type);
    precedence_t prec = get_precedence(p->lexer.cur_token.type);
    lexer_next_token(&p->lexer);
    expression_t *right = parse_expression(p, prec);
    if (!right) {
        return NULL;
    }
    expression_t *res = expression_make_logical(p->alloc, op, left, right);
    if (!res) {
        expression_destroy(right);
        return NULL;
    }
    return res;
}

static expression_t* parse_dot_expression(parser_t *p, expression_t *left) {
    lexer_next_token(&p->lexer);
    
    if (!lexer_expect_current(&p->lexer, TOKEN_IDENT)) {
        return NULL;
    }

    char *str = token_duplicate_literal(p->alloc, &p->lexer.cur_token);
    expression_t *index = expression_make_string_literal(p->alloc, str);
    if (!index) {
        allocator_free(p->alloc, str);
        return NULL;
    }
    index->pos = p->lexer.cur_token.pos;

    lexer_next_token(&p->lexer);

    expression_t *res = expression_make_index(p->alloc, left, index);
    if (!res) {
        expression_destroy(index);
        return NULL;
    }
    return res;
}

static precedence_t get_precedence(token_type_t tk) {
    switch (tk) {
        case TOKEN_EQ:              return PRECEDENCE_EQUALS;
        case TOKEN_NOT_EQ:          return PRECEDENCE_EQUALS;
        case TOKEN_LT:              return PRECEDENCE_LESSGREATER;
        case TOKEN_LTE:             return PRECEDENCE_LESSGREATER;
        case TOKEN_GT:              return PRECEDENCE_LESSGREATER;
        case TOKEN_GTE:             return PRECEDENCE_LESSGREATER;
        case TOKEN_PLUS:            return PRECEDENCE_SUM;
        case TOKEN_MINUS:           return PRECEDENCE_SUM;
        case TOKEN_SLASH:           return PRECEDENCE_PRODUCT;
        case TOKEN_ASTERISK:        return PRECEDENCE_PRODUCT;
        case TOKEN_PERCENT:         return PRECEDENCE_PRODUCT;
        case TOKEN_LPAREN:          return PRECEDENCE_CALL;
        case TOKEN_LBRACKET:        return PRECEDENCE_INDEX;
        case TOKEN_ASSIGN:          return PRECEDENCE_ASSIGN;
        case TOKEN_PLUS_ASSIGN:     return PRECEDENCE_ASSIGN;
        case TOKEN_MINUS_ASSIGN:    return PRECEDENCE_ASSIGN;
        case TOKEN_ASTERISK_ASSIGN: return PRECEDENCE_ASSIGN;
        case TOKEN_SLASH_ASSIGN:    return PRECEDENCE_ASSIGN;
        case TOKEN_PERCENT_ASSIGN:  return PRECEDENCE_ASSIGN;
        case TOKEN_BIT_AND_ASSIGN:  return PRECEDENCE_ASSIGN;
        case TOKEN_BIT_OR_ASSIGN:   return PRECEDENCE_ASSIGN;
        case TOKEN_BIT_XOR_ASSIGN:  return PRECEDENCE_ASSIGN;
        case TOKEN_LSHIFT_ASSIGN:   return PRECEDENCE_ASSIGN;
        case TOKEN_RSHIFT_ASSIGN:   return PRECEDENCE_ASSIGN;
        case TOKEN_DOT:             return PRECEDENCE_DOT;
        case TOKEN_AND:             return PRECEDENCE_LOGICAL_AND;
        case TOKEN_OR:              return PRECEDENCE_LOGICAL_OR;
        case TOKEN_BIT_OR:          return PRECEDENCE_BIT_OR;
        case TOKEN_BIT_XOR:         return PRECEDENCE_BIT_XOR;
        case TOKEN_BIT_AND:         return PRECEDENCE_BIT_AND;
        case TOKEN_LSHIFT:          return PRECEDENCE_SHIFT;
        case TOKEN_RSHIFT:          return PRECEDENCE_SHIFT;
        default:                    return PRECEDENCE_LOWEST;
    }
}

static operator_t token_to_operator(token_type_t tk) {
    switch (tk) {
        case TOKEN_ASSIGN:          return OPERATOR_ASSIGN;
        case TOKEN_PLUS:            return OPERATOR_PLUS;
        case TOKEN_MINUS:           return OPERATOR_MINUS;
        case TOKEN_BANG:            return OPERATOR_BANG;
        case TOKEN_ASTERISK:        return OPERATOR_ASTERISK;
        case TOKEN_SLASH:           return OPERATOR_SLASH;
        case TOKEN_LT:              return OPERATOR_LT;
        case TOKEN_LTE:             return OPERATOR_LTE;
        case TOKEN_GT:              return OPERATOR_GT;
        case TOKEN_GTE:             return OPERATOR_GTE;
        case TOKEN_EQ:              return OPERATOR_EQ;
        case TOKEN_NOT_EQ:          return OPERATOR_NOT_EQ;
        case TOKEN_PERCENT:         return OPERATOR_MODULUS;
        case TOKEN_AND:             return OPERATOR_LOGICAL_AND;
        case TOKEN_OR:              return OPERATOR_LOGICAL_OR;
        case TOKEN_PLUS_ASSIGN:     return OPERATOR_PLUS;
        case TOKEN_MINUS_ASSIGN:    return OPERATOR_MINUS;
        case TOKEN_ASTERISK_ASSIGN: return OPERATOR_ASTERISK;
        case TOKEN_SLASH_ASSIGN:    return OPERATOR_SLASH;
        case TOKEN_PERCENT_ASSIGN:  return OPERATOR_MODULUS;
        case TOKEN_BIT_AND_ASSIGN:  return OPERATOR_BIT_AND;
        case TOKEN_BIT_OR_ASSIGN:   return OPERATOR_BIT_OR;
        case TOKEN_BIT_XOR_ASSIGN:  return OPERATOR_BIT_XOR;
        case TOKEN_LSHIFT_ASSIGN:   return OPERATOR_LSHIFT;
        case TOKEN_RSHIFT_ASSIGN:   return OPERATOR_RSHIFT;
        case TOKEN_BIT_AND:         return OPERATOR_BIT_AND;
        case TOKEN_BIT_OR:          return OPERATOR_BIT_OR;
        case TOKEN_BIT_XOR:         return OPERATOR_BIT_XOR;
        case TOKEN_LSHIFT:          return OPERATOR_LSHIFT;
        case TOKEN_RSHIFT:          return OPERATOR_RSHIFT;
        default: {
            APE_ASSERT(false);
            return OPERATOR_NONE;
        }
    }
}

static char escape_char(const char c) {
    switch (c) {
        case '\"': return '\"';
        case '\\': return '\\';
        case '/':  return '/';
        case 'b':  return '\b';
        case 'f':  return '\f';
        case 'n':  return '\n';
        case 'r':  return '\r';
        case 't':  return '\t';
        case '0':  return '\0';
        default: return c;
    }
}

static char* process_and_copy_string(allocator_t *alloc, const char *input, size_t len) {
    char *output = allocator_malloc(alloc, len + 1);
    if (!output) {
        return NULL;
    }
    
    size_t in_i = 0;
    size_t out_i = 0;

    while (input[in_i] != '\0' && in_i < len) {
        if (input[in_i] == '\\') {
            in_i++;
            output[out_i] = escape_char(input[in_i]);
            if (output[out_i] < 0) {
                goto error;
            }
        } else {
            output[out_i] = input[in_i];
        }
        out_i++;
        in_i++;
    }
    output[out_i] = '\0';
    return output;
error:
    allocator_free(alloc, output);
    return NULL;
}

static expression_t* wrap_expression_in_function_call(allocator_t *alloc, expression_t *expr, const char *function_name) {
    token_t fn_token;
    token_init(&fn_token, TOKEN_IDENT, function_name, (int)strlen(function_name));
    fn_token.pos = expr->pos;

    ident_t *ident = ident_make(alloc, fn_token);
    if (!ident) {
        return NULL;
    }
    ident->pos = fn_token.pos;

    expression_t *function_ident_expr = expression_make_ident(alloc, ident);;
    if (!function_ident_expr) {
        ident_destroy(ident);
        return NULL;
    }
    function_ident_expr->pos = expr->pos;
    ident = NULL;

    ptrarray(expression_t) *args = ptrarray_make(alloc);
    if (!args) {
        expression_destroy(function_ident_expr);
        return NULL;
    }

    bool ok = ptrarray_add(args, expr);
    if (!ok) {
        ptrarray_destroy(args);
        expression_destroy(function_ident_expr);
        return NULL;
    }

    expression_t *call_expr = expression_make_call(alloc, function_ident_expr, args);
    if (!call_expr) {
        ptrarray_destroy(args);
        expression_destroy(function_ident_expr);
        return NULL;
    }
    call_expr->pos = expr->pos;

    return call_expr;
}
//FILE_END
//FILE_START:global_store.c
#ifndef APE_AMALGAMATED
#include "global_store.h"

#include "symbol_table.h"
#include "builtins.h"
#endif

typedef struct global_store {
    allocator_t *alloc;
    dict(symbol_t) *symbols;
    array(object_t) *objects;
} global_store_t;

global_store_t *global_store_make(allocator_t *alloc, gcmem_t *mem) {
    global_store_t *store = allocator_malloc(alloc, sizeof(global_store_t));
    if (!store) {
        return NULL;
    }
    memset(store, 0, sizeof(global_store_t));
    store->alloc = alloc;
    store->symbols = dict_make(alloc, symbol_copy, symbol_destroy);
    if (!store->symbols) {
        goto err;
    }
    store->objects = array_make(alloc, object_t);
    if (!store->objects) {
        goto err;
    }

    if (mem) {
        for (int i = 0; i < builtins_count(); i++) {
            const char *name = builtins_get_name(i);
            object_t builtin = object_make_native_function(mem, name, builtins_get_fn(i), NULL, 0);
            if (object_is_null(builtin)) {
                goto err;
            }
            bool ok = global_store_set(store, name, builtin);
            if (!ok) {
                goto err;
            }
        }
    }

    return store;
err:
    global_store_destroy(store);
    return NULL;
}

void global_store_destroy(global_store_t *store) {
    if (!store) {
        return;
    }
    dict_destroy_with_items(store->symbols);
    array_destroy(store->objects);
    allocator_free(store->alloc, store);
}

const symbol_t* global_store_get_symbol(global_store_t *store, const char *name) {
    return dict_get(store->symbols, name);
}

object_t global_store_get_object(global_store_t *store, const char *name) {
    const symbol_t *symbol = global_store_get_symbol(store, name);
    if (!symbol) {
        return object_make_null();
    }
    APE_ASSERT(symbol->type == SYMBOL_APE_GLOBAL);
    object_t *res = array_get(store->objects, symbol->index);
    return *res;
}

bool global_store_set(global_store_t *store, const char *name, object_t object) {
    const symbol_t *existing_symbol = global_store_get_symbol(store, name);
    if (existing_symbol) {
        bool ok = array_set(store->objects, existing_symbol->index, &object);
        return ok;
    }
    int ix = array_count(store->objects);
    bool ok = array_add(store->objects, &object);
    if (!ok) {
        return false;
    }
    symbol_t *symbol = symbol_make(store->alloc, name, SYMBOL_APE_GLOBAL, ix, false);
    if (!symbol) {
        goto err;
    }
    ok = dict_set(store->symbols, name, symbol);
    if (!ok) {
        symbol_destroy(symbol);
        goto err;
    }
    return true;
err:
    array_pop(store->objects, NULL);
    return false;
}

object_t global_store_get_object_at(global_store_t *store, int ix, bool *out_ok) {
    object_t *res = array_get(store->objects, ix);
    if (!res) {
        *out_ok = false;
        return object_make_null();
    }
    *out_ok = true;
    return *res;
}

bool global_store_set_object_at(global_store_t *store, int ix, object_t object) {
    bool ok = array_set(store->objects, ix, &object);
    return ok;
}

object_t *global_store_get_object_data(global_store_t *store) {
    return array_data(store->objects);
}

int global_store_get_object_count(global_store_t *store) {
    return array_count(store->objects);
}
//FILE_END
//FILE_START:symbol_table.c
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef APE_AMALGAMATED
#include "symbol_table.h"
#include "global_store.h"
#endif

static block_scope_t* block_scope_copy(block_scope_t *scope);
static block_scope_t* block_scope_make(allocator_t *alloc, int offset);
static void block_scope_destroy(block_scope_t *scope);
static bool set_symbol(symbol_table_t *table, symbol_t *symbol);
static int next_symbol_index(symbol_table_t *table);
static int count_num_definitions(symbol_table_t *table);

symbol_t *symbol_make(allocator_t *alloc, const char *name, symbol_type_t type, int index, bool assignable) {
    symbol_t *symbol = allocator_malloc(alloc, sizeof(symbol_t));
    if (!symbol) {
        return NULL;
    }
    memset(symbol, 0, sizeof(symbol_t));
    symbol->alloc = alloc;
    symbol->name = ape_strdup(alloc, name);
    if (!symbol->name) {
        allocator_free(alloc, symbol);
        return NULL;
    }
    symbol->type = type;
    symbol->index = index;
    symbol->assignable = assignable;
    return symbol;
}

void symbol_destroy(symbol_t *symbol) {
    if (!symbol) {
        return;
    }
    allocator_free(symbol->alloc, symbol->name);
    allocator_free(symbol->alloc, symbol);
}

symbol_t* symbol_copy(symbol_t *symbol) {
    return symbol_make(symbol->alloc, symbol->name, symbol->type, symbol->index, symbol->assignable);
}

symbol_table_t *symbol_table_make(allocator_t *alloc, symbol_table_t *outer, global_store_t *global_store, int module_global_offset) {
    symbol_table_t *table = allocator_malloc(alloc, sizeof(symbol_table_t));
    if (!table) {
        return NULL;
    }
    memset(table, 0, sizeof(symbol_table_t));
    table->alloc = alloc;
    table->max_num_definitions = 0;
    table->outer = outer;
    table->global_store = global_store;
    table->module_global_offset = module_global_offset;

    table->block_scopes = ptrarray_make(alloc);
    if (!table->block_scopes) {
        goto err;
    }

    table->free_symbols = ptrarray_make(alloc);
    if (!table->free_symbols) {
        goto err;
    }

    table->module_global_symbols = ptrarray_make(alloc);
    if (!table->module_global_symbols) {
        goto err;
    }

    bool ok = symbol_table_push_block_scope(table);
    if (!ok) {
        goto err;
    }

    return table;
err:
    symbol_table_destroy(table);
    return NULL;
}

void symbol_table_destroy(symbol_table_t *table) {
    if (!table) {
        return;
    }

    while (ptrarray_count(table->block_scopes) > 0) {
        symbol_table_pop_block_scope(table);
    }
    ptrarray_destroy(table->block_scopes);
    ptrarray_destroy_with_items(table->module_global_symbols, symbol_destroy);
    ptrarray_destroy_with_items(table->free_symbols, symbol_destroy);
    allocator_t *alloc = table->alloc;
    memset(table, 0, sizeof(symbol_table_t));
    allocator_free(alloc, table);
}

symbol_table_t* symbol_table_copy(symbol_table_t *table) {
    symbol_table_t *copy = allocator_malloc(table->alloc, sizeof(symbol_table_t));
    if (!copy) {
        return NULL;
    }
    memset(copy, 0, sizeof(symbol_table_t));
    copy->alloc = table->alloc;
    copy->outer = table->outer;
    copy->global_store = table->global_store;
    copy->block_scopes = ptrarray_copy_with_items(table->block_scopes, block_scope_copy, block_scope_destroy);
    if (!copy->block_scopes) {
        goto err;
    }
    copy->free_symbols = ptrarray_copy_with_items(table->free_symbols, symbol_copy, symbol_destroy);
    if (!copy->free_symbols) {
        goto err;
    }
    copy->module_global_symbols = ptrarray_copy_with_items(table->module_global_symbols, symbol_copy, symbol_destroy);
    if (!copy->module_global_symbols) {
        goto err;
    }
    copy->max_num_definitions = table->max_num_definitions;
    copy->module_global_offset = table->module_global_offset;
    return copy;
err:
    symbol_table_destroy(copy);
    return NULL;
}

bool symbol_table_add_module_symbol(symbol_table_t *st, symbol_t *symbol) {
    if (symbol->type != SYMBOL_MODULE_GLOBAL) {
        APE_ASSERT(false);
        return false;
    }
    if (symbol_table_symbol_is_defined(st, symbol->name)) {
        return true; // todo: make sure it should be true in this case
    }
    symbol_t *copy = symbol_copy(symbol);
    if (!copy) {
        return false;
    }
    bool ok = set_symbol(st, copy);
    if (!ok) {
        symbol_destroy(copy);
        return false;
    }
    return true;
}

const symbol_t *symbol_table_define(symbol_table_t *table, const char *name, bool assignable) {
    const symbol_t *global_symbol = global_store_get_symbol(table->global_store, name);
    if (global_symbol) {
        return NULL;
    }

    if (strchr(name, ':')) {
        return NULL; // module symbol
    }
    if (APE_STREQ(name, "this")) {
        return NULL; // "this" is reserved
    }
    symbol_type_t symbol_type = table->outer == NULL ? SYMBOL_MODULE_GLOBAL : SYMBOL_LOCAL;
    int ix = next_symbol_index(table);
    symbol_t *symbol = symbol_make(table->alloc, name, symbol_type, ix, assignable);
    if (!symbol) {
        return NULL;
    }

    bool global_symbol_added = false;
    bool ok = false;

    if (symbol_type == SYMBOL_MODULE_GLOBAL && ptrarray_count(table->block_scopes) == 1) {
        symbol_t *global_symbol_copy = symbol_copy(symbol);
        if (!global_symbol_copy) {
            symbol_destroy(symbol);
            return NULL;
        }
        ok = ptrarray_add(table->module_global_symbols, global_symbol_copy);
        if (!ok) {
            symbol_destroy(global_symbol_copy);
            symbol_destroy(symbol);
            return NULL;
        }
        global_symbol_added = true;
    }

    ok = set_symbol(table, symbol);
    if (!ok) {
        if (global_symbol_added) {
            symbol_t *global_symbol_copy = ptrarray_pop(table->module_global_symbols);
            symbol_destroy(global_symbol_copy);
        }
        symbol_destroy(symbol);
        return NULL;
    }

    block_scope_t *top_scope = ptrarray_top(table->block_scopes);
    top_scope->num_definitions++;
    int definitions_count = count_num_definitions(table);
    if (definitions_count > table->max_num_definitions) {
        table->max_num_definitions = definitions_count;
    }

    return symbol;
}

const symbol_t *symbol_table_define_free(symbol_table_t *st, const symbol_t *original) {
    symbol_t *copy = symbol_make(st->alloc, original->name, original->type, original->index, original->assignable);
    if (!copy) {
        return NULL;
    }
    bool ok = ptrarray_add(st->free_symbols, copy);
    if (!ok) {
        symbol_destroy(copy);
        return NULL;
    }

    symbol_t *symbol = symbol_make(st->alloc, original->name, SYMBOL_FREE, ptrarray_count(st->free_symbols) - 1, original->assignable);
    if (!symbol) {
        return NULL;
    }

    ok = set_symbol(st, symbol);
    if (!ok) {
        symbol_destroy(symbol);
        return NULL;
    }

    return symbol;
}

const symbol_t * symbol_table_define_function_name(symbol_table_t *st, const char *name, bool assignable) {
    if (strchr(name, ':')) {
        return NULL; // module symbol
    }
    symbol_t *symbol = symbol_make(st->alloc, name, SYMBOL_FUNCTION, 0, assignable);
    if (!symbol) {
        return NULL;
    }

    bool ok = set_symbol(st, symbol);
    if (!ok) {
        symbol_destroy(symbol);
        return NULL;
    }

    return symbol;
}

const symbol_t *symbol_table_define_this(symbol_table_t *st) {
    symbol_t *symbol = symbol_make(st->alloc, "this", SYMBOL_THIS, 0, false);
    if (!symbol) {
        return NULL;
    }

    bool ok = set_symbol(st, symbol);
    if (!ok) {
        symbol_destroy(symbol);
        return NULL;
    }

    return symbol;
}

const symbol_t *symbol_table_resolve(symbol_table_t *table, const char *name) {
    const symbol_t *symbol = NULL;
    block_scope_t *scope = NULL;

    symbol = global_store_get_symbol(table->global_store, name);
    if (symbol) {
        return symbol;
    }

    for (int i = ptrarray_count(table->block_scopes) - 1; i >= 0; i--) {
        scope = ptrarray_get(table->block_scopes, i);
        symbol = dict_get(scope->store, name);
        if (symbol) {
            break;
        }
    }

    if (symbol && symbol->type == SYMBOL_THIS) {
        symbol = symbol_table_define_free(table, symbol);
    }

    if (!symbol && table->outer) {
        symbol = symbol_table_resolve(table->outer, name);
        if (!symbol) {
            return NULL;
        }
        if (symbol->type == SYMBOL_MODULE_GLOBAL || symbol->type == SYMBOL_APE_GLOBAL) {
            return symbol;
        }
        symbol = symbol_table_define_free(table, symbol);
    }
    return symbol;
}

bool symbol_table_symbol_is_defined(symbol_table_t *table, const char *name) { // todo: rename to something more obvious
    const symbol_t *symbol = global_store_get_symbol(table->global_store, name);
    if (symbol) {
        return true;
    }

    block_scope_t *top_scope = ptrarray_top(table->block_scopes);
    symbol = dict_get(top_scope->store, name);
    if (symbol) {
        return true;
    }
    return false;
}

bool symbol_table_push_block_scope(symbol_table_t *table) {

    int block_scope_offset = 0;
    block_scope_t *prev_block_scope = ptrarray_top(table->block_scopes);
    if (prev_block_scope) {
        block_scope_offset = table->module_global_offset + prev_block_scope->offset + prev_block_scope->num_definitions;
    } else {
        block_scope_offset = table->module_global_offset;
    }

    block_scope_t *new_scope = block_scope_make(table->alloc, block_scope_offset);
    if (!new_scope) {
        return false;
    }
    bool ok = ptrarray_push(table->block_scopes, new_scope);
    if (!ok) {
        block_scope_destroy(new_scope);
        return false;
    }
    return true;
}

void symbol_table_pop_block_scope(symbol_table_t *table) {
    block_scope_t *top_scope = ptrarray_top(table->block_scopes);
    ptrarray_pop(table->block_scopes);
    block_scope_destroy(top_scope);
}

block_scope_t* symbol_table_get_block_scope(symbol_table_t *table) {
    block_scope_t *top_scope = ptrarray_top(table->block_scopes);
    return top_scope;
}

bool symbol_table_is_module_global_scope(symbol_table_t *table) {
    return table->outer == NULL;
}

bool symbol_table_is_top_block_scope(symbol_table_t *table) {
    return ptrarray_count(table->block_scopes) == 1;
}

bool symbol_table_is_top_global_scope(symbol_table_t *table) {
    return symbol_table_is_module_global_scope(table) && symbol_table_is_top_block_scope(table);
}

int symbol_table_get_module_global_symbol_count(const symbol_table_t *table) {
    return ptrarray_count(table->module_global_symbols);
}

const symbol_t * symbol_table_get_module_global_symbol_at(const symbol_table_t *table, int ix) {
    return ptrarray_get(table->module_global_symbols, ix);
}

// INTERNAL
static block_scope_t* block_scope_make(allocator_t *alloc, int offset) {
    block_scope_t *new_scope = allocator_malloc(alloc, sizeof(block_scope_t));
    if (!new_scope) {
        return NULL;
    }
    memset(new_scope, 0, sizeof(block_scope_t));
    new_scope->alloc = alloc;
    new_scope->store = dict_make(alloc, symbol_copy, symbol_destroy);
    if (!new_scope->store) {
        block_scope_destroy(new_scope);
        return NULL;
    }
    new_scope->num_definitions = 0;
    new_scope->offset = offset;
    return new_scope;
}

static void block_scope_destroy(block_scope_t *scope) {
    dict_destroy_with_items(scope->store);
    allocator_free(scope->alloc, scope);
}

static block_scope_t* block_scope_copy(block_scope_t *scope) {
    block_scope_t *copy = allocator_malloc(scope->alloc, sizeof(block_scope_t));
    if (!copy) {
        return NULL;
    }
    memset(copy, 0, sizeof(block_scope_t));
    copy->alloc = scope->alloc;
    copy->num_definitions = scope->num_definitions;
    copy->offset = scope->offset;
    copy->store = dict_copy_with_items(scope->store);
    if (!copy->store) {
        block_scope_destroy(copy);
        return NULL;
    }
    return copy;
}

static bool set_symbol(symbol_table_t *table, symbol_t *symbol) {
    block_scope_t *top_scope = ptrarray_top(table->block_scopes);
    symbol_t *existing = dict_get(top_scope->store, symbol->name);
    if (existing) {
        symbol_destroy(existing);
    }
    return dict_set(top_scope->store, symbol->name, symbol);
}

static int next_symbol_index(symbol_table_t *table) {
    block_scope_t *top_scope = ptrarray_top(table->block_scopes);
    int ix = top_scope->offset + top_scope->num_definitions;
    return ix;
}

static int count_num_definitions(symbol_table_t *table) {
    int count = 0;
    for (int i = ptrarray_count(table->block_scopes) - 1; i >= 0; i--) {
        block_scope_t *scope = ptrarray_get(table->block_scopes, i);
        count += scope->num_definitions;
    }
    return count;
}
//FILE_END
//FILE_START:code.c
#include <stdlib.h>

#ifndef APE_AMALGAMATED
#include "code.h"

#include "common.h"
#include "collections.h"
#endif

static opcode_definition_t g_definitions[OPCODE_MAX + 1] = {
    {"NONE", 0, {0}},
    {"CONSTANT", 1, {2}},
    {"ADD", 0, {0}},
    {"POP", 0, {0}},
    {"SUB", 0, {0}},
    {"MUL", 0, {0}},
    {"DIV", 0, {0}},
    {"MOD", 0, {0}},
    {"TRUE", 0, {0}},
    {"FALSE", 0, {0}},
    {"COMPARE", 0, {0}},
    {"EQUAL", 0, {0}},
    {"NOT_EQUAL", 0, {0}},
    {"GREATER_THAN", 0, {0}},
    {"GREATER_THAN_EQUAL", 0, {0}},
    {"MINUS", 0, {0}},
    {"BANG", 0, {0}},
    {"JUMP", 1, {2}},
    {"JUMP_IF_FALSE", 1, {2}},
    {"JUMP_IF_TRUE", 1, {2}},
    {"NULL", 0, {0}},
    {"GET_MODULE_GLOBAL", 1, {2}},
    {"SET_MODULE_GLOBAL", 1, {2}},
    {"DEFINE_MODULE_GLOBAL", 1, {2}},
    {"ARRAY", 1, {2}},
    {"MAP_START", 1, {2}},
    {"MAP_END", 1, {2}},
    {"GET_THIS", 0, {0}},
    {"GET_INDEX", 0, {0}},
    {"SET_INDEX", 0, {0}},
    {"GET_VALUE_AT", 0, {0}},
    {"CALL", 1, {1}},
    {"RETURN_VALUE", 0, {0}},
    {"RETURN", 0, {0}},
    {"GET_LOCAL", 1, {1}},
    {"DEFINE_LOCAL", 1, {1}},
    {"SET_LOCAL", 1, {1}},
    {"GET_APE_GLOBAL", 1, {2}},
    {"FUNCTION", 2, {2, 1}},
    {"GET_FREE", 1, {1}},
    {"SET_FREE", 1, {1}},
    {"CURRENT_FUNCTION", 0, {0}},
    {"DUP", 0, {0}},
    {"NUMBER", 1, {8}},
    {"LEN", 0, {0}},
    {"SET_RECOVER", 1, {2}},
    {"OR", 0, {0}},
    {"XOR", 0, {0}},
    {"AND", 0, {0}},
    {"LSHIFT", 0, {0}},
    {"RSHIFT", 0, {0}},
    {"INVALID_MAX", 0, {0}},
};

opcode_definition_t* opcode_lookup(opcode_t op) {
    if (op <= OPCODE_NONE || op >= OPCODE_MAX) {
        return NULL;
    }
    return &g_definitions[op];
}

const char *opcode_get_name(opcode_t op) {
    if (op <= OPCODE_NONE || op >= OPCODE_MAX) {
        return NULL;
    }
    return g_definitions[op].name;
}

int code_make(opcode_t op, int operands_count, uint64_t *operands, array(uint8_t) *res) {
    opcode_definition_t *def = opcode_lookup(op);
    if (!def) {
        return 0;
    }

    int instr_len = 1;
    for (int i = 0; i < def->num_operands; i++) {
        instr_len += def->operand_widths[i];
    }

    uint8_t val = op;
    bool ok = false;

    ok = array_add(res, &val);
    if (!ok) {
        return 0;
    }

#define APPEND_BYTE(n) do {\
    val = (uint8_t)(operands[i] >> (n * 8));\
    ok = array_add(res, &val);\
    if (!ok) {\
        return 0;\
    }\
} while (0)

    for (int i = 0; i < operands_count; i++) {
        int width = def->operand_widths[i];
        switch (width) {
            case 1: {
                APPEND_BYTE(0);
                break;
            }
            case 2: {
                APPEND_BYTE(1);
                APPEND_BYTE(0);
                break;
            }
            case 4: {
                APPEND_BYTE(3);
                APPEND_BYTE(2);
                APPEND_BYTE(1);
                APPEND_BYTE(0);
                break;
            }
            case 8: {
                APPEND_BYTE(7);
                APPEND_BYTE(6);
                APPEND_BYTE(5);
                APPEND_BYTE(4);
                APPEND_BYTE(3);
                APPEND_BYTE(2);
                APPEND_BYTE(1);
                APPEND_BYTE(0);
                break;
            }
            default: {
                APE_ASSERT(false);
                break;
            }
        }
    }
#undef APPEND_BYTE
    return instr_len;
}

void code_to_string(uint8_t *code, src_pos_t *source_positions, size_t code_size, strbuf_t *res) {
    unsigned pos = 0;
    while (pos < code_size) {
        uint8_t op = code[pos];
        opcode_definition_t *def = opcode_lookup(op);
        APE_ASSERT(def);
        if (source_positions) {
            src_pos_t src_pos = source_positions[pos];
            strbuf_appendf(res, "%d:%-4d\t%04d\t%s", src_pos.line, src_pos.column, pos, def->name);
        } else {
            strbuf_appendf(res, "%04d %s", pos, def->name);
        }
        pos += 1;

        uint64_t operands[2];
        code_read_operands(def, code + pos, operands);
        for (int i = 0; i < def->num_operands; i++) {
            if (op == OPCODE_NUMBER) {
                double val_double = ape_uint64_to_double(operands[i]);
                strbuf_appendf(res, " %1.17g", val_double);
            } else {
                strbuf_appendf(res, " %llu", (long long unsigned int)operands[i]);
            }
            pos += def->operand_widths[i];
        }
        strbuf_append(res, "\n");

    }
    return;
}

bool code_read_operands(opcode_definition_t *def, uint8_t *instr, uint64_t out_operands[2]) {
    int offset = 0;
    for (int i = 0; i < def->num_operands; i++) {
        int operand_width = def->operand_widths[i];
        switch (operand_width) {
            case 1: {
                out_operands[i] = instr[offset];
                break;
            }
            case 2: {
                uint64_t operand = 0;
                operand = operand | (instr[offset] << 8);
                operand = operand | (instr[offset + 1]);
                out_operands[i] = operand;
                break;
            }
            case 4: {
                uint64_t operand = 0;
                operand = operand | (instr[offset + 0] << 24);
                operand = operand | (instr[offset + 1] << 16);
                operand = operand | (instr[offset + 2] << 8);
                operand = operand | (instr[offset + 3]);
                out_operands[i] = operand;
                break;
            }
            case 8: {
                uint64_t operand = 0;
                operand = operand | ((uint64_t)instr[offset + 0] << 56);
                operand = operand | ((uint64_t)instr[offset + 1] << 48);
                operand = operand | ((uint64_t)instr[offset + 2] << 40);
                operand = operand | ((uint64_t)instr[offset + 3] << 32);
                operand = operand | ((uint64_t)instr[offset + 4] << 24);
                operand = operand | ((uint64_t)instr[offset + 5] << 16);
                operand = operand | ((uint64_t)instr[offset + 6] << 8);
                operand = operand | ((uint64_t)instr[offset + 7]);
                out_operands[i] = operand;
                break;
            }
            default: {
                APE_ASSERT(false);
                return false;
            }
        }
        offset += operand_width;
    }
    return true;;
}
//FILE_END
//FILE_START:compilation_scope.c
#ifndef APE_AMALGAMATED
#include "compilation_scope.h"
#endif

compilation_scope_t *compilation_scope_make(allocator_t *alloc, compilation_scope_t *outer) {
    compilation_scope_t *scope = allocator_malloc(alloc, sizeof(compilation_scope_t));
    if (!scope) {
        return NULL;
    }
    memset(scope, 0, sizeof(compilation_scope_t));
    scope->alloc = alloc;
    scope->outer = outer;
    scope->bytecode = array_make(alloc, uint8_t);
    if (!scope->bytecode) {
        goto err;
    }
    scope->src_positions = array_make(alloc, src_pos_t);
    if (!scope->src_positions) {
        goto err;
    }
    scope->break_ip_stack = array_make(alloc, int);
    if (!scope->break_ip_stack) {
        goto err;
    }
    scope->continue_ip_stack = array_make(alloc, int);
    if (!scope->continue_ip_stack) {
        goto err;
    }
    return scope;
err:
    compilation_scope_destroy(scope);
    return NULL;
}

void compilation_scope_destroy(compilation_scope_t *scope) {
    array_destroy(scope->continue_ip_stack);
    array_destroy(scope->break_ip_stack);
    array_destroy(scope->bytecode);
    array_destroy(scope->src_positions);
    allocator_free(scope->alloc, scope);
}

compilation_result_t* compilation_scope_orphan_result(compilation_scope_t *scope) {
    compilation_result_t *res = compilation_result_make(scope->alloc,
                                                        array_data(scope->bytecode),
                                                        array_data(scope->src_positions),
                                                        array_count(scope->bytecode));
    if (!res) {
        return NULL;
    }
    array_orphan_data(scope->bytecode);
    array_orphan_data(scope->src_positions);
    return res;
}

compilation_result_t* compilation_result_make(allocator_t *alloc, uint8_t *bytecode, src_pos_t *src_positions, int count) {
    compilation_result_t *res = allocator_malloc(alloc, sizeof(compilation_result_t));
    if (!res) {
        return NULL;
    }
    memset(res, 0, sizeof(compilation_result_t));
    res->alloc = alloc;
    res->bytecode = bytecode;
    res->src_positions = src_positions;
    res->count = count;
    return res;
}

void compilation_result_destroy(compilation_result_t *res) {
    if (!res) {
        return;
    }
    allocator_free(res->alloc, res->bytecode);
    allocator_free(res->alloc, res->src_positions);
    allocator_free(res->alloc, res);
}
//FILE_END
//FILE_START:optimisation.c
#ifndef APE_AMALGAMATED
#include "optimisation.h"
#endif

static expression_t* optimise_infix_expression(expression_t* expr);
static expression_t* optimise_prefix_expression(expression_t* expr);

expression_t* optimise_expression(expression_t* expr) {
    switch (expr->type) {
        case EXPRESSION_INFIX: return optimise_infix_expression(expr);
        case EXPRESSION_PREFIX: return optimise_prefix_expression(expr);
        default: return NULL;
    }
}

// INTERNAL
static expression_t* optimise_infix_expression(expression_t* expr) {
    expression_t *left = expr->infix.left;
    expression_t *left_optimised = optimise_expression(left);
    if (left_optimised) {
        left = left_optimised;
    }

    expression_t *right = expr->infix.right;
    expression_t *right_optimised = optimise_expression(right);
    if (right_optimised) {
        right = right_optimised;
    }

    expression_t *res = NULL;

    bool left_is_numeric = left->type == EXPRESSION_NUMBER_LITERAL || left->type == EXPRESSION_BOOL_LITERAL;
    bool right_is_numeric = right->type == EXPRESSION_NUMBER_LITERAL || right->type == EXPRESSION_BOOL_LITERAL;

    bool left_is_string = left->type == EXPRESSION_STRING_LITERAL;
    bool right_is_string = right->type == EXPRESSION_STRING_LITERAL;

    allocator_t *alloc = expr->alloc;
    if (left_is_numeric && right_is_numeric) {
        double left_val = left->type == EXPRESSION_NUMBER_LITERAL ? left->number_literal : left->bool_literal;
        double right_val = right->type == EXPRESSION_NUMBER_LITERAL ? right->number_literal : right->bool_literal;
        int64_t left_val_int = (int64_t)left_val;
        int64_t right_val_int = (int64_t)right_val;
        switch (expr->infix.op) {
            case OPERATOR_PLUS:     { res = expression_make_number_literal(alloc, left_val + right_val); break; }
            case OPERATOR_MINUS:    { res = expression_make_number_literal(alloc, left_val - right_val); break; }
            case OPERATOR_ASTERISK: { res = expression_make_number_literal(alloc, left_val * right_val); break; }
            case OPERATOR_SLASH:    { res = expression_make_number_literal(alloc, left_val / right_val); break; }
            case OPERATOR_LT:       { res = expression_make_bool_literal(alloc, left_val < right_val); break; }
            case OPERATOR_LTE:      { res = expression_make_bool_literal(alloc, left_val <= right_val); break; }
            case OPERATOR_GT:       { res = expression_make_bool_literal(alloc, left_val > right_val); break; }
            case OPERATOR_GTE:      { res = expression_make_bool_literal(alloc, left_val >= right_val); break; }
            case OPERATOR_EQ:       { res = expression_make_bool_literal(alloc, APE_DBLEQ(left_val, right_val)); break; }
            case OPERATOR_NOT_EQ:   { res = expression_make_bool_literal(alloc, !APE_DBLEQ(left_val, right_val)); break; }
            case OPERATOR_MODULUS:  { res = expression_make_number_literal(alloc, fmod(left_val, right_val)); break; }
            case OPERATOR_BIT_AND:  { res = expression_make_number_literal(alloc, (double)(left_val_int & right_val_int)); break; }
            case OPERATOR_BIT_OR:   { res = expression_make_number_literal(alloc, (double)(left_val_int | right_val_int)); break; }
            case OPERATOR_BIT_XOR:  { res = expression_make_number_literal(alloc, (double)(left_val_int ^ right_val_int)); break; }
            case OPERATOR_LSHIFT:   { res = expression_make_number_literal(alloc, (double)(left_val_int << right_val_int)); break; }
            case OPERATOR_RSHIFT:   { res = expression_make_number_literal(alloc, (double)(left_val_int >> right_val_int)); break; }
            default: {
                break;
            }
        }
    } else if (expr->infix.op == OPERATOR_PLUS && left_is_string && right_is_string) {
        const char* left_val = left->string_literal;
        const char* right_val = right->string_literal;
        char *res_str = ape_stringf(alloc, "%s%s", left_val, right_val);
        if (res_str) {
            res = expression_make_string_literal(alloc, res_str);
            if (!res) {
                allocator_free(alloc, res_str);
            }
        }
    }

    expression_destroy(left_optimised);
    expression_destroy(right_optimised);

    if (res) {
        res->pos = expr->pos;
    }

    return res;
}

expression_t* optimise_prefix_expression(expression_t* expr) {
    expression_t *right = expr->prefix.right;
    expression_t *right_optimised = optimise_expression(right);
    if (right_optimised) {
        right = right_optimised;
    }
    expression_t *res = NULL;
    if (expr->prefix.op == OPERATOR_MINUS && right->type == EXPRESSION_NUMBER_LITERAL) {
        res = expression_make_number_literal(expr->alloc, -right->number_literal);
    } else if (expr->prefix.op == OPERATOR_BANG && right->type == EXPRESSION_BOOL_LITERAL) {
        res = expression_make_bool_literal(expr->alloc, !right->bool_literal);
    }
    expression_destroy(right_optimised);
    if (res) {
        res->pos = expr->pos;
    }
    return res;
}
//FILE_END
//FILE_START:compiler.c
#include <stdlib.h>
#include <math.h>

#ifndef APE_AMALGAMATED
#include "compiler.h"

#include "ape.h"
#include "ast.h"
#include "object.h"
#include "gc.h"
#include "code.h"
#include "symbol_table.h"
#include "errors.h"
#include "optimisation.h"
#endif

typedef struct module {
    allocator_t *alloc;
    char *name;
    ptrarray(symbol_t) *symbols;
} module_t;

typedef struct file_scope {
    allocator_t *alloc;
    parser_t *parser;
    symbol_table_t *symbol_table;
    compiled_file_t *file;
    ptrarray(char) *loaded_module_names;
} file_scope_t;

typedef struct compiler {
    allocator_t *alloc;
    const ape_config_t *config;
    gcmem_t *mem;
    errors_t *errors;
    ptrarray(compiled_file_t) *files;
    global_store_t *global_store;
    array(object_t) *constants;
    compilation_scope_t *compilation_scope;
    ptrarray(file_scope_t) *file_scopes;
    array(src_pos_t) *src_positions_stack;
    dict(module_t) *modules;
} compiler_t;

static bool compiler_init(compiler_t *comp,
                          allocator_t *alloc,
                          const ape_config_t *config,
                          gcmem_t *mem, errors_t *errors,
                          ptrarray(compiled_file_t) *files,
                          global_store_t *global_store);
static void compiler_deinit(compiler_t *comp);

static bool compiler_init_shallow_copy(compiler_t *copy, compiler_t *src); // used to restore compiler's state if something fails

static int emit(compiler_t *comp, opcode_t op, int operands_count, uint64_t *operands);
static compilation_scope_t* get_compilation_scope(compiler_t *comp);
static bool push_compilation_scope(compiler_t *comp);
static void pop_compilation_scope(compiler_t *comp);
static bool push_symbol_table(compiler_t *comp, int global_offset);
static void pop_symbol_table(compiler_t *comp);
static opcode_t get_last_opcode(compiler_t *comp);
static bool compile_code(compiler_t *comp, const char *code);
static bool compile_statements(compiler_t *comp, ptrarray(statement_t) *statements);
static bool import_module(compiler_t *comp, const statement_t *import_stmt);
static bool compile_statement(compiler_t *comp, const statement_t *stmt);
static bool compile_expression(compiler_t *comp, expression_t *expr);
static bool compile_code_block(compiler_t *comp, const code_block_t *block);
static int  add_constant(compiler_t *comp, object_t obj);
static void change_uint16_operand(compiler_t *comp, int ip, uint16_t operand);
static bool last_opcode_is(compiler_t *comp, opcode_t op);
static bool read_symbol(compiler_t *comp, const symbol_t *symbol);
static bool write_symbol(compiler_t *comp, const symbol_t *symbol, bool define);

static bool push_break_ip(compiler_t *comp, int ip);
static void pop_break_ip(compiler_t *comp);
static int  get_break_ip(compiler_t *comp);

static bool push_continue_ip(compiler_t *comp, int ip);
static void pop_continue_ip(compiler_t *comp);
static int  get_continue_ip(compiler_t *comp);

static int  get_ip(compiler_t *comp);

static array(src_pos_t)* get_src_positions(compiler_t *comp);
static array(uint8_t)*   get_bytecode(compiler_t *comp);

static file_scope_t* file_scope_make(compiler_t *comp, compiled_file_t *file);
static void file_scope_destroy(file_scope_t *file_scope);
static bool push_file_scope(compiler_t *comp, const char *filepath);
static void pop_file_scope(compiler_t *comp);

static void set_compilation_scope(compiler_t *comp, compilation_scope_t *scope);

static module_t* module_make(allocator_t *alloc, const char *name);
static void module_destroy(module_t *module);
static module_t* module_copy(module_t *module);
static bool module_add_symbol(module_t *module, const symbol_t *symbol);

static const char* get_module_name(const char *path);
static const symbol_t* define_symbol(compiler_t *comp, src_pos_t pos, const char *name, bool assignable, bool can_shadow);

static bool is_comparison(operator_t op);

compiler_t *compiler_make(allocator_t *alloc, const ape_config_t *config, gcmem_t *mem, errors_t *errors, ptrarray(compiled_file_t) *files, global_store_t *global_store) {
    compiler_t *comp = allocator_malloc(alloc, sizeof(compiler_t));
    if (!comp) {
        return NULL;
    }
    bool ok = compiler_init(comp, alloc, config, mem, errors, files, global_store);
    if (!ok) {
        allocator_free(alloc, comp);
        return NULL;
    }
    return comp;
}

void compiler_destroy(compiler_t *comp) {
    if (!comp) {
        return;
    }
    allocator_t *alloc = comp->alloc;
    compiler_deinit(comp);
    allocator_free(alloc, comp);
}

compilation_result_t* compiler_compile(compiler_t *comp, const char *code) {
    compilation_scope_t *compilation_scope = get_compilation_scope(comp);

    APE_ASSERT(array_count(comp->src_positions_stack) == 0);
    APE_ASSERT(array_count(compilation_scope->bytecode) == 0);
    APE_ASSERT(array_count(compilation_scope->break_ip_stack) == 0);
    APE_ASSERT(array_count(compilation_scope->continue_ip_stack) == 0);

    array_clear(comp->src_positions_stack);
    array_clear(compilation_scope->bytecode);
    array_clear(compilation_scope->src_positions);
    array_clear(compilation_scope->break_ip_stack);
    array_clear(compilation_scope->continue_ip_stack);

    compiler_t comp_shallow_copy;
    bool ok = compiler_init_shallow_copy(&comp_shallow_copy, comp);
    if (!ok) {
        return NULL;
    }

    ok = compile_code(comp, code);
    if (!ok) {
        goto err;
    }

    compilation_scope = get_compilation_scope(comp); // might've changed
    APE_ASSERT(compilation_scope->outer == NULL);

    compilation_scope = get_compilation_scope(comp);
    compilation_result_t *res = compilation_scope_orphan_result(compilation_scope);
    if (!res) {
        goto err;
    }
    compiler_deinit(&comp_shallow_copy);
    return res;
err:
    compiler_deinit(comp);
    *comp = comp_shallow_copy;
    return NULL;
}

compilation_result_t* compiler_compile_file(compiler_t *comp, const char *path) {
    char *code = NULL;
    compiled_file_t *file = NULL;
    compilation_result_t *res = NULL;

    if (!comp->config->fileio.read_file.read_file) { // todo: read code function
        errors_add_error(comp->errors, ERROR_COMPILATION, src_pos_invalid, "File read function not configured");
        goto err;
    }

    code = comp->config->fileio.read_file.read_file(comp->config->fileio.read_file.context, path);
    if (!code) {
        errors_add_errorf(comp->errors, ERROR_COMPILATION, src_pos_invalid, "Reading file \"%s\" failed", path);
        goto err;
    }

    file = compiled_file_make(comp->alloc, path);
    if (!file) {
        goto err;
    }

    bool ok = ptrarray_add(comp->files, file);
    if (!ok) {
        compiled_file_destroy(file);
        goto err;
    }

    APE_ASSERT(ptrarray_count(comp->file_scopes) == 1);
    file_scope_t *file_scope = ptrarray_top(comp->file_scopes);
    if (!file_scope) {
        goto err;
    }
    
    compiled_file_t *prev_file = file_scope->file; // todo: push file scope instead?
    file_scope->file = file;

    res = compiler_compile(comp, code);
    if (!res) {
        file_scope->file = prev_file;
        goto err;
    }
    file_scope->file = prev_file;

    allocator_free(comp->alloc, code);
    return res;
err:
    allocator_free(comp->alloc, code);
    return NULL;
}

symbol_table_t* compiler_get_symbol_table(compiler_t *comp) {
    file_scope_t *file_scope = ptrarray_top(comp->file_scopes);
    if (!file_scope) {
        APE_ASSERT(false);
        return NULL;
    }
    return file_scope->symbol_table;
}

void compiler_set_symbol_table(compiler_t *comp, symbol_table_t *table) {
    file_scope_t *file_scope = ptrarray_top(comp->file_scopes);
    if (!file_scope) {
        APE_ASSERT(false);
        return;
    }
    file_scope->symbol_table = table;
}

array(object_t)* compiler_get_constants(const compiler_t *comp) {
    return comp->constants;
}

// INTERNAL
static bool compiler_init(compiler_t *comp,
                          allocator_t *alloc,
                          const ape_config_t *config,
                          gcmem_t *mem, errors_t *errors,
                          ptrarray(compiled_file_t) *files,
                          global_store_t *global_store) {

    memset(comp, 0, sizeof(compiler_t));
    comp->alloc = alloc;
    comp->config = config;
    comp->mem = mem;
    comp->errors = errors;
    comp->files = files;
    comp->global_store = global_store;

    comp->file_scopes = ptrarray_make(alloc);
    if (!comp->file_scopes) {
        goto err;
    }
    comp->constants = array_make(alloc, object_t);
    if (!comp->constants) {
        goto err;
    }
    comp->src_positions_stack = array_make(alloc, src_pos_t);
    if (!comp->src_positions_stack) {
        goto err;
    }
    comp->modules = dict_make(alloc, module_copy, module_destroy);
    if (!comp->modules) {
        goto err;
    }
    bool ok = push_compilation_scope(comp);
    if (!ok) {
        goto err;
    }
    ok = push_file_scope(comp, "none");
    if (!ok) {
        goto err;
    }
    return true;
err:
    compiler_deinit(comp);
    return false;
}

static void compiler_deinit(compiler_t *comp) {
    if (!comp) {
        return;
    }
    while (ptrarray_count(comp->file_scopes) > 0) {
        pop_file_scope(comp);
    }
    while (get_compilation_scope(comp)) {
        pop_compilation_scope(comp);
    }
    dict_destroy_with_items(comp->modules);
    array_destroy(comp->src_positions_stack);

    array_destroy(comp->constants);
    ptrarray_destroy(comp->file_scopes);
    memset(comp, 0, sizeof(compiler_t));
}

static bool compiler_init_shallow_copy(compiler_t *copy, compiler_t *src) {
    bool ok = compiler_init(copy, src->alloc, src->config, src->mem, src->errors, src->files, src->global_store);
    if (!ok) {
        return false;
    }

    symbol_table_t *src_st = compiler_get_symbol_table(src);
    APE_ASSERT(ptrarray_count(src->file_scopes) == 1);
    APE_ASSERT(src_st->outer == NULL);
    symbol_table_t *src_st_copy = symbol_table_copy(src_st);
    if (!src_st_copy) {
        goto err;
    }
    symbol_table_t *copy_st = compiler_get_symbol_table(copy);
    symbol_table_destroy(copy_st);
    copy_st = NULL;
    compiler_set_symbol_table(copy, src_st_copy);

    dict(module_t) *modules_copy = dict_copy_with_items(src->modules);
    if (!modules_copy) {
        goto err;
    }
    dict_destroy_with_items(copy->modules);
    copy->modules = modules_copy;

    array(object_t) *constants_copy = array_copy(src->constants);
    if (!constants_copy) {
        goto err;
    }
    array_destroy(copy->constants);
    copy->constants = constants_copy;

    file_scope_t *src_file_scope = ptrarray_top(src->file_scopes);
    file_scope_t *copy_file_scope = ptrarray_top(copy->file_scopes);

    ptrarray(char) *src_loaded_module_names = src_file_scope->loaded_module_names;
    ptrarray(char) *copy_loaded_module_names = copy_file_scope->loaded_module_names;

    for (int i = 0; i < ptrarray_count(src_loaded_module_names); i++) {
        const char *loaded_name = ptrarray_get(src_loaded_module_names, i);
        char *loaded_name_copy = ape_strdup(copy->alloc, loaded_name);
        if (!loaded_name_copy) {
            goto err;
        }
        ok = ptrarray_add(copy_loaded_module_names, loaded_name_copy);
        if (!ok) {
            allocator_free(copy->alloc, loaded_name_copy);
            goto err;
        }
    }

    return true;
err:
    compiler_deinit(copy);
    return false;
}

static int emit(compiler_t *comp, opcode_t op, int operands_count, uint64_t *operands) {
    int ip = get_ip(comp);
    int len = code_make(op, operands_count, operands, get_bytecode(comp));
    if (len == 0) {
        return -1;
    }
    for (int i = 0; i < len; i++) {
        src_pos_t *src_pos = array_top(comp->src_positions_stack);
        APE_ASSERT(src_pos->line >= 0);
        APE_ASSERT(src_pos->column >= 0);
        bool ok = array_add(get_src_positions(comp), src_pos);
        if (!ok) {
            return -1;
        }
    }
    compilation_scope_t *compilation_scope = get_compilation_scope(comp);
    compilation_scope->last_opcode = op;
    return ip;
}

static compilation_scope_t* get_compilation_scope(compiler_t *comp) {
    return comp->compilation_scope;
}

static bool push_compilation_scope(compiler_t *comp) {
    compilation_scope_t *current_scope = get_compilation_scope(comp);
    compilation_scope_t *new_scope = compilation_scope_make(comp->alloc, current_scope);
    if (!new_scope) {
        return false;
    }
    set_compilation_scope(comp, new_scope);
    return true;
}

static void pop_compilation_scope(compiler_t *comp) {
    compilation_scope_t *current_scope = get_compilation_scope(comp);
    APE_ASSERT(current_scope);
    set_compilation_scope(comp, current_scope->outer);
    compilation_scope_destroy(current_scope);
}

static bool push_symbol_table(compiler_t *comp, int global_offset) {
    file_scope_t *file_scope = ptrarray_top(comp->file_scopes);
    if (!file_scope) {
        APE_ASSERT(false);
        return false;
    }
    symbol_table_t *current_table = file_scope->symbol_table;
    file_scope->symbol_table = symbol_table_make(comp->alloc, current_table, comp->global_store, global_offset);
    if (!file_scope->symbol_table) {
        file_scope->symbol_table = current_table;
        return false;
    }
    return true;
}

static void pop_symbol_table(compiler_t *comp) {
    file_scope_t *file_scope = ptrarray_top(comp->file_scopes);
    if (!file_scope) {
        APE_ASSERT(false);
        return;
    }
    symbol_table_t *current_table = file_scope->symbol_table;
    if (!current_table) {
        APE_ASSERT(false);
        return;
    }
    file_scope->symbol_table = current_table->outer;
    symbol_table_destroy(current_table);
}

static opcode_t get_last_opcode(compiler_t *comp) {
    compilation_scope_t *current_scope = get_compilation_scope(comp);
    return current_scope->last_opcode;
}

static bool compile_code(compiler_t *comp, const char *code) {
    file_scope_t *file_scope = ptrarray_top(comp->file_scopes);
    APE_ASSERT(file_scope);

    ptrarray(statement_t) *statements = parser_parse_all(file_scope->parser, code, file_scope->file);
    if (!statements) {
        // errors are added by parser
        return false;
    }

    bool ok = compile_statements(comp, statements);

    ptrarray_destroy_with_items(statements, statement_destroy);

    // Left for debugging purposes
//    if (ok) {
//        strbuf_t *buf = strbuf_make(NULL);
//        code_to_string(array_data(comp->compilation_scope->bytecode),
//                       array_data(comp->compilation_scope->src_positions),
//                       array_count(comp->compilation_scope->bytecode), buf);
//        puts(strbuf_get_string(buf));
//        strbuf_destroy(buf);
//    }

    return ok;
}

static bool compile_statements(compiler_t *comp, ptrarray(statement_t) *statements) {
    bool ok = true;
    for (int i = 0; i < ptrarray_count(statements); i++) {
        const statement_t *stmt = ptrarray_get(statements, i);
        ok = compile_statement(comp, stmt);
        if (!ok) {
            break;
        }
    }
    return ok;
}

static bool import_module(compiler_t *comp, const statement_t *import_stmt) { // todo: split into smaller functions
    bool result = false;
    char *filepath = NULL;
    char *code = NULL;

    file_scope_t *file_scope = ptrarray_top(comp->file_scopes);

    const char *module_path = import_stmt->import.path;
    const char *module_name = get_module_name(module_path);

    for (int i = 0; i < ptrarray_count(file_scope->loaded_module_names); i++) {
        const char *loaded_name = ptrarray_get(file_scope->loaded_module_names, i);
        if (kg_streq(loaded_name, module_name)) {
            errors_add_errorf(comp->errors, ERROR_COMPILATION, import_stmt->pos, "Module \"%s\" was already imported", module_name);
            result = false;
            goto end;
        }
    }

    strbuf_t *filepath_buf = strbuf_make(comp->alloc);
    if (!filepath_buf) {
        result = false;
        goto end;
    }
    if (kg_is_path_absolute(module_path)) {
        strbuf_appendf(filepath_buf, "%s.ape", module_path);
    } else {
        strbuf_appendf(filepath_buf, "%s%s.ape", file_scope->file->dir_path, module_path);
    }

    if (strbuf_failed(filepath_buf)) {
        strbuf_destroy(filepath_buf);
        result = false;
        goto end;
    }

    const char *filepath_non_canonicalised = strbuf_get_string(filepath_buf);
    filepath = kg_canonicalise_path(comp->alloc, filepath_non_canonicalised);
    strbuf_destroy(filepath_buf);
    if (!filepath) {
        result = false;
        goto end;
    }

    symbol_table_t *symbol_table = compiler_get_symbol_table(comp);
    if (symbol_table->outer != NULL || ptrarray_count(symbol_table->block_scopes) > 1) {
        errors_add_error(comp->errors, ERROR_COMPILATION, import_stmt->pos, "Modules can only be imported in global scope");
        result = false;
        goto end;
    }

    for (int i = 0; i < ptrarray_count(comp->file_scopes); i++) {
        file_scope_t *fs = ptrarray_get(comp->file_scopes, i);
        if (APE_STREQ(fs->file->path, filepath)) {
            errors_add_errorf(comp->errors, ERROR_COMPILATION, import_stmt->pos, "Cyclic reference of file \"%s\"", filepath);
            result = false;
            goto end;
        }
    }

    module_t *module = dict_get(comp->modules, filepath);
    if (!module) { // todo: create new module function
        if (!comp->config->fileio.read_file.read_file) {
            errors_add_errorf(comp->errors, ERROR_COMPILATION, import_stmt->pos, "Cannot import module \"%s\", file read function not configured", filepath);
            result = false;
            goto end;
        }

        code = comp->config->fileio.read_file.read_file(comp->config->fileio.read_file.context, filepath);
        if (!code) {
            errors_add_errorf(comp->errors, ERROR_COMPILATION, import_stmt->pos, "Reading module file \"%s\" failed", filepath);
            result = false;
            goto end;
        }

        module = module_make(comp->alloc, module_name);
        if (!module) {
            result = false;
            goto end;
        }

        bool ok = push_file_scope(comp, filepath);
        if (!ok) {
            module_destroy(module);
            result = false;
            goto end;
        }

        ok = compile_code(comp, code);
        if (!ok) {
            module_destroy(module);
            result = false;
            goto end;
        }

        symbol_table_t *st = compiler_get_symbol_table(comp);
        for (int i = 0; i < symbol_table_get_module_global_symbol_count(st); i++) {
            const symbol_t *symbol = symbol_table_get_module_global_symbol_at(st, i);
            module_add_symbol(module, symbol);
        }

        pop_file_scope(comp);

        ok = dict_set(comp->modules, filepath, module);
        if (!ok) {
            module_destroy(module);
            result = false;
            goto end;
        }
    }

    for (int i = 0; i < ptrarray_count(module->symbols); i++) {
        symbol_t *symbol = ptrarray_get(module->symbols, i);
        bool ok = symbol_table_add_module_symbol(symbol_table, symbol);
        if (!ok) {
            result = false;
            goto end;
        }
    }

    char *name_copy = ape_strdup(comp->alloc, module_name);
    if (!name_copy) {
        result = false;
        goto end;
    }

    bool ok = ptrarray_add(file_scope->loaded_module_names, name_copy);
    if (!ok) {
        allocator_free(comp->alloc, name_copy);
        result = false;
        goto end;
    }
    
    result = true;

end:
    allocator_free(comp->alloc, filepath);
    allocator_free(comp->alloc, code);
    return result;
}

static bool compile_statement(compiler_t *comp, const statement_t *stmt) {
    bool ok = false;
    int ip = -1;

    ok = array_push(comp->src_positions_stack, &stmt->pos);
    if (!ok) {
        return false;
    }

    compilation_scope_t *compilation_scope = get_compilation_scope(comp);
    symbol_table_t *symbol_table = compiler_get_symbol_table(comp);
    switch (stmt->type) {
        case STATEMENT_EXPRESSION: {
            ok = compile_expression(comp, stmt->expression);
            if (!ok) {
                return false;
            }
            ip = emit(comp, OPCODE_POP, 0, NULL);
            if (ip < 0) {
                return false;
            }
            break;
        }
        case STATEMENT_DEFINE: {
            ok = compile_expression(comp, stmt->define.value);
            if (!ok) {
                return false;
            }
            
            const symbol_t *symbol = define_symbol(comp, stmt->define.name->pos, stmt->define.name->value, stmt->define.assignable, false);
            if (!symbol) {
                return false;
            }

            ok = write_symbol(comp, symbol, true);
            if (!ok) {
                return false;
            }

            break;
        }
        case STATEMENT_IF: {
            const if_statement_t *if_stmt = &stmt->if_statement;

            array(int) *jump_to_end_ips = array_make(comp->alloc, int);
            if (!jump_to_end_ips) {
                goto statement_if_error;
            }

            for (int i = 0; i < ptrarray_count(if_stmt->cases); i++) {
                if_case_t *if_case = ptrarray_get(if_stmt->cases, i);

                ok = compile_expression(comp, if_case->test);
                if (!ok) {
                    goto statement_if_error;
                }

                int next_case_jump_ip = emit(comp, OPCODE_JUMP_IF_FALSE, 1, (uint64_t[]){0xbeef});

                ok = compile_code_block(comp, if_case->consequence);
                if (!ok) {
                    goto statement_if_error;
                }

                // don't emit jump for the last statement
                if (i < (ptrarray_count(if_stmt->cases) - 1) || if_stmt->alternative) {
                    int jump_to_end_ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){0xbeef});
                    bool ok = array_add(jump_to_end_ips, &jump_to_end_ip);
                    if (!ok) {
                        goto statement_if_error;
                    }
                }

                int after_elif_ip = get_ip(comp);
                change_uint16_operand(comp, next_case_jump_ip + 1, after_elif_ip);
            }

            if (if_stmt->alternative) {
                ok = compile_code_block(comp, if_stmt->alternative);
                if (!ok) {
                    goto statement_if_error;
                }
            }

            int after_alt_ip = get_ip(comp);

            for (int i = 0; i < array_count(jump_to_end_ips); i++) {
                int *pos = array_get(jump_to_end_ips, i);
                change_uint16_operand(comp, *pos + 1, after_alt_ip);
            }

            array_destroy(jump_to_end_ips);

            break;
        statement_if_error:
            array_destroy(jump_to_end_ips);
            return false;
        }
        case STATEMENT_RETURN_VALUE: {
            if (compilation_scope->outer == NULL) {
                errors_add_errorf(comp->errors, ERROR_COMPILATION, stmt->pos, "Nothing to return from");
                return false;
            }
            ip = -1;
            if (stmt->return_value) {
                ok = compile_expression(comp, stmt->return_value);
                if (!ok) {
                    return false;
                }
                ip = emit(comp, OPCODE_RETURN_VALUE, 0, NULL);
            } else {
                ip = emit(comp, OPCODE_RETURN, 0, NULL);
            }
            if (ip < 0) {
                return false;
            }
            break;
        }
        case STATEMENT_WHILE_LOOP: {
            const while_loop_statement_t *loop = &stmt->while_loop;

            int before_test_ip = get_ip(comp);

            ok = compile_expression(comp, loop->test);
            if (!ok) {
                return false;
            }

            int after_test_ip = get_ip(comp);
            ip = emit(comp, OPCODE_JUMP_IF_TRUE, 1, (uint64_t[]){after_test_ip + 6});
            if (ip < 0) {
                return false;
            }

            int jump_to_after_body_ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){0xdead});
            if (jump_to_after_body_ip < 0) {
                return false;
            }

            ok = push_continue_ip(comp, before_test_ip);
            if (!ok) {
                return false;
            }

            ok = push_break_ip(comp, jump_to_after_body_ip);
            if (!ok) {
                return false;
            }

            ok = compile_code_block(comp, loop->body);
            if (!ok) {
                return false;
            }

            pop_break_ip(comp);
            pop_continue_ip(comp);

            ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){before_test_ip});
            if (ip < 0) {
                return false;
            }

            int after_body_ip = get_ip(comp);
            change_uint16_operand(comp, jump_to_after_body_ip + 1, after_body_ip);

            break;
        }
        case STATEMENT_BREAK: {
            int break_ip = get_break_ip(comp);
            if (break_ip < 0) {
                errors_add_errorf(comp->errors, ERROR_COMPILATION, stmt->pos, "Nothing to break from.");
                return false;
            }
            ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){break_ip});
            if (ip < 0) {
                return false;
            }
            break;
        }
        case STATEMENT_CONTINUE: {
            int continue_ip = get_continue_ip(comp);
            if (continue_ip < 0) {
                errors_add_errorf(comp->errors, ERROR_COMPILATION, stmt->pos, "Nothing to continue from.");
                return false;
            }
            ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){continue_ip});
            if (ip < 0) {
                return false;
            }
            break;
        }
        case STATEMENT_FOREACH: {
            const foreach_statement_t *foreach = &stmt->foreach;
            ok = symbol_table_push_block_scope(symbol_table);
            if (!ok) {
                return false;
            }

            // Init
            const symbol_t *index_symbol = define_symbol(comp, stmt->pos, "@i", false, true);
            if (!index_symbol) {
                return false;
            }

            ip = emit(comp, OPCODE_NUMBER, 1, (uint64_t[]){0});
            if (ip < 0) {
                return false;
            }

            ok = write_symbol(comp, index_symbol, true);
            if (!ok) {
                return false;
            }

            const symbol_t *source_symbol = NULL;
            if (foreach->source->type == EXPRESSION_IDENT) {
                source_symbol = symbol_table_resolve(symbol_table, foreach->source->ident->value);
                if (!source_symbol) {
                    errors_add_errorf(comp->errors, ERROR_COMPILATION, foreach->source->pos,
                                      "Symbol \"%s\" could not be resolved", foreach->source->ident->value);
                    return false;
                }
            } else {
                ok = compile_expression(comp, foreach->source);
                if (!ok) {
                    return false;
                }
                source_symbol = define_symbol(comp, foreach->source->pos, "@source", false, true);
                if (!source_symbol) {
                    return false;
                }
                ok = write_symbol(comp, source_symbol, true);
                if (!ok) {
                    return false;
                }
            }

            // Update
            int jump_to_after_update_ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){0xbeef});
            if (jump_to_after_update_ip < 0) {
                return false;
            }

            int update_ip = get_ip(comp);
            ok = read_symbol(comp, index_symbol);
            if (!ok) {
                return false;
            }

            ip = emit(comp, OPCODE_NUMBER, 1, (uint64_t[]){ape_double_to_uint64(1)});
            if (ip < 0) {
                return false;
            }

            ip = emit(comp, OPCODE_ADD, 0, NULL);
            if (ip < 0) {
                return false;
            }

            ok = write_symbol(comp, index_symbol, false);
            if (!ok) {
                return false;
            }

            int after_update_ip = get_ip(comp);
            change_uint16_operand(comp, jump_to_after_update_ip + 1, after_update_ip);

            // Test
            ok = array_push(comp->src_positions_stack, &foreach->source->pos);
            if (!ok) {
                return false;
            }

            ok = read_symbol(comp, source_symbol);
            if (!ok) {
                return false;
            }

            ip = emit(comp, OPCODE_LEN, 0, NULL);
            if (ip < 0) {
                return false;
            }

            array_pop(comp->src_positions_stack, NULL);
            ok = read_symbol(comp, index_symbol);
            if (!ok) {
                return false;
            }

            ip = emit(comp, OPCODE_COMPARE, 0, NULL);
            if (ip < 0) {
                return false;
            }

            ip = emit(comp, OPCODE_EQUAL, 0, NULL);
            if (ip < 0) {
                return false;
            }

            int after_test_ip = get_ip(comp);
            ip = emit(comp, OPCODE_JUMP_IF_FALSE, 1, (uint64_t[]){after_test_ip + 6});
            if (ip < 0) {
                return false;
            }

            int jump_to_after_body_ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){0xdead});
            if (jump_to_after_body_ip < 0) {
                return false;
            }

            ok = read_symbol(comp, source_symbol);
            if (!ok) {
                return false;
            }

            ok = read_symbol(comp, index_symbol);
            if (!ok) {
                return false;
            }

            ip = emit(comp, OPCODE_GET_VALUE_AT, 0, NULL);
            if (ip < 0) {
                return false;
            }

            const symbol_t *iter_symbol  = define_symbol(comp, foreach->iterator->pos, foreach->iterator->value, false, false);
            if (!iter_symbol) {
                return false;
            }
            
            ok = write_symbol(comp, iter_symbol, true);
            if (!ok) {
                return false;
            }

            // Body
            ok = push_continue_ip(comp, update_ip);
            if (!ok) {
                return false;
            }

            ok = push_break_ip(comp, jump_to_after_body_ip);
            if (!ok) {
                return false;
            }

            ok = compile_code_block(comp, foreach->body);
            if (!ok) {
                return false;
            }

            pop_break_ip(comp);
            pop_continue_ip(comp);

            ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){update_ip});
            if (ip < 0) {
                return false;
            }

            int after_body_ip = get_ip(comp);
            change_uint16_operand(comp, jump_to_after_body_ip + 1, after_body_ip);

            symbol_table_pop_block_scope(symbol_table);
            break;
        }
        case STATEMENT_FOR_LOOP: {
            const for_loop_statement_t *loop = &stmt->for_loop;

            ok = symbol_table_push_block_scope(symbol_table);
            if (!ok) {
                return false;
            }

            // Init
            int jump_to_after_update_ip = 0;
            bool ok = false;
            if (loop->init) {
                ok = compile_statement(comp, loop->init);
                if (!ok) {
                    return false;
                }
                jump_to_after_update_ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){0xbeef});
                if (jump_to_after_update_ip < 0) {
                    return false;
                }
            }

            // Update
            int update_ip = get_ip(comp);
            if (loop->update) {
                ok = compile_expression(comp, loop->update);
                if (!ok) {
                    return false;
                }
                ip = emit(comp, OPCODE_POP, 0, NULL);
                if (ip < 0) {
                    return false;
                }
            }

            if (loop->init) {
                int after_update_ip = get_ip(comp);
                change_uint16_operand(comp, jump_to_after_update_ip + 1, after_update_ip);
            }

            // Test
            if (loop->test) {
                ok = compile_expression(comp, loop->test);
                if (!ok) {
                    return false;
                }
            } else {
                ip = emit(comp, OPCODE_TRUE, 0, NULL);
                if (ip < 0) {
                    return false;
                }
            }
            int after_test_ip = get_ip(comp);

            ip = emit(comp, OPCODE_JUMP_IF_TRUE, 1, (uint64_t[]){after_test_ip + 6});
            if (ip < 0) {
                return false;
            }
            int jmp_to_after_body_ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){0xdead});
            if (jmp_to_after_body_ip < 0) {
                return false;
            }

            // Body
            ok = push_continue_ip(comp, update_ip);
            if (!ok) {
                return false;
            }

            ok = push_break_ip(comp, jmp_to_after_body_ip);
            if (!ok) {
                return false;
            }

            ok = compile_code_block(comp, loop->body);
            if (!ok) {
                return false;
            }

            pop_break_ip(comp);
            pop_continue_ip(comp);

            ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){update_ip});
            if (ip < 0) {
                return false;
            }

            int after_body_ip = get_ip(comp);
            change_uint16_operand(comp, jmp_to_after_body_ip + 1, after_body_ip);

            symbol_table_pop_block_scope(symbol_table);
            break;
        }
        case STATEMENT_BLOCK: {
            ok = compile_code_block(comp, stmt->block);
            if (!ok) {
                return false;
            }
            break;
        }
        case STATEMENT_IMPORT: {
            ok = import_module(comp, stmt);
            if (!ok) {
                return false;
            }
            break;
        }
        case STATEMENT_RECOVER: {
            const recover_statement_t *recover = &stmt->recover;

            if (symbol_table_is_module_global_scope(symbol_table)) {
                errors_add_error(comp->errors, ERROR_COMPILATION, stmt->pos,
                                 "Recover statement cannot be defined in global scope");
                return false;
            }

            if (!symbol_table_is_top_block_scope(symbol_table)) {
                errors_add_error(comp->errors, ERROR_COMPILATION, stmt->pos,
                                 "Recover statement cannot be defined within other statements");
                return false;
            }

            int recover_ip = emit(comp, OPCODE_SET_RECOVER, 1, (uint64_t[]){0xbeef});
            if (recover_ip < 0) {
                return false;
            }

            int jump_to_after_recover_ip = emit(comp, OPCODE_JUMP, 1, (uint64_t[]){0xbeef});
            if (jump_to_after_recover_ip < 0) {
                return false;
            }

            int after_jump_to_recover_ip = get_ip(comp);
            change_uint16_operand(comp, recover_ip + 1, after_jump_to_recover_ip);

            ok = symbol_table_push_block_scope(symbol_table);
            if (!ok) {
                return false;
            }

            const symbol_t *error_symbol = define_symbol(comp, recover->error_ident->pos, recover->error_ident->value, false, false);
            if (!error_symbol) {
                return false;
            }

            ok = write_symbol(comp, error_symbol, true);
            if (!ok) {
                return false;
            }

            ok = compile_code_block(comp, recover->body);
            if (!ok) {
                return false;
            }

            if (!last_opcode_is(comp, OPCODE_RETURN) && !last_opcode_is(comp, OPCODE_RETURN_VALUE)) {
                errors_add_error(comp->errors, ERROR_COMPILATION, stmt->pos,
                                 "Recover body must end with a return statement");
                return false;
            }

            symbol_table_pop_block_scope(symbol_table);

            int after_recover_ip = get_ip(comp);
            change_uint16_operand(comp, jump_to_after_recover_ip + 1, after_recover_ip);

            break;
        }
        default: {
            APE_ASSERT(false);
            return false;
        }
    }
    array_pop(comp->src_positions_stack, NULL);
    return true;
}

static bool compile_expression(compiler_t *comp, expression_t *expr) {
    bool ok = false;
    int ip = -1;

    expression_t *expr_optimised = optimise_expression(expr);
    if (expr_optimised) {
        expr = expr_optimised;
    }

    ok = array_push(comp->src_positions_stack, &expr->pos);
    if (!ok) {
        return false;
    }

    compilation_scope_t *compilation_scope = get_compilation_scope(comp);
    symbol_table_t *symbol_table = compiler_get_symbol_table(comp);

    bool res = false;

    switch (expr->type) {
        case EXPRESSION_INFIX: {
            bool rearrange = false;

            opcode_t op = OPCODE_NONE;
            switch (expr->infix.op) {
                case OPERATOR_PLUS:     op = OPCODE_ADD; break;
                case OPERATOR_MINUS:    op = OPCODE_SUB; break;
                case OPERATOR_ASTERISK: op = OPCODE_MUL; break;
                case OPERATOR_SLASH:    op = OPCODE_DIV; break;
                case OPERATOR_MODULUS:  op = OPCODE_MOD; break;
                case OPERATOR_EQ:       op = OPCODE_EQUAL; break;
                case OPERATOR_NOT_EQ:   op = OPCODE_NOT_EQUAL; break;
                case OPERATOR_GT:       op = OPCODE_GREATER_THAN; break;
                case OPERATOR_GTE:      op = OPCODE_GREATER_THAN_EQUAL; break;
                case OPERATOR_LT:       op = OPCODE_GREATER_THAN; rearrange = true; break;
                case OPERATOR_LTE:      op = OPCODE_GREATER_THAN_EQUAL; rearrange = true; break;
                case OPERATOR_BIT_OR:   op = OPCODE_OR; break;
                case OPERATOR_BIT_XOR:  op = OPCODE_XOR; break;
                case OPERATOR_BIT_AND:  op = OPCODE_AND; break;
                case OPERATOR_LSHIFT:   op = OPCODE_LSHIFT; break;
                case OPERATOR_RSHIFT:   op = OPCODE_RSHIFT; break;
                default: {
                    errors_add_errorf(comp->errors, ERROR_COMPILATION, expr->pos, "Unknown infix operator");
                    goto error;
                }
            }

            expression_t *left = rearrange ? expr->infix.right : expr->infix.left;
            expression_t *right = rearrange ? expr->infix.left : expr->infix.right;

            ok = compile_expression(comp, left);
            if (!ok) {
                goto error;
            }

            ok = compile_expression(comp, right);
            if (!ok) {
                goto error;
            }

            if (is_comparison(expr->infix.op)) {
                ip = emit(comp, OPCODE_COMPARE, 0, NULL);
                if (ip < 0) {
                    goto error;
                }
            }

            ip = emit(comp, op, 0, NULL);
            if (ip < 0) {
                goto error;
            }

            break;
        }
        case EXPRESSION_NUMBER_LITERAL: {
            double number = expr->number_literal;
            ip = emit(comp, OPCODE_NUMBER, 1, (uint64_t[]){ape_double_to_uint64(number)});
            if (ip < 0) {
                goto error;
            }

            break;
        }
        case EXPRESSION_STRING_LITERAL: {
            object_t obj = object_make_string(comp->mem, expr->string_literal);
            if (object_is_null(obj)) {
                goto error;
            }

            int pos = add_constant(comp, obj);
            if (pos < 0) {
                goto error;
            }

            ip = emit(comp, OPCODE_CONSTANT, 1, (uint64_t[]){pos});
            if (ip < 0) {
                goto error;
            }

            break;
        }
        case EXPRESSION_NULL_LITERAL: {
            ip = emit(comp, OPCODE_NULL, 0, NULL);
            if (ip < 0) {
                goto error;
            }
            break;
        }
        case EXPRESSION_BOOL_LITERAL: {
            ip = emit(comp, expr->bool_literal ? OPCODE_TRUE : OPCODE_FALSE, 0, NULL);
            if (ip < 0) {
                goto error;
            }
            break;
        }
        case EXPRESSION_ARRAY_LITERAL: {
            for (int i = 0; i < ptrarray_count(expr->array); i++) {
                ok = compile_expression(comp, ptrarray_get(expr->array, i));
                if (!ok) {
                    goto error;
                }
            }
            ip = emit(comp, OPCODE_ARRAY, 1, (uint64_t[]){ptrarray_count(expr->array)});
            if (ip < 0) {
                goto error;
            }
            break;
        }
        case EXPRESSION_MAP_LITERAL: {
            const map_literal_t *map = &expr->map;
            int len = ptrarray_count(map->keys);
            ip = emit(comp, OPCODE_MAP_START, 1, (uint64_t[]){len});
            if (ip < 0) {
                goto error;
            }

            for (int i = 0; i < len; i++) {
                expression_t *key = ptrarray_get(map->keys, i);
                expression_t *val = ptrarray_get(map->values, i);

                ok = compile_expression(comp, key);
                if (!ok) {
                    goto error;
                }

                ok = compile_expression(comp, val);
                if (!ok) {
                    goto error;
                }
            }

            ip = emit(comp, OPCODE_MAP_END, 1, (uint64_t[]){len});
            if (ip < 0) {
                goto error;
            }

            break;
        }
        case EXPRESSION_PREFIX: {
            ok = compile_expression(comp, expr->prefix.right);
            if (!ok) {
                goto error;
            }

            opcode_t op = OPCODE_NONE;
            switch (expr->prefix.op) {
                case OPERATOR_MINUS: op = OPCODE_MINUS; break;
                case OPERATOR_BANG: op = OPCODE_BANG; break;
                default: {
                    errors_add_errorf(comp->errors, ERROR_COMPILATION, expr->pos, "Unknown prefix operator.");
                    goto error;
                }
            }
            ip = emit(comp, op, 0, NULL);
            if (ip < 0) {
                goto error;
            }

            break;
        }
        case EXPRESSION_IDENT: {
            const ident_t *ident = expr->ident;
            const symbol_t *symbol = symbol_table_resolve(symbol_table, ident->value);
            if (!symbol) {
                errors_add_errorf(comp->errors, ERROR_COMPILATION, ident->pos,
                                  "Symbol \"%s\" could not be resolved", ident->value);
                goto error;
            }
            ok = read_symbol(comp, symbol);
            if (!ok) {
                goto error;
            }

            break;
        }
        case EXPRESSION_INDEX: {
            const index_expression_t *index = &expr->index_expr;
            ok = compile_expression(comp, index->left);
            if (!ok) {
                goto error;
            }
            ok = compile_expression(comp, index->index);
            if (!ok) {
                goto error;
            }
            ip = emit(comp, OPCODE_GET_INDEX, 0, NULL);
            if (ip < 0) {
                goto error;
            }

            break;
        }
        case EXPRESSION_FUNCTION_LITERAL: {
            const fn_literal_t *fn = &expr->fn_literal;

            ok = push_compilation_scope(comp);
            if (!ok) {
                goto error;
            }

            ok = push_symbol_table(comp, 0);
            if (!ok) {
                goto error;
            }

            compilation_scope = get_compilation_scope(comp);
            symbol_table = compiler_get_symbol_table(comp);

            if (fn->name) {
                const symbol_t *fn_symbol = symbol_table_define_function_name(symbol_table, fn->name, false);
                if (!fn_symbol) {
                    errors_add_errorf(comp->errors, ERROR_COMPILATION, expr->pos,
                                               "Cannot define symbol \"%s\"", fn->name);
                    goto error;
                }
            }

            const symbol_t *this_symbol = symbol_table_define_this(symbol_table);
            if (!this_symbol) {
                errors_add_error(comp->errors, ERROR_COMPILATION, expr->pos, "Cannot define \"this\" symbol");
                goto error;
            }

            for (int i = 0; i < ptrarray_count(expr->fn_literal.params); i++) {
                ident_t *param = ptrarray_get(expr->fn_literal.params, i);
                const symbol_t *param_symbol = define_symbol(comp, param->pos, param->value, true, false);
                if (!param_symbol) {
                    goto error;
                }
            }

            ok = compile_statements(comp, fn->body->statements);
            if (!ok) {
                goto error;
            }
        
            if (!last_opcode_is(comp, OPCODE_RETURN_VALUE) && !last_opcode_is(comp, OPCODE_RETURN)) {
                ip = emit(comp, OPCODE_RETURN, 0, NULL);
                if (ip < 0) {
                    goto error;
                }
            }

            ptrarray(symbol_t) *free_symbols = symbol_table->free_symbols;
            symbol_table->free_symbols = NULL; // because it gets destroyed with compiler_pop_compilation_scope()

            int num_locals = symbol_table->max_num_definitions;
            
            compilation_result_t *comp_res = compilation_scope_orphan_result(compilation_scope);
            if (!comp_res) {
                ptrarray_destroy_with_items(free_symbols, symbol_destroy);
                goto error;
            }
            pop_symbol_table(comp);
            pop_compilation_scope(comp);
            compilation_scope = get_compilation_scope(comp);
            symbol_table = compiler_get_symbol_table(comp);
            
            object_t obj = object_make_function(comp->mem, fn->name, comp_res, true,
                                                num_locals, ptrarray_count(fn->params), 0);

            if (object_is_null(obj)) {
                ptrarray_destroy_with_items(free_symbols, symbol_destroy);
                compilation_result_destroy(comp_res);
                goto error;
            }

            for (int i = 0; i < ptrarray_count(free_symbols); i++) {
                symbol_t *symbol = ptrarray_get(free_symbols, i);
                ok = read_symbol(comp, symbol);
                if (!ok) {
                    ptrarray_destroy_with_items(free_symbols, symbol_destroy);
                    goto error;
                }
            }

            int pos = add_constant(comp, obj);
            if (pos < 0) {
                ptrarray_destroy_with_items(free_symbols, symbol_destroy);
                goto error;
            }

            ip = emit(comp, OPCODE_FUNCTION, 2, (uint64_t[]){pos, ptrarray_count(free_symbols)});
            if (ip < 0) {
                ptrarray_destroy_with_items(free_symbols, symbol_destroy);
                goto error;
            }

            ptrarray_destroy_with_items(free_symbols, symbol_destroy);

            break;
        }
        case EXPRESSION_CALL: {
            ok = compile_expression(comp, expr->call_expr.function);
            if (!ok) {
                goto error;
            }

            for (int i = 0; i < ptrarray_count(expr->call_expr.args); i++) {
                expression_t *arg_expr = ptrarray_get(expr->call_expr.args, i);
                ok = compile_expression(comp, arg_expr);
                if (!ok) {
                    goto error;
                }
            }

            ip = emit(comp, OPCODE_CALL, 1, (uint64_t[]){ptrarray_count(expr->call_expr.args)});
            if (ip < 0) {
                goto error;
            }

            break;
        }
        case EXPRESSION_ASSIGN: {
            const assign_expression_t *assign = &expr->assign;
            if (assign->dest->type != EXPRESSION_IDENT && assign->dest->type != EXPRESSION_INDEX) {
                errors_add_errorf(comp->errors, ERROR_COMPILATION, assign->dest->pos,
                                          "Expression is not assignable.");
                goto error;
            }

            ok = compile_expression(comp, assign->source);
            if (!ok) {
                goto error;
            }

            ip = emit(comp, OPCODE_DUP, 0, NULL);
            if (ip < 0) {
                goto error;
            }

            ok = array_push(comp->src_positions_stack, &assign->dest->pos);
            if (!ok) {
                goto error;
            }

            if (assign->dest->type == EXPRESSION_IDENT) {
                const ident_t *ident = assign->dest->ident;
                const symbol_t *symbol = symbol_table_resolve(symbol_table, ident->value);
                if (!symbol) {
                    errors_add_errorf(comp->errors, ERROR_COMPILATION, assign->dest->pos,
                                              "Symbol \"%s\" could not be resolved", ident->value);
                    goto error;
                }
                if (!symbol->assignable) {
                    errors_add_errorf(comp->errors, ERROR_COMPILATION, assign->dest->pos,
                                              "Symbol \"%s\" is not assignable", ident->value);
                    goto error;
                }
                ok = write_symbol(comp, symbol, false);
                if (!ok) {
                    goto error;
                }
            } else if (assign->dest->type == EXPRESSION_INDEX) {
                const index_expression_t *index = &assign->dest->index_expr;
                ok = compile_expression(comp, index->left);
                if (!ok) {
                    goto error;
                }
                ok = compile_expression(comp, index->index);
                if (!ok) {
                    goto error;
                }
                ip = emit(comp, OPCODE_SET_INDEX, 0, NULL);
                if (ip < 0) {
                    goto error;
                }
            }
            array_pop(comp->src_positions_stack, NULL);
            break;
        }
        case EXPRESSION_LOGICAL: {
            const logical_expression_t* logi = &expr->logical;

            ok = compile_expression(comp, logi->left);
            if (!ok) {
                goto error;
            }

            ip = emit(comp, OPCODE_DUP, 0, NULL);
            if (ip < 0) {
                goto error;
            }

            int after_left_jump_ip = 0;
            if (logi->op == OPERATOR_LOGICAL_AND) {
                after_left_jump_ip = emit(comp, OPCODE_JUMP_IF_FALSE, 1, (uint64_t[]){0xbeef});
            } else {
                after_left_jump_ip = emit(comp, OPCODE_JUMP_IF_TRUE, 1, (uint64_t[]){0xbeef});
            }

            if (after_left_jump_ip < 0) {
                goto error;
            }

            ip = emit(comp, OPCODE_POP, 0, NULL);
            if (ip < 0) {
                goto error;
            }

            ok = compile_expression(comp, logi->right);
            if (!ok) {
                goto error;
            }

            int after_right_ip = get_ip(comp);
            change_uint16_operand(comp, after_left_jump_ip + 1, after_right_ip);

            break;
        }
        default: {
            APE_ASSERT(false);
            break;
        }
    }
    res = true;
    goto end;
error:
    res = false;
end:
    array_pop(comp->src_positions_stack, NULL);
    expression_destroy(expr_optimised);
    return res;
}

static bool compile_code_block(compiler_t *comp, const code_block_t *block) {
    symbol_table_t *symbol_table = compiler_get_symbol_table(comp);
    if (!symbol_table) {
        return false;
    }

    bool ok = symbol_table_push_block_scope(symbol_table);
    if (!ok) {
        return false;
    }

    if (ptrarray_count(block->statements) == 0) {
        int ip = emit(comp, OPCODE_NULL, 0, NULL);
        if (ip < 0) {
            return false;
        }
        ip = emit(comp, OPCODE_POP, 0, NULL);
        if (ip < 0) {
            return false;
        }
    }

    for (int i = 0; i < ptrarray_count(block->statements); i++) {
        const statement_t *stmt = ptrarray_get(block->statements, i);
        bool ok = compile_statement(comp, stmt);
        if (!ok) {
            return false;
        }
    }
    symbol_table_pop_block_scope(symbol_table);
    return true;
}

static int add_constant(compiler_t *comp, object_t obj) {
    bool ok = array_add(comp->constants, &obj);
    if (!ok) {
        return -1;
    }
    int pos = array_count(comp->constants) - 1;
    return pos;
}

static void change_uint16_operand(compiler_t *comp, int ip, uint16_t operand) {
    array(uint8_t) *bytecode = get_bytecode(comp);
    if ((ip + 1) >= array_count(bytecode)) {
        APE_ASSERT(false);
        return;
    }
    uint8_t hi = (uint8_t)(operand >> 8);
    array_set(bytecode, ip, &hi);
    uint8_t lo = (uint8_t)(operand);
    array_set(bytecode, ip + 1, &lo);
}

static bool last_opcode_is(compiler_t *comp, opcode_t op) {
    opcode_t last_opcode = get_last_opcode(comp);
    return last_opcode == op;
}

static bool read_symbol(compiler_t *comp, const symbol_t *symbol) {
    int ip = -1;
    if (symbol->type == SYMBOL_MODULE_GLOBAL) {
        ip = emit(comp, OPCODE_GET_MODULE_GLOBAL, 1, (uint64_t[]){symbol->index});
    } else if (symbol->type == SYMBOL_APE_GLOBAL) {
        ip = emit(comp, OPCODE_GET_APE_GLOBAL, 1, (uint64_t[]){symbol->index});
    } else if (symbol->type == SYMBOL_LOCAL) {
        ip = emit(comp, OPCODE_GET_LOCAL, 1, (uint64_t[]){symbol->index});
    } else if (symbol->type == SYMBOL_FREE) {
        ip = emit(comp, OPCODE_GET_FREE, 1, (uint64_t[]){symbol->index});
    } else if (symbol->type == SYMBOL_FUNCTION) {
        ip = emit(comp, OPCODE_CURRENT_FUNCTION, 0, NULL);
    } else if (symbol->type == SYMBOL_THIS) {
        ip = emit(comp, OPCODE_GET_THIS, 0, NULL);
    }
    return ip >= 0;
}

static bool write_symbol(compiler_t *comp, const symbol_t *symbol, bool define) {
    int ip = -1;
    if (symbol->type == SYMBOL_MODULE_GLOBAL) {
        if (define) {
            ip = emit(comp, OPCODE_DEFINE_MODULE_GLOBAL, 1, (uint64_t[]){symbol->index});
        } else {
            ip = emit(comp, OPCODE_SET_MODULE_GLOBAL, 1, (uint64_t[]){symbol->index});
        }
    } else if (symbol->type == SYMBOL_LOCAL) {
        if (define) {
            ip = emit(comp, OPCODE_DEFINE_LOCAL, 1, (uint64_t[]){symbol->index});
        } else {
            ip = emit(comp, OPCODE_SET_LOCAL, 1, (uint64_t[]){symbol->index});
        }
    } else if (symbol->type == SYMBOL_FREE) {
        ip = emit(comp, OPCODE_SET_FREE, 1, (uint64_t[]){symbol->index});
    }
    return ip >= 0;
}

static bool push_break_ip(compiler_t *comp, int ip) {
    compilation_scope_t *comp_scope = get_compilation_scope(comp);
    return array_push(comp_scope->break_ip_stack, &ip);
}

static void pop_break_ip(compiler_t *comp) {
    compilation_scope_t *comp_scope = get_compilation_scope(comp);
    if (array_count(comp_scope->break_ip_stack) == 0) {
        APE_ASSERT(false);
        return;
    }
    array_pop(comp_scope->break_ip_stack, NULL);
}

static int get_break_ip(compiler_t *comp) {
    compilation_scope_t *comp_scope = get_compilation_scope(comp);
    if (array_count(comp_scope->break_ip_stack) == 0) {
        return -1;
    }
    int *res = array_top(comp_scope->break_ip_stack);
    return *res;
}

static bool push_continue_ip(compiler_t *comp, int ip) {
    compilation_scope_t *comp_scope = get_compilation_scope(comp);
    return array_push(comp_scope->continue_ip_stack, &ip);
}

static void pop_continue_ip(compiler_t *comp) {
    compilation_scope_t *comp_scope = get_compilation_scope(comp);
    if (array_count(comp_scope->continue_ip_stack) == 0) {
        APE_ASSERT(false);
        return;
    }
    array_pop(comp_scope->continue_ip_stack, NULL);
}

static int get_continue_ip(compiler_t *comp) {
    compilation_scope_t *comp_scope = get_compilation_scope(comp);
    if (array_count(comp_scope->continue_ip_stack) == 0) {
        APE_ASSERT(false);
        return -1;
    }
    int *res = array_top(comp_scope->continue_ip_stack);
    return *res;
}

static int get_ip(compiler_t *comp) {
    compilation_scope_t *compilation_scope = get_compilation_scope(comp);
    return array_count(compilation_scope->bytecode);
}

static array(src_pos_t)* get_src_positions(compiler_t *comp) {
    compilation_scope_t *compilation_scope = get_compilation_scope(comp);
    return compilation_scope->src_positions;
}

static array(uint8_t)* get_bytecode(compiler_t *comp) {
    compilation_scope_t *compilation_scope = get_compilation_scope(comp);
    return compilation_scope->bytecode;
}

static file_scope_t* file_scope_make(compiler_t *comp, compiled_file_t *file) {
    file_scope_t *file_scope = allocator_malloc(comp->alloc, sizeof(file_scope_t));
    if (!file_scope) {
        return NULL;
    }
    memset(file_scope, 0, sizeof(file_scope_t));
    file_scope->alloc = comp->alloc;
    file_scope->parser = parser_make(comp->alloc, comp->config, comp->errors);
    if (!file_scope->parser) {
        goto err;
    }
    file_scope->symbol_table = NULL;
    file_scope->file = file;
    file_scope->loaded_module_names = ptrarray_make(comp->alloc);
    if (!file_scope->loaded_module_names) {
        goto err;
    }
    return file_scope;
err:
    file_scope_destroy(file_scope);
    return NULL;
}

static void file_scope_destroy(file_scope_t *scope) {
    for (int i = 0; i < ptrarray_count(scope->loaded_module_names); i++) {
        void *name = ptrarray_get(scope->loaded_module_names, i);
        allocator_free(scope->alloc, name);
    }
    ptrarray_destroy(scope->loaded_module_names);
    parser_destroy(scope->parser);
    allocator_free(scope->alloc, scope);
}

static bool push_file_scope(compiler_t *comp, const char *filepath) {
    symbol_table_t *prev_st = NULL;
    if (ptrarray_count(comp->file_scopes) > 0) {
        prev_st = compiler_get_symbol_table(comp);
    }

    compiled_file_t *file = compiled_file_make(comp->alloc, filepath);
    if (!file) {
        return false;
    }

    bool ok = ptrarray_add(comp->files, file);
    if (!ok) {
        compiled_file_destroy(file);
        return false;
    }

    file_scope_t *file_scope = file_scope_make(comp, file);
    if (!file_scope) {
        return false;
    }

    ok = ptrarray_push(comp->file_scopes, file_scope);
    if (!ok) {
        file_scope_destroy(file_scope);
        return false;
    }

    int global_offset = 0;
    if (prev_st) {
        block_scope_t *prev_st_top_scope = symbol_table_get_block_scope(prev_st);
        global_offset = prev_st_top_scope->offset + prev_st_top_scope->num_definitions;
    }

    ok = push_symbol_table(comp, global_offset);
    if (!ok) {
        ptrarray_pop(comp->file_scopes);
        file_scope_destroy(file_scope);
        return false;
    }

    return true;
}

static void pop_file_scope(compiler_t *comp) {
    symbol_table_t *popped_st = compiler_get_symbol_table(comp);
    block_scope_t *popped_st_top_scope = symbol_table_get_block_scope(popped_st);
    int popped_num_defs = popped_st_top_scope->num_definitions;

    while (compiler_get_symbol_table(comp)) {
        pop_symbol_table(comp);
    }
    file_scope_t *scope = ptrarray_top(comp->file_scopes);
    if (!scope) {
        APE_ASSERT(false);
        return;
    }
    file_scope_destroy(scope);

    ptrarray_pop(comp->file_scopes);

    if (ptrarray_count(comp->file_scopes) > 0) {
        symbol_table_t *current_st = compiler_get_symbol_table(comp);
        block_scope_t *current_st_top_scope = symbol_table_get_block_scope(current_st);
        current_st_top_scope->num_definitions += popped_num_defs;
    }
}

static void set_compilation_scope(compiler_t *comp, compilation_scope_t *scope) {
    comp->compilation_scope = scope;
}

static module_t* module_make(allocator_t *alloc, const char *name) {
    module_t *module = allocator_malloc(alloc, sizeof(module_t));
    if (!module) {
        return NULL;
    }
    memset(module, 0, sizeof(module_t));
    module->alloc = alloc;
    module->name = ape_strdup(alloc, name);
    if (!module->name) {
        module_destroy(module);
        return NULL;
    }
    module->symbols = ptrarray_make(alloc);
    if (!module->symbols) {
        module_destroy(module);
        return NULL;
    }
    return module;
}

static void module_destroy(module_t *module) {
    if (!module) {
        return;
    }
    allocator_free(module->alloc, module->name);
    ptrarray_destroy_with_items(module->symbols, symbol_destroy);
    allocator_free(module->alloc, module);
}

static module_t* module_copy(module_t *src) {
    module_t *copy = allocator_malloc(src->alloc, sizeof(module_t));
    if (!copy) {
        return NULL;
    }
    memset(copy, 0, sizeof(module_t));
    copy->alloc = src->alloc;
    copy->name = ape_strdup(copy->alloc, src->name);
    if (!copy->name) {
        module_destroy(copy);
        return NULL;
    }
    copy->symbols = ptrarray_copy_with_items(src->symbols, symbol_copy, symbol_destroy);
    if (!copy->symbols) {
        module_destroy(copy);
        return NULL;
    }
    return copy;
}

static const char* get_module_name(const char *path) {
    const char *last_slash_pos = strrchr(path, '/');
    if (last_slash_pos) {
        return last_slash_pos + 1;
    }
    return path;
}

static bool module_add_symbol(module_t *module, const symbol_t *symbol) {
    strbuf_t *name_buf = strbuf_make(module->alloc);
    if (!name_buf) {
        return false;
    }
    bool ok = strbuf_appendf(name_buf, "%s::%s", module->name, symbol->name);
    if (!ok) {
        strbuf_destroy(name_buf);
        return false;
    }
    symbol_t *module_symbol = symbol_make(module->alloc, strbuf_get_string(name_buf), SYMBOL_MODULE_GLOBAL, symbol->index, false);
    strbuf_destroy(name_buf);
    if (!module_symbol) {
        return false;
    }
    ok = ptrarray_add(module->symbols, module_symbol);
    if (!ok) {
        symbol_destroy(module_symbol);
        return false;
    }
    return true;
}

static const symbol_t* define_symbol(compiler_t *comp, src_pos_t pos, const char *name, bool assignable, bool can_shadow) {
    symbol_table_t *symbol_table = compiler_get_symbol_table(comp);
    if (!can_shadow && !symbol_table_is_top_global_scope(symbol_table)) {
        const symbol_t *current_symbol = symbol_table_resolve(symbol_table, name);
        if (current_symbol) {
            errors_add_errorf(comp->errors, ERROR_COMPILATION, pos, "Symbol \"%s\" is already defined", name);
            return NULL;
        }
    }

    const symbol_t *symbol = symbol_table_define(symbol_table, name, assignable);
    if (!symbol) {
        errors_add_errorf(comp->errors, ERROR_COMPILATION, pos, "Cannot define symbol \"%s\"", name);
        return false;
    }

    return symbol;
}

static bool is_comparison(operator_t op) {
    switch (op) {
        case OPERATOR_EQ:
        case OPERATOR_NOT_EQ:
        case OPERATOR_GT:
        case OPERATOR_GTE:
        case OPERATOR_LT:
        case OPERATOR_LTE:
            return true;
        default:
            return false;
    }
    return false;
}
//FILE_END
//FILE_START:object.c
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <float.h>
#include <math.h>

#ifndef APE_AMALGAMATED
#include "object.h"
#include "code.h"
#include "compiler.h"
#include "traceback.h"
#include "gc.h"
#endif

#define OBJECT_PATTERN          0xfff8000000000000
#define OBJECT_HEADER_MASK      0xffff000000000000
#define OBJECT_ALLOCATED_HEADER 0xfffc000000000000
#define OBJECT_BOOL_HEADER      0xfff9000000000000
#define OBJECT_NULL_PATTERN     0xfffa000000000000

static object_t object_deep_copy_internal(gcmem_t *mem, object_t obj, valdict(object_t, object_t) *copies);
static bool object_equals_wrapped(const object_t *a, const object_t *b);
static unsigned long object_hash(object_t *obj_ptr);
static unsigned long object_hash_string(const char *str);
static unsigned long object_hash_double(double val);
static array(object_t)* object_get_allocated_array(object_t object);
static bool object_is_number(object_t obj);
static uint64_t get_type_tag(object_type_t type);
static bool freevals_are_allocated(function_t *fun);

object_t object_make_from_data(object_type_t type, object_data_t *data) {
    object_t object;
    object.handle = OBJECT_PATTERN;
    uint64_t type_tag = get_type_tag(type) & 0x7;
    object.handle |= (type_tag << 48);
    object.handle |= (uintptr_t)data; // assumes no pointer exceeds 48 bits
    return object;
}

object_t object_make_number(double val) {
    object_t o = { .number = val };
    if ((o.handle & OBJECT_PATTERN) == OBJECT_PATTERN) {
        o.handle = 0x7ff8000000000000;
    }
    return o;
}

object_t object_make_bool(bool val) {
    return (object_t) { .handle = OBJECT_BOOL_HEADER | val };
}

object_t object_make_null() {
    return (object_t) { .handle = OBJECT_NULL_PATTERN };
}

object_t object_make_string(gcmem_t *mem, const char *string) {
    object_data_t *obj = gcmem_alloc_object_data(mem, OBJECT_STRING);
    if (!obj) {
        return object_make_null();
    }

    int len = (int)strlen(string);
    if ((len + 1) < OBJECT_STRING_BUF_SIZE) {
        memcpy(obj->string.value_buf, string, len + 1);
        obj->string.is_allocated = false;
    } else {
        obj->string.value_allocated = ape_strdup(mem->alloc, string);
        if (!obj->string.value_allocated) {
            return object_make_null();
        }
        obj->string.is_allocated = true;
    }
    obj->string.hash = object_hash_string(string);
    return object_make_from_data(OBJECT_STRING, obj);
}

object_t object_make_string_no_copy(gcmem_t *mem, char *string) {
    object_data_t *obj = gcmem_alloc_object_data(mem, OBJECT_STRING);
    if (!obj) {
        return object_make_null();
    }

    int len = (int)strlen(string);
    if ((len + 1) < OBJECT_STRING_BUF_SIZE) {
        memcpy(obj->string.value_buf, string, len + 1);
        obj->string.is_allocated = false;
        obj->string.hash = object_hash_string(string);
        allocator_free(mem->alloc, string);
        string = NULL;
    } else {
        obj->string.value_allocated = string;
        obj->string.is_allocated = true;
        obj->string.hash = object_hash_string(string);
    }
    return object_make_from_data(OBJECT_STRING, obj);
}

object_t object_make_stringf(gcmem_t *mem, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int to_write = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    va_start(args, fmt);
    char *res = (char*)allocator_malloc(mem->alloc, to_write + 1);
    if (!res) {
        return object_make_null();
    }
    int written = vsprintf(res, fmt, args);
    (void)written;
    APE_ASSERT(written == to_write);
    va_end(args);
    object_t res_obj = object_make_string_no_copy(mem, res);
    if (object_is_null(res_obj)) {
        allocator_free(mem->alloc, res);
        return object_make_null();
    }
    return res_obj;
}

object_t object_make_native_function(gcmem_t *mem, const char *name, native_fn fn, void *data, int data_len) {
    if (data_len > NATIVE_FN_MAX_DATA_LEN) {
        return object_make_null();
    }
    object_data_t *obj = gcmem_alloc_object_data(mem, OBJECT_NATIVE_FUNCTION);
    if (!obj) {
        return object_make_null();
    }
    obj->native_function.name = ape_strdup(mem->alloc, name);
    if (!obj->native_function.name) {
        return object_make_null();
    }
    obj->native_function.fn = fn;
    if (data) {
        memcpy(obj->native_function.data, data, data_len);
    }
    obj->native_function.data_len = data_len;
    return object_make_from_data(OBJECT_NATIVE_FUNCTION, obj);
}

object_t object_make_array(gcmem_t *mem) {
    return object_make_array_with_capacity(mem, 8);
}

object_t object_make_array_with_capacity(gcmem_t *mem, unsigned capacity) {
    object_data_t *data = gcmem_get_object_data_from_pool(mem, OBJECT_ARRAY);
    if (data) {
        array_clear(data->array);
        return object_make_from_data(OBJECT_ARRAY, data);
    }
    data = gcmem_alloc_object_data(mem, OBJECT_ARRAY);
    if (!data) {
        return object_make_null();
    }
    data->array = array_make_with_capacity(mem->alloc, capacity, sizeof(object_t));
    if (!data->array) {
        return object_make_null();
    }
    return object_make_from_data(OBJECT_ARRAY, data);
}

object_t object_make_map(gcmem_t *mem) {
    return object_make_map_with_capacity(mem, 32);
}

object_t object_make_map_with_capacity(gcmem_t *mem, unsigned capacity) {
    object_data_t *data = gcmem_get_object_data_from_pool(mem, OBJECT_MAP);
    if (data) {
        valdict_clear(data->map);
        return object_make_from_data(OBJECT_MAP, data);
    }
    data = gcmem_alloc_object_data(mem, OBJECT_MAP);
    if (!data) {
        return object_make_null();
    }
    data->map = valdict_make_with_capacity(mem->alloc, capacity, sizeof(object_t), sizeof(object_t));
    if (!data->map) {
        return object_make_null();
    }
    valdict_set_hash_function(data->map, (collections_hash_fn)object_hash);
    valdict_set_equals_function(data->map, (collections_equals_fn)object_equals_wrapped);
    return object_make_from_data(OBJECT_MAP, data);
}

object_t object_make_error(gcmem_t *mem, const char *error) {
    char *error_str = ape_strdup(mem->alloc, error);
    if (!error_str) {
        return object_make_null();
    }
    object_t res = object_make_error_no_copy(mem, error_str);
    if (object_is_null(res)) {
        allocator_free(mem->alloc, error_str);
        return object_make_null();
    }
    return res;
}

object_t object_make_error_no_copy(gcmem_t *mem, char *error) {
    object_data_t *data = gcmem_alloc_object_data(mem, OBJECT_ERROR);
    if (!data) {
        return object_make_null();
    }
    data->error.message = error;
    data->error.traceback = NULL;
    return object_make_from_data(OBJECT_ERROR, data);
}

object_t object_make_errorf(gcmem_t *mem, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int to_write = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    va_start(args, fmt);
    char *res = (char*)allocator_malloc(mem->alloc, to_write + 1);
    if (!res) {
        return object_make_null();
    }
    int written = vsprintf(res, fmt, args);
    (void)written;
    APE_ASSERT(written == to_write);
    va_end(args);
    object_t res_obj = object_make_error_no_copy(mem, res);
    if (object_is_null(res_obj)) {
        allocator_free(mem->alloc, res);
        return object_make_null();
    }
    return res_obj;
}

object_t object_make_function(gcmem_t *mem, const char *name, compilation_result_t *comp_res, bool owns_data,
                              int num_locals, int num_args,
                              int free_vals_count) {

    object_data_t *data = gcmem_alloc_object_data(mem, OBJECT_FUNCTION);
    if (!data) {
        return object_make_null();
    }
    if (owns_data) {
        data->function.name = name ? ape_strdup(mem->alloc, name) : ape_strdup(mem->alloc, "anonymous");
        if (!data->function.name) {
            return object_make_null();
        }
    } else {
        data->function.const_name = name ? name : "anonymous";
    }
    data->function.comp_result = comp_res;
    data->function.owns_data = owns_data;
    data->function.num_locals = num_locals;
    data->function.num_args = num_args;
    if (free_vals_count >= APE_ARRAY_LEN(data->function.free_vals_buf)) {
        data->function.free_vals_allocated = allocator_malloc(mem->alloc, sizeof(object_t) * free_vals_count);
        if (!data->function.free_vals_allocated) {
            return object_make_null();
        }
    }
    data->function.free_vals_count = free_vals_count;
    return object_make_from_data(OBJECT_FUNCTION, data);
}

object_t object_make_external(gcmem_t *mem, void *data) {
    object_data_t *obj = gcmem_alloc_object_data(mem, OBJECT_EXTERNAL);
    if (!obj) {
        return object_make_null();
    }
    obj->external.data = data;
    obj->external.data_destroy_fn = NULL;
    obj->external.data_copy_fn = NULL;
    return object_make_from_data(OBJECT_EXTERNAL, obj);
}

void object_deinit(object_t obj) {
    if (object_is_allocated(obj)) {
        object_data_t *data = object_get_allocated_data(obj);
        object_data_deinit(data);
    }
}

void object_data_deinit(object_data_t *data) {
    switch (data->type) {
        case OBJECT_FREED: {
            APE_ASSERT(false);
            return;
        }
        case OBJECT_STRING: {
            if (data->string.is_allocated) {
                allocator_free(data->mem->alloc, data->string.value_allocated);
            }
            break;
        }
        case OBJECT_FUNCTION: {
            if (data->function.owns_data) {
                allocator_free(data->mem->alloc, data->function.name);
                compilation_result_destroy(data->function.comp_result);
            }
            if (freevals_are_allocated(&data->function)) {
                allocator_free(data->mem->alloc, data->function.free_vals_allocated);
            }
            break;
        }
        case OBJECT_ARRAY: {
            array_destroy(data->array);
            break;
        }
        case OBJECT_MAP: {
            valdict_destroy(data->map);
            break;
        }
        case OBJECT_NATIVE_FUNCTION: {
            allocator_free(data->mem->alloc, data->native_function.name);
            break;
        }
        case OBJECT_EXTERNAL: {
            if (data->external.data_destroy_fn) {
                data->external.data_destroy_fn(data->external.data);
            }
            break;
        }
        case OBJECT_ERROR: {
            allocator_free(data->mem->alloc, data->error.message);
            traceback_destroy(data->error.traceback);
            break;
        }
        default: {
            break;
        }
    }
    data->type = OBJECT_FREED;
}

bool object_is_allocated(object_t object) {
    return (object.handle & OBJECT_ALLOCATED_HEADER) == OBJECT_ALLOCATED_HEADER;
}

gcmem_t* object_get_mem(object_t obj) {
    object_data_t *data = object_get_allocated_data(obj);
    return data->mem;
}

bool object_is_hashable(object_t obj) {
    object_type_t type = object_get_type(obj);
    switch (type) {
        case OBJECT_STRING: return true;
        case OBJECT_NUMBER: return true;
        case OBJECT_BOOL:   return true;
        default: return false;
    }
}

void object_to_string(object_t obj, strbuf_t *buf, bool quote_str) {
    object_type_t type = object_get_type(obj);
    switch (type) {
        case OBJECT_FREED: {
            strbuf_append(buf, "FREED");
            break;
        }
        case OBJECT_NONE: {
            strbuf_append(buf, "NONE");
            break;
        }
        case OBJECT_NUMBER: {
            double number = object_get_number(obj);
            strbuf_appendf(buf, "%1.10g", number);
            break;
        }
        case OBJECT_BOOL: {
            strbuf_append(buf, object_get_bool(obj) ? "true" : "false");
            break;
        }
        case OBJECT_STRING: {
            const char *string = object_get_string(obj);
            if (quote_str) {
                strbuf_appendf(buf, "\"%s\"", string);
            } else {
                strbuf_append(buf, string);
            }
            break;
        }
        case OBJECT_NULL: {
            strbuf_append(buf, "null");
            break;
        }
        case OBJECT_FUNCTION: {
            const function_t *function = object_get_function(obj);
            strbuf_appendf(buf, "CompiledFunction: %s\n", object_get_function_name(obj));
            code_to_string(function->comp_result->bytecode, function->comp_result->src_positions, function->comp_result->count, buf);
            break;
        }
        case OBJECT_ARRAY: {
            strbuf_append(buf, "[");
            for (int i = 0; i < object_get_array_length(obj); i++) {
                object_t iobj = object_get_array_value_at(obj, i);
                object_to_string(iobj, buf, true);
                if (i < (object_get_array_length(obj) - 1)) {
                    strbuf_append(buf, ", ");
                }
            }
            strbuf_append(buf, "]");
            break;
        }
        case OBJECT_MAP: {
            strbuf_append(buf, "{");
            for (int i = 0; i < object_get_map_length(obj); i++) {
                object_t key = object_get_map_key_at(obj, i);
                object_t val = object_get_map_value_at(obj, i);
                object_to_string(key, buf, true);
                strbuf_append(buf, ": ");
                object_to_string(val, buf, true);
                if (i < (object_get_map_length(obj) - 1)) {
                    strbuf_append(buf, ", ");
                }
            }
            strbuf_append(buf, "}");
            break;
        }
        case OBJECT_NATIVE_FUNCTION: {
            strbuf_append(buf, "NATIVE_FUNCTION");
            break;
        }
        case OBJECT_EXTERNAL: {
            strbuf_append(buf, "EXTERNAL");
            break;
        }
        case OBJECT_ERROR: {
            strbuf_appendf(buf, "ERROR: %s\n", object_get_error_message(obj));
            traceback_t *traceback = object_get_error_traceback(obj);
            APE_ASSERT(traceback);
            if (traceback) {
                strbuf_append(buf, "Traceback:\n");
                traceback_to_string(traceback, buf);
            }
            break;
        }
        case OBJECT_ANY: {
            APE_ASSERT(false);
        }
    }
}

const char *object_get_type_name(const object_type_t type) {
    switch (type) {
        case OBJECT_NONE:            return "NONE";
        case OBJECT_FREED:           return "NONE";
        case OBJECT_NUMBER:          return "NUMBER";
        case OBJECT_BOOL:            return "BOOL";
        case OBJECT_STRING:          return "STRING";
        case OBJECT_NULL:            return "NULL";
        case OBJECT_NATIVE_FUNCTION: return "NATIVE_FUNCTION";
        case OBJECT_ARRAY:           return "ARRAY";
        case OBJECT_MAP:             return "MAP";
        case OBJECT_FUNCTION:        return "FUNCTION";
        case OBJECT_EXTERNAL:        return "EXTERNAL";
        case OBJECT_ERROR:           return "ERROR";
        case OBJECT_ANY:             return "ANY";
    }
    return "NONE";
}

char* object_get_type_union_name(allocator_t *alloc, const object_type_t type) {
    if (type == OBJECT_ANY || type == OBJECT_NONE || type == OBJECT_FREED) {
        return ape_strdup(alloc, object_get_type_name(type));
    }
    strbuf_t *res = strbuf_make(alloc);
    if (!res) {
        return NULL;
    }
    bool in_between = false;
#define CHECK_TYPE(t) \
    do {\
        if ((type & t) == t) {\
            if (in_between) {\
                strbuf_append(res, "|");\
            }\
            strbuf_append(res, object_get_type_name(t));\
            in_between = true;\
        }\
    } while (0)

    CHECK_TYPE(OBJECT_NUMBER);
    CHECK_TYPE(OBJECT_BOOL);
    CHECK_TYPE(OBJECT_STRING);
    CHECK_TYPE(OBJECT_NULL);
    CHECK_TYPE(OBJECT_NATIVE_FUNCTION);
    CHECK_TYPE(OBJECT_ARRAY);
    CHECK_TYPE(OBJECT_MAP);
    CHECK_TYPE(OBJECT_FUNCTION);
    CHECK_TYPE(OBJECT_EXTERNAL);
    CHECK_TYPE(OBJECT_ERROR);

    return strbuf_get_string_and_destroy(res);
}

char* object_serialize(allocator_t *alloc, object_t object) {
    strbuf_t *buf = strbuf_make(alloc);
    if (!buf) {
        return NULL;
    }
    object_to_string(object, buf, true);
    char *string = strbuf_get_string_and_destroy(buf);
    return string;
}

object_t object_deep_copy(gcmem_t *mem, object_t obj) {
    valdict(object_t, object_t) *copies = valdict_make(mem->alloc, object_t, object_t);
    if (!copies) {
        return object_make_null();
    }
    object_t res = object_deep_copy_internal(mem, obj, copies);
    valdict_destroy(copies);
    return res;
}

object_t object_copy(gcmem_t *mem, object_t obj) {
    object_t copy = object_make_null();
    object_type_t type = object_get_type(obj);
    switch (type) {
        case OBJECT_ANY:
        case OBJECT_FREED:
        case OBJECT_NONE: {
            APE_ASSERT(false);
            copy = object_make_null();
            break;
        }
        case OBJECT_NUMBER:
        case OBJECT_BOOL:
        case OBJECT_NULL:
        case OBJECT_FUNCTION:
        case OBJECT_NATIVE_FUNCTION:
        case OBJECT_ERROR: {
            copy = obj;
            break;
        }
        case OBJECT_STRING: {
            const char *str = object_get_string(obj);
            copy = object_make_string(mem, str);
            break;
        }
        case OBJECT_ARRAY: {
            int len = object_get_array_length(obj);
            copy = object_make_array_with_capacity(mem, len);
            if (object_is_null(copy)) {
                return object_make_null();
            }
            for (int i = 0; i < len; i++) {
                object_t item = object_get_array_value_at(obj, i);
                bool ok = object_add_array_value(copy, item);
                if (!ok) {
                    return object_make_null();
                }
            }
            break;
        }
        case OBJECT_MAP: {
            copy = object_make_map(mem);
            for (int i = 0; i < object_get_map_length(obj); i++) {
                object_t key = object_get_map_key_at(obj, i);
                object_t val = object_get_map_value_at(obj, i);
                bool ok = object_set_map_value(copy, key, val);
                if (!ok) {
                    return object_make_null();
                }
            }
            break;
        }
        case OBJECT_EXTERNAL: {
            copy = object_make_external(mem, NULL);
            if (object_is_null(copy)) {
                return object_make_null();
            }
            external_data_t *external = object_get_external_data(obj);
            void *data_copy = NULL;
            if (external->data_copy_fn) {
                data_copy = external->data_copy_fn(external->data);
            } else {
                data_copy = external->data;
            }
            object_set_external_data(copy, data_copy);
            object_set_external_destroy_function(copy, external->data_destroy_fn);
            object_set_external_copy_function(copy, external->data_copy_fn);
            break;
        }
    }
    return copy;
}

double object_compare(object_t a, object_t b) {
    if (a.handle == b.handle) {
        return 0;
    }
    
    object_type_t a_type = object_get_type(a);
    object_type_t b_type = object_get_type(b);

    if ((a_type == OBJECT_NUMBER || a_type == OBJECT_BOOL || a_type == OBJECT_NULL)
        && (b_type == OBJECT_NUMBER || b_type == OBJECT_BOOL || b_type == OBJECT_NULL)) {
        double left_val = object_get_number(a);
        double right_val = object_get_number(b);
        return left_val - right_val;
    } else if (a_type == b_type && a_type == OBJECT_STRING) {
        const char *left_string = object_get_string(a);
        const char *right_string = object_get_string(b);
        return strcmp(left_string, right_string);
    } else {
        intptr_t a_data_val = (intptr_t)object_get_allocated_data(a);
        intptr_t b_data_val = (intptr_t)object_get_allocated_data(b);
        return (double)(a_data_val - b_data_val);
    }
}

bool object_equals(object_t a, object_t b) {
    object_type_t a_type = object_get_type(a);
    object_type_t b_type = object_get_type(b);

    if (a_type != b_type) {
        return false;
    }
    double res = object_compare(a, b);
    return APE_DBLEQ(res, 0);
}

external_data_t* object_get_external_data(object_t object) {
    APE_ASSERT(object_get_type(object) == OBJECT_EXTERNAL);
    object_data_t *data = object_get_allocated_data(object);
    return &data->external;
}

bool object_set_external_destroy_function(object_t object, external_data_destroy_fn destroy_fn) {
    APE_ASSERT(object_get_type(object) == OBJECT_EXTERNAL);
    external_data_t* data = object_get_external_data(object);
    if (!data) {
        return false;
    }
    data->data_destroy_fn = destroy_fn;
    return true;
}

object_data_t* object_get_allocated_data(object_t object) {
    APE_ASSERT(object_is_allocated(object) || object_get_type(object) == OBJECT_NULL);
    return (object_data_t*)(object.handle & ~OBJECT_HEADER_MASK);
}

bool object_get_bool(object_t obj) {
    if (object_is_number(obj)) {
        return obj.handle;
    }
    return obj.handle & (~OBJECT_HEADER_MASK);
}

double object_get_number(object_t obj) {
    if (object_is_number(obj)) { // todo: optimise? always return number?
        return obj.number;
    }
    return (double)(obj.handle & (~OBJECT_HEADER_MASK));
}

const char * object_get_string(object_t object) {
    APE_ASSERT(object_get_type(object) == OBJECT_STRING);
    object_data_t *data = object_get_allocated_data(object);
    if (data->string.is_allocated) {
        return data->string.value_allocated;
    } else {
        return data->string.value_buf;
    }
}

function_t* object_get_function(object_t object) {
    APE_ASSERT(object_get_type(object) == OBJECT_FUNCTION);
    object_data_t *data = object_get_allocated_data(object);
    return &data->function;
}

native_function_t* object_get_native_function(object_t obj) {
    object_data_t *data = object_get_allocated_data(obj);
    return &data->native_function;
}

object_type_t object_get_type(object_t obj) {
    if (object_is_number(obj)) {
        return OBJECT_NUMBER;
    }
    uint64_t tag = (obj.handle >> 48) & 0x7;
    switch (tag) {
        case 0: return OBJECT_NONE;
        case 1: return OBJECT_BOOL;
        case 2: return OBJECT_NULL;
        case 4: {
            object_data_t *data = object_get_allocated_data(obj);
            return data->type;
        }
        default: return OBJECT_NONE;
    }
}

bool object_is_numeric(object_t obj) {
    object_type_t type = object_get_type(obj);
    return type == OBJECT_NUMBER || type == OBJECT_BOOL;
}

bool object_is_null(object_t obj) {
    return object_get_type(obj) == OBJECT_NULL;
}

bool object_is_callable(object_t obj) {
    object_type_t type = object_get_type(obj);
    return type == OBJECT_NATIVE_FUNCTION || type == OBJECT_FUNCTION;
}

const char* object_get_function_name(object_t obj) {
    APE_ASSERT(object_get_type(obj) == OBJECT_FUNCTION);
    object_data_t *data = object_get_allocated_data(obj);
    APE_ASSERT(data);
    if (!data) {
        return NULL;
    }

    if (data->function.owns_data) {
        return data->function.name;
    } else {
        return data->function.const_name;
    }
}

object_t object_get_function_free_val(object_t obj, int ix) {
    APE_ASSERT(object_get_type(obj) == OBJECT_FUNCTION);
    object_data_t *data = object_get_allocated_data(obj);
    APE_ASSERT(data);
    if (!data) {
        return object_make_null();
    }
    function_t *fun = &data->function;
    APE_ASSERT(ix >= 0 && ix < fun->free_vals_count);
    if (ix < 0 || ix >= fun->free_vals_count) {
        return object_make_null();
    }
    if (freevals_are_allocated(fun)) {
        return fun->free_vals_allocated[ix];
    } else {
        return fun->free_vals_buf[ix];
    }
}

void object_set_function_free_val(object_t obj, int ix, object_t val) {
    APE_ASSERT(object_get_type(obj) == OBJECT_FUNCTION);
    object_data_t *data = object_get_allocated_data(obj);
    APE_ASSERT(data);
    if (!data) {
        return;
    }
    function_t *fun = &data->function;
    APE_ASSERT(ix >= 0 && ix < fun->free_vals_count);
    if (ix < 0 || ix >= fun->free_vals_count) {
        return;
    }
    if (freevals_are_allocated(fun)) {
        fun->free_vals_allocated[ix] = val;
    } else {
        fun->free_vals_buf[ix] = val;
    }
}

object_t* object_get_function_free_vals(object_t obj) {
    APE_ASSERT(object_get_type(obj) == OBJECT_FUNCTION);
    object_data_t *data = object_get_allocated_data(obj);
    APE_ASSERT(data);
    if (!data) {
        return NULL;
    }
    function_t *fun = &data->function;
    if (freevals_are_allocated(fun)) {
        return fun->free_vals_allocated;
    } else {
        return fun->free_vals_buf;
    }
}

const char* object_get_error_message(object_t object) {
    APE_ASSERT(object_get_type(object) == OBJECT_ERROR);
    object_data_t *data = object_get_allocated_data(object);
    return data->error.message;
}

void object_set_error_traceback(object_t object, traceback_t *traceback) {
    APE_ASSERT(object_get_type(object) == OBJECT_ERROR);
    if (object_get_type(object) != OBJECT_ERROR) {
        return;
    }
    object_data_t *data = object_get_allocated_data(object);
    APE_ASSERT(data->error.traceback == NULL);
    data->error.traceback = traceback;
}

traceback_t* object_get_error_traceback(object_t object) {
    APE_ASSERT(object_get_type(object) == OBJECT_ERROR);
    object_data_t *data = object_get_allocated_data(object);
    return data->error.traceback;
}

bool object_set_external_data(object_t object, void *ext_data) {
    APE_ASSERT(object_get_type(object) == OBJECT_EXTERNAL);
    external_data_t* data = object_get_external_data(object);
    if (!data) {
        return false;
    }
    data->data = ext_data;
    return true;
}

bool object_set_external_copy_function(object_t object, external_data_copy_fn copy_fn) {
    APE_ASSERT(object_get_type(object) == OBJECT_EXTERNAL);
    external_data_t* data = object_get_external_data(object);
    if (!data) {
        return false;
    }
    data->data_copy_fn = copy_fn;
    return true;
}

object_t object_get_array_value_at(object_t object, int ix) {
    APE_ASSERT(object_get_type(object) == OBJECT_ARRAY);
    array(object_t)* array = object_get_allocated_array(object);
    if (ix < 0 || ix >= array_count(array)) {
        return object_make_null();
    }
    object_t *res = array_get(array, ix);
    if (!res) {
        return object_make_null();
    }
    return *res;
}

bool object_set_array_value_at(object_t object, int ix, object_t val) {
    APE_ASSERT(object_get_type(object) == OBJECT_ARRAY);
    array(object_t)* array = object_get_allocated_array(object);
    if (ix < 0 || ix >= array_count(array)) {
        return false;
    }
    return array_set(array, ix, &val);
}

bool object_add_array_value(object_t object, object_t val) {
    APE_ASSERT(object_get_type(object) == OBJECT_ARRAY);
    array(object_t)* array = object_get_allocated_array(object);
    return array_add(array, &val);
}

int object_get_array_length(object_t object) {
    APE_ASSERT(object_get_type(object) == OBJECT_ARRAY);
    array(object_t)* array = object_get_allocated_array(object);
    return array_count(array);
}

APE_INTERNAL bool object_remove_array_value_at(object_t object, int ix) {
    array(object_t)* array = object_get_allocated_array(object);
    return array_remove_at(array, ix);
}

int object_get_map_length(object_t object) {
    APE_ASSERT(object_get_type(object) == OBJECT_MAP);
    object_data_t *data = object_get_allocated_data(object);
    return valdict_count(data->map);
}

object_t object_get_map_key_at(object_t object, int ix) {
    APE_ASSERT(object_get_type(object) == OBJECT_MAP);
    object_data_t *data = object_get_allocated_data(object);
    object_t *res = valdict_get_key_at(data->map, ix);
    if (!res) {
        return object_make_null();
    }
    return *res;
}

object_t object_get_map_value_at(object_t object, int ix) {
    APE_ASSERT(object_get_type(object) == OBJECT_MAP);
    object_data_t *data = object_get_allocated_data(object);
    object_t *res = valdict_get_value_at(data->map, ix);
    if (!res) {
        return object_make_null();
    }
    return *res;
}

bool object_set_map_value_at(object_t object, int ix, object_t val) {
    APE_ASSERT(object_get_type(object) == OBJECT_MAP);
    if (ix >= object_get_map_length(object)) {
        return false;
    }
    object_data_t *data = object_get_allocated_data(object);
    return valdict_set_value_at(data->map, ix, &val);
}

object_t object_get_kv_pair_at(gcmem_t *mem, object_t object, int ix) {
    APE_ASSERT(object_get_type(object) == OBJECT_MAP);
    object_data_t *data = object_get_allocated_data(object);
    if (ix >= valdict_count(data->map)) {
        return object_make_null();
    }
    object_t key = object_get_map_key_at(object, ix);
    object_t val = object_get_map_value_at(object, ix);
    object_t res = object_make_map(mem);
    if (object_is_null(res)) {
        return object_make_null();
    }

    object_t key_obj = object_make_string(mem, "key");
    if (object_is_null(key_obj)) {
        return object_make_null();
    }
    object_set_map_value(res, key_obj, key);

    object_t val_obj = object_make_string(mem, "value");
    if (object_is_null(val_obj)) {
        return object_make_null();
    }
    object_set_map_value(res, val_obj, val);

    return res;
}

bool object_set_map_value(object_t object, object_t key, object_t val) {
    APE_ASSERT(object_get_type(object) == OBJECT_MAP);
    object_data_t *data = object_get_allocated_data(object);
    return valdict_set(data->map, &key, &val);
}

object_t object_get_map_value(object_t object, object_t key) {
    APE_ASSERT(object_get_type(object) == OBJECT_MAP);
    object_data_t *data = object_get_allocated_data(object);
    object_t *res = valdict_get(data->map, &key);
    if (!res) {
        return object_make_null();
    }
    return *res;
}

bool object_map_has_key(object_t object, object_t key) {
    APE_ASSERT(object_get_type(object) == OBJECT_MAP);
    object_data_t *data = object_get_allocated_data(object);
    object_t *res = valdict_get(data->map, &key);
    return res != NULL;
}

// INTERNAL
static object_t object_deep_copy_internal(gcmem_t *mem, object_t obj, valdict(object_t, object_t) *copies) {
    object_t *copy_ptr = valdict_get(copies, &obj);
    if (copy_ptr) {
        return *copy_ptr;
    }

    object_t copy = object_make_null()  ;

    object_type_t type = object_get_type(obj);
    switch (type) {
        case OBJECT_FREED:
        case OBJECT_ANY:
        case OBJECT_NONE: {
            APE_ASSERT(false);
            copy = object_make_null();
            break;
        }
        case OBJECT_NUMBER:
        case OBJECT_BOOL:
        case OBJECT_NULL:
        case OBJECT_NATIVE_FUNCTION: {
            copy = obj;
            break;
        }
        case OBJECT_STRING: {
            const char *str = object_get_string(obj);
            copy = object_make_string(mem, str);
            break;
        }
        case OBJECT_FUNCTION: {
            function_t *function = object_get_function(obj);
            uint8_t *bytecode_copy = NULL;
            src_pos_t *src_positions_copy = NULL;
            compilation_result_t *comp_res_copy = NULL;

            bytecode_copy = allocator_malloc(mem->alloc, sizeof(uint8_t) * function->comp_result->count);
            if (!bytecode_copy) {
                return object_make_null();
            }
            memcpy(bytecode_copy, function->comp_result->bytecode, sizeof(uint8_t) * function->comp_result->count);

            src_positions_copy = allocator_malloc(mem->alloc, sizeof(src_pos_t) * function->comp_result->count);
            if (!src_positions_copy) {
                allocator_free(mem->alloc, bytecode_copy);
                return object_make_null();
            }
            memcpy(src_positions_copy, function->comp_result->src_positions, sizeof(src_pos_t) * function->comp_result->count);

            comp_res_copy = compilation_result_make(mem->alloc, bytecode_copy, src_positions_copy, function->comp_result->count); // todo: add compilation result copy function
            if (!comp_res_copy) {
                allocator_free(mem->alloc, src_positions_copy);
                allocator_free(mem->alloc, bytecode_copy);
                return object_make_null();
            }

            copy = object_make_function(mem, object_get_function_name(obj), comp_res_copy, true,
                                        function->num_locals, function->num_args, 0);
            if (object_is_null(copy)) {
                compilation_result_destroy(comp_res_copy);
                return object_make_null();
            }

            bool ok = valdict_set(copies, &obj, &copy);
            if (!ok) {
                return object_make_null();
            }

            function_t *function_copy = object_get_function(copy);
            if (freevals_are_allocated(function)) {
                function_copy->free_vals_allocated = allocator_malloc(mem->alloc, sizeof(object_t) * function->free_vals_count);
                if (!function_copy->free_vals_allocated) {
                    return object_make_null();
                }
            }

            function_copy->free_vals_count = function->free_vals_count;
            for (int i = 0; i < function->free_vals_count; i++) {
                object_t free_val = object_get_function_free_val(obj, i);
                object_t free_val_copy = object_deep_copy_internal(mem, free_val, copies);
                if (!object_is_null(free_val) && object_is_null(free_val_copy)) {
                    return object_make_null();
                }
                object_set_function_free_val(copy, i, free_val_copy);
            }
            break;
        }
        case OBJECT_ARRAY: {
            int len = object_get_array_length(obj);
            copy = object_make_array_with_capacity(mem, len);
            if (object_is_null(copy)) {
                return object_make_null();
            }
            bool ok = valdict_set(copies, &obj, &copy);
            if (!ok) {
                return object_make_null();
            }
            for (int i = 0; i < len; i++) {
                object_t item = object_get_array_value_at(obj, i);
                object_t item_copy = object_deep_copy_internal(mem, item, copies);
                if (!object_is_null(item) && object_is_null(item_copy)) {
                    return object_make_null();
                }
                bool ok = object_add_array_value(copy, item_copy);
                if (!ok) {
                    return object_make_null();
                }
            }
            break;
        }
        case OBJECT_MAP: {
            copy = object_make_map(mem);
            if (object_is_null(copy)) {
                return object_make_null();
            }
            bool ok = valdict_set(copies, &obj, &copy);
            if (!ok) {
                return object_make_null();
            }
            for (int i = 0; i < object_get_map_length(obj); i++) {
                object_t key = object_get_map_key_at(obj, i);
                object_t val = object_get_map_value_at(obj, i);

                object_t key_copy = object_deep_copy_internal(mem, key, copies);
                if (!object_is_null(key) && object_is_null(key_copy)) {
                    return object_make_null();
                }

                object_t val_copy = object_deep_copy_internal(mem, val, copies);
                if (!object_is_null(val) && object_is_null(val_copy)) {
                    return object_make_null();
                }

                bool ok = object_set_map_value(copy, key_copy, val_copy);
                if (!ok) {
                    return object_make_null();
                }
            }
            break;
        }
        case OBJECT_EXTERNAL: {
            copy = object_copy(mem, obj);
            break;
        }
        case OBJECT_ERROR: {
            copy = obj;
            break;
        }
    }
    return copy;
}


static bool object_equals_wrapped(const object_t *a_ptr, const object_t *b_ptr) {
    object_t a = *a_ptr;
    object_t b = *b_ptr;
    return object_equals(a, b);
}

static unsigned long object_hash(object_t *obj_ptr) {
    object_t obj = *obj_ptr;
    object_type_t type = object_get_type(obj);

    switch (type) {
        case OBJECT_NUMBER: {
            double val = object_get_number(obj);
            return object_hash_double(val);
        }
        case OBJECT_BOOL: {
            bool val = object_get_bool(obj);
            return val;
        }
        case OBJECT_STRING: {
            object_data_t *data = object_get_allocated_data(obj);
            return data->string.hash;
        }
        default: {
            return 0;
        }
    }
}

static unsigned long object_hash_string(const char *str) { /* djb2 */
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

static unsigned long object_hash_double(double val) { /* djb2 */
    uint32_t *val_ptr = (uint32_t*)&val;
    unsigned long hash = 5381;
    hash = ((hash << 5) + hash) + val_ptr[0];
    hash = ((hash << 5) + hash) + val_ptr[1];
    return hash;
}

array(object_t)* object_get_allocated_array(object_t object) {
    APE_ASSERT(object_get_type(object) == OBJECT_ARRAY);
    object_data_t *data = object_get_allocated_data(object);
    return data->array;
}

static bool object_is_number(object_t o) {
    return (o.handle & OBJECT_PATTERN) != OBJECT_PATTERN;
}

static uint64_t get_type_tag(object_type_t type) {
    switch (type) {
        case OBJECT_NONE: return 0;
        case OBJECT_BOOL: return 1;
        case OBJECT_NULL: return 2;
        default:          return 4;
    }
}

static bool freevals_are_allocated(function_t *fun) {
    return fun->free_vals_count >= APE_ARRAY_LEN(fun->free_vals_buf);
}
//FILE_END
//FILE_START:gc.c
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef APE_AMALGAMATED
#include "gc.h"
#include "object.h"
#endif

static object_data_pool_t* get_pool_for_type(gcmem_t *mem, object_type_t type);
static bool can_data_be_put_in_pool(gcmem_t *mem, object_data_t *data);

gcmem_t *gcmem_make(allocator_t *alloc) {
    gcmem_t *mem = allocator_malloc(alloc, sizeof(gcmem_t));
    if (!mem) {
        return NULL;
    }
    memset(mem, 0, sizeof(gcmem_t));
    mem->alloc = alloc;
    mem->objects = ptrarray_make(alloc);
    if (!mem->objects) {
        goto error;
    }
    mem->objects_back = ptrarray_make(alloc);
    if (!mem->objects_back) {
        goto error;
    }
    mem->objects_not_gced = array_make(alloc, object_t);
    if (!mem->objects_not_gced) {
        goto error;
    }
    mem->allocations_since_sweep = 0;
    mem->data_only_pool.count = 0;

    for (int i = 0; i < GCMEM_POOLS_NUM; i++) {
        object_data_pool_t *pool = &mem->pools[i];
        mem->pools[i].count = 0;
        memset(pool, 0, sizeof(object_data_pool_t));
    }

    return mem;
error:
    gcmem_destroy(mem);
    return NULL;
}

void gcmem_destroy(gcmem_t *mem) {
    if (!mem) {
        return;
    }

    array_destroy(mem->objects_not_gced);
    ptrarray_destroy(mem->objects_back);

    for (int i = 0; i < ptrarray_count(mem->objects); i++) {
        object_data_t *obj = ptrarray_get(mem->objects, i);
        object_data_deinit(obj);
        allocator_free(mem->alloc, obj);
    }
    ptrarray_destroy(mem->objects);

    for (int i = 0; i < GCMEM_POOLS_NUM; i++) {
        object_data_pool_t *pool = &mem->pools[i];
        for (int j = 0; j < pool->count; j++) {
            object_data_t *data = pool->data[j];
            object_data_deinit(data);
            allocator_free(mem->alloc, data);
        }
        memset(pool, 0, sizeof(object_data_pool_t));
    }

    for (int i = 0; i < mem->data_only_pool.count; i++) {
        allocator_free(mem->alloc, mem->data_only_pool.data[i]);
    }

    allocator_free(mem->alloc, mem);
}

object_data_t* gcmem_alloc_object_data(gcmem_t *mem, object_type_t type) {
    object_data_t *data = NULL;
    mem->allocations_since_sweep++;
    if (mem->data_only_pool.count > 0) {
        data = mem->data_only_pool.data[mem->data_only_pool.count - 1];
        mem->data_only_pool.count--;
    } else {
        data = allocator_malloc(mem->alloc, sizeof(object_data_t));
        if (!data) {
            return NULL;
        }
    }

    memset(data, 0, sizeof(object_data_t));

    APE_ASSERT(ptrarray_count(mem->objects_back) >= ptrarray_count(mem->objects));
    // we want to make sure that appending to objects_back never fails in sweep
    // so this only reserves space there.
    bool ok = ptrarray_add(mem->objects_back, data);
    if (!ok) {
        allocator_free(mem->alloc, data);
        return NULL;
    }
    ok = ptrarray_add(mem->objects, data);
    if (!ok) {
        allocator_free(mem->alloc, data);
        return NULL;
    }
    data->mem = mem;
    data->type = type;
    return data;
}

object_data_t* gcmem_get_object_data_from_pool(gcmem_t *mem, object_type_t type) {
    object_data_pool_t *pool = get_pool_for_type(mem, type);
    if (!pool || pool->count <= 0) {
        return NULL;
    }
    object_data_t *data = pool->data[pool->count - 1];

    APE_ASSERT(ptrarray_count(mem->objects_back) >= ptrarray_count(mem->objects));

    // we want to make sure that appending to objects_back never fails in sweep
    // so this only reserves space there.
    bool ok = ptrarray_add(mem->objects_back, data);
    if (!ok) {
        return NULL;
    }
    ok = ptrarray_add(mem->objects, data);
    if (!ok) {
        return NULL;
    }

    pool->count--;
    return data;
}

void gc_unmark_all(gcmem_t *mem) {
    for (int i = 0; i < ptrarray_count(mem->objects); i++) {
        object_data_t *data = ptrarray_get(mem->objects, i);
        data->gcmark = false;
    }
}

void gc_mark_objects(object_t *objects, int count) {
    for (int i = 0; i < count; i++) {
        object_t obj = objects[i];
        gc_mark_object(obj);
    }
}

void gc_mark_object(object_t obj) {
    if (!object_is_allocated(obj)) {
        return;
    }

    object_data_t *data = object_get_allocated_data(obj);
    if (data->gcmark) {
        return;
    }

    data->gcmark = true;
    switch (data->type) {
        case OBJECT_MAP: {
            int len = object_get_map_length(obj);
            for (int i = 0; i < len; i++) {
                object_t key = object_get_map_key_at(obj, i);
                if (object_is_allocated(key)) {
                    object_data_t *key_data = object_get_allocated_data(key);
                    if (!key_data->gcmark) {
                        gc_mark_object(key);
                    }
                }
                object_t val = object_get_map_value_at(obj, i);
                if (object_is_allocated(val)) {
                    object_data_t *val_data = object_get_allocated_data(val);
                    if (!val_data->gcmark) {
                        gc_mark_object(val);
                    }
                }
            }
            break;
        }
        case OBJECT_ARRAY: {
            int len = object_get_array_length(obj);
            for (int i = 0; i < len; i++) {
                object_t val = object_get_array_value_at(obj, i);
                if (object_is_allocated(val)) {
                    object_data_t *val_data = object_get_allocated_data(val);
                    if (!val_data->gcmark) {
                        gc_mark_object(val);
                    }
                }
            }
            break;
        }
        case OBJECT_FUNCTION: {
            function_t *function = object_get_function(obj);
            for (int i = 0; i < function->free_vals_count; i++) {
                object_t free_val = object_get_function_free_val(obj, i);
                gc_mark_object(free_val);
                if (object_is_allocated(free_val)) {
                    object_data_t *free_val_data = object_get_allocated_data(free_val);
                    if (!free_val_data->gcmark) {
                        gc_mark_object(free_val);
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void gc_sweep(gcmem_t *mem) {
    gc_mark_objects(array_data(mem->objects_not_gced), array_count(mem->objects_not_gced));

    APE_ASSERT(ptrarray_count(mem->objects_back) >= ptrarray_count(mem->objects));

    ptrarray_clear(mem->objects_back);
    for (int i = 0; i < ptrarray_count(mem->objects); i++) {
        object_data_t *data = ptrarray_get(mem->objects, i);
        if (data->gcmark) {
            // this should never fail because objects_back's size should be equal to objects
            bool ok = ptrarray_add(mem->objects_back, data);
            (void)ok;
            APE_ASSERT(ok);
        } else {
            if (can_data_be_put_in_pool(mem, data)) {
                object_data_pool_t *pool = get_pool_for_type(mem, data->type);
                pool->data[pool->count] = data;
                pool->count++;
            } else {
                object_data_deinit(data);
                if (mem->data_only_pool.count < GCMEM_POOL_SIZE) {
                    mem->data_only_pool.data[mem->data_only_pool.count] = data;
                    mem->data_only_pool.count++;
                } else {
                    allocator_free(mem->alloc, data);
                }
            }
        }
    }
    ptrarray(object_t) *objs_temp = mem->objects;
    mem->objects = mem->objects_back;
    mem->objects_back = objs_temp;
    mem->allocations_since_sweep = 0;
}

bool gc_disable_on_object(object_t obj) {
    if (!object_is_allocated(obj)) {
        return false;
    }
    object_data_t *data = object_get_allocated_data(obj);
    if (array_contains(data->mem->objects_not_gced, &obj)) {
        return false;
    }
    bool ok = array_add(data->mem->objects_not_gced, &obj);
    return ok;
}

void gc_enable_on_object(object_t obj) {
    if (!object_is_allocated(obj)) {
        return;
    }
    object_data_t *data = object_get_allocated_data(obj);
    array_remove_item(data->mem->objects_not_gced, &obj);
}

APE_INTERNAL int gc_should_sweep(gcmem_t *mem) {
    return mem->allocations_since_sweep > GCMEM_SWEEP_INTERVAL;
}

// INTERNAL
static object_data_pool_t* get_pool_for_type(gcmem_t *mem, object_type_t type) {
    switch (type) {
        case OBJECT_ARRAY: return &mem->pools[0];
        case OBJECT_MAP:   return &mem->pools[1];
        default:           return NULL;
    }
}

static bool can_data_be_put_in_pool(gcmem_t *mem, object_data_t *data) {
    object_t obj = object_make_from_data(data->type, data);

    // this is to ensure that large objects won't be kept in pool indefinitely
    switch (data->type) {
        case OBJECT_ARRAY: {
            if (object_get_array_length(obj) > 1024) {
                return false;
            }
            break;
        }
        case OBJECT_MAP: {
            if (object_get_map_length(obj) > 1024) {
                return false;
            }
            break;
        }
        default:
            break;
    }

    object_data_pool_t *pool = get_pool_for_type(mem, data->type);
    if (!pool || pool->count >= GCMEM_POOL_SIZE) {
        return false;
    }

    return true;
}
//FILE_END
//FILE_START:builtins.c
#include <stdlib.h>
#include <stdio.h>

#ifndef APE_AMALGAMATED
#include "builtins.h"

#include "common.h"
#include "object.h"
#include "vm.h"
#endif

static object_t len_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t first_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t last_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t rest_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t reverse_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t array_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t append_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t remove_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t remove_at_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t println_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t print_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t read_file_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t write_file_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t to_str_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t char_to_str_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t range_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t keys_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t values_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t copy_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t deep_copy_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t concat_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t error_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t crash_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t assert_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t random_seed_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t random_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t slice_fn(vm_t *vm, void *data, int argc, object_t *args);

// Type checks
static object_t is_string_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t is_array_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t is_map_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t is_number_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t is_bool_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t is_null_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t is_function_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t is_external_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t is_error_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t is_native_function_fn(vm_t *vm, void *data, int argc, object_t *args);

// Math
static object_t sqrt_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t pow_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t sin_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t cos_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t tan_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t log_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t ceil_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t floor_fn(vm_t *vm, void *data, int argc, object_t *args);
static object_t abs_fn(vm_t *vm, void *data, int argc, object_t *args);

static bool check_args(vm_t *vm, bool generate_error, int argc, object_t *args, int expected_argc, object_type_t *expected_types);
#define CHECK_ARGS(vm, generate_error, argc, args, ...) \
    check_args(\
        (vm),\
        (generate_error),\
        (argc),\
        (args),\
        sizeof((object_type_t[]){__VA_ARGS__}) / sizeof(object_type_t),\
        (object_type_t[]){__VA_ARGS__})

static struct {
    const char *name;
    native_fn fn;
} g_native_functions[] = {
    {"len",         len_fn},
    {"println",     println_fn},
    {"print",       print_fn},
    {"read_file",   read_file_fn},
    {"write_file",  write_file_fn},
    {"first",       first_fn},
    {"last",        last_fn},
    {"rest",        rest_fn},
    {"append",      append_fn},
    {"remove",      remove_fn},
    {"remove_at",   remove_at_fn},
    {"to_str",      to_str_fn},
    {"range",       range_fn},
    {"keys",        keys_fn},
    {"values",      values_fn},
    {"copy",        copy_fn},
    {"deep_copy",   deep_copy_fn},
    {"concat",      concat_fn},
    {"char_to_str", char_to_str_fn},
    {"reverse",     reverse_fn},
    {"array",       array_fn},
    {"error",       error_fn},
    {"crash",       crash_fn},
    {"assert",      assert_fn},
    {"random_seed", random_seed_fn},
    {"random",      random_fn},
    {"slice",       slice_fn},

    // Type checks
    {"is_string",   is_string_fn},
    {"is_array",    is_array_fn},
    {"is_map",      is_map_fn},
    {"is_number",   is_number_fn},
    {"is_bool",     is_bool_fn},
    {"is_null",     is_null_fn},
    {"is_function", is_function_fn},
    {"is_external", is_external_fn},
    {"is_error",    is_error_fn},
    {"is_native_function", is_native_function_fn},

    // Math
    {"sqrt",  sqrt_fn},
    {"pow",   pow_fn},
    {"sin",   sin_fn},
    {"cos",   cos_fn},
    {"tan",   tan_fn},
    {"log",   log_fn},
    {"ceil",  ceil_fn},
    {"floor", floor_fn},
    {"abs",   abs_fn},
};

int builtins_count() {
    return APE_ARRAY_LEN(g_native_functions);
}

native_fn builtins_get_fn(int ix) {
    return g_native_functions[ix].fn;
}

const char* builtins_get_name(int ix) {
    return g_native_functions[ix].name;
}

// INTERNAL
static object_t len_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_STRING | OBJECT_ARRAY | OBJECT_MAP)) {
        return object_make_null();
    }

    object_t arg = args[0];
    object_type_t type = object_get_type(arg);
    if (type == OBJECT_STRING) {
        const char *str = object_get_string(arg);
        int len = (int)strlen(str);
        return object_make_number(len);
    } else if (type == OBJECT_ARRAY) {
        int len = object_get_array_length(arg);
        return object_make_number(len);
    } else if (type == OBJECT_MAP) {
        int len = object_get_map_length(arg);
        return object_make_number(len);
    }

    return object_make_null();
}

static object_t first_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ARRAY)) {
        return object_make_null();
    }
     object_t arg = args[0];
    return object_get_array_value_at(arg, 0);
}

static object_t last_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ARRAY)) {
        return object_make_null();
    }
    object_t arg = args[0];
    return object_get_array_value_at(arg, object_get_array_length(arg) - 1);
}

static object_t rest_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ARRAY)) {
        return object_make_null();
    }
    object_t arg = args[0];
    int len = object_get_array_length(arg);
    if (len == 0) {
        return object_make_null();
    }

    object_t res = object_make_array(vm->mem);
    if (object_is_null(res)) {
        return object_make_null();
    }
    for (int i = 1; i < len; i++) {
        object_t item = object_get_array_value_at(arg, i);
        bool ok = object_add_array_value(res, item);
        if (!ok) {
            return object_make_null();
        }
    }
    return res;
}

static object_t reverse_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ARRAY | OBJECT_STRING)) {
        return object_make_null();
    }
    object_t arg = args[0];
    object_type_t type = object_get_type(arg);
    if (type == OBJECT_ARRAY) {
        int len = object_get_array_length(arg);
        object_t res = object_make_array_with_capacity(vm->mem, len);
        if (object_is_null(res)) {
            return object_make_null();
        }
        for (int i = 0; i < len; i++) {
            object_t obj = object_get_array_value_at(arg, i);
            bool ok = object_set_array_value_at(res, len - i - 1, obj);
            if (!ok) {
                return object_make_null();
            }
        }
        return res;
    } else if (type == OBJECT_STRING) {
        const char *str = object_get_string(arg);
        int len = (int)strlen(str);
        char *res_buf = allocator_malloc(vm->alloc, len + 1);
        if (!res_buf) {
            return object_make_null();
        }
        for (int i = 0; i < len; i++) {
            res_buf[len - i - 1] = str[i];
        }
        res_buf[len] = '\0';
        object_t res = object_make_string_no_copy(vm->mem, res_buf);
        if (object_is_null(res)) {
            allocator_free(vm->alloc, res_buf);
            return object_make_null();
        }
    }
    return object_make_null();
}

static object_t array_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (argc == 1) {
        if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER)) {
            return object_make_null();
        }
        int capacity = (int)object_get_number(args[0]);
        object_t res = object_make_array_with_capacity(vm->mem, capacity);
        if (object_is_null(res)) {
            return object_make_null();
        }
        object_t obj_null = object_make_null();
        for (int i = 0; i < capacity; i++) {
            bool ok = object_add_array_value(res, obj_null);
            if (!ok) {
                return object_make_null();
            }
        }
        return res;
    } else if (argc == 2) {
        if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER, OBJECT_ANY)) {
            return object_make_null();
        }
        int capacity = (int)object_get_number(args[0]);
        object_t res = object_make_array_with_capacity(vm->mem, capacity);
        if (object_is_null(res)) {
            return object_make_null();
        }
        for (int i = 0; i < capacity; i++) {
            bool ok = object_add_array_value(res, args[1]);
            if (!ok) {
                return object_make_null();
            }
        }
        return res;
    }
    CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER);
    return object_make_null();
}

static object_t append_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ARRAY, OBJECT_ANY)) {
        return object_make_null();
    }
    bool ok = object_add_array_value(args[0], args[1]);
    if (!ok) {
        return object_make_null();
    }
    int len = object_get_array_length(args[0]);
    return object_make_number(len);
}

static object_t println_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;

    const ape_config_t *config = vm->config;
    
    if (!config->stdio.write.write) {
        return object_make_null(); // todo: runtime error?
    }

    strbuf_t *buf = strbuf_make(vm->alloc);
    if (!buf) {
        return object_make_null();
    }
    for (int i = 0; i < argc; i++) {
        object_t arg = args[i];
        object_to_string(arg, buf, false);
    }
    strbuf_append(buf, "\n");
    if (strbuf_failed(buf)) {
        strbuf_destroy(buf);
        return object_make_null();
    }
    config->stdio.write.write(config->stdio.write.context, strbuf_get_string(buf), strbuf_get_length(buf));
    strbuf_destroy(buf);
    return object_make_null();
}

static object_t print_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    const ape_config_t *config = vm->config;

    if (!config->stdio.write.write) {
        return object_make_null(); // todo: runtime error?
    }

    strbuf_t *buf = strbuf_make(vm->alloc);
    if (!buf) {
        return object_make_null();
    }
    for (int i = 0; i < argc; i++) {
        object_t arg = args[i];
        object_to_string(arg, buf, false);
    }
    if (strbuf_failed(buf)) {
        strbuf_destroy(buf);
        return object_make_null();
    }
    config->stdio.write.write(config->stdio.write.context, strbuf_get_string(buf), strbuf_get_length(buf));
    strbuf_destroy(buf);
    return object_make_null();
}

static object_t write_file_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_STRING, OBJECT_STRING)) {
        return object_make_null();
    }

    const ape_config_t *config = vm->config;

    if (!config->fileio.write_file.write_file) {
        return object_make_null();
    }

    const char *path = object_get_string(args[0]);
    const char *string = object_get_string(args[1]);
    int string_size = (int)strlen(string) + 1;

    int written = (int)config->fileio.write_file.write_file(config->fileio.write_file.context, path, string, string_size);
    
    return object_make_number(written);
}

static object_t read_file_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_STRING)) {
        return object_make_null();
    }

    const ape_config_t *config = vm->config;

    if (!config->fileio.read_file.read_file) {
        return object_make_null();
    }

    const char *path = object_get_string(args[0]);

    char *contents = config->fileio.read_file.read_file(config->fileio.read_file.context, path);
    if (!contents) {
        return object_make_null();
    }
    object_t res = object_make_string(vm->mem, contents);
    allocator_free(vm->alloc, contents);
    return res;
}

static object_t to_str_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_STRING | OBJECT_NUMBER | OBJECT_BOOL | OBJECT_NULL | OBJECT_MAP | OBJECT_ARRAY)) {
        return object_make_null();
    }
    object_t arg = args[0];
    strbuf_t *buf = strbuf_make(vm->alloc);
    if (!buf) {
        return object_make_null();
    }
    object_to_string(arg, buf, false);
    if (strbuf_failed(buf)) {
        strbuf_destroy(buf);
        return object_make_null();
    }
    object_t res = object_make_string(vm->mem, strbuf_get_string(buf));
    strbuf_destroy(buf);
    return res;
}

static object_t char_to_str_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER)) {
        return object_make_null();
    }

    double val = object_get_number(args[0]);

    char c = (char)val;
    char str[2] = {c, '\0'};
    return object_make_string(vm->mem, str);
}

static object_t range_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    for (int i = 0; i < argc; i++) {
        object_type_t type = object_get_type(args[i]);
        if (type != OBJECT_NUMBER) {
            const char *type_str = object_get_type_name(type);
            const char *expected_str = object_get_type_name(OBJECT_NUMBER);
            errors_add_errorf(vm->errors, ERROR_RUNTIME, src_pos_invalid,
                                       "Invalid argument %d passed to range, got %s instead of %s",
                                       i, type_str, expected_str);
            return object_make_null();
        }
    }

    int start = 0;
    int end = 0;
    int step = 1;

    if (argc == 1) {
        end = (int)object_get_number(args[0]);
    } else if (argc == 2) {
        start = (int)object_get_number(args[0]);
        end = (int)object_get_number(args[1]);
    } else if (argc == 3) {
        start = (int)object_get_number(args[0]);
        end = (int)object_get_number(args[1]);
        step = (int)object_get_number(args[2]);
    } else {
        errors_add_errorf(vm->errors, ERROR_RUNTIME, src_pos_invalid, "Invalid number of arguments passed to range, got %d", argc);
        return object_make_null();
    }

    if (step == 0) {
        errors_add_error(vm->errors, ERROR_RUNTIME, src_pos_invalid, "range step cannot be 0");
        return object_make_null();
    }

    object_t res = object_make_array(vm->mem);
    if (object_is_null(res)) {
        return object_make_null();
    }
    for (int i = start; i < end; i += step) {
        object_t item = object_make_number(i);
        bool ok = object_add_array_value(res, item);
        if (!ok) {
            return object_make_null();
        }
    }
    return res;
}

static object_t keys_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_MAP)) {
        return object_make_null();
    }
    object_t arg = args[0];
    object_t res = object_make_array(vm->mem);
    if (object_is_null(res)) {
        return object_make_null();
    }
    int len = object_get_map_length(arg);
    for (int i = 0; i < len; i++) {
        object_t key = object_get_map_key_at(arg, i);
        bool ok = object_add_array_value(res, key);
        if (!ok) {
            return object_make_null();
        }
    }
    return res;
}

static object_t values_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_MAP)) {
        return object_make_null();
    }
    object_t arg = args[0];
    object_t res = object_make_array(vm->mem);
    if (object_is_null(res)) {
        return object_make_null();
    }
    int len = object_get_map_length(arg);
    for (int i = 0; i < len; i++) {
        object_t key = object_get_map_value_at(arg, i);
        bool ok = object_add_array_value(res, key);
        if (!ok) {
            return object_make_null();
        }
    }
    return res;
}

static object_t copy_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_copy(vm->mem, args[0]);
}

static object_t deep_copy_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_deep_copy(vm->mem, args[0]);
}

static object_t concat_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ARRAY | OBJECT_STRING, OBJECT_ARRAY | OBJECT_STRING)) {
        return object_make_null();
    }
    object_type_t type = object_get_type(args[0]);
    object_type_t item_type = object_get_type(args[1]);
    if (type == OBJECT_ARRAY) {
        if (item_type != OBJECT_ARRAY) {
            const char *item_type_str = object_get_type_name(item_type);
            errors_add_errorf(vm->errors, ERROR_RUNTIME, src_pos_invalid,
                              "Invalid argument 2 passed to concat, got %s",
                              item_type_str);
            return object_make_null();
        }
        for (int i = 0; i < object_get_array_length(args[1]); i++) {
            object_t item = object_get_array_value_at(args[1], i);
            bool ok = object_add_array_value(args[0], item);
            if (!ok) {
                return object_make_null();
            }
        }
        return object_make_number(object_get_array_length(args[0]));
    } else if (type == OBJECT_STRING) {
        if (!CHECK_ARGS(vm, true, argc, args, OBJECT_STRING, OBJECT_STRING)) {
            return object_make_null();
        }
        const char *str = object_get_string(args[0]);
        int len = (int)strlen(str);
        const char *arg_str = object_get_string(args[1]);
        int arg_str_len = (int)strlen(arg_str);
        char *res_buf = allocator_malloc(vm->alloc, len + arg_str_len + 1);
        if (!res_buf) {
            return object_make_null();
        }
        for (int i = 0; i < len; i++) {
            res_buf[i] = str[i];
        }
        for (int i = 0; i < arg_str_len; i++) {
            res_buf[len + i] = arg_str[i];
        }
        res_buf[len + arg_str_len] = '\0';
        object_t res = object_make_string_no_copy(vm->mem, res_buf);
        if (object_is_null(res)) {
            allocator_free(vm->alloc, res_buf);
            return  object_make_null();
        }
    }
    return object_make_null();
}

static object_t remove_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ARRAY, OBJECT_ANY)) {
        return object_make_null();
    }

    int ix = -1;
    for (int i = 0; i < object_get_array_length(args[0]); i++) {
        object_t obj = object_get_array_value_at(args[0], i);
        if (object_equals(obj, args[1])) {
            ix = i;
            break;
        }
    }

    if (ix == -1) {
        return object_make_bool(false);
    }

    bool res = object_remove_array_value_at(args[0], ix);
    return object_make_bool(res);
}

static object_t remove_at_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ARRAY, OBJECT_NUMBER)) {
        return object_make_null();
    }

    object_type_t type = object_get_type(args[0]);
    int ix = (int)object_get_number(args[1]);

    switch (type) {
        case OBJECT_ARRAY: {
            bool res = object_remove_array_value_at(args[0], ix);
            return object_make_bool(res);
        }
        default:
            break;
    }

    return object_make_bool(true);
}


static object_t error_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (argc == 1 && object_get_type(args[0]) == OBJECT_STRING) {
        return object_make_error(vm->mem, object_get_string(args[0]));
    } else {
        return object_make_error(vm->mem, "");
    }
}

static object_t crash_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (argc == 1 && object_get_type(args[0]) == OBJECT_STRING) {
        errors_add_error(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), object_get_string(args[0]));
    } else {
        errors_add_error(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "");
    }
    return object_make_null();
}

static object_t assert_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_BOOL)) {
        return object_make_null();
    }

    if (!object_get_bool(args[0])) {
        errors_add_error(vm->errors, ERROR_RUNTIME, src_pos_invalid, "assertion failed");
        return object_make_null();
    }

    return object_make_bool(true);
}

static object_t random_seed_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER)) {
        return object_make_null();
    }
    int seed = (int)object_get_number(args[0]);
    srand(seed);
    return object_make_bool(true);
}

static object_t random_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    double res = (double)rand() / RAND_MAX;
    if (argc == 0) {
        return object_make_number(res);
    } else if (argc == 2) {
        if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER, OBJECT_NUMBER)) {
            return object_make_null();
        }
        double min = object_get_number(args[0]);
        double max = object_get_number(args[1]);
        if (min >= max) {
            errors_add_error(vm->errors, ERROR_RUNTIME, src_pos_invalid, "max is bigger than min");
            return object_make_null();
        }
        double range = max - min;
        res = min + (res * range);
        return object_make_number(res);
    } else {
        errors_add_error(vm->errors, ERROR_RUNTIME, src_pos_invalid, "Invalid number or arguments");
        return object_make_null();
    }
}

static object_t slice_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_STRING | OBJECT_ARRAY, OBJECT_NUMBER)) {
        return object_make_null();
    }
    object_type_t arg_type = object_get_type(args[0]);
    int index = (int)object_get_number(args[1]);
    if (arg_type == OBJECT_ARRAY) {
        int len = object_get_array_length(args[0]);
        if (index < 0) {
            index = len + index;
            if (index < 0) {
                index = 0;
            }
        }
        object_t res = object_make_array_with_capacity(vm->mem, len - index);
        if (object_is_null(res)) {
            return object_make_null();
        }
        for (int i = index; i < len; i++) {
            object_t item = object_get_array_value_at(args[0], i);
            bool ok = object_add_array_value(res, item);
            if (!ok) {
                return object_make_null();
            }
        }
        return res;
    } else if (arg_type == OBJECT_STRING) {
        const char *str = object_get_string(args[0]);
        int len = (int)strlen(str);
        if (index < 0) {
            index = len + index;
            if (index < 0) {
                return object_make_string(vm->mem, "");
            }
        }
        if (index >= len) {
            return object_make_string(vm->mem, "");
        }
        char *res_str = allocator_malloc(vm->alloc, len - index + 1);
        if (!res_str) {
            return object_make_null();
        }
        memset(res_str, 0, len - index + 1);
        for (int i = index; i < len; i++) {
            char c = str[i];
            res_str[i - index] = c;
        }
        object_t res = object_make_string_no_copy(vm->mem, res_str);
        if (object_is_null(res)) {
            allocator_free(vm->alloc, res_str);
            return object_make_null();
        }
        return res;
    } else {
        const char *type_str = object_get_type_name(arg_type);
        errors_add_errorf(vm->errors, ERROR_RUNTIME, src_pos_invalid,
                          "Invalid argument 0 passed to slice, got %s instead", type_str);
        return object_make_null();
    }
}

//-----------------------------------------------------------------------------
// Type checks
//-----------------------------------------------------------------------------

static object_t is_string_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_make_bool(object_get_type(args[0]) == OBJECT_STRING);
}

static object_t is_array_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_make_bool(object_get_type(args[0]) == OBJECT_ARRAY);
}

static object_t is_map_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_make_bool(object_get_type(args[0]) == OBJECT_MAP);
}

static object_t is_number_fn(vm_t *vm, void *data, int argc, object_t *args){
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_make_bool(object_get_type(args[0]) == OBJECT_NUMBER);
}

static object_t is_bool_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_make_bool(object_get_type(args[0]) == OBJECT_BOOL);
}

static object_t is_null_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_make_bool(object_get_type(args[0]) == OBJECT_NULL);
}

static object_t is_function_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_make_bool(object_get_type(args[0]) == OBJECT_FUNCTION);
}

static object_t is_external_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_make_bool(object_get_type(args[0]) == OBJECT_EXTERNAL);
}

static object_t is_error_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_make_bool(object_get_type(args[0]) == OBJECT_ERROR);
}

static object_t is_native_function_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_ANY)) {
        return object_make_null();
    }
    return object_make_bool(object_get_type(args[0]) == OBJECT_NATIVE_FUNCTION);
}

//-----------------------------------------------------------------------------
// Math
//-----------------------------------------------------------------------------

static object_t sqrt_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER)) {
        return object_make_null();
    }
    double arg = object_get_number(args[0]);
    double res = sqrt(arg);
    return object_make_number(res);
}

static object_t pow_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER, OBJECT_NUMBER)) {
        return object_make_null();
    }
    double arg1 = object_get_number(args[0]);
    double arg2 = object_get_number(args[1]);
    double res = pow(arg1, arg2);
    return object_make_number(res);
}

static object_t sin_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER)) {
        return object_make_null();
    }
    double arg = object_get_number(args[0]);
    double res = sin(arg);
    return object_make_number(res);
}

static object_t cos_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER)) {
        return object_make_null();
    }
    double arg = object_get_number(args[0]);
    double res = cos(arg);
    return object_make_number(res);
}

static object_t tan_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER)) {
        return object_make_null();
    }
    double arg = object_get_number(args[0]);
    double res = tan(arg);
    return object_make_number(res);
}

static object_t log_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER)) {
        return object_make_null();
    }
    double arg = object_get_number(args[0]);
    double res = log(arg);
    return object_make_number(res);
}

static object_t ceil_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER)) {
        return object_make_null();
    }
    double arg = object_get_number(args[0]);
    double res = ceil(arg);
    return object_make_number(res);
}

static object_t floor_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER)) {
        return object_make_null();
    }
    double arg = object_get_number(args[0]);
    double res = floor(arg);
    return object_make_number(res);
}

static object_t abs_fn(vm_t *vm, void *data, int argc, object_t *args) {
    (void)data;
    if (!CHECK_ARGS(vm, true, argc, args, OBJECT_NUMBER)) {
        return object_make_null();
    }
    double arg = object_get_number(args[0]);
    double res = fabs(arg);
    return object_make_number(res);
}

static bool check_args(vm_t *vm, bool generate_error, int argc, object_t *args, int expected_argc, object_type_t *expected_types) {
    if (argc != expected_argc) {
        if (generate_error) {
            errors_add_errorf(vm->errors, ERROR_RUNTIME, src_pos_invalid,
                              "Invalid number or arguments, got %d instead of %d",
                              argc, expected_argc);
        }
        return false;
    }

    for (int i = 0; i < argc; i++) {
       object_t arg = args[i];
        object_type_t type = object_get_type(arg);
        object_type_t expected_type = expected_types[i];
        if (!(type & expected_type)) {
            if (generate_error) {
                const char *type_str = object_get_type_name(type);
                char *expected_type_str = object_get_type_union_name(vm->alloc, expected_type);
                if (!expected_type_str) {
                    return false;
                }
                errors_add_errorf(vm->errors, ERROR_RUNTIME, src_pos_invalid,
                                  "Invalid argument %d type, got %s, expected %s",
                                  i, type_str, expected_type_str);
                allocator_free(vm->alloc, expected_type_str);
            }
            return false;
        }
    }
    return true;
}
//FILE_END
//FILE_START:traceback.c
#ifndef APE_AMALGAMATED
#include "traceback.h"
#include "vm.h"
#include "compiler.h"
#include "collections.h"
#endif

traceback_t* traceback_make(allocator_t *alloc) {
    traceback_t *traceback = allocator_malloc(alloc, sizeof(traceback_t));
    if (!traceback) {
        return NULL;
    }
    memset(traceback, 0, sizeof(traceback_t));
    traceback->alloc = alloc;
    traceback->items = array_make(alloc, traceback_item_t);
    if (!traceback->items) {
        traceback_destroy(traceback);
        return NULL;
    }
    return traceback;
}

void traceback_destroy(traceback_t *traceback) {
    if (!traceback) {
        return;
    }
    for (int i = 0; i < array_count(traceback->items); i++) {
        traceback_item_t *item = array_get(traceback->items, i);
        allocator_free(traceback->alloc, item->function_name);
    }
    array_destroy(traceback->items);
    allocator_free(traceback->alloc, traceback);
}

bool traceback_append(traceback_t *traceback, const char *function_name, src_pos_t pos) {
    traceback_item_t item;
    item.function_name = ape_strdup(traceback->alloc, function_name);
    if (!item.function_name) {
        return false;
    }
    item.pos = pos;
    bool ok = array_add(traceback->items, &item);
    if (!ok) {
        allocator_free(traceback->alloc, item.function_name);
        return false;
    }
    return true;
}

bool traceback_append_from_vm(traceback_t *traceback, vm_t *vm) {
    for (int i = vm->frames_count - 1; i >= 0; i--) {
        frame_t *frame = &vm->frames[i];
        bool ok = traceback_append(traceback, object_get_function_name(frame->function), frame_src_position(frame));
        if (!ok) {
            return false;
        }
    }
    return true;
}

bool traceback_to_string(const traceback_t *traceback, strbuf_t *buf) {
    int depth  = array_count(traceback->items);
    for (int i = 0; i < depth; i++) {
        traceback_item_t *item = array_get(traceback->items, i);
        const char *filename = traceback_item_get_filepath(item);
        if (item->pos.line >= 0 && item->pos.column >= 0) {
            strbuf_appendf(buf, "%s in %s on %d:%d\n", item->function_name, filename, item->pos.line, item->pos.column);
        } else {
            strbuf_appendf(buf, "%s\n", item->function_name);
        }
    }
    return !strbuf_failed(buf);
}

const char* traceback_item_get_line(traceback_item_t *item) {
    if (!item->pos.file) {
        return NULL;
    }
    ptrarray(char*) *lines = item->pos.file->lines;
    if (item->pos.line >= ptrarray_count(lines)) {
        return NULL;
    }
    const char *line = ptrarray_get(lines, item->pos.line);
    return line;
}

const char* traceback_item_get_filepath(traceback_item_t *item) {
    if (!item->pos.file) {
        return NULL;
    }
    return item->pos.file->path;
}
//FILE_END
//FILE_START:frame.c
#include <stdlib.h>

#ifndef APE_AMALGAMATED
#include "frame.h"
#include "compiler.h"
#endif

bool frame_init(frame_t* frame, object_t function_obj, int base_pointer) {
    if (object_get_type(function_obj) != OBJECT_FUNCTION) {
        return false;
    }
    function_t* function = object_get_function(function_obj);
    frame->function = function_obj;
    frame->ip = 0;
    frame->base_pointer = base_pointer;
    frame->src_ip = 0;
    frame->bytecode = function->comp_result->bytecode;
    frame->src_positions = function->comp_result->src_positions;
    frame->bytecode_size = function->comp_result->count;
    frame->recover_ip = -1;
    frame->is_recovering = false;
    return true;
}

opcode_val_t frame_read_opcode(frame_t* frame){
    frame->src_ip = frame->ip;
    return frame_read_uint8(frame);
}

uint64_t frame_read_uint64(frame_t* frame) {
    const uint8_t *data = frame->bytecode + frame->ip;
    frame->ip += 8;
    uint64_t res = 0;
    res |= (uint64_t)data[7];
    res |= (uint64_t)data[6] << 8;
    res |= (uint64_t)data[5] << 16;
    res |= (uint64_t)data[4] << 24;
    res |= (uint64_t)data[3] << 32;
    res |= (uint64_t)data[2] << 40;
    res |= (uint64_t)data[1] << 48;
    res |= (uint64_t)data[0] << 56;
    return res;
}

uint16_t frame_read_uint16(frame_t* frame) {
    const uint8_t *data = frame->bytecode + frame->ip;
    frame->ip += 2;
    return (data[0] << 8) | data[1];
}

uint8_t frame_read_uint8(frame_t* frame) {
    const uint8_t *data = frame->bytecode + frame->ip;
    frame->ip += 1;
    return data[0];
}

src_pos_t frame_src_position(const frame_t *frame) {
    if (frame->src_positions) {
        return frame->src_positions[frame->src_ip];
    }
    return src_pos_invalid;
}
//FILE_END
//FILE_START:vm.c
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#ifndef APE_AMALGAMATED
#include "vm.h"

#include "code.h"
#include "compiler.h"
#include "traceback.h"
#include "builtins.h"
#include "gc.h"
#endif

static void set_sp(vm_t *vm, int new_sp);
static void stack_push(vm_t *vm, object_t obj);
static object_t stack_pop(vm_t *vm);
static object_t stack_get(vm_t *vm, int nth_item);

static void this_stack_push(vm_t *vm, object_t obj);
static object_t this_stack_pop(vm_t *vm);
static object_t this_stack_get(vm_t *vm, int nth_item);

static bool push_frame(vm_t *vm, frame_t frame);
static bool pop_frame(vm_t *vm);
static void run_gc(vm_t *vm, array(object_t) *constants);
static bool call_object(vm_t *vm, object_t callee, int num_args);
static object_t call_native_function(vm_t *vm, object_t callee, src_pos_t src_pos, int argc, object_t *args);
static bool check_assign(vm_t *vm, object_t old_value, object_t new_value);
static bool try_overload_operator(vm_t *vm, object_t left, object_t right, opcode_t op, bool *out_overload_found);

vm_t *vm_make(allocator_t *alloc, const ape_config_t *config, gcmem_t *mem, errors_t *errors, global_store_t *global_store) {
    vm_t *vm = allocator_malloc(alloc, sizeof(vm_t));
    if (!vm) {
        return NULL;
    }
    memset(vm, 0, sizeof(vm_t));
    vm->alloc = alloc;
    vm->config = config;
    vm->mem = mem;
    vm->errors = errors;
    vm->global_store = global_store;
    vm->globals_count = 0;
    vm->sp = 0;
    vm->this_sp = 0;
    vm->frames_count = 0;
    vm->last_popped = object_make_null();
    vm->running = false;

    for (int i = 0; i < OPCODE_MAX; i++) {
        vm->operator_oveload_keys[i] = object_make_null();
    }
#define SET_OPERATOR_OVERLOAD_KEY(op, key) do {\
    object_t key_obj = object_make_string(vm->mem, key);\
    if (object_is_null(key_obj)) {\
        goto err;\
    }\
    vm->operator_oveload_keys[op] = key_obj;\
} while (0)
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_ADD,     "__operator_add__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_SUB,     "__operator_sub__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_MUL,     "__operator_mul__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_DIV,     "__operator_div__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_MOD,     "__operator_mod__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_OR,      "__operator_or__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_XOR,     "__operator_xor__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_AND,     "__operator_and__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_LSHIFT,  "__operator_lshift__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_RSHIFT,  "__operator_rshift__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_MINUS,   "__operator_minus__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_BANG,    "__operator_bang__");
    SET_OPERATOR_OVERLOAD_KEY(OPCODE_COMPARE, "__cmp__");
#undef SET_OPERATOR_OVERLOAD_KEY

    return vm;
err:
    vm_destroy(vm);
    return NULL;
}

void vm_destroy(vm_t *vm) {
    if (!vm) {
        return;
    }
    allocator_free(vm->alloc, vm);
}

void vm_reset(vm_t *vm) {
    vm->sp = 0;
    vm->this_sp = 0;
    while (vm->frames_count > 0) {
        pop_frame(vm);
    }
}

bool vm_run(vm_t *vm, compilation_result_t *comp_res, array(object_t) *constants) {
#ifdef APE_DEBUG
    int old_sp = vm->sp;
#endif
    int old_this_sp = vm->this_sp;
    int old_frames_count = vm->frames_count;
    object_t main_fn = object_make_function(vm->mem, "main", comp_res, false, 0, 0, 0);
    if (object_is_null(main_fn)) {
        return false;
    }
    stack_push(vm, main_fn);
    bool res = vm_execute_function(vm, main_fn, constants);
    while (vm->frames_count > old_frames_count) {
        pop_frame(vm);
    }
    APE_ASSERT(vm->sp == old_sp);
    vm->this_sp = old_this_sp;
    return res;
}

object_t vm_call(vm_t *vm, array(object_t) *constants, object_t callee, int argc, object_t *args) {
    object_type_t type = object_get_type(callee);
    if (type == OBJECT_FUNCTION) {
#ifdef APE_DEBUG
        int old_sp = vm->sp;
#endif
        int old_this_sp = vm->this_sp;
        int old_frames_count = vm->frames_count;
        stack_push(vm, callee);
        for (int i = 0; i < argc; i++) {
            stack_push(vm, args[i]);
        }
        bool ok = vm_execute_function(vm, callee, constants);
        if (!ok) {
            return object_make_null();
        }
        while (vm->frames_count > old_frames_count) {
            pop_frame(vm);
        }
        APE_ASSERT(vm->sp == old_sp);
        vm->this_sp = old_this_sp;
        return vm_get_last_popped(vm);
    } else if (type == OBJECT_NATIVE_FUNCTION) {
        return call_native_function(vm, callee, src_pos_invalid, argc, args);
    } else {
        errors_add_error(vm->errors, ERROR_USER, src_pos_invalid, "Object is not callable");
        return object_make_null();
    }
}

bool vm_execute_function(vm_t *vm, object_t function, array(object_t) *constants) {
    if (vm->running) {
        errors_add_error(vm->errors, ERROR_USER, src_pos_invalid, "VM is already executing code");
        return false;
    }

    function_t *function_function = object_get_function(function); // naming is hard
    frame_t new_frame;
    bool ok = false;
    ok = frame_init(&new_frame, function, vm->sp - function_function->num_args);
    if (!ok) {
        return false;
    }
    ok = push_frame(vm, new_frame);
    if (!ok) {
        errors_add_error(vm->errors, ERROR_USER, src_pos_invalid, "Pushing frame failed");
        return false;
    }

    vm->running = true;
    vm->last_popped = object_make_null();

    bool check_time = false;
    double max_exec_time_ms = 0;
    if (vm->config) {
        check_time = vm->config->max_execution_time_set;
        max_exec_time_ms = vm->config->max_execution_time_ms;
    }
    unsigned time_check_interval = 1000;
    unsigned time_check_counter = 0;
    ape_timer_t timer;
    memset(&timer, 0, sizeof(ape_timer_t));
    if (check_time) {
        timer = ape_timer_start();
    }

    while (vm->current_frame->ip < vm->current_frame->bytecode_size) {
        opcode_val_t opcode = frame_read_opcode(vm->current_frame);
        switch (opcode) {
            case OPCODE_CONSTANT: {
                uint16_t constant_ix = frame_read_uint16(vm->current_frame);
                object_t *constant = array_get(constants, constant_ix);
                if (!constant) {
                    errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                      "Constant at %d not found", constant_ix);
                    goto err;
                }
                stack_push(vm, *constant);
                break;
            }
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_MOD:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_AND:
            case OPCODE_LSHIFT:
            case OPCODE_RSHIFT:
            {
                object_t right = stack_pop(vm);
                object_t left = stack_pop(vm);
                object_type_t left_type = object_get_type(left);
                object_type_t right_type = object_get_type(right);
                if (object_is_numeric(left) && object_is_numeric(right)) {
                    double right_val = object_get_number(right);
                    double left_val = object_get_number(left);

                    int64_t left_val_int = (int64_t)left_val;
                    int64_t right_val_int = (int64_t)right_val;

                    double res = 0;
                    switch (opcode) {
                        case OPCODE_ADD:    res = left_val + right_val; break;
                        case OPCODE_SUB:    res = left_val - right_val; break;
                        case OPCODE_MUL:    res = left_val * right_val; break;
                        case OPCODE_DIV:    res = left_val / right_val; break;
                        case OPCODE_MOD:    res = fmod(left_val, right_val); break;
                        case OPCODE_OR:     res = (double)(left_val_int | right_val_int); break;
                        case OPCODE_XOR:    res = (double)(left_val_int ^ right_val_int); break;
                        case OPCODE_AND:    res = (double)(left_val_int & right_val_int); break;
                        case OPCODE_LSHIFT: res = (double)(left_val_int << right_val_int); break;
                        case OPCODE_RSHIFT: res = (double)(left_val_int >> right_val_int); break;
                        default: APE_ASSERT(false); break;
                    }
                    stack_push(vm, object_make_number(res));
                } else if (left_type == OBJECT_STRING  && right_type == OBJECT_STRING && opcode == OPCODE_ADD) {
                    const char* right_val = object_get_string(right);
                    const char* left_val = object_get_string(left);
                    object_t res_obj = object_make_stringf(vm->mem, "%s%s", left_val, right_val);
                    if (object_is_null(res_obj)) {
                        goto err;
                    }
                    stack_push(vm, res_obj);
                } else {
                    bool overload_found = false;
                    bool ok = try_overload_operator(vm, left, right, opcode, &overload_found);
                    if (!ok) {
                        goto err;
                    }
                    if (!overload_found) {
                        const char *opcode_name = opcode_get_name(opcode);
                        const char *left_type_name = object_get_type_name(left_type);
                        const char *right_type_name = object_get_type_name(right_type);
                        errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                          "Invalid operand types for %s, got %s and %s",
                                          opcode_name, left_type_name, right_type_name);
                        goto err;
                    }
                }
                break;
            }
            case OPCODE_POP: {
                stack_pop(vm);
                break;
            }
            case OPCODE_TRUE: {
                stack_push(vm, object_make_bool(true));
                break;
            }
            case OPCODE_FALSE: {
                stack_push(vm, object_make_bool(false));
                break;
            }
            case OPCODE_COMPARE: {
                object_t right = stack_pop(vm);
                object_t left = stack_pop(vm);
                bool is_overloaded = false;
                bool ok = try_overload_operator(vm, left, right, OPCODE_COMPARE, &is_overloaded);
                if (!ok) {
                    goto err;
                }
                if (!is_overloaded) {
                    double comparison_res = object_compare(left, right);
                    object_t res = object_make_number(comparison_res);
                    stack_push(vm, res);
                }
                break;
            }
            case OPCODE_EQUAL:
            case OPCODE_NOT_EQUAL:
            case OPCODE_GREATER_THAN:
            case OPCODE_GREATER_THAN_EQUAL:
            {
                object_t value = stack_pop(vm);
                double comparison_res = object_get_number(value);
                bool res_val = false;
                switch (opcode) {
                    case OPCODE_EQUAL: res_val = APE_DBLEQ(comparison_res, 0); break;
                    case OPCODE_NOT_EQUAL: res_val = !APE_DBLEQ(comparison_res, 0); break;
                    case OPCODE_GREATER_THAN: res_val = comparison_res > 0; break;
                    case OPCODE_GREATER_THAN_EQUAL: {
                        res_val = comparison_res > 0 || APE_DBLEQ(comparison_res, 0);
                        break;
                    }
                    default: APE_ASSERT(false); break;
                }
                object_t res = object_make_bool(res_val);
                stack_push(vm, res);
                break;
            }
            case OPCODE_MINUS:
            {
                object_t operand = stack_pop(vm);
                object_type_t operand_type = object_get_type(operand);
                if (operand_type == OBJECT_NUMBER) {
                    double val = object_get_number(operand);
                    object_t res = object_make_number(-val);
                    stack_push(vm, res);
                } else {
                    bool overload_found = false;
                    bool ok = try_overload_operator(vm, operand, object_make_null(), OPCODE_MINUS, &overload_found);
                    if (!ok) {
                        goto err;
                    }
                    if (!overload_found) {
                        const char *operand_type_string = object_get_type_name(operand_type);
                        errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                          "Invalid operand type for MINUS, got %s",
                                          operand_type_string);
                        goto err;
                    }
                }
                break;
            }
            case OPCODE_BANG: {
                object_t operand = stack_pop(vm);
                object_type_t type = object_get_type(operand);
                if (type == OBJECT_BOOL) {
                    object_t res = object_make_bool(!object_get_bool(operand));
                    stack_push(vm, res);
                } else if (type == OBJECT_NULL) {
                    object_t res = object_make_bool(true);
                    stack_push(vm, res);
                } else {
                    bool overload_found = false;
                    bool ok = try_overload_operator(vm, operand, object_make_null(), OPCODE_BANG, &overload_found);
                    if (!ok) {
                        goto err;
                    }
                    if (!overload_found) {
                        object_t res = object_make_bool(false);
                        stack_push(vm, res);
                    }
                }
                break;
            }
            case OPCODE_JUMP: {
                uint16_t pos = frame_read_uint16(vm->current_frame);
                vm->current_frame->ip = pos;
                break;
            }
            case OPCODE_JUMP_IF_FALSE: {
                uint16_t pos = frame_read_uint16(vm->current_frame);
                object_t test = stack_pop(vm);
                if (!object_get_bool(test)) {
                    vm->current_frame->ip = pos;
                }
                break;
            }
            case OPCODE_JUMP_IF_TRUE: {
                uint16_t pos = frame_read_uint16(vm->current_frame);
                object_t test = stack_pop(vm);
                if (object_get_bool(test)) {
                    vm->current_frame->ip = pos;
                }
                break;
            }
            case OPCODE_NULL: {
                stack_push(vm, object_make_null());
                break;
            }
            case OPCODE_DEFINE_MODULE_GLOBAL: {
                uint16_t ix = frame_read_uint16(vm->current_frame);
                object_t value = stack_pop(vm);
                vm_set_global(vm, ix, value);
                break;
            }
            case OPCODE_SET_MODULE_GLOBAL: {
                uint16_t ix = frame_read_uint16(vm->current_frame);
                object_t new_value = stack_pop(vm);
                object_t old_value = vm_get_global(vm, ix);
                if (!check_assign(vm, old_value, new_value)) {
                    goto err;
                }
                vm_set_global(vm, ix, new_value);
                break;
            }
            case OPCODE_GET_MODULE_GLOBAL: {
                uint16_t ix = frame_read_uint16(vm->current_frame);
                object_t global = vm->globals[ix];
                stack_push(vm, global);
                break;
            }
            case OPCODE_ARRAY: {
                uint16_t count = frame_read_uint16(vm->current_frame);
                object_t array_obj = object_make_array_with_capacity(vm->mem, count);
                if (object_is_null(array_obj)) {
                    goto err;
                }
                object_t *items = vm->stack + vm->sp - count;
                for (int i = 0; i < count; i++) {
                    object_t item = items[i];
                    ok = object_add_array_value(array_obj, item);
                    if (!ok) {
                        goto err;
                    }
                }
                set_sp(vm, vm->sp - count);
                stack_push(vm, array_obj);
                break;
            }
            case OPCODE_MAP_START: {
                uint16_t count = frame_read_uint16(vm->current_frame);
                object_t map_obj = object_make_map_with_capacity(vm->mem, count);
                if (object_is_null(map_obj)) {
                    goto err;
                }
                this_stack_push(vm, map_obj);
                break;
            }
            case OPCODE_MAP_END: {
                uint16_t kvp_count = frame_read_uint16(vm->current_frame);
                uint16_t items_count = kvp_count * 2;
                object_t map_obj = this_stack_pop(vm);
                object_t *kv_pairs = vm->stack + vm->sp - items_count;
                for (int i = 0; i < items_count; i += 2) {
                    object_t key = kv_pairs[i];
                    if (!object_is_hashable(key)) {
                        object_type_t key_type = object_get_type(key);
                        const char *key_type_name = object_get_type_name(key_type);
                        errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                          "Key of type %s is not hashable", key_type_name);
                        goto err;
                    }
                    object_t val = kv_pairs[i + 1];
                    ok = object_set_map_value(map_obj, key, val);
                    if (!ok) {
                        goto err;
                    }
                }
                set_sp(vm, vm->sp - items_count);
                stack_push(vm, map_obj);
                break;
            }
            case OPCODE_GET_THIS: {
                object_t obj = this_stack_get(vm, 0);
                stack_push(vm, obj);
                break;
            }
            case OPCODE_GET_INDEX: {
                object_t index = stack_pop(vm);
                object_t left = stack_pop(vm);
                object_type_t left_type = object_get_type(left);
                object_type_t index_type = object_get_type(index);
                const char *left_type_name = object_get_type_name(left_type);
                const char *index_type_name = object_get_type_name(index_type);

                if (left_type != OBJECT_ARRAY && left_type != OBJECT_MAP && left_type != OBJECT_STRING) {
                    errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                      "Type %s is not indexable", left_type_name);
                    goto err;
                }

                object_t res = object_make_null();

                if (left_type == OBJECT_ARRAY) {
                    if (index_type != OBJECT_NUMBER) {
                        errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                          "Cannot index %s with %s", left_type_name, index_type_name);
                        goto err;
                    }
                    int ix = (int)object_get_number(index);
                    if (ix < 0) {
                        ix = object_get_array_length(left) + ix;
                    }
                    if (ix >= 0 && ix < object_get_array_length(left)) {
                        res = object_get_array_value_at(left, ix);
                    }
                } else if (left_type == OBJECT_MAP) {
                    res = object_get_map_value(left, index);
                } else if (left_type == OBJECT_STRING) {
                    const char *str = object_get_string(left);
                    int ix = (int)object_get_number(index);
                    if (ix >= 0 && ix < (int)strlen(str)) {
                        char res_str[2] = {str[ix], '\0'};
                        res = object_make_string(vm->mem, res_str);
                    }
                }
                stack_push(vm, res);
                break;
            }
            case OPCODE_GET_VALUE_AT: {
                object_t index = stack_pop(vm);
                object_t left = stack_pop(vm);
                object_type_t left_type = object_get_type(left);
                object_type_t index_type = object_get_type(index);
                const char *left_type_name = object_get_type_name(left_type);
                const char *index_type_name = object_get_type_name(index_type);

                if (left_type != OBJECT_ARRAY && left_type != OBJECT_MAP && left_type != OBJECT_STRING) {
                    errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                      "Type %s is not indexable", left_type_name);
                    goto err;
                }

                object_t res = object_make_null();
                if (index_type != OBJECT_NUMBER) {
                    errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                      "Cannot index %s with %s", left_type_name, index_type_name);
                    goto err;
                }
                int ix = (int)object_get_number(index);

                if (left_type == OBJECT_ARRAY) {
                    res = object_get_array_value_at(left, ix);
                } else if (left_type == OBJECT_MAP) {
                    res = object_get_kv_pair_at(vm->mem, left, ix);
                } else if (left_type == OBJECT_STRING) {
                    const char *str = object_get_string(left);
                    int ix = (int)object_get_number(index);
                    if (ix >= 0 && ix < (int)strlen(str)) {
                        char res_str[2] = {str[ix], '\0'};
                        res = object_make_string(vm->mem, res_str);
                    }
                }
                stack_push(vm, res);
                break;
            }
            case OPCODE_CALL: {
                uint8_t num_args = frame_read_uint8(vm->current_frame);
                object_t callee = stack_get(vm, num_args);
                bool ok = call_object(vm, callee, num_args);
                if (!ok) {
                    goto err;
                }
                break;
            }
            case OPCODE_RETURN_VALUE: {
                object_t res = stack_pop(vm);
                bool ok = pop_frame(vm);
                if (!ok) {
                    goto end;
                }
                stack_push(vm, res);
                break;
            }
            case OPCODE_RETURN: {
                bool ok = pop_frame(vm);
                stack_push(vm, object_make_null());
                if (!ok) {
                    stack_pop(vm);
                    goto end;
                }
                break;
            }
            case OPCODE_DEFINE_LOCAL: {
                uint8_t pos = frame_read_uint8(vm->current_frame);
                vm->stack[vm->current_frame->base_pointer + pos] = stack_pop(vm);
                break;
            }
            case OPCODE_SET_LOCAL: {
                uint8_t pos = frame_read_uint8(vm->current_frame);
                object_t new_value = stack_pop(vm);
                object_t old_value = vm->stack[vm->current_frame->base_pointer + pos];
                if (!check_assign(vm, old_value, new_value)) {
                    goto err;
                }
                vm->stack[vm->current_frame->base_pointer + pos] = new_value;
                break;
            }
            case OPCODE_GET_LOCAL: {
                uint8_t pos = frame_read_uint8(vm->current_frame);
                object_t val = vm->stack[vm->current_frame->base_pointer + pos];
                stack_push(vm, val);
                break;
            }
            case OPCODE_GET_APE_GLOBAL: {
                uint16_t ix = frame_read_uint16(vm->current_frame);
                bool ok = false;
                object_t val = global_store_get_object_at(vm->global_store, ix, &ok);
                if (!ok) {
                    errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "Global value %d not found", ix);
                    goto err;
                }
                stack_push(vm, val);
                break;
            }
            case OPCODE_FUNCTION: {
                uint16_t constant_ix = frame_read_uint16(vm->current_frame);
                uint8_t num_free = frame_read_uint8(vm->current_frame);
                object_t *constant = array_get(constants, constant_ix);
                if (!constant) {
                    errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "Constant %d not found", constant_ix);
                    goto err;
                }
                object_type_t constant_type = object_get_type(*constant);
                if (constant_type != OBJECT_FUNCTION) {
                    const char *type_name = object_get_type_name(constant_type);
                    errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "%s is not a function", type_name);
                    goto err;
                }

                const function_t *constant_function = object_get_function(*constant);
                object_t function_obj = object_make_function(vm->mem, object_get_function_name(*constant),
                                                            constant_function->comp_result, false,
                                                            constant_function->num_locals, constant_function->num_args,
                                                            num_free);
                if (object_is_null(function_obj)) {
                    goto err;
                }
                for (int i = 0; i < num_free; i++) {
                    object_t free_val = vm->stack[vm->sp - num_free + i];
                    object_set_function_free_val(function_obj, i, free_val);
                }
                set_sp(vm, vm->sp - num_free);
                stack_push(vm, function_obj);
                break;
            }
            case OPCODE_GET_FREE: {
                uint8_t free_ix = frame_read_uint8(vm->current_frame);
                object_t val = object_get_function_free_val(vm->current_frame->function, free_ix);
                stack_push(vm, val);
                break;
            }
            case OPCODE_SET_FREE: {
                uint8_t free_ix = frame_read_uint8(vm->current_frame);
                object_t val = stack_pop(vm);
                object_set_function_free_val(vm->current_frame->function, free_ix, val);
                break;
            }
            case OPCODE_CURRENT_FUNCTION: {
                object_t current_function = vm->current_frame->function;
                stack_push(vm, current_function);
                break;
            }
            case OPCODE_SET_INDEX: {
                object_t index = stack_pop(vm);
                object_t left = stack_pop(vm);
                object_t new_value = stack_pop(vm);
                object_type_t left_type = object_get_type(left);
                object_type_t index_type = object_get_type(index);
                const char *left_type_name = object_get_type_name(left_type);
                const char *index_type_name = object_get_type_name(index_type);

                if (left_type != OBJECT_ARRAY && left_type != OBJECT_MAP) {
                    errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                      "Type %s is not indexable", left_type_name);
                    goto err;
                }

                if (left_type == OBJECT_ARRAY) {
                    if (index_type != OBJECT_NUMBER) {
                        errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                          "Cannot index %s with %s", left_type_name, index_type_name);
                        goto err;
                    }
                    int ix = (int)object_get_number(index);
                    ok = object_set_array_value_at(left, ix, new_value);
                    if (!ok) {
                        errors_add_error(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "Setting array item failed (out of bounds?)");
                        goto err;
                    }
                } else if (left_type == OBJECT_MAP) {
                    object_t old_value = object_get_map_value(left, index);
                    if (!check_assign(vm, old_value, new_value)) {
                        goto err;
                    }
                    ok = object_set_map_value(left, index, new_value);
                    if (!ok) {
                        goto err;
                    }
                }
                break;
            }
            case OPCODE_DUP: {
                object_t val = stack_get(vm, 0);
                stack_push(vm, val);
                break;
            }
            case OPCODE_LEN: {
                object_t val = stack_pop(vm);
                int len = 0;
                object_type_t type = object_get_type(val);
                if (type == OBJECT_ARRAY) {
                    len = object_get_array_length(val);
                } else if (type == OBJECT_MAP) {
                    len = object_get_map_length(val);
                } else if (type == OBJECT_STRING) {
                    const char *str = object_get_string(val);
                    len = (int)strlen(str);
                } else {
                    const char *type_name = object_get_type_name(type);
                    errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "Cannot get length of %s", type_name);
                    goto err;
                }
                stack_push(vm, object_make_number(len));
                break;
            }
            case OPCODE_NUMBER: {
                uint64_t val = frame_read_uint64(vm->current_frame);
                double val_double = ape_uint64_to_double(val);
                object_t obj = object_make_number(val_double);
                stack_push(vm, obj);
                break;
            }
            case OPCODE_SET_RECOVER: {
                uint16_t recover_ip = frame_read_uint16(vm->current_frame);
                vm->current_frame->recover_ip = recover_ip;
                break;
            }
            default: {
                APE_ASSERT(false);
                errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "Unknown opcode: 0x%x", opcode);
                goto err;
            }
        }

        if (check_time) {
            time_check_counter++;
            if (time_check_counter > time_check_interval) {
                int elapsed_ms = (int)ape_timer_get_elapsed_ms(&timer);
                if (elapsed_ms > max_exec_time_ms) {
                    errors_add_errorf(vm->errors, ERROR_TIMEOUT, frame_src_position(vm->current_frame), "Execution took more than %1.17g ms", max_exec_time_ms);
                    goto err;
                }
                time_check_counter = 0;
            }
        }
    err:
        if (errors_get_count(vm->errors) > 0) {
            error_t *err = errors_get_last_error(vm->errors);
            if (err->type == ERROR_RUNTIME && errors_get_count(vm->errors) == 1) {
                int recover_frame_ix = -1;
                for (int i = vm->frames_count - 1; i >= 0; i--) {
                    frame_t *frame = &vm->frames[i];
                    if (frame->recover_ip >= 0 && !frame->is_recovering) {
                        recover_frame_ix = i;
                        break;
                    }
                }
                if (recover_frame_ix < 0) {
                    goto end;
                } else {
                    if (!err->traceback) {
                        err->traceback = traceback_make(vm->alloc);
                    }
                    if (err->traceback) {
                        traceback_append_from_vm(err->traceback, vm);
                    }
                    while (vm->frames_count > (recover_frame_ix + 1)) {
                        pop_frame(vm);
                    }
                    object_t err_obj = object_make_error(vm->mem, err->message);
                    if (!object_is_null(err_obj)) {
                        object_set_error_traceback(err_obj, err->traceback);
                        err->traceback = NULL;
                    }
                    stack_push(vm, err_obj);
                    vm->current_frame->ip = vm->current_frame->recover_ip;
                    vm->current_frame->is_recovering = true;
                    errors_clear(vm->errors);
                }
            } else {
                goto end;
            }
        }
        if (gc_should_sweep(vm->mem)) {
            run_gc(vm, constants);
        }
    }

end:
    if (errors_get_count(vm->errors) > 0) {
        error_t *err = errors_get_last_error(vm->errors);
        if (!err->traceback) {
            err->traceback = traceback_make(vm->alloc);
        }
        if (err->traceback) {
            traceback_append_from_vm(err->traceback, vm);
        }
    }

    run_gc(vm, constants);

    vm->running = false;
    return errors_get_count(vm->errors) == 0;
}

object_t vm_get_last_popped(vm_t *vm) {
    return vm->last_popped;
}

bool vm_has_errors(vm_t *vm) {
    return errors_get_count(vm->errors) > 0;
}

bool vm_set_global(vm_t *vm, int ix, object_t val) {
    if (ix >= VM_MAX_GLOBALS) {
        APE_ASSERT(false);
        errors_add_error(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "Global write out of range");
        return false;
    }
    vm->globals[ix] = val;
    if (ix >= vm->globals_count) {
        vm->globals_count = ix + 1;
    }
    return true;
}

object_t vm_get_global(vm_t *vm, int ix) {
    if (ix >= VM_MAX_GLOBALS) {
        APE_ASSERT(false);
        errors_add_error(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "Global read out of range");
        return object_make_null();
    }
    return vm->globals[ix];
}

// INTERNAL
static void set_sp(vm_t *vm, int new_sp) {
    if (new_sp > vm->sp) { // to avoid gcing freed objects
        int count = new_sp - vm->sp;
        size_t bytes_count = count * sizeof(object_t);
        memset(vm->stack + vm->sp, 0, bytes_count);
    }
    vm->sp = new_sp;
}

static void stack_push(vm_t *vm, object_t obj) {
#ifdef APE_DEBUG
    if (vm->sp >= VM_STACK_SIZE) {
        APE_ASSERT(false);
        errors_add_error(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "Stack overflow");
        return;
    }
    if (vm->current_frame) {
        frame_t *frame = vm->current_frame;
        function_t *current_function = object_get_function(frame->function);
        int num_locals = current_function->num_locals;
        APE_ASSERT(vm->sp >= (frame->base_pointer + num_locals));
    }
#endif
    vm->stack[vm->sp] = obj;
    vm->sp++;
}

static object_t stack_pop(vm_t *vm) {
#ifdef APE_DEBUG
    if (vm->sp == 0) {
        errors_add_error(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "Stack underflow");
        APE_ASSERT(false);
        return object_make_null();
    }
    if (vm->current_frame) {
        frame_t *frame = vm->current_frame;
        function_t *current_function = object_get_function(frame->function);
        int num_locals = current_function->num_locals;
        APE_ASSERT((vm->sp - 1) >= (frame->base_pointer + num_locals));
    }
#endif
    vm->sp--;
    object_t res = vm->stack[vm->sp];
    vm->last_popped = res;
    return res;
}

static object_t stack_get(vm_t *vm, int nth_item) {
    int ix = vm->sp - 1 - nth_item;
#ifdef APE_DEBUG
    if (ix < 0 || ix >= VM_STACK_SIZE) {
        errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                  "Invalid stack index: %d", nth_item);
        APE_ASSERT(false);
        return object_make_null();
    }
#endif
    return vm->stack[ix];
}

static void this_stack_push(vm_t *vm, object_t obj) {
#ifdef APE_DEBUG
    if (vm->this_sp >= VM_THIS_STACK_SIZE) {
        APE_ASSERT(false);
        errors_add_error(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "this stack overflow");
        return;
    }
#endif
    vm->this_stack[vm->this_sp] = obj;
    vm->this_sp++;
}

static object_t this_stack_pop(vm_t *vm) {
#ifdef APE_DEBUG
    if (vm->this_sp == 0) {
        errors_add_error(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame), "this stack underflow");
        APE_ASSERT(false);
        return object_make_null();
    }
#endif
    vm->this_sp--;
    return vm->this_stack[vm->this_sp];
}

static object_t this_stack_get(vm_t *vm, int nth_item) {
    int ix = vm->this_sp - 1 - nth_item;
#ifdef APE_DEBUG
    if (ix < 0 || ix >= VM_THIS_STACK_SIZE) {
        errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                   "Invalid this stack index: %d", nth_item);
        APE_ASSERT(false);
        return object_make_null();
    }
#endif
    return vm->this_stack[ix];
}

static bool push_frame(vm_t *vm, frame_t frame) {
    if (vm->frames_count >= VM_MAX_FRAMES) {
        APE_ASSERT(false);
        return false;
    }
    vm->frames[vm->frames_count] = frame;
    vm->current_frame = &vm->frames[vm->frames_count];
    vm->frames_count++;
    function_t *frame_function = object_get_function(frame.function);
    set_sp(vm, frame.base_pointer + frame_function->num_locals);
    return true;
}

static bool pop_frame(vm_t *vm) {
    set_sp(vm, vm->current_frame->base_pointer - 1);
    if (vm->frames_count <= 0) {
        APE_ASSERT(false);
        vm->current_frame = NULL;
        return false;
    }
    vm->frames_count--;
    if (vm->frames_count == 0) {
        vm->current_frame = NULL;
        return false;
    }
    vm->current_frame = &vm->frames[vm->frames_count - 1];
    return true;
}

static void run_gc(vm_t *vm, array(object_t) *constants) {
    gc_unmark_all(vm->mem);
    gc_mark_objects(global_store_get_object_data(vm->global_store), global_store_get_object_count(vm->global_store));
    gc_mark_objects(array_data(constants), array_count(constants));
    gc_mark_objects(vm->globals, vm->globals_count);
    for (int i = 0; i < vm->frames_count; i++) {
        frame_t *frame = &vm->frames[i];
        gc_mark_object(frame->function);
    }
    gc_mark_objects(vm->stack, vm->sp);
    gc_mark_objects(vm->this_stack, vm->this_sp);
    gc_mark_object(vm->last_popped);
    gc_mark_objects(vm->operator_oveload_keys, OPCODE_MAX);
    gc_sweep(vm->mem);
}

static bool call_object(vm_t *vm, object_t callee, int num_args) {
    object_type_t callee_type = object_get_type(callee);
    if (callee_type == OBJECT_FUNCTION) {
        function_t *callee_function = object_get_function(callee);
        if (num_args != callee_function->num_args) {
            errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                              "Invalid number of arguments to \"%s\", expected %d, got %d",
                              object_get_function_name(callee), callee_function->num_args, num_args);
            return false;
        }
        frame_t callee_frame;
        bool ok = frame_init(&callee_frame, callee, vm->sp - num_args);
        if (!ok) {
            errors_add_error(vm->errors, ERROR_RUNTIME, src_pos_invalid, "Frame init failed in call_object");
            return false;
        }
        ok = push_frame(vm, callee_frame);
        if (!ok) {
            errors_add_error(vm->errors, ERROR_RUNTIME, src_pos_invalid, "Pushing frame failed in call_object");
            return false;
        }
    } else if (callee_type == OBJECT_NATIVE_FUNCTION) {
        object_t *stack_pos = vm->stack + vm->sp - num_args;
        object_t res = call_native_function(vm, callee, frame_src_position(vm->current_frame), num_args, stack_pos);
        if (vm_has_errors(vm)) {
            return false;
        }
        set_sp(vm, vm->sp - num_args - 1);
        stack_push(vm, res);
    } else {
        const char *callee_type_name = object_get_type_name(callee_type);
        errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                          "%s object is not callable", callee_type_name);
        return false;
    }
    return true;
}

static object_t call_native_function(vm_t *vm, object_t callee, src_pos_t src_pos, int argc, object_t *args) {
    native_function_t *native_fun = object_get_native_function(callee);
    object_t res = native_fun->fn(vm, native_fun->data, argc, args);
    if (errors_has_errors(vm->errors) && !APE_STREQ(native_fun->name, "crash")) {
        error_t *err = errors_get_last_error(vm->errors);
        err->pos = src_pos;
        err->traceback = traceback_make(vm->alloc);
        if (err->traceback) {
            traceback_append(err->traceback, native_fun->name, src_pos_invalid);
        }
        return object_make_null();
    }
    object_type_t res_type = object_get_type(res);
    if (res_type == OBJECT_ERROR) {
        traceback_t *traceback = traceback_make(vm->alloc);
        if (traceback) {
            // error builtin is treated in a special way
            if (!APE_STREQ(native_fun->name, "error")) {
                traceback_append(traceback, native_fun->name, src_pos_invalid);
            }
            traceback_append_from_vm(traceback, vm);
            object_set_error_traceback(res, traceback);
        }
    }
    return res;
}

static bool check_assign(vm_t *vm, object_t old_value, object_t new_value) {
    object_type_t old_value_type = object_get_type(old_value);
    object_type_t new_value_type = object_get_type(new_value);
    if (old_value_type == OBJECT_NULL || new_value_type == OBJECT_NULL) {
        return true;
    }
    if (old_value_type != new_value_type) {
        errors_add_errorf(vm->errors, ERROR_RUNTIME, frame_src_position(vm->current_frame),
                                   "Trying to assign variable of type %s to %s",
                                   object_get_type_name(new_value_type),
                                   object_get_type_name(old_value_type)
                                   );
        return false;
    }
    return true;
}

static bool try_overload_operator(vm_t *vm, object_t left, object_t right, opcode_t op, bool *out_overload_found) {
    *out_overload_found = false;
    object_type_t left_type = object_get_type(left);
    object_type_t right_type = object_get_type(right);
    if (left_type != OBJECT_MAP && right_type != OBJECT_MAP) {
        *out_overload_found = false;
        return true;
    }

    int num_operands = 2;
    if (op == OPCODE_MINUS || op == OPCODE_BANG) {
        num_operands = 1;
    }

    object_t key = vm->operator_oveload_keys[op];
    object_t callee = object_make_null();
    if (left_type == OBJECT_MAP) {
        callee = object_get_map_value(left, key);
    }
    if (!object_is_callable(callee)) {
        if (right_type == OBJECT_MAP) {
            callee = object_get_map_value(right, key);
        }

        if (!object_is_callable(callee)) {
            *out_overload_found = false;
            return true;
        }
    }

    *out_overload_found = true;

    stack_push(vm, callee);
    stack_push(vm, left);
    if (num_operands == 2) {
        stack_push(vm, right);
    }
    return call_object(vm, callee, num_operands);
}

//FILE_END
//FILE_START:ape.c
#include "ape.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define APE_IMPL_VERSION_MAJOR 0
#define APE_IMPL_VERSION_MINOR 12
#define APE_IMPL_VERSION_PATCH 1

#if (APE_VERSION_MAJOR != APE_IMPL_VERSION_MAJOR)\
 || (APE_VERSION_MINOR != APE_IMPL_VERSION_MINOR)\
 || (APE_VERSION_PATCH != APE_IMPL_VERSION_PATCH)
    #error "Version mismatch"
#endif

#ifndef APE_AMALGAMATED
#include "ape.h"
#include "gc.h"
#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"
#include "errors.h"
#include "symbol_table.h"
#include "traceback.h"
#include "global_store.h"
#endif

typedef struct native_fn_wrapper {
    ape_native_fn fn;
    ape_t *ape;
    void *data;
} native_fn_wrapper_t;

typedef struct ape_program {
    ape_t *ape;
    compilation_result_t *comp_res;
} ape_program_t;

typedef struct ape {
    allocator_t alloc;
    gcmem_t *mem;
    ptrarray(compiled_file_t) *files;
    global_store_t *global_store;
    compiler_t *compiler;
    vm_t *vm;
    errors_t errors;
    ape_config_t config;

    allocator_t custom_allocator;

} ape_t;

static void ape_deinit(ape_t *ape);
static object_t ape_native_fn_wrapper(vm_t *vm, void *data, int argc, object_t *args);
static object_t ape_object_to_object(ape_object_t obj);
static ape_object_t object_to_ape_object(object_t obj);
static ape_object_t ape_object_make_native_function_with_name(ape_t *ape, const char *name, ape_native_fn fn, void *data);

static void reset_state(ape_t *ape);
static void set_default_config(ape_t *ape);
static char* read_file_default(void *ctx, const char *filename);
static size_t write_file_default(void* context, const char *path, const char *string, size_t string_size);
static size_t stdout_write_default(void* context, const void *data, size_t size);

static void* ape_malloc(void *ctx, size_t size);
static void ape_free(void *ctx, void *ptr);

//-----------------------------------------------------------------------------
// Ape
//-----------------------------------------------------------------------------
ape_t *ape_make(void) {
    return ape_make_ex(NULL, NULL, NULL);
}

ape_t* ape_make_ex(ape_malloc_fn malloc_fn, ape_free_fn free_fn, void *ctx) {
    allocator_t custom_alloc = allocator_make((allocator_malloc_fn)malloc_fn, (allocator_free_fn)free_fn, ctx);
    
    ape_t *ape = allocator_malloc(&custom_alloc, sizeof(ape_t));
    if (!ape) {
        return NULL;
    }

    memset(ape, 0, sizeof(ape_t));
    ape->alloc = allocator_make(ape_malloc, ape_free, ape);
    ape->custom_allocator = custom_alloc;

    set_default_config(ape);

    errors_init(&ape->errors);

    ape->mem = gcmem_make(&ape->alloc);
    if (!ape->mem) {
        goto err;
    }

    ape->files = ptrarray_make(&ape->alloc);
    if (!ape->files) {
        goto err;
    }

    ape->global_store = global_store_make(&ape->alloc, ape->mem);
    if (!ape->global_store) {
        goto err;
    }

    ape->compiler = compiler_make(&ape->alloc, &ape->config, ape->mem, &ape->errors, ape->files, ape->global_store);
    if (!ape->compiler) {
        goto err;
    }
    
    ape->vm = vm_make(&ape->alloc, &ape->config, ape->mem, &ape->errors, ape->global_store);
    if (!ape->vm) {
        goto err;
    }

    return ape;
err:
    ape_deinit(ape);
    allocator_free(&custom_alloc, ape);
    return NULL;
}

void ape_destroy(ape_t *ape) {
    if (!ape) {
        return;
    }
    ape_deinit(ape);
    allocator_t alloc = ape->alloc;
    allocator_free(&alloc, ape);
}

void ape_free_allocated(ape_t *ape, void *ptr) {
    allocator_free(&ape->alloc, ptr);
}

void ape_set_repl_mode(ape_t *ape, bool enabled) {
    ape->config.repl_mode = enabled;
}

bool ape_set_timeout(ape_t *ape, double max_execution_time_ms) {
    if (!ape_timer_platform_supported()) {
        ape->config.max_execution_time_ms = 0;
        ape->config.max_execution_time_set = false;
        return false;
    }

    if (max_execution_time_ms >= 0) {
        ape->config.max_execution_time_ms = max_execution_time_ms;
        ape->config.max_execution_time_set = true;
    } else {
        ape->config.max_execution_time_ms = 0;
        ape->config.max_execution_time_set = false;
    }
    return true;
}

void ape_set_stdout_write_function(ape_t *ape, ape_stdout_write_fn stdout_write, void *context) {
    ape->config.stdio.write.write = stdout_write;
    ape->config.stdio.write.context = context;
}

void ape_set_file_write_function(ape_t *ape, ape_write_file_fn file_write, void *context) {
    ape->config.fileio.write_file.write_file = file_write;
    ape->config.fileio.write_file.context = context;
}

void ape_set_file_read_function(ape_t *ape, ape_read_file_fn file_read, void *context) {
    ape->config.fileio.read_file.read_file = file_read;
    ape->config.fileio.read_file.context = context;
}

ape_program_t* ape_compile(ape_t *ape, const char *code) {
    ape_clear_errors(ape);

    compilation_result_t *comp_res = NULL;

    comp_res = compiler_compile(ape->compiler, code);
    if (!comp_res || errors_get_count(&ape->errors) > 0) {
        goto err;
    }

    ape_program_t *program = allocator_malloc(&ape->alloc, sizeof(ape_program_t));
    if (!program) {
        goto err;
    }
    program->ape = ape;
    program->comp_res = comp_res;
    return program;

err:
    compilation_result_destroy(comp_res);
    return NULL;
}

ape_program_t* ape_compile_file(ape_t *ape, const char *path) {
    ape_clear_errors(ape);

    compilation_result_t *comp_res = NULL;

    comp_res = compiler_compile_file(ape->compiler, path);
    if (!comp_res || errors_get_count(&ape->errors) > 0) {
        goto err;
    }

    ape_program_t *program = allocator_malloc(&ape->alloc, sizeof(ape_program_t));
    if (!program) {
        goto err;
    }

    program->ape = ape;
    program->comp_res = comp_res;
    return program;

err:
    compilation_result_destroy(comp_res);
    return NULL;
}

ape_object_t ape_execute_program(ape_t *ape, const ape_program_t *program) {
    reset_state(ape);
 
    if (ape != program->ape) {
        errors_add_error(&ape->errors, ERROR_USER, src_pos_invalid, "ape program was compiled with a different ape instance");
        return ape_object_make_null();
    }

    bool ok = vm_run(ape->vm, program->comp_res, compiler_get_constants(ape->compiler));
    if (!ok || errors_get_count(&ape->errors) > 0) {
        return ape_object_make_null();
    }

    APE_ASSERT(ape->vm->sp == 0);

    object_t res = vm_get_last_popped(ape->vm);
    if (object_get_type(res) == OBJECT_NONE) {
        return ape_object_make_null();
    }

    return object_to_ape_object(res);
}

void ape_program_destroy(ape_program_t *program) {
    if (!program) {
        return;
    }
    compilation_result_destroy(program->comp_res);
    allocator_free(&program->ape->alloc, program);
}

ape_object_t ape_execute(ape_t *ape, const char *code) {
    reset_state(ape);

    compilation_result_t *comp_res = NULL;

    comp_res = compiler_compile(ape->compiler, code);
    if (!comp_res || errors_get_count(&ape->errors) > 0) {
        goto err;
    }

    bool ok = vm_run(ape->vm, comp_res, compiler_get_constants(ape->compiler));
    if (!ok || errors_get_count(&ape->errors) > 0) {
        goto err;
    }

    APE_ASSERT(ape->vm->sp == 0);

    object_t res = vm_get_last_popped(ape->vm);
    if (object_get_type(res) == OBJECT_NONE) {
        goto err;
    }

    compilation_result_destroy(comp_res);

    return object_to_ape_object(res);

err:
    compilation_result_destroy(comp_res);
    return ape_object_make_null();
}

ape_object_t ape_execute_file(ape_t *ape, const char *path) {
    reset_state(ape);

    compilation_result_t *comp_res = NULL;

    comp_res = compiler_compile_file(ape->compiler, path);
    if (!comp_res || errors_get_count(&ape->errors) > 0) {
        goto err;
    }

    bool ok = vm_run(ape->vm, comp_res, compiler_get_constants(ape->compiler));
    if (!ok || errors_get_count(&ape->errors) > 0) {
        goto err;
    }

    APE_ASSERT(ape->vm->sp == 0);

    object_t res = vm_get_last_popped(ape->vm);
    if (object_get_type(res) == OBJECT_NONE) {
        goto err;
    }

    compilation_result_destroy(comp_res);

    return object_to_ape_object(res);

err:
    compilation_result_destroy(comp_res);
    return ape_object_make_null();
}

ape_object_t ape_call(ape_t *ape, const char *function_name, int argc, ape_object_t *args) {
    reset_state(ape);

    object_t callee = ape_object_to_object(ape_get_object(ape, function_name));
    if (object_get_type(callee) == OBJECT_NULL) {
        return ape_object_make_null();
    }
    object_t res = vm_call(ape->vm, compiler_get_constants(ape->compiler), callee, argc, (object_t*)args);
    if (errors_get_count(&ape->errors) > 0) {
        return ape_object_make_null();
    }
    return object_to_ape_object(res);
}

bool ape_has_errors(const ape_t *ape) {
    return ape_errors_count(ape) > 0;
}

int ape_errors_count(const ape_t *ape) {
    return errors_get_count(&ape->errors);
}

void ape_clear_errors(ape_t *ape) {
    errors_clear(&ape->errors);
}

const ape_error_t* ape_get_error(const ape_t *ape, int index) {
    return (const ape_error_t*)errors_getc(&ape->errors, index);
}

bool ape_set_native_function(ape_t *ape, const char *name, ape_native_fn fn, void *data) {
    ape_object_t obj = ape_object_make_native_function_with_name(ape, name, fn, data);
    if (ape_object_is_null(obj)) {
        return false;
    }
    return ape_set_global_constant(ape, name, obj);
}

bool ape_set_global_constant(ape_t *ape, const char *name, ape_object_t obj) {
    return global_store_set(ape->global_store, name, ape_object_to_object(obj));
}

ape_object_t ape_get_object(ape_t *ape, const char *name) {
    symbol_table_t *st = compiler_get_symbol_table(ape->compiler);
    const symbol_t *symbol = symbol_table_resolve(st, name);
    if (!symbol) {
        errors_add_errorf(&ape->errors, ERROR_USER, src_pos_invalid, "Symbol \"%s\" is not defined", name);
        return ape_object_make_null();
    }
    object_t res = object_make_null();
    if (symbol->type == SYMBOL_MODULE_GLOBAL) {
        res = vm_get_global(ape->vm, symbol->index);
    } else if (symbol->type == SYMBOL_APE_GLOBAL) {
        bool ok = false;
        res = global_store_get_object_at(ape->global_store, symbol->index, &ok);
        if (!ok) {
            errors_add_errorf(&ape->errors, ERROR_USER, src_pos_invalid, "Failed to get global object at %d", symbol->index);
            return ape_object_make_null();
        }
    } else {
        errors_add_errorf(&ape->errors, ERROR_USER, src_pos_invalid, "Value associated with symbol \"%s\" could not be loaded", name);
        return ape_object_make_null();
    }
    return object_to_ape_object(res);
}

bool ape_check_args(ape_t *ape, bool generate_error, int argc, ape_object_t *args, int expected_argc, int *expected_types) {
    if (argc != expected_argc) {
        if (generate_error) {
            ape_set_runtime_errorf(ape, "Invalid number or arguments, got %d instead of %d", argc, expected_argc);
        }
        return false;
    }
    
    for(int i = 0; i < argc; i++) {
        ape_object_t arg = args[i];
        ape_object_type_t type = ape_object_get_type(arg);
        ape_object_type_t expected_type = expected_types[i];
        if (!(type & expected_type)) {
            if (generate_error) {
                const char *type_str = ape_object_get_type_name(type);
                const char *expected_type_str = ape_object_get_type_name(expected_type);
                ape_set_runtime_errorf(ape, "Invalid argument type, got %s, expected %s", type_str, expected_type_str);
            }
            return false;
        }
    }
    return true;
}

//-----------------------------------------------------------------------------
// Ape object
//-----------------------------------------------------------------------------

ape_object_t ape_object_make_number(double val) {
    return object_to_ape_object(object_make_number(val));
}

ape_object_t ape_object_make_bool(bool val) {
    return object_to_ape_object(object_make_bool(val));
}

ape_object_t ape_object_make_string(ape_t *ape, const char *str) {
    return object_to_ape_object(object_make_string(ape->mem, str));
}

ape_object_t ape_object_make_stringf(ape_t *ape, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int to_write = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    va_start(args, fmt);
    char *res = (char*)allocator_malloc(&ape->alloc, to_write + 1);
    if (!res) {
        return ape_object_make_null();
    }
    int written = vsprintf(res, fmt, args);
    (void)written;
    APE_ASSERT(written == to_write);
    va_end(args);
    return object_to_ape_object(object_make_string_no_copy(ape->mem, res));
}

ape_object_t ape_object_make_null() {
    return object_to_ape_object(object_make_null());
}

ape_object_t ape_object_make_array(ape_t *ape) {
    return object_to_ape_object(object_make_array(ape->mem));
}

ape_object_t ape_object_make_map(ape_t *ape) {
    return object_to_ape_object(object_make_map(ape->mem));
}

ape_object_t ape_object_make_native_function(ape_t *ape, ape_native_fn fn, void *data) {
    return ape_object_make_native_function_with_name(ape, "", fn, data);
}

ape_object_t ape_object_make_error(ape_t *ape, const char *msg) {
    return object_to_ape_object(object_make_error(ape->mem, msg));
}

ape_object_t ape_object_make_errorf(ape_t *ape, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int to_write = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    va_start(args, fmt);
    char *res = (char*)allocator_malloc(&ape->alloc, to_write + 1);
    if (!res) {
        return ape_object_make_null();
    }
    int written = vsprintf(res, fmt, args);
    (void)written;
    APE_ASSERT(written == to_write);
    va_end(args);
    return object_to_ape_object(object_make_error_no_copy(ape->mem, res));
}

ape_object_t ape_object_make_external(ape_t *ape, void *data) {
    object_t res = object_make_external(ape->mem, data);
    return object_to_ape_object(res);
}

char *ape_object_serialize(ape_t *ape, ape_object_t obj) {
    return object_serialize(&ape->alloc, ape_object_to_object(obj));
}

bool ape_object_disable_gc(ape_object_t ape_obj) {
    object_t obj = ape_object_to_object(ape_obj);
    return gc_disable_on_object(obj);
}

void ape_object_enable_gc(ape_object_t ape_obj) {
    object_t obj = ape_object_to_object(ape_obj);
    gc_enable_on_object(obj);
}

bool ape_object_equals(ape_object_t ape_a, ape_object_t ape_b){
    object_t a = ape_object_to_object(ape_a);
    object_t b = ape_object_to_object(ape_b);
    return object_equals(a, b);
}

bool ape_object_is_null(ape_object_t obj) {
    return ape_object_get_type(obj) == APE_OBJECT_NULL;
}

ape_object_t ape_object_copy(ape_object_t ape_obj) {
    object_t obj = ape_object_to_object(ape_obj);
    gcmem_t *mem = object_get_mem(obj);
    object_t res = object_copy(mem, obj);
    return object_to_ape_object(res);
}

ape_object_t ape_object_deep_copy(ape_object_t ape_obj) {
    object_t obj = ape_object_to_object(ape_obj);
    gcmem_t *mem = object_get_mem(obj);
    object_t res = object_deep_copy(mem, obj);
    return object_to_ape_object(res);
}

void ape_set_runtime_error(ape_t *ape, const char *message) {
    errors_add_error(&ape->errors, ERROR_RUNTIME, src_pos_invalid, message);
}

void ape_set_runtime_errorf(ape_t *ape, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int to_write = vsnprintf(NULL, 0, fmt, args);
    (void)to_write;
    va_end(args);
    va_start(args, fmt);
    char res[ERROR_MESSAGE_MAX_LENGTH];
    int written = vsnprintf(res, ERROR_MESSAGE_MAX_LENGTH, fmt, args);
    (void)written;
    APE_ASSERT(to_write == written);
    va_end(args);
    errors_add_error(&ape->errors, ERROR_RUNTIME, src_pos_invalid, res);
}

ape_object_type_t ape_object_get_type(ape_object_t ape_obj) {
    object_t obj = ape_object_to_object(ape_obj);
    switch (object_get_type(obj)) {
        case OBJECT_NONE:            return APE_OBJECT_NONE;
        case OBJECT_ERROR:           return APE_OBJECT_ERROR;
        case OBJECT_NUMBER:          return APE_OBJECT_NUMBER;
        case OBJECT_BOOL:            return APE_OBJECT_BOOL;
        case OBJECT_STRING:          return APE_OBJECT_STRING;
        case OBJECT_NULL:            return APE_OBJECT_NULL;
        case OBJECT_NATIVE_FUNCTION: return APE_OBJECT_NATIVE_FUNCTION;
        case OBJECT_ARRAY:           return APE_OBJECT_ARRAY;
        case OBJECT_MAP:             return APE_OBJECT_MAP;
        case OBJECT_FUNCTION:        return APE_OBJECT_FUNCTION;
        case OBJECT_EXTERNAL:        return APE_OBJECT_EXTERNAL;
        case OBJECT_FREED:           return APE_OBJECT_FREED;
        case OBJECT_ANY:             return APE_OBJECT_ANY;
        default:                     return APE_OBJECT_NONE;
    }
}

const char* ape_object_get_type_string(ape_object_t obj) {
    return ape_object_get_type_name(ape_object_get_type(obj));
}

const char* ape_object_get_type_name(ape_object_type_t type) {
    switch (type) {
        case APE_OBJECT_NONE:            return "NONE";
        case APE_OBJECT_ERROR:           return "ERROR";
        case APE_OBJECT_NUMBER:          return "NUMBER";
        case APE_OBJECT_BOOL:            return "BOOL";
        case APE_OBJECT_STRING:          return "STRING";
        case APE_OBJECT_NULL:            return "NULL";
        case APE_OBJECT_NATIVE_FUNCTION: return "NATIVE_FUNCTION";
        case APE_OBJECT_ARRAY:           return "ARRAY";
        case APE_OBJECT_MAP:             return "MAP";
        case APE_OBJECT_FUNCTION:        return "FUNCTION";
        case APE_OBJECT_EXTERNAL:        return "EXTERNAL";
        case APE_OBJECT_FREED:           return "FREED";
        case APE_OBJECT_ANY:             return "ANY";
        default:                         return "NONE";
    }
}

double ape_object_get_number(ape_object_t obj) {
    return object_get_number(ape_object_to_object(obj));
}

bool ape_object_get_bool(ape_object_t obj) {
    return object_get_bool(ape_object_to_object(obj));
}

const char *ape_object_get_string(ape_object_t obj) {
    return object_get_string(ape_object_to_object(obj));
}

const char *ape_object_get_error_message(ape_object_t obj) {
    return object_get_error_message(ape_object_to_object(obj));
}

const ape_traceback_t* ape_object_get_error_traceback(ape_object_t ape_obj) {
    object_t obj = ape_object_to_object(ape_obj);
    return (const ape_traceback_t*)object_get_error_traceback(obj);
}

bool ape_object_set_external_destroy_function(ape_object_t object, ape_data_destroy_fn destroy_fn) {
    return object_set_external_destroy_function(ape_object_to_object(object), (external_data_destroy_fn)destroy_fn);
}

bool ape_object_set_external_copy_function(ape_object_t object, ape_data_copy_fn copy_fn) {
    return object_set_external_copy_function(ape_object_to_object(object), (external_data_copy_fn)copy_fn);
}

//-----------------------------------------------------------------------------
// Ape object array
//-----------------------------------------------------------------------------

int ape_object_get_array_length(ape_object_t obj) {
    return object_get_array_length(ape_object_to_object(obj));
}

ape_object_t ape_object_get_array_value(ape_object_t obj, int ix) {
    object_t res = object_get_array_value_at(ape_object_to_object(obj), ix);
    return object_to_ape_object(res);
}

const char* ape_object_get_array_string(ape_object_t obj, int ix) {
    ape_object_t object = ape_object_get_array_value(obj, ix);
    if (ape_object_get_type(object) != APE_OBJECT_STRING) {
        return NULL;
    }
    return ape_object_get_string(object);
}

double ape_object_get_array_number(ape_object_t obj, int ix) {
    ape_object_t object = ape_object_get_array_value(obj, ix);
    if (ape_object_get_type(object) != APE_OBJECT_NUMBER) {
        return 0;
    }
    return ape_object_get_number(object);
}

bool ape_object_get_array_bool(ape_object_t obj, int ix) {
    ape_object_t object = ape_object_get_array_value(obj, ix);
    if (ape_object_get_type(object) != APE_OBJECT_BOOL) {
        return 0;
    }
    return ape_object_get_bool(object);
}

bool ape_object_set_array_value(ape_object_t ape_obj, int ix, ape_object_t ape_value) {
    object_t obj = ape_object_to_object(ape_obj);
    object_t value = ape_object_to_object(ape_value);
    return object_set_array_value_at(obj, ix, value);
}

bool ape_object_set_array_string(ape_object_t obj, int ix, const char *string) {
    gcmem_t *mem = object_get_mem(ape_object_to_object(obj));
    if (!mem) {
        return false;
    }
    object_t new_value = object_make_string(mem, string);
    return ape_object_set_array_value(obj, ix, object_to_ape_object(new_value));
}

bool ape_object_set_array_number(ape_object_t obj, int ix, double number) {
    object_t new_value = object_make_number(number);
    return ape_object_set_array_value(obj, ix, object_to_ape_object(new_value));
}

bool ape_object_set_array_bool(ape_object_t obj, int ix, bool value) {
    object_t new_value = object_make_bool(value);
    return ape_object_set_array_value(obj, ix, object_to_ape_object(new_value));
}

bool ape_object_add_array_value(ape_object_t ape_obj, ape_object_t ape_value) {
    object_t obj = ape_object_to_object(ape_obj);
    object_t value = ape_object_to_object(ape_value);
    return object_add_array_value(obj, value);
}

bool ape_object_add_array_string(ape_object_t obj, const char *string) {
    gcmem_t *mem = object_get_mem(ape_object_to_object(obj));
    if (!mem) {
        return false;
    }
    object_t new_value = object_make_string(mem, string);
    return ape_object_add_array_value(obj, object_to_ape_object(new_value));
}

bool ape_object_add_array_number(ape_object_t obj, double number) {
    object_t new_value = object_make_number(number);
    return ape_object_add_array_value(obj, object_to_ape_object(new_value));
}

bool ape_object_add_array_bool(ape_object_t obj, bool value) {
    object_t new_value = object_make_bool(value);
    return ape_object_add_array_value(obj, object_to_ape_object(new_value));
}

//-----------------------------------------------------------------------------
// Ape object map
//-----------------------------------------------------------------------------

int ape_object_get_map_length(ape_object_t obj) {
    return object_get_map_length(ape_object_to_object(obj));
}

ape_object_t ape_object_get_map_key_at(ape_object_t ape_obj, int ix) {
    object_t obj = ape_object_to_object(ape_obj);
    return object_to_ape_object(object_get_map_key_at(obj, ix));
}

ape_object_t ape_object_get_map_value_at(ape_object_t ape_obj, int ix) {
    object_t obj = ape_object_to_object(ape_obj);
    object_t res = object_get_map_value_at(obj, ix);
    return object_to_ape_object(res);
}

bool ape_object_set_map_value_at(ape_object_t ape_obj, int ix, ape_object_t ape_val) {
    object_t obj = ape_object_to_object(ape_obj);
    object_t val = ape_object_to_object(ape_val);
    return object_set_map_value_at(obj, ix, val);
}

bool ape_object_set_map_value_with_value_key(ape_object_t obj, ape_object_t key, ape_object_t val) {
    return object_set_map_value(ape_object_to_object(obj), ape_object_to_object(key), ape_object_to_object(val));
}

bool ape_object_set_map_value(ape_object_t obj, const char *key, ape_object_t value) {
    gcmem_t *mem = object_get_mem(ape_object_to_object(obj));
    if (!mem) {
        return false;
    }
    object_t key_object = object_make_string(mem, key);
    if (object_is_null(key_object)) {
        return false;
    }
    return ape_object_set_map_value_with_value_key(obj, object_to_ape_object(key_object), value);
}

bool ape_object_set_map_string(ape_object_t obj, const char *key, const char *string) {
    gcmem_t *mem = object_get_mem(ape_object_to_object(obj));
    if (!mem) {
        return false;
    }
    object_t string_object = object_make_string(mem, string);
    if (object_is_null(string_object)) {
        return false;
    }
    return ape_object_set_map_value(obj, key, object_to_ape_object(string_object));
}

bool ape_object_set_map_number(ape_object_t obj, const char *key, double number) {
    object_t number_object = object_make_number(number);
    return ape_object_set_map_value(obj, key, object_to_ape_object(number_object));
}

bool ape_object_set_map_bool(ape_object_t obj, const char *key, bool value) {
    object_t bool_object = object_make_bool(value);
    return ape_object_set_map_value(obj, key, object_to_ape_object(bool_object));
}

ape_object_t ape_object_get_map_value_with_value_key(ape_object_t obj, ape_object_t key) {
    return object_to_ape_object(object_get_map_value(ape_object_to_object(obj), ape_object_to_object(key)));
}

ape_object_t ape_object_get_map_value(ape_object_t object, const char *key) {
    gcmem_t *mem = object_get_mem(ape_object_to_object(object));
    if (!mem) {
        return ape_object_make_null();
    }
    object_t key_object = object_make_string(mem, key);
    if (object_is_null(key_object)) {
        return ape_object_make_null();
    }
    ape_object_t res = ape_object_get_map_value_with_value_key(object, object_to_ape_object(key_object));
    return res;
}

const char* ape_object_get_map_string(ape_object_t object, const char *key) {
    ape_object_t res = ape_object_get_map_value(object, key);
    return ape_object_get_string(res);
}

double ape_object_get_map_number(ape_object_t object, const char *key) {
    ape_object_t res = ape_object_get_map_value(object, key);
    return ape_object_get_number(res);
}

bool ape_object_get_map_bool(ape_object_t object, const char *key) {
    ape_object_t res = ape_object_get_map_value(object, key);
    return ape_object_get_bool(res);
}

bool ape_object_map_has_key(ape_object_t ape_object, const char *key) {
    object_t object = ape_object_to_object(ape_object);
    gcmem_t *mem = object_get_mem(object);
    if (!mem) {
        return false;
    }
    object_t key_object = object_make_string(mem, key);
    if (object_is_null(key_object)) {
        return false;
    }
    return object_map_has_key(object, key_object);
}

//-----------------------------------------------------------------------------
// Ape error
//-----------------------------------------------------------------------------

const char* ape_error_get_message(const ape_error_t *ape_error) {
    const error_t *error = (const error_t*)ape_error;
    return error->message;
}

const char* ape_error_get_filepath(const ape_error_t *ape_error) {
    const error_t *error = (const error_t*)ape_error;
    if (!error->pos.file) {
        return NULL;
    }
    return error->pos.file->path;
}

const char* ape_error_get_line(const ape_error_t *ape_error) {
    const error_t *error = (const error_t*)ape_error;
    if (!error->pos.file) {
        return NULL;
    }
    ptrarray(char*) *lines = error->pos.file->lines;
    if (error->pos.line >= ptrarray_count(lines)) {
        return NULL;
    }
    const char *line = ptrarray_get(lines, error->pos.line);
    return line;
}

int ape_error_get_line_number(const ape_error_t *ape_error) {
    const error_t *error = (const error_t*)ape_error;
    if (error->pos.line < 0) {
        return -1;
    }
    return error->pos.line + 1;
}

int ape_error_get_column_number(const ape_error_t *ape_error) {
    const error_t *error = (const error_t*)ape_error;
    if (error->pos.column < 0) {
        return -1;
    }
    return error->pos.column + 1;
}

ape_error_type_t ape_error_get_type(const ape_error_t *ape_error) {
    const error_t *error = (const error_t*)ape_error;
    switch (error->type) {
        case ERROR_NONE:        return APE_ERROR_NONE;
        case ERROR_PARSING:     return APE_ERROR_PARSING;
        case ERROR_COMPILATION: return APE_ERROR_COMPILATION;
        case ERROR_RUNTIME:     return APE_ERROR_RUNTIME;
        case ERROR_TIMEOUT:     return APE_ERROR_TIMEOUT;
        case ERROR_ALLOCATION:  return APE_ERROR_ALLOCATION;
        case ERROR_USER:        return APE_ERROR_USER;
        default:                return APE_ERROR_NONE;
    }
}

const char* ape_error_get_type_string(const ape_error_t *error) {
    return ape_error_type_to_string(ape_error_get_type(error));
}

const char* ape_error_type_to_string(ape_error_type_t type) {
    switch (type) {
        case APE_ERROR_PARSING:     return "PARSING";
        case APE_ERROR_COMPILATION: return "COMPILATION";
        case APE_ERROR_RUNTIME:     return "RUNTIME";
        case APE_ERROR_TIMEOUT:     return "TIMEOUT";
        case APE_ERROR_ALLOCATION:  return "ALLOCATION";
        case APE_ERROR_USER:        return "USER";
        default:                    return "NONE";
    }
}

char* ape_error_serialize(ape_t *ape, const ape_error_t *err) {
    const char *type_str = ape_error_get_type_string(err);
    const char *filename = ape_error_get_filepath(err);
    const char *line = ape_error_get_line(err);
    int line_num = ape_error_get_line_number(err);
    int col_num = ape_error_get_column_number(err);
    strbuf_t *buf = strbuf_make(&ape->alloc);
    if (!buf) {
        return NULL;
    }
    if (line) {
        strbuf_append(buf, line);
        strbuf_append(buf, "\n");
        if (col_num >= 0) {
            for (int j = 0; j < (col_num - 1); j++) {
                strbuf_append(buf, " ");
            }
            strbuf_append(buf, "^\n");
        }
    }
    strbuf_appendf(buf, "%s ERROR in \"%s\" on %d:%d: %s\n", type_str,
           filename, line_num, col_num, ape_error_get_message(err));
    const ape_traceback_t *traceback = ape_error_get_traceback(err);
    if (traceback) {
        strbuf_appendf(buf, "Traceback:\n");
        traceback_to_string((const traceback_t*)ape_error_get_traceback(err), buf);
    }
    if (strbuf_failed(buf)) {
        strbuf_destroy(buf);
        return NULL;
    }
    return strbuf_get_string_and_destroy(buf);
}

const ape_traceback_t* ape_error_get_traceback(const ape_error_t *ape_error) {
    const error_t *error = (const error_t*)ape_error;
    return (const ape_traceback_t*)error->traceback;
}

//-----------------------------------------------------------------------------
// Ape traceback
//-----------------------------------------------------------------------------

int ape_traceback_get_depth(const ape_traceback_t *ape_traceback) {
    const traceback_t *traceback = (const traceback_t*)ape_traceback;
    return array_count(traceback->items);
}

const char* ape_traceback_get_filepath(const ape_traceback_t *ape_traceback, int depth) {
    const traceback_t *traceback = (const traceback_t*)ape_traceback;
    traceback_item_t *item = array_get(traceback->items, depth);
    if (!item) {
        return NULL;
    }
    return traceback_item_get_filepath(item);
}

const char* ape_traceback_get_line(const ape_traceback_t *ape_traceback, int depth) {
    const traceback_t *traceback = (const traceback_t*)ape_traceback;
    traceback_item_t *item = array_get(traceback->items, depth);
    if (!item) {
        return NULL;
    }
    return traceback_item_get_line(item);
}

int ape_traceback_get_line_number(const ape_traceback_t *ape_traceback, int depth) {
    const traceback_t *traceback = (const traceback_t*)ape_traceback;
    traceback_item_t *item = array_get(traceback->items, depth);
    if (!item) {
        return -1;
    }
    return item->pos.line;
}

int ape_traceback_get_column_number(const ape_traceback_t *ape_traceback, int depth) {
    const traceback_t *traceback = (const traceback_t*)ape_traceback;
    traceback_item_t *item = array_get(traceback->items, depth);
    if (!item) {
        return -1;
    }
    return item->pos.column;
}

const char* ape_traceback_get_function_name(const ape_traceback_t *ape_traceback, int depth) {
    const traceback_t *traceback = (const traceback_t*)ape_traceback;
    traceback_item_t *item = array_get(traceback->items, depth);
    if (!item) {
        return "";
    }
    return item->function_name;
}

//-----------------------------------------------------------------------------
// Ape internal
//-----------------------------------------------------------------------------
static void ape_deinit(ape_t *ape) {
    vm_destroy(ape->vm);
    compiler_destroy(ape->compiler);
    global_store_destroy(ape->global_store);
    gcmem_destroy(ape->mem);
    ptrarray_destroy_with_items(ape->files, compiled_file_destroy);
    errors_deinit(&ape->errors);
}

static object_t ape_native_fn_wrapper(vm_t *vm, void *data, int argc, object_t *args) {
    (void)vm;
    native_fn_wrapper_t *wrapper = (native_fn_wrapper_t*)data;
    APE_ASSERT(vm == wrapper->ape->vm);
    ape_object_t res = wrapper->fn(wrapper->ape, wrapper->data, argc, (ape_object_t*)args);
    if (ape_has_errors(wrapper->ape)) {
        return object_make_null();
    }
    return ape_object_to_object(res);
}

static object_t ape_object_to_object(ape_object_t obj) {
    return (object_t){ .handle = obj._internal };
}

static ape_object_t object_to_ape_object(object_t obj) {
    return (ape_object_t){ ._internal = obj.handle };
}

static ape_object_t ape_object_make_native_function_with_name(ape_t *ape, const char *name, ape_native_fn fn, void *data) {
    native_fn_wrapper_t wrapper;
    memset(&wrapper, 0, sizeof(native_fn_wrapper_t));
    wrapper.fn = fn;
    wrapper.ape = ape;
    wrapper.data = data;
    object_t wrapper_native_function = object_make_native_function(ape->mem, name, ape_native_fn_wrapper, &wrapper, sizeof(wrapper));
    if (object_is_null(wrapper_native_function)) {
        return ape_object_make_null();
    }
    return object_to_ape_object(wrapper_native_function);
}

static void reset_state(ape_t *ape) {
    ape_clear_errors(ape);
    vm_reset(ape->vm);
}

static void set_default_config(ape_t *ape) {
    memset(&ape->config, 0, sizeof(ape_config_t));
    ape_set_repl_mode(ape, false);
    ape_set_timeout(ape, -1);
    ape_set_file_read_function(ape, read_file_default, ape);
    ape_set_file_write_function(ape, write_file_default, ape);
    ape_set_stdout_write_function(ape, stdout_write_default, ape);
}

static char* read_file_default(void *ctx, const char *filename){
    ape_t *ape = (ape_t*)ctx;
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
    file_contents = (char*)allocator_malloc(&ape->alloc, sizeof(char) * (size_to_read + 1));
    if (!file_contents) {
        fclose(fp);
        return NULL;
    }
    size_read = fread(file_contents, 1, size_to_read, fp);
    if (ferror(fp)) {
        fclose(fp);
        free(file_contents);
        return NULL;
    }
    fclose(fp);
    file_contents[size_read] = '\0';
    return file_contents;
}

static size_t write_file_default(void* ctx, const char *path, const char *string, size_t string_size) {
    (void)ctx;
    FILE *fp = fopen(path, "w");
    if (!fp) {
        return 0;
    }
    size_t written = fwrite(string, 1, string_size, fp);
    fclose(fp);
    return written;
}

static size_t stdout_write_default(void* ctx, const void *data, size_t size) {
    (void)ctx;
    return fwrite(data, 1, size, stdout);
}

static void* ape_malloc(void *ctx, size_t size) {
    ape_t *ape = (ape_t*)ctx;
    void *res = allocator_malloc(&ape->custom_allocator, size);
    if (!res) {
        errors_add_error(&ape->errors, ERROR_ALLOCATION, src_pos_invalid, "Allocation failed");
    }
    return res;
}

static void ape_free(void *ctx, void *ptr) {
    ape_t *ape = (ape_t*)ctx;
    allocator_free(&ape->custom_allocator, ptr);
}
//FILE_END
