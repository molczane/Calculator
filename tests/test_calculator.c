#include "calculator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

static void expect_expression(const char *expression, BigIntStatus expected_status,
                              const char *expected_result) {
    char *result = NULL;
    const BigIntStatus status = calculator_evaluate(expression, &result);
    if (status != expected_status || (status == BIGINT_OK
            && (result == NULL || strcmp(result, expected_result) != 0))
            || (status != BIGINT_OK && result != NULL)) {
        fprintf(stderr, "Unexpected result for %.80s: status=%d, result=%.80s\n",
                expression, status, result != NULL ? result : "(null)");
        ++failures;
    }
    free(result);
}

static char *allocate(size_t size) {
    char *buffer = malloc(size);
    if (buffer == NULL) {
        fprintf(stderr, "Cannot allocate test data\n");
        exit(1);
    }
    return buffer;
}

static void test_long_numbers(void) {
    const size_t count = 10000;
    char *nines = allocate(count + 1);
    char *expected = allocate(count + 2);
    char *expression = allocate(count + 8);
    memset(nines, '9', count);
    nines[count] = '\0';

    expected[0] = '1';
    memset(expected + 1, '0', count);
    expected[count + 1] = '\0';
    snprintf(expression, count + 8, "%s + 1", nines);
    expect_expression(expression, BIGINT_OK, expected);

    snprintf(expression, count + 8, "%s - 1", expected);
    expect_expression(expression, BIGINT_OK, nines);

    expected[0] = '8';
    memset(expected + 1, '9', count - 1);
    expected[count] = '1';
    snprintf(expression, count + 8, "%s * 9", nines);
    expect_expression(expression, BIGINT_OK, expected);

    memset(expected, '1', count);
    expected[count] = '\0';
    snprintf(expression, count + 8, "%s / 9", nines);
    expect_expression(expression, BIGINT_OK, expected);

    free(expression);
    free(expected);
    free(nines);
}

int main(void) {
    const struct {
        const char *expression;
        const char *result;
    } cases[] = {
        {"0 + 0", "0"},
        {"999 + 1", "1000"},
        {"1 + 999", "1000"},
        {"12345678901234567890 + 98765432109876543210", "111111111011111111100"},
        {"1000000000000000000000000 - 1", "999999999999999999999999"},
        {"1 - 1000000000000000000000000", "-999999999999999999999999"},
        {"123456789 - 123456789", "0"},
        {"-12 + -8", "-20"},
        {"-12 + 8", "-4"},
        {"12 + -8", "4"},
        {"-8 + 12", "4"},
        {"8 + -12", "-4"},
        {"-12 - -8", "-4"},
        {"-8 - -12", "4"},
        {"-12 - 8", "-20"},
        {"12 - -8", "20"},
        {"-0 - 0", "0"},
        {"0 - -5", "5"},
        {"-5 - 0", "-5"},
        {"0 * -999", "0"},
        {"99 * 99", "9801"},
        {"123456789 * 987654321", "121932631112635269"},
        {"-12 * 8", "-96"},
        {"12 * -8", "-96"},
        {"-12 * -8", "96"},
        {"12345678901234567890 * 10", "123456789012345678900"},
        {"0 / 7", "0"},
        {"7 / 2", "3"},
        {"-7 / 2", "-3"},
        {"7 / -2", "-3"},
        {"-7 / -2", "3"},
        {"-1 / 2", "0"},
        {"12345678901234567890 / 10", "1234567890123456789"},
        {"1000000000000000000000000 / 3", "333333333333333333333333"},
        {"121932631112635269 / 987654321", "123456789"},
        {"999999999999999999999999 / 999999999999999999999999", "1"},
        {"1005 / 5", "201"},
        {"1000001 / 10", "100000"},
        {"123 / 9999999999999999999999", "0"},
        {"\t +00012  +  -00002 \r", "10"},
        {"0000 * -0005", "0"},
        {"00012 / +0003", "4"},
        {"1--2", "3"},
        {"1+-2", "-1"},
        {"1++2", "3"},
        {"-1*-2", "2"},
        {"1-2", "-1"},
        {"-0/-2", "0"}
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        expect_expression(cases[i].expression, BIGINT_OK, cases[i].result);
    }
    const char *invalid[] = {
        "", " \t", "1", "+", "1 +", "* 2", "1 % 2", "1 ** 2", "--1 + 2",
        "1 + --2", "1 + 2 + 3", "1.5 + 2", "1e9 + 2", "1 2 + 3", "1 + 2abc",
        "1 + (2)", "- 1 + 2", "1 + + 2", "a + 2", "1 / /2"
    };
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        expect_expression(invalid[i], BIGINT_INVALID_INPUT, NULL);
    }
    expect_expression("1 / 0", BIGINT_DIVISION_BY_ZERO, NULL);
    expect_expression("0 / -000", BIGINT_DIVISION_BY_ZERO, NULL);

    char *result = NULL;
    if (bigint_calculate("12x", '+', "2", &result) != BIGINT_INVALID_INPUT || result != NULL
            || bigint_calculate("1", '+', "-", &result) != BIGINT_INVALID_INPUT || result != NULL
            || bigint_calculate("1", '%', "2", &result) != BIGINT_INVALID_INPUT || result != NULL) {
        fprintf(stderr, "Big integer input validation failed\n");
        free(result);
        ++failures;
    }
    test_long_numbers();
    if (failures != 0) {
        fprintf(stderr, "%d arithmetic checks failed\n", failures);
        return 1;
    }
    puts("Arithmetic checks passed, including 10,000-digit operands");
    return 0;
}
