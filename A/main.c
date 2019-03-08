/* 
 * Автор: Васюков Алексей, АПО-12
 * 
 * Функционал: удаление лишних пробелов 
 * в введенном тексте.
 * 
 * Copyright © 2019. All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 128
#define ERR_MESSAGE "[error]"

char *get_string(FILE *stream);
char **get_text(size_t *number, FILE *stream);

void remove_extra_spaces_from_string(char *string);
void remove_extra_spaces_from_text(char **str_array, const size_t number);

char **reallocate_text(char **str_array, const size_t old_size,
                                         const size_t new_size);
void free_text(char **str_array, const size_t number);

void print_text(const char *arr[], const size_t number, FILE *stream);
void print_error(const char *message, FILE *stream);

int main(void) {
    size_t str_number = 0;
    char **str_array = NULL;

    str_array = get_text(&str_number, stdin);
    remove_extra_spaces_from_text(str_array, str_number);

    if (str_array || !str_number) {
        print_text((const char **)str_array, str_number, stdout);
        free_text(str_array, str_number);
    } else {
        print_error(ERR_MESSAGE, stdout);
    }

    return 0;
}

void remove_extra_spaces_from_string(char *string) {
    if (NULL == string) {
        return;
    }

    bool flag = true;

    for (char *proc = string; *proc != '\0'; ++proc) {
        if (' ' != *proc) {
            *string++ = *proc;
            flag = true;
        } else if (flag) {
            *string++ = *proc;
            flag = false;
        }
    }

    *string = '\0';
}

void remove_extra_spaces_from_text(char **str_array, const size_t number) {
    if (NULL == str_array || !number) {
        return;
    }

    for (size_t i = 0; i < number; ++i) {
        remove_extra_spaces_from_string(str_array[i]);
    }
}

char *get_string(FILE *stream) {
    if (NULL == stream) {
        return NULL;
    }

    char *string = NULL;
    char *buff = malloc(SIZE * sizeof(char));

    size_t len = 0;
    size_t buff_len = 0;

    while (fgets(buff, SIZE, stream)) {
        if (NULL == buff) {
            if (string) {
                free(string);
                string = NULL;
            }

            break;
        }

        buff_len = strlen(buff);

        char *read = realloc(string, len + buff_len + 1);
        if (NULL == read) {
            if (string) {
                free(string);
                string = NULL;
            }

            break;
        }

        string = read;

        memcpy(string + len, buff, buff_len + 1);
        len += buff_len;

        if ('\n' == string[len - 1]) {
            string[len - 1] = '\0';
            break;
        }
    }

    if (buff) {
        free(buff);
    }

    return string;
}

char **reallocate_text(char **str_array, const size_t old_size,
                                         const size_t new_size) {
    char **tmp = realloc(str_array, new_size * sizeof(char *));
    if (NULL == tmp) {
        free_text(str_array, old_size);
    }

    str_array = tmp;

    return str_array;
}

char **get_text(size_t *number, FILE *stream) {
    if (NULL == stream) {
        return NULL;
    }

    char **str_array = NULL;
    char *string = NULL;

    size_t size = SIZE;
    bool flag = true;

    while (NULL != (string = get_string(stream))) {
        if (!*number || *number == size) {
            str_array = reallocate_text(str_array, size, 2 * size);
            if (NULL == str_array) {
                flag = false;
                break;
            }

            size *= 2;
        }

        str_array[*number] = string;
        ++*number;
    }

    if (flag && (*number > 0)) {
        str_array = reallocate_text(str_array, size, *number);
    }

    return str_array;
}

void print_text(const char *arr[], const size_t number, FILE *stream) {
    for (size_t i = 0; i < number; ++i) {
        fprintf(stream, "%s\n", arr[i]);
    }
}

void print_error(const char *message, FILE *stream) {
    fprintf(stream, "%s\n", message);
}

void free_text(char **str_array, const size_t number) {
    for (size_t i = 0; i < number; ++i) {
        free(str_array[i]);
    }

    free(str_array);
}
