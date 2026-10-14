#include "bigint.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *digits;
    size_t length;
    int negative;
} Number;

static int parse_number(const char *text, Number *number) {
    number->negative = 0;
    if (*text == '-' || *text == '+') {
        number->negative = *text == '-';
        ++text;
    }
    if (*text == '\0') {
        return 0;
    }
    for (const char *digit = text; *digit != '\0'; ++digit) {
        if (*digit < '0' || *digit > '9') {
            return 0;
        }
    }
    while (text[0] == '0' && text[1] != '\0') {
        ++text;
    }
    number->digits = text;
    number->length = strlen(text);
    if (number->length == 1 && text[0] == '0') {
        number->negative = 0;
    }
    return 1;
}

static char *allocate_digits(size_t length) {
    if (length == SIZE_MAX) {
        return NULL;
    }
    return malloc(length + 1);
}

static char *copy_digits(const char *digits, size_t length) {
    char *copy = allocate_digits(length);
    if (copy != NULL) {
        memcpy(copy, digits, length);
        copy[length] = '\0';
    }
    return copy;
}

static size_t trim_zeroes(char *digits, size_t length) {
    size_t first = 0;
    while (first + 1 < length && digits[first] == '0') {
        ++first;
    }
    if (first != 0) {
        memmove(digits, digits + first, length - first + 1);
    }
    return length - first;
}

static int compare(const char *left, size_t left_length,
                   const char *right, size_t right_length) {
    if (left_length != right_length) {
        return left_length > right_length ? 1 : -1;
    }
    const int order = memcmp(left, right, left_length);
    return (order > 0) - (order < 0);
}

static char *add(const Number *left, const Number *right) {
    const size_t length = left->length > right->length ? left->length : right->length;
    if (length > SIZE_MAX - 2) {
        return NULL;
    }
    char *result = allocate_digits(length + 1);
    if (result == NULL) {
        return NULL;
    }
    size_t i = left->length;
    size_t j = right->length;
    int carry = 0;
    result[length + 1] = '\0';
    for (size_t position = length; position > 0; --position) {
        int sum = carry;
        if (i > 0) {
            sum += left->digits[--i] - '0';
        }
        if (j > 0) {
            sum += right->digits[--j] - '0';
        }
        result[position] = (char)('0' + sum % 10);
        carry = sum / 10;
    }
    result[0] = (char)('0' + carry);
    trim_zeroes(result, length + 1);
    return result;
}

/* left >= right; output may be the same buffer as left. */
static size_t subtract_into(const char *left, size_t left_length,
                            const char *right, size_t right_length, char *output) {
    size_t j = right_length;
    int borrow = 0;
    for (size_t i = left_length; i > 0; --i) {
        int digit = left[i - 1] - '0' - borrow;
        if (j > 0) {
            digit -= right[--j] - '0';
        }
        borrow = digit < 0;
        if (borrow != 0) {
            digit += 10;
        }
        output[i - 1] = (char)('0' + digit);
    }
    output[left_length] = '\0';
    return trim_zeroes(output, left_length);
}

static char *subtract(const Number *left, const Number *right) {
    char *result = allocate_digits(left->length);
    if (result != NULL) {
        subtract_into(left->digits, left->length, right->digits, right->length, result);
    }
    return result;
}

static char *multiply(const Number *left, const Number *right) {
    if (left->digits[0] == '0' || right->digits[0] == '0') {
        return copy_digits("0", 1);
    }
    if (left->length >= SIZE_MAX - right->length) {
        return NULL;
    }
    const size_t length = left->length + right->length;
    char *result = allocate_digits(length);
    if (result == NULL) {
        return NULL;
    }
    memset(result, '0', length);
    result[length] = '\0';
    for (size_t i = left->length; i > 0; --i) {
        int carry = 0;
        for (size_t j = right->length; j > 0; --j) {
            const size_t position = i + j - 1;
            const int product = (left->digits[i - 1] - '0') * (right->digits[j - 1] - '0')
                                + (result[position] - '0') + carry;
            result[position] = (char)('0' + product % 10);
            carry = product / 10;
        }
        result[i - 1] = (char)('0' + carry);
    }
    trim_zeroes(result, length);
    return result;
}

static char *divide(const Number *left, const Number *right) {
    if (compare(left->digits, left->length, right->digits, right->length) < 0) {
        return copy_digits("0", 1);
    }
    if (right->length > SIZE_MAX - 2) {
        return NULL;
    }
    char *quotient = allocate_digits(left->length);
    char *remainder = allocate_digits(right->length + 1);
    if (quotient == NULL || remainder == NULL) {
        free(quotient);
        free(remainder);
        return NULL;
    }
    size_t remainder_length = 1;
    remainder[0] = '0';
    remainder[1] = '\0';
    for (size_t i = 0; i < left->length; ++i) {
        if (remainder_length == 1 && remainder[0] == '0') {
            remainder[0] = left->digits[i];
        } else {
            remainder[remainder_length++] = left->digits[i];
            remainder[remainder_length] = '\0';
        }
        int digit = 0;
        /* Bringing down one digit requires at most nine subtractions. */
        while (compare(remainder, remainder_length, right->digits, right->length) >= 0) {
            remainder_length = subtract_into(remainder, remainder_length,
                                             right->digits, right->length, remainder);
            ++digit;
        }
        quotient[i] = (char)('0' + digit);
    }
    quotient[left->length] = '\0';
    trim_zeroes(quotient, left->length);
    free(remainder);
    return quotient;
}

static char *apply_sign(char *digits, int negative) {
    if (digits == NULL || negative == 0 || digits[0] == '0') {
        return digits;
    }
    const size_t length = strlen(digits);
    char *signed_digits = length <= SIZE_MAX - 2 ? allocate_digits(length + 1) : NULL;
    if (signed_digits != NULL) {
        signed_digits[0] = '-';
        memcpy(signed_digits + 1, digits, length + 1);
    }
    free(digits);
    return signed_digits;
}

BigIntStatus bigint_calculate(const char *left, char operation,
                             const char *right, char **result) {
    *result = NULL;
    Number a;
    Number b;
    if (!parse_number(left, &a) || !parse_number(right, &b)) {
        return BIGINT_INVALID_INPUT;
    }
    char *digits;
    int negative;
    switch (operation) {
        case '-':
        case '+': {
            if (operation == '-' && b.digits[0] != '0') {
                b.negative = !b.negative;
            }
            if (a.negative == b.negative) {
                digits = add(&a, &b);
                negative = a.negative;
            } else if (compare(a.digits, a.length, b.digits, b.length) >= 0) {
                digits = subtract(&a, &b);
                negative = a.negative;
            } else {
                digits = subtract(&b, &a);
                negative = b.negative;
            }
            break;
        }
        case '*':
            digits = multiply(&a, &b);
            negative = a.negative != b.negative;
            break;
        case '/':
            if (b.digits[0] == '0') {
                return BIGINT_DIVISION_BY_ZERO;
            }
            digits = divide(&a, &b);
            negative = a.negative != b.negative;
            break;
        default:
            return BIGINT_INVALID_INPUT;
    }
    *result = apply_sign(digits, negative);
    return *result != NULL ? BIGINT_OK : BIGINT_NO_MEMORY;
}
