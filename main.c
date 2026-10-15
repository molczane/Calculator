#define _POSIX_C_SOURCE 200809L

#include "calculator.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum
{
    READ_OK,
    READ_END,
    READ_ERROR,
    READ_NO_MEMORY
} ReadStatus;

static int reserve_line(char** line, size_t* capacity, size_t needed)
{
    if (needed <= *capacity)
    {
        return 1;
    }
    size_t new_capacity = *capacity == 0 ? 256 : *capacity;
    while (new_capacity < needed)
    {
        if (new_capacity > SIZE_MAX / 2)
        {
            new_capacity = needed;
            break;
        }
        new_capacity *= 2;
    }
    char* resized = realloc(*line, new_capacity);
    if (resized == NULL)
    {
        return 0;
    }
    *line = resized;
    *capacity = new_capacity;
    return 1;
}

static ReadStatus read_line(FILE* input, char** line, size_t* capacity, int* has_nul)
{
    size_t length = 0;
    *has_nul = 0;
    int character;
    while ((character = fgetc(input)) != EOF && character != '\n')
    {
        if (length > SIZE_MAX - 2 || !reserve_line(line, capacity, length + 2))
        {
            return READ_NO_MEMORY;
        }
        if (character == '\0')
        {
            *has_nul = 1;
        }
        (*line)[length++] = (char)character;
    }
    if (ferror(input))
    {
        return READ_ERROR;
    }
    if (character == EOF && length == 0)
    {
        return READ_END;
    }
    if (!reserve_line(line, capacity, length + 1))
    {
        return READ_NO_MEMORY;
    }
    (*line)[length] = '\0';
    return READ_OK;
}

static int is_blank(const char* line)
{
    while (*line != '\0')
    {
        if (!isspace((unsigned char)*line))
        {
            return 0;
        }
        ++line;
    }
    return 1;
}

static FILE* open_output(FILE* file_input, const char* file_path)
{
    /* Compare open descriptors before truncation, including linked paths. */
    const int descriptor = open(file_path, O_WRONLY | O_CREAT, 0666);
    if (descriptor < 0)
    {
        perror("Cannot open output file");
        return NULL;
    }
    struct stat input_info;
    struct stat output_info;
    if (fstat(fileno(file_input), &input_info) != 0 || fstat(descriptor, &output_info) != 0)
    {
        perror("Cannot inspect input/output files");
        close(descriptor);
        return NULL;
    }
    if (input_info.st_dev == output_info.st_dev && input_info.st_ino == output_info.st_ino)
    {
        fprintf(stderr, "Input and output must be different files\n");
        close(descriptor);
        return NULL;
    }
    if (ftruncate(descriptor, 0) != 0)
    {
        perror("Cannot truncate output file");
        close(descriptor);
        return NULL;
    }
    FILE* output = fdopen(descriptor, "w");
    if (output == NULL)
    {
        perror("Cannot prepare output file");
        close(descriptor);
    }
    return output;
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input.txt> <results.txt>\n", argv[0]);
        return 2;
    }
    FILE* input = fopen(argv[1], "rb");
    if (input == NULL)
    {
        perror("Cannot open input file");
        return 1;
    }

    FILE* output = open_output(input, argv[2]);
    if (output == NULL)
    {
        fclose(input);
        return 1;
    }

    char* line = NULL;
    size_t capacity = 0;
    size_t line_number = 0;
    int exit_status = 0;
    for (;;)
    {
        int has_nul;
        const ReadStatus read_status = read_line(input, &line, &capacity, &has_nul);
        if (read_status == READ_END)
        {
            break;
        }
        if (read_status == READ_ERROR || read_status == READ_NO_MEMORY)
        {
            fprintf(stderr, "%s\n", read_status == READ_ERROR
                                        ? "Cannot read input file"
                                        : "Not enough memory for input line");
            exit_status = 1;
            break;
        }
        ++line_number;
        if (!has_nul && is_blank(line))
        {
            continue;
        }
        char* result = NULL;
        const BigIntStatus status = has_nul
                                        ? BIGINT_INVALID_INPUT
                                        : calculator_evaluate(line, &result);
        if (status == BIGINT_NO_MEMORY)
        {
            fprintf(stderr, "Line %zu: %s\n", line_number, calculator_status_message(status));
            exit_status = 1;
            break;
        }
        int written;
        if (status == BIGINT_OK)
        {
            written = fprintf(output, "%s\n", result);
        }
        else
        {
            written = fprintf(output, "ERROR: %s\n", calculator_status_message(status));
            fprintf(stderr, "Line %zu: %s\n", line_number, calculator_status_message(status));
            exit_status = 1;
        }
        free(result);
        if (written < 0)
        {
            perror("Cannot write output file");
            exit_status = 1;
            break;
        }
    }
    free(line);
    if (fclose(input) != 0)
    {
        perror("Cannot close input file");
        exit_status = 1;
    }
    if (fclose(output) != 0)
    {
        perror("Cannot finish output file");
        exit_status = 1;
    }
    return exit_status;
}
