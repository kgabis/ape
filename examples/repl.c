#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ape.h"

#define PROMPT ">> "

static void print_errors(ape_t *ape, const char *line);
static ape_object_t exit_repl(ape_t *ape, void *data, int argc, ape_object_t *args);

#ifdef APE_REPL_MAIN
int main(int argc, char *argv[]) {
#else
void repl_start() {
#endif
#if 0
} // unconfuse xcode
#endif
    bool exit = false;

    ape_t *ape = ape_make();

    ape_set_repl_mode(ape, true);
    ape_set_timeout(ape, 100.0);

    ape_set_native_function(ape, "exit", exit_repl, &exit);

    while (!exit) {
        printf("%s", PROMPT);
        char *line = NULL;
        size_t len = 0;
        getline(&line, &len, stdin);
        if (!line) {
            continue;
        }
        line[strlen(line) - 1] = '\0'; // removes last \n

        ape_object_t res = ape_execute(ape, line);
        if (ape_has_errors(ape)) {
            print_errors(ape, line);
            free(line);
            continue;
        }

        char *object_str = ape_object_serialize(ape, res);
        puts(object_str);
        free(object_str);
    }

    ape_destroy(ape);
}

// INTERNAL
static void print_errors(ape_t *ape, const char *line) {
    int count = ape_errors_count(ape);
    for (int i = 0; i < count; i++) {
        const ape_error_t *err = ape_get_error(ape, i);
        char *err_str = ape_error_serialize(ape, err);
        puts(err_str);
        free(err_str);
    }
}

static ape_object_t exit_repl(ape_t *ape, void *data, int argc, ape_object_t *args) {
    bool *exit_repl = (bool*)data;
    *exit_repl = true;
    return ape_object_make_null();
}
