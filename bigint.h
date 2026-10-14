#ifndef CALCULATOR_BIGINT_H
#define CALCULATOR_BIGINT_H

typedef enum {
    BIGINT_OK,
    BIGINT_INVALID_INPUT,
    BIGINT_DIVISION_BY_ZERO,
    BIGINT_NO_MEMORY
} BigIntStatus;

/* On success, the caller owns *result and must free it; otherwise it is NULL. */
BigIntStatus bigint_calculate(const char *left, char operation,
                             const char *right, char **result);

#endif
