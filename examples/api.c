#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ape.h"

static ape_object_t external_add(ape_t *ape, void *data, int argc, ape_object_t *args);

int main() {
    ape_t *ape = ape_make();

    // Calling Ape functions from C code
    ape_execute(ape, "fn add(a, b) { return a + b }");    
    ape_object_t res = APE_CALL(ape, "add", ape_object_make_number(42), ape_object_make_number(42));
    assert(ape_object_get_number(res) == 84);

    // Calling C functions from Ape code
    ape_set_native_function(ape, "external_add", external_add, NULL);
    ape_execute(ape, "assert(external_add(42, 42) == 84)");
    assert(!ape_has_errors(ape));

    // Handling errors
    ape_execute(ape, "external_add()");
    assert(ape_has_errors(ape));
    for (int i = 0; i < ape_errors_count(ape); i++) {
        const ape_error_t *err = ape_get_error(ape, i);
        char *err_string = ape_error_serialize(ape, err);
        puts(err_string);
        ape_free_allocated(ape, err_string);
    }

    ape_destroy(ape);
    return 0;
}

static ape_object_t external_add(ape_t *ape, void *data, int argc, ape_object_t *args) {
    // Generating erros if number of arguments is invalid or if they're of invalid type
    if (!APE_CHECK_ARGS(ape, true, argc, args, APE_OBJECT_NUMBER, APE_OBJECT_NUMBER)) {
        return ape_object_make_null();
    }

    double a = ape_object_get_number(args[0]);
    double b = ape_object_get_number(args[1]);

    return ape_object_make_number(a + b);
}