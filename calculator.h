#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "bigint.h"

/* On success, the caller owns *result and must free it; otherwise it is NULL. */
BigIntStatus calculator_evaluate(const char *expression, char **result);
const char *calculator_status_message(BigIntStatus status);

#endif
