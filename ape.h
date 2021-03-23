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

#ifndef ape_h
#define ape_h

#ifdef __cplusplus
extern "C"
{
#endif
#if 0
} // unconfuse xcode
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef _MSC_VER
#define __attribute__(x)
#endif

#define APE_VERSION_MAJOR 0
#define APE_VERSION_MINOR 7
#define APE_VERSION_PATCH 5

#define APE_VERSION_STRING "0.7.5"

typedef struct ape ape_t;
typedef struct ape_object { uint64_t _internal; } ape_object_t;
typedef struct ape_error ape_error_t;
typedef struct ape_program ape_program_t;
typedef struct ape_traceback ape_traceback_t;

typedef enum ape_error_type {
    APE_ERROR_NONE = 0,
    APE_ERROR_PARSING,
    APE_ERROR_COMPILATION,
    APE_ERROR_RUNTIME,
    APE_ERROR_USER, // from ape_add_error() or ape_add_errorf()
} ape_error_type_t;

typedef enum ape_object_type {
    APE_OBJECT_NONE            = 0,
    APE_OBJECT_ERROR           = 1 << 0,
    APE_OBJECT_NUMBER          = 1 << 1,
    APE_OBJECT_BOOL            = 1 << 2,
    APE_OBJECT_STRING          = 1 << 3,
    APE_OBJECT_NULL            = 1 << 4,
    APE_OBJECT_NATIVE_FUNCTION = 1 << 5,
    APE_OBJECT_ARRAY           = 1 << 6,
    APE_OBJECT_MAP             = 1 << 7,
    APE_OBJECT_FUNCTION        = 1 << 8,
    APE_OBJECT_EXTERNAL        = 1 << 9,
    APE_OBJECT_FREED           = 1 << 10,
    APE_OBJECT_ANY             = 0xffff, // for checking types with &
} ape_object_type_t;

typedef ape_object_t (*ape_native_fn)(ape_t *ape, void *data, int argc, ape_object_t *args);
typedef void*        (*ape_malloc_fn)(size_t size);
typedef void         (*ape_free_fn)(void *ptr);
typedef void         (*ape_data_destroy_fn)(void* data);
typedef void*        (*ape_data_copy_fn)(void* data);

typedef size_t (*ape_stdout_write_fn)(void* context, const void *data, size_t data_size);
typedef char*  (*ape_read_file_fn)(void* context, const char *path);
typedef size_t (*ape_write_file_fn)(void* context, const char *path, const char *string, size_t string_size);

//-----------------------------------------------------------------------------
// Ape API
//-----------------------------------------------------------------------------
void ape_set_memory_functions(ape_malloc_fn malloc_fn, ape_free_fn free_fn);

ape_t* ape_make(void);
void   ape_destroy(ape_t *ape);

void ape_set_repl_mode(ape_t *ape, bool enabled);

void ape_set_stdout_write_function(ape_t *ape, ape_stdout_write_fn stdout_write, void *context);
void ape_set_file_write_function(ape_t *ape, ape_write_file_fn file_write, void *context);
void ape_set_file_read_function(ape_t *ape, ape_read_file_fn file_read, void *context);

ape_program_t* ape_compile(ape_t *ape, const char *code);
ape_program_t* ape_compile_file(ape_t *ape, const char *path);
ape_object_t   ape_execute_program(ape_t *ape, const ape_program_t *program);
void           ape_program_destroy(ape_program_t *program);

ape_object_t  ape_execute(ape_t *ape, const char *code);
ape_object_t  ape_execute_file(ape_t *ape, const char *path);

ape_object_t  ape_call(ape_t *ape, const char *function_name, int argc, ape_object_t *args);
#define APE_CALL(ape, function_name, ...) \
    ape_call(\
        (ape),\
        (function_name),\
        sizeof((ape_object_t[]){__VA_ARGS__}) / sizeof(ape_object_t),\
        (ape_object_t[]){__VA_ARGS__})

void ape_set_runtime_error(ape_t *ape, const char *message);
void ape_set_runtime_errorf(ape_t *ape, const char *format, ...) __attribute__ ((format (printf, 2, 3)));
bool ape_has_errors(const ape_t *ape);
int  ape_errors_count(const ape_t *ape);
const ape_error_t* ape_get_error(const ape_t *ape, int index);

bool ape_set_native_function(ape_t *ape, const char *name, ape_native_fn fn, void *data);
bool ape_set_global_constant(ape_t *ape, const char *name, ape_object_t obj);
ape_object_t ape_get_object(ape_t *ape, const char *name);

bool ape_check_args(ape_t *ape, bool generate_error, int argc, ape_object_t *args, int expected_argc, int *expected_types);
#define APE_CHECK_ARGS(ape, generate_error, argc, args, ...)\
    ape_check_args(\
        (ape),\
        (generate_error),\
        (argc),\
        (args),\
        sizeof((int[]){__VA_ARGS__}) / sizeof(int),\
        (int[]){__VA_ARGS__})

//-----------------------------------------------------------------------------
// Ape object
//-----------------------------------------------------------------------------

ape_object_t ape_object_make_number(double val);
ape_object_t ape_object_make_bool(bool val);
ape_object_t ape_object_make_null(void);
ape_object_t ape_object_make_string(ape_t *ape, const char *str);
ape_object_t ape_object_make_stringf(ape_t *ape, const char *format, ...) __attribute__ ((format (printf, 2, 3)));
ape_object_t ape_object_make_array(ape_t *ape);
ape_object_t ape_object_make_map(ape_t *ape);
ape_object_t ape_object_make_native_function(ape_t *ape, ape_native_fn fn, void *data);
ape_object_t ape_object_make_error(ape_t *ape, const char *message);
ape_object_t ape_object_make_errorf(ape_t *ape, const char *format, ...) __attribute__ ((format (printf, 2, 3)));
ape_object_t ape_object_make_external(ape_t *ape, void *data);

char* ape_object_serialize(ape_object_t obj);

void ape_object_disable_gc(ape_object_t obj);
void ape_object_enable_gc(ape_object_t obj);

bool ape_object_equals(ape_object_t a, ape_object_t b);

ape_object_t ape_object_copy(ape_object_t obj);
ape_object_t ape_object_deep_copy(ape_object_t obj);

ape_object_type_t ape_object_get_type(ape_object_t obj);
const char*       ape_object_get_type_string(ape_object_t obj);
const char*       ape_object_get_type_name(ape_object_type_t type);

double       ape_object_get_number(ape_object_t obj);
bool         ape_object_get_bool(ape_object_t obj);
const char * ape_object_get_string(ape_object_t obj);

const char*            ape_object_get_error_message(ape_object_t obj);
const ape_traceback_t* ape_object_get_error_traceback(ape_object_t obj);

bool ape_object_set_external_destroy_function(ape_object_t object, ape_data_destroy_fn destroy_fn);
bool ape_object_set_external_copy_function(ape_object_t object, ape_data_copy_fn copy_fn);

//-----------------------------------------------------------------------------
// Ape object array
//-----------------------------------------------------------------------------

int ape_object_get_array_length(ape_object_t obj);

ape_object_t ape_object_get_array_value(ape_object_t object, int ix);
const char*  ape_object_get_array_string(ape_object_t object, int ix);
double       ape_object_get_array_number(ape_object_t object, int ix);
bool         ape_object_get_array_bool(ape_object_t object, int ix);

bool ape_object_set_array_value(ape_object_t object, int ix, ape_object_t value);
bool ape_object_set_array_string(ape_object_t object, int ix, const char *string);
bool ape_object_set_array_number(ape_object_t object, int ix, double number);
bool ape_object_set_array_bool(ape_object_t object, int ix, bool value);

bool ape_object_add_array_value(ape_object_t object, ape_object_t value);
bool ape_object_add_array_string(ape_object_t object, const char *string);
bool ape_object_add_array_number(ape_object_t object, double number);
bool ape_object_add_array_bool(ape_object_t object, bool value);

//-----------------------------------------------------------------------------
// Ape object map
//-----------------------------------------------------------------------------

int          ape_object_get_map_length(ape_object_t obj);
ape_object_t ape_object_get_map_key_at(ape_object_t object, int ix);
ape_object_t ape_object_get_map_value_at(ape_object_t object, int ix);
bool         ape_object_set_map_value_at(ape_object_t object, int ix, ape_object_t val);

bool ape_object_set_map_value_with_value_key(ape_object_t object, ape_object_t key, ape_object_t value);
bool ape_object_set_map_value(ape_object_t object, const char *key, ape_object_t value);
bool ape_object_set_map_string(ape_object_t object, const char *key, const char *string);
bool ape_object_set_map_number(ape_object_t object, const char *key, double number);
bool ape_object_set_map_bool(ape_object_t object, const char *key, bool value);

ape_object_t  ape_object_get_map_value_with_value_key(ape_object_t object, ape_object_t key);
ape_object_t  ape_object_get_map_value(ape_object_t object, const char *key);
const char*   ape_object_get_map_string(ape_object_t object, const char *key);
double        ape_object_get_map_number(ape_object_t object, const char *key);
bool          ape_object_get_map_bool(ape_object_t object, const char *key);

bool ape_object_map_has_key(ape_object_t object, const char *key);

//-----------------------------------------------------------------------------
// Ape error
//-----------------------------------------------------------------------------
const char*      ape_error_get_message(const ape_error_t *error);
const char*      ape_error_get_filepath(const ape_error_t *error);
const char*      ape_error_get_line(const ape_error_t *error);
int              ape_error_get_line_number(const ape_error_t *error);
int              ape_error_get_column_number(const ape_error_t *error);
ape_error_type_t ape_error_get_type(const ape_error_t *error);
const char*      ape_error_get_type_string(const ape_error_t *error);
const char*      ape_error_type_to_string(ape_error_type_t type);
char*            ape_error_serialize(const ape_error_t *error);
const ape_traceback_t* ape_error_get_traceback(const ape_error_t *error);

//-----------------------------------------------------------------------------
// Ape traceback
//-----------------------------------------------------------------------------
int         ape_traceback_get_depth(const ape_traceback_t *traceback);
const char* ape_traceback_get_filepath(const ape_traceback_t *traceback, int depth);
const char* ape_traceback_get_line(const ape_traceback_t *traceback, int depth);
int         ape_traceback_get_line_number(const ape_traceback_t *traceback, int depth);
int         ape_traceback_get_column_number(const ape_traceback_t *traceback, int depth);
const char* ape_traceback_get_function_name(const ape_traceback_t *traceback, int depth);

#ifdef __cplusplus
}
#endif

#endif /* ape_h */
