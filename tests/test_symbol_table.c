#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"

#include "test_symbol_table.h"

#include <assert.h>
#include <stdio.h>

#include "symbol_table.h"
#include "common.h"

void test_define(symbol_table_t *table, const char *name, symbol_type_t type, int index);
void test_defines(void);
void test_resolve_global(void);
void test_resolve_local(void);
void test_resolve_nested_local(void);
void test_resolve_native_functions(void);
void test_resolve_free(void);
void test_resolve_unresolvable_free(void);
void test_define_and_resolve_function_name(void);
void test_shadowing_function_name(void);

void symbol_table_test() {
    puts("### Symbol table test");
    test_defines();
    test_resolve_global();
    test_resolve_local();
    test_resolve_nested_local();
    test_resolve_native_functions();
    test_resolve_free();
    test_resolve_unresolvable_free();
    test_define_and_resolve_function_name();
    test_shadowing_function_name();
}

void test_define(symbol_table_t *table, const char *name, symbol_type_t type, int index) {
    symbol_t *s = symbol_table_define(table, name, true);
    assert(s);
    assert(APE_STREQ(s->name, name));
    assert(s->type == type);
    assert(s->index == index);
}

void test_defines() {
    symbol_table_t *global = symbol_table_make(NULL);
    symbol_table_t *first_local = symbol_table_make(global);
    symbol_table_t *second_local = symbol_table_make(first_local);

    test_define(global, "a", SYMBOL_GLOBAL, 0);
    test_define(global, "b", SYMBOL_GLOBAL, 1);

    test_define(first_local, "c", SYMBOL_LOCAL, 0);
    test_define(first_local, "d", SYMBOL_LOCAL, 1);

    test_define(second_local, "e", SYMBOL_LOCAL, 0);
    test_define(second_local, "f", SYMBOL_LOCAL, 1);
}

void test_resolve_global() {
    symbol_table_t *table = symbol_table_make(NULL);
    symbol_table_define(table, "a", true);
    symbol_table_define(table, "b", true);

    symbol_t *expected[] = {
        symbol_make("a", SYMBOL_GLOBAL, 0, true),
        symbol_make("b", SYMBOL_GLOBAL, 1, true),
    };

    for (int i = 0; i < APE_ARRAY_LEN(expected); i++) {
        symbol_t *sym = expected[i];
        symbol_t *res = symbol_table_resolve(table, sym->name);
        assert(res);
        assert(APE_STREQ(sym->name, res->name));
        assert(sym->type == res->type);
        assert(sym->index == res->index);
    }
}

void test_resolve_local() {
    symbol_table_t *global = symbol_table_make(NULL);
    symbol_table_define(global, "a", true);
    symbol_table_define(global, "b", true);

    symbol_table_t *local = symbol_table_make(global);
    symbol_table_define(local, "c", true);
    symbol_table_define(local, "d", true);

    symbol_t *expected[] = {
        symbol_make("a", SYMBOL_GLOBAL, 0, true),
        symbol_make("b", SYMBOL_GLOBAL, 1, true),
        symbol_make("c", SYMBOL_LOCAL, 0, true),
        symbol_make("d", SYMBOL_LOCAL, 1, true),
    };

    for (int i = 0; i < APE_ARRAY_LEN(expected); i++) {
        symbol_t *sym = expected[i];
        symbol_t *res = symbol_table_resolve(local, sym->name);
        assert(res);
        assert(APE_STREQ(sym->name, res->name));
        assert(sym->type == res->type);
        assert(sym->index == res->index);
    }
}

void test_resolve_nested_local() {
    symbol_table_t *global = symbol_table_make(NULL);
    symbol_table_define(global, "a", true);
    symbol_table_define(global, "b", true);

    symbol_table_t *first_local = symbol_table_make(global);
    symbol_table_define(first_local, "c", true);
    symbol_table_define(first_local, "d", true);

    symbol_table_t *second_local = symbol_table_make(first_local);
    symbol_table_define(second_local, "e", true);
    symbol_table_define(second_local, "f", true);

    struct {
        symbol_table_t *table;
        int symbols_count;
        symbol_t *symbols[256];
    } tests[] = {
        {
            first_local,
            4,
            {
                symbol_make("a", SYMBOL_GLOBAL, 0, true),
                symbol_make("b", SYMBOL_GLOBAL, 1, true),
                symbol_make("c", SYMBOL_LOCAL, 0, true),
                symbol_make("d", SYMBOL_LOCAL, 1, true),
            }
        },
        {
            second_local,
            4,
            {
                symbol_make("a", SYMBOL_GLOBAL, 0, true),
                symbol_make("b", SYMBOL_GLOBAL, 1, true),
                symbol_make("e", SYMBOL_LOCAL, 0, true),
                symbol_make("f", SYMBOL_LOCAL, 1, true),
            }
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];

        for (int j = 0; j < test.symbols_count; j++) {
            symbol_t *sym = test.symbols[j];
            symbol_t *res = symbol_table_resolve(test.table, sym->name);
            assert(res);
            assert(APE_STREQ(sym->name, res->name));
            assert(sym->type == res->type);
            assert(sym->index == res->index);
        }
    }
}

void test_resolve_native_functions() {
    symbol_table_t *global = symbol_table_make(NULL);
    symbol_table_t *first_local = symbol_table_make(global);
    symbol_table_t *second_local = symbol_table_make(first_local);

    symbol_table_t *tables[] = {global, first_local, second_local};

    struct {
        const char *name;
        int ix;
    } expected[] = {
        {"a", 0},
        {"c", 1},
        {"e", 2},
        {"f", 3},
    };

    for (int i = 0; i < APE_ARRAY_LEN(expected); i++) {
        typeof(expected[0]) *exp = &expected[i];
        symbol_table_define_native_function(global, exp->name, exp->ix);
    }

    for (int i = 0; i < APE_ARRAY_LEN(tables); i++) {
        symbol_table_t *table = tables[i];
        for (int j = 0; j < APE_ARRAY_LEN(expected); j++) {
            typeof(expected[0]) *exp = &expected[j];
            symbol_t *symbol = symbol_table_resolve(table, exp->name);
            assert(symbol);
            assert(APE_STREQ(symbol->name, exp->name));
            assert(symbol->index == exp->ix);
            assert(symbol->type == SYMBOL_NATIVE_FUNCTION);
        }
    }
}

void test_resolve_free() {
    symbol_table_t *global = symbol_table_make(NULL);
    symbol_table_define(global, "a", true);
    symbol_table_define(global, "b", true);

    symbol_table_t *first_local = symbol_table_make(global);
    symbol_table_define(first_local, "c", true);
    symbol_table_define(first_local, "d", true);

    symbol_table_t *second_local = symbol_table_make(first_local);
    symbol_table_define(second_local, "e", true);
    symbol_table_define(second_local, "f", true);

    struct {
        symbol_table_t *table;
        int symbols_count;
        symbol_t *symbols[256];
        int free_symbols_count;
        symbol_t *free_symbols[256];
    } tests[] = {
        {
            first_local,
            4,
            {
                symbol_make("a", SYMBOL_GLOBAL, 0, true),
                symbol_make("b", SYMBOL_GLOBAL, 1, true),
                symbol_make("c", SYMBOL_LOCAL, 0, true),
                symbol_make("d", SYMBOL_LOCAL, 1, true),
            },
            0,
            {0},
        },
        {
            second_local,
            4,
            {
                symbol_make("a", SYMBOL_GLOBAL, 0, true),
                symbol_make("b", SYMBOL_GLOBAL, 1, true),
                symbol_make("c", SYMBOL_FREE, 0, true),
                symbol_make("d", SYMBOL_FREE, 1, true),
                symbol_make("e", SYMBOL_LOCAL, 0, true),
                symbol_make("f", SYMBOL_LOCAL, 1, true),
            },
            2,
            {
                symbol_make("c", SYMBOL_LOCAL, 0, true),
                symbol_make("d", SYMBOL_LOCAL, 1, true),
            },
        },
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];

        for (int j = 0; j < test.symbols_count; j++) {
            symbol_t *sym = test.symbols[j];
            symbol_t *res = symbol_table_resolve(test.table, sym->name);
            assert(res);
            assert(APE_STREQ(sym->name, res->name));
            assert(sym->type == res->type);
            assert(sym->index == res->index);
        }

        assert(ptrarray_count(test.table->free_symbols) == test.free_symbols_count);
        
        for (int j = 0; j < test.free_symbols_count; j++) {
            symbol_t *sym = test.free_symbols[j];
            symbol_t *res = ptrarray_get(test.table->free_symbols, j);
            assert(res);
            assert(APE_STREQ(sym->name, res->name));
            assert(sym->type == res->type);
            assert(sym->index == res->index);
        }
    }
}

void test_resolve_unresolvable_free() {
    symbol_table_t *global = symbol_table_make(NULL);
    symbol_table_define(global, "a", true);

    symbol_table_t *first_local = symbol_table_make(global);
    symbol_table_define(first_local, "c", true);

    symbol_table_t *second_local = symbol_table_make(first_local);
    symbol_table_define(second_local, "e", true);
    symbol_table_define(second_local, "f", true);

    symbol_t *expected[] = {
        symbol_make("a", SYMBOL_GLOBAL, 0, true),
        symbol_make("c", SYMBOL_FREE, 0, true),
        symbol_make("e", SYMBOL_LOCAL, 0, true),
        symbol_make("f", SYMBOL_LOCAL, 1, true),
    };

    for (int i = 0; i < APE_ARRAY_LEN(expected); i++) {
        symbol_t *sym = expected[i];
        symbol_t *res = symbol_table_resolve(second_local, sym->name);
        assert(res);
        assert(APE_STREQ(sym->name, res->name));
        assert(sym->type == res->type);
        assert(sym->index == res->index);
    }

    const char *expected_unresolvable[] = {"b", "d"};

    for (int i = 0; i < APE_ARRAY_LEN(expected_unresolvable); i++) {
        const char *name = expected_unresolvable[i];
        symbol_t *res = symbol_table_resolve(global, name);
        assert(res == NULL);
    }
}

void test_define_and_resolve_function_name() {
    symbol_table_t *global = symbol_table_make(NULL);
    symbol_table_define_function_name(global, "a", true);

    symbol_t *symbol = symbol_table_resolve(global, "a");
    assert(symbol);
    assert(APE_STREQ(symbol->name, "a"));
    assert(symbol->type == SYMBOL_FUNCTION);
    assert(symbol->index == 0);
}

void test_shadowing_function_name() {
    symbol_table_t *global = symbol_table_make(NULL);
    symbol_table_define_function_name(global, "a", true);
    symbol_table_define(global, "a", true);

    symbol_t *symbol = symbol_table_resolve(global, "a");
    assert(symbol);
    assert(APE_STREQ(symbol->name, "a"));
    assert(symbol->type == SYMBOL_GLOBAL);
    assert(symbol->index == 0);
}

#pragma GCC diagnostic pop
