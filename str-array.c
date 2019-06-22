#include "str-array.h"

#include <stdlib.h>

str_array *create_str_array(void)
{
    return calloc(1, sizeof (str_array));
}

void destroy_str_array(str_array *array)
{
    free(array->strings);
    free(array->lengths);
    free(array);
}

void str_array_append(str_array *array, const char *string, size_t size)
{
    if (array->count >= array->alloc) {
        size_t n = array->alloc * 2 + 4;
        array->alloc = n;
        array->strings = realloc(array->strings, n * sizeof *array->strings);
        array->lengths = realloc(array->lengths, n * sizeof *array->lengths);
    }
    array->strings[array->count] = string;
    array->lengths[array->count] = size;
    array->count++;
}

void str_array_extend(str_array *dest, const str_array *src)
{
    for (size_t i = 0; i < src->count; i++) {
        str_array_append(dest, src->strings[i], src->lengths[i]);
    }
}

void print_str_array(const str_array * array, FILE *f)
{
    for (size_t i = 0; i < array->count; i++) {
        if (array->lengths[i] >= 0) {
            fwrite(array->strings[i], array->lengths[i], 1, f);
        } else {
            fputs(array->strings[i], f);
        }
    }
}

bool str_array_first_line(const str_array  *arr,
                          str_array_cursor *start,
                          str_array_cursor *end)
{
    end->array  = arr;
    end->chunk  = 0;
    end->offset = 0;
    end->ptr    = arr->strings[0];
    return str_array_next_line(start, end);
}

bool str_array_next_line(str_array_cursor *start, str_array_cursor *end)
{
    const str_array *arr    = end->array;
    size_t           chunk  = end->chunk;
    size_t           offset = end->offset;
    const char      *str    = arr->strings[chunk];
    size_t           len    = arr->lengths[chunk];
    if (len < 0 ? !str[offset] : offset >= len) {
        chunk++;
        if (chunk >= arr->count) {
            return false;
        }
        str = arr->strings[chunk];
        len = arr->lengths[chunk];
    }
    while (len < 0 ? str[offset] : offset < len) {
        if (str[offset] == '\n') {
            break;
        }
        str++;
        offset++;
    }

    *start      = *end;
    end->chunk  = chunk;
    end->offset = offset;
    end->ptr    = arr->strings[chunk] + offset;
    return true;
}

