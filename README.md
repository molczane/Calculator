# Calculator

A simple C calculator for very long numbers stored as decimal characters. It
supports `+`, `-`, `*`, and `/` using the same digit-by-digit methods taught at
school: carrying, borrowing, long multiplication, and long division. Operands
are never converted to built-in integer or floating-point types, and no big
number library is used. Small integer operations are used only for individual
digits, carries, and buffer indexes.

Numbers and input lines use dynamically allocated buffers with no fixed digit
limit. Their size is limited by available memory and computation time; using
characters avoids numeric overflow, but does not remove the need for safe
allocation, bounds checks, and cleanup.

## Input and output

The initial version operates on signed whole numbers. Division discards the
remainder and truncates toward zero (`7 / 2` gives `3`, `-7 / 2` gives `-3`).
Decimal fractions, exponent notation, parentheses, and chained expressions are
not supported.

Pass an input text file and an output text file:

```sh
./build/Calculator input.txt results.txt
```

Write one `number operator number` expression per line. Spaces around the
operator are optional. Leading zeros and a leading `+` or `-` on each number
are accepted. Blank lines are skipped. LF, CRLF, and a final line without a
newline are accepted.

Example `input.txt`:

```text
999999999999999999999999999999 + 1
1000000000000000000000000000000 - 1
-12345678901234567890 * 9
12345678901234567890 / 10
```

Corresponding `results.txt`:

```text
1000000000000000000000000000000
999999999999999999999999999999
-111111110111111111010
1234567890123456789
```

Each nonblank input line produces one output line. Results omit leading zeros;
zero is always `0`. An invalid expression produces `ERROR: invalid expression`,
and division by zero produces `ERROR: division by zero`. These errors do not
prevent subsequent lines from being processed, but make the program exit with
status `1`. Allocation and file errors are reported on standard error and stop
processing. Usage errors exit with status `2`; a successful run exits with `0`.

The output file is created or overwritten. The input and output must refer to
different files, including through symbolic or hard links. A failed run can
leave a partial output file. Input is plain text; embedded NUL bytes are
rejected as invalid expressions.

## Build and test

Use CMake 3.21 or newer and a compiler supporting C23 (including the C2x draft
mode used by AppleClang). Use CLion's CMake `Calculator` configuration, or run:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/Calculator examples/input.txt build/results.txt
```

The implementation uses standard C for parsing and arithmetic. The file driver
uses POSIX `fstat` and `fileno` to prevent truncating the input when both paths
refer to the same file. This targets macOS and other POSIX systems.

Addition and subtraction take linear time in the number of digits. School
multiplication and division can take quadratic time, so very long calculations
can be slow even when enough memory is available.
