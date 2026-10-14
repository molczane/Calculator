#include "calculator.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void skip_space(char **cursor) {
    while (isspace((unsigned char)**cursor)) {
        ++*cursor;
    }
}

static int read_number(char **cursor, char **start, char **end) {
    skip_space(cursor);
    *start = *cursor;
    if (**cursor == '+' || **cursor == '-') {
        ++*cursor;
    }
    char *digits = *cursor;
    while (**cursor >= '0' && **cursor <= '9') {
        ++*cursor;
    }
    *end = *cursor;
    return *cursor != digits;
}

BigIntStatus calculator_evaluate(const char *expression, char **result) {
    *result = NULL;
    const size_t length = strlen(expression);
    if (length == SIZE_MAX) {
        return BIGINT_NO_MEMORY;
    }
    char *copy = malloc(length + 1);
    if (copy == NULL) {
        return BIGINT_NO_MEMORY;
    }
    memcpy(copy, expression, length + 1);
    char *cursor = copy;
    char *left;
    char *left_end;
    char *right;
    char *right_end;
    BigIntStatus status = BIGINT_INVALID_INPUT;
    if (!read_number(&cursor, &left, &left_end)) {
        goto cleanup;
    }
    skip_space(&cursor);
    const char operation = *cursor;
    if (operation != '+' && operation != '-' && operation != '*' && operation != '/') {
        goto cleanup;
    }
    ++cursor;
    if (!read_number(&cursor, &right, &right_end)) {
        goto cleanup;
    }
    skip_space(&cursor);
    if (*cursor != '\0') {
        goto cleanup;
    }
    *left_end = '\0';
    *right_end = '\0';
    status = bigint_calculate(left, operation, right, result);

cleanup:
    free(copy);
    return status;
}

const char *calculator_status_message(BigIntStatus status) {
    switch (status) {
        case BIGINT_OK:
            return "success";
        case BIGINT_INVALID_INPUT:
            return "invalid expression";
        case BIGINT_DIVISION_BY_ZERO:
            return "division by zero";
        case BIGINT_NO_MEMORY:
            return "not enough memory";
    }
    return "unknown error";
}
