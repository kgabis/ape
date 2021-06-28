#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
#endif

#include "test_code.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "code.h"
#include "common.h"

static void test_code_make(void);
static void test_instr_strings(void);
static void test_read_operands(void);

void code_test() {
    puts("### Code test");
    test_code_make();
    test_read_operands();
    test_instr_strings();
    puts("\tOK");
}

static void test_code_make() {
    struct {
        opcode_t op;
        int operands_count;
        uint64_t operands[2];
        int expected_len;
        uint8_t expected[16];
    } tests[] = {
        {OPCODE_CONSTANT, 1, {0xfffe}, 3, {OPCODE_CONSTANT, 0xff, 0xfe}},
        {OPCODE_ADD, 0, {0}, 1, {OPCODE_ADD}},
        {OPCODE_GET_LOCAL, 1, {0xff}, 2, {OPCODE_GET_LOCAL, 0xff}},
        {OPCODE_FUNCTION, 2, {0xfffe, 0xff}, 4, {OPCODE_FUNCTION, 0xff, 0xfe, 0xff}},
        {OPCODE_NUMBER, 1, {0x89abcdef}, 9, {OPCODE_NUMBER, 0x0, 0x0, 0x0, 0x0, 0x89, 0xab, 0xcd, 0xef}},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        array(uint8_t) *instruction = array_make(NULL, uint8_t);
        bool ok = code_make(test.op, test.operands_count, test.operands, instruction);
        assert(ok);
        assert(array_count(instruction) == test.expected_len);
        for (int j = 0; j < array_count(instruction); j++) {
            uint8_t *b = array_get(instruction, j);
            uint8_t tb = test.expected[j];
            if (*b != tb) {
                printf("Wrong byte at %d: got 0x%x, expected 0x%x\n", j, *b, tb);
                assert(false);
            }
        }
        array_destroy(instruction);
    }
}

static void test_instr_strings() {
    array(uint8_t) *code = array_make(NULL, uint8_t);
    code_make(OPCODE_ADD, 0, (uint64_t[]){0}, code);
    code_make(OPCODE_GET_LOCAL, 1, (uint64_t[]){0x1}, code);
    code_make(OPCODE_CONSTANT, 1, (uint64_t[]){0x2}, code);
    code_make(OPCODE_CONSTANT, 1, (uint64_t[]){0xffff}, code);
    code_make(OPCODE_FUNCTION, 2, (uint64_t[]){0xffff, 0xff}, code);
    code_make(OPCODE_NUMBER, 1, (uint64_t[]){0x3ff3333333333333}, code);

    const char *expected = "\
0000 ADD\n\
0001 GET_LOCAL 1\n\
0003 CONSTANT 2\n\
0006 CONSTANT 65535\n\
0009 FUNCTION 65535 255\n\
0013 NUMBER 1.2\n\
";

    strbuf_t *buf = strbuf_make(NULL);
    code_to_string(array_data(code), NULL, array_count(code), buf);
    const char *serialized = strbuf_get_string(buf);
    assert(APE_STREQ(serialized, expected));
    strbuf_destroy(buf);

    array_destroy(code);
}

static void test_read_operands() {
    struct {
        opcode_t op;
        int operands_count;
        uint64_t operands[2];
    } tests[] = {
        {OPCODE_CONSTANT, 1, {0xfffe}},
        {OPCODE_GET_LOCAL, 1, {0xff}},
        {OPCODE_FUNCTION, 2, {0xfffe, 0xff}},
    };

    for (int i = 0; i < APE_ARRAY_LEN(tests); i++) {
        typeof(tests[0]) test = tests[i];
        array(uint8_t) *instruction = array_make(NULL, uint8_t);
        bool ok = code_make(test.op, test.operands_count, test.operands, instruction);
        assert(ok);
        opcode_definition_t *def = opcode_lookup(test.op);
        assert(def);
        uint8_t *instruction_bytes = array_data(instruction);
        int operands_count = 0;
        uint64_t operands[2];
        ok = code_read_operands(def, instruction_bytes + 1, operands);
        assert(def->num_operands == test.operands_count);
        for (int j = 0; j < operands_count; j++) {
            uint64_t op = operands[j];
            uint64_t test_op = test.operands[j];
            assert(op == test_op);
        }
    }
}

#pragma GCC diagnostic pop
