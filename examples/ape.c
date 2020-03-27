#include <stdio.h>
#include <stdlib.h>

#include "ape.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s FILE\n", argv[0]);
        return 1;
    }
    ape_t *ape = ape_make();
    ape_execute_file(ape, argv[1]);
    bool ok = !ape_has_errors(ape);
    if (!ok) {
        int count = ape_errors_count(ape);
        for (int i = 0; i < count; i++) {
            const ape_error_t *err = ape_get_error(ape, i);
            char *err_str = ape_error_serialize(err);
            puts(err_str);
            free(err_str);
        }
    }
    ape_destroy(ape);
    return ok;
}