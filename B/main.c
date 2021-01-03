/* 
 * Автор: Васюков Алексей, АПО-12
 * 
 * Функционал: парсер логических выражений.
 * 
 * Copyright © 2019. All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 128
#define ERR_MESSAGE "[error]"

#define TRUE "True"
#define FALSE "False"

#define AND "and"
#define OR "or"
#define NOT "not"
#define XOR "xor"

#define OPEN_BRACKET '('
#define CLOSE_BRACKET ')'
#define END_OF_DECLARATION ';'
#define EQUAL '='
#define SPACE ' '

#define OR_SIGN '+'
#define XOR_SIGN '^'
#define AND_SIGN '*'
#define NOT_SIGN '!'

#define NUMBER_OF_KEYWORDS 4
#define KEYWORDS { OR, XOR, AND, NOT }

#define OK 0
#define IN_ERROR (-1)
#define EMPTY_LINE (-2)
#define INCORRECT_DECLARATION (-3)
#define MEM_ERROR (-4)

struct var_t {
    char *name;
    bool value;
};

char *get_string(FILE *stream);
char **get_text(size_t *number, FILE *stream);

void remove_extra_spaces_from_string(char *string);
void remove_extra_spaces_from_text(char **str_array, const size_t number);

char **reallocate_text(char **str_array, const size_t old_size,
                                         const size_t new_size);
void free_text(char **str_array, const size_t number);

int logical_parser(FILE *stream);
int logical_calc(struct var_t **list, const size_t number, const char *string);

bool left_assoc(const char op);
bool process(bool *stack, size_t *size, const char op);
int priority(const char op);
char get_sign(const char *string);

int get_var(struct var_t **var, const char *string);
struct var_t **get_list(char **str_array, const size_t str_number,
                        size_t *list_number, int *result);

struct var_t **reallocate_list(struct var_t **list, const size_t old_size,
                                         const size_t new_size);
void free_list(struct var_t **list, const size_t number);

void print(const char *message, FILE *stream);

int main(void) {
    int result = logical_parser(stdin);
    char *message = (1 == result)
                  ? TRUE
                  : (0 == result)
                  ? FALSE
                  : ERR_MESSAGE;

    print(message, stdout);

    return 0;
}

void free_list(struct var_t **list, const size_t number) {
    if (NULL == list) {
        return;
    }

    for (size_t i = 0; i < number; ++i) {
        if (NULL != list[i]) {
            if (NULL != list[i]->name) {
                free(list[i]->name);
            }

            free(list[i]);
        }
    }

    free(list);
}

// <name>=<True|False>; (пробелы допустимы)
int get_var(struct var_t **var, const char *string) {
    if (NULL == string) {
        return EMPTY_LINE;
    }

    char *name = NULL;
    bool value = false;

    size_t len = strlen(string);

    size_t i = 0;
    for ( ; i < len && string[i] >= 'a' && string[i] <= 'z'; ++i) {}
    if (!i || len == i || (SPACE != string[i] && EQUAL != string[i])) {
        return INCORRECT_DECLARATION;
    }

    name = malloc((i + 1) * sizeof(char));
    if (NULL == name) {
        return MEM_ERROR;
    }

    memcpy(name, string, i);
    name[i] = '\0';

    char *keywords[NUMBER_OF_KEYWORDS] = KEYWORDS;
    for (size_t i = 0; i < NUMBER_OF_KEYWORDS; ++i) {
        if (!strcmp(keywords[i], name)) {
            free(name);
            return INCORRECT_DECLARATION;
        }
    }

    for ( ; i < len && SPACE == string[i]; ++i) {}
    if (len == i || EQUAL != string[i]) {
        free(name);
        return INCORRECT_DECLARATION;
    }

    ++i;

    for ( ; i < len && SPACE == string[i]; ++i) {}
    if ((len == i || (TRUE[0] != string[i] && FALSE[0] != string[i]))) {
        free(name);
        return INCORRECT_DECLARATION;
    }

    value = (TRUE[0] == string[i]);
    char *cmp = value ? TRUE : FALSE;
    size_t j = 0;
    for ( ; i < len && j < strlen(cmp); ++i, ++j) {
        if (string[i] != cmp[j]) {
            free(name);
            return INCORRECT_DECLARATION;
        }
    }

    for ( ; i < len && SPACE == string[i]; ++i) {}
    if (i == len || END_OF_DECLARATION != string[i]) {
        free(name);
        return INCORRECT_DECLARATION;
    }

    *var = malloc(sizeof(struct var_t));
    if (NULL == *var) {
        free(name);
        return MEM_ERROR;
    }

    (*var)->name = name;
    (*var)->value = value;

    return OK;
}

struct var_t **reallocate_list(struct var_t **list, const size_t old_size,
                                         const size_t new_size) {
    struct var_t **tmp = realloc(list, new_size * sizeof(struct var_t *));
    if (NULL == tmp) {
        free_list(list, old_size);
    }

    list = tmp;

    return list;
}

struct var_t **get_list(char **str_array, const size_t str_number,
                        size_t *list_number, int *result) {
    struct var_t **list = NULL;
    size_t list_size = SIZE;
    *list_number = 0;

    struct var_t *var = NULL;

    for (size_t i = 0; i < str_number; ++i) {
        *result = i;
        if (END_OF_DECLARATION != str_array[i][strlen(str_array[i]) - 1]) {
            break;
        }

        *result = get_var(&var, str_array[i]);

        if (*result < 0) {
            break;
        }

        if (!*list_number || *list_number == list_size) {
            list = reallocate_list(list, list_size, 2 * list_size);
            if (NULL == list) {
                *result = MEM_ERROR;
                break;
            }

            list_size *= 2;
        }

        if (*result < 0) {
            break;
        }

        size_t j = 0;
        for ( ; j < *list_number; ++j) {
            if (!strcmp(list[j]->name, var->name)) {
                list[i]->value = var->value;
                break;
            }
        }

        if (j == *list_number) {
            list[(*list_number)++] = var;
        }
    }

    if (*result < 0) {
        if (var) {
            free(var);
        }

        if (list) {
            free_list(list, list_size);
        }
    } else {
        list = reallocate_list(list, list_size, *list_number);
        if (NULL == list) {
            *result = MEM_ERROR;
        }
    }

    return list;
}

char get_sign(const char *string) {
    if (NULL == string || strlen(string) < strlen(NOT)) {
        return IN_ERROR;
    }

    if (!strncmp(string, NOT, strlen(NOT))) {
        return NOT_SIGN;
    }

    if (!strncmp(string, AND, strlen(AND))) {
        return AND_SIGN;
    }

    if (!strncmp(string, OR, strlen(OR))) {
        return OR_SIGN;
    }

    if (!strncmp(string, XOR, strlen(XOR))) {
        return XOR_SIGN;
    }

    return IN_ERROR;
}

int priority(const char op) {
    if (NOT_SIGN == op) {
        return 3;
    }

    return (OR_SIGN == op || XOR_SIGN == op) ? 1
            : (AND_SIGN == op) ? 2
            : IN_ERROR;
}

bool process(bool *stack, size_t *size, const char op) {
    if (!stack || *size < 1) {
        return false;
    }

    bool check = false;

    if (NOT_SIGN == op) {
        stack[*size - 1] = !stack[*size - 1];
        check = true;
    } else if (*size >= 2
    && (OR_SIGN == op || AND_SIGN == op || XOR_SIGN == op)) {
        bool right = stack[--*size];
        bool left = stack[*size - 1];
        bool result = false;

        switch (op) {
            case OR_SIGN:
                result = left || right;
                break;
            case AND_SIGN:
                result = left && right;
                break;
            case XOR_SIGN:
                result = !(left == right);
                break;
        }

        stack[*size - 1] = result;
        check = true;
    }

    return check;
}

bool left_assoc(const char op) {
    return !(NOT_SIGN == op);
}

// Вычисление логического выражения,
// переводимого в обратную польскую нотацию
int logical_calc(struct var_t **list, const size_t number, const char *string) {
    if (NULL == string) {
        return 0;
    }

    size_t len = strlen(string);

    // Операнды
    bool *stack = calloc(len, sizeof(bool));
    if (NULL == stack) {
        return MEM_ERROR;
    }

    size_t stack_size = 0;

    // Операции
    char *op = calloc(len, sizeof(char));
    if (NULL == op) {
        if (stack) {
            free(stack);
        }

        return MEM_ERROR;
    }

    size_t op_size = 0;

    char cur_op = -1;

    for (size_t i = 0; i < len; ++i) {
        if (SPACE != string[i]) {
            if (OPEN_BRACKET == string[i]) {  // Открывающая скобка
                op[op_size++] = OPEN_BRACKET;
            } else if (CLOSE_BRACKET == string[i]) {  // Закрывающая скобка
                size_t check = 0;
                while (OPEN_BRACKET != op[op_size - 1]) {
                    ++check;
                    if (!process(stack, &stack_size, op[--op_size])) {
                        if (stack) {
                            free(stack);
                        }

                        if (op) {
                            free(op);
                        }

                        return IN_ERROR;
                    }
                }

                if (!check && stack_size > 1) {
                    if (stack) {
                        free(stack);
                    }

                    if (op) {
                        free(op);
                    }

                    return IN_ERROR;
                }

                --op_size;
            } else if ((cur_op = get_sign(string + i)) > 0) {
                // Операция
                size_t shift = (OR_SIGN == cur_op) ? strlen(OR) : strlen(NOT);
                while (i < len && shift > 0) {
                    ++i;
                    --shift;
                }

                while (op_size != 0
                && ((left_assoc(cur_op)
                && priority(op[op_size - 1]) >= priority(cur_op))
                || (!left_assoc(cur_op)
                && priority(op[op_size - 1]) > priority(cur_op)))) {
                    process(stack, &stack_size, op[--op_size]);
                }

                op[op_size++] = cur_op;
            } else {
                char *operand = malloc((len - i + 1) * sizeof(char));
                if (NULL == operand) {
                    if (stack) {
                        free(stack);
                    }

                    if (op) {
                        free(op);
                    }

                    return MEM_ERROR;
                }

                size_t j = 0;
                bool flag = false;

                if (TRUE[0] == string[i] || FALSE[0] == string[i]) {
                    // True, False
                    char *cmp = (TRUE[0] == string[i]) ? TRUE : FALSE;
                    while (i < len && j < strlen(cmp)) {
                        operand[j++] = string[i++];
                    }
                    operand[j] = '\0';

                    flag = !strcmp(operand, cmp);
                    if (flag) {
                        stack[stack_size++] = (TRUE[0] == operand[0]);
                    }
                } else {
                    // Переменная из списка
                    while (i < len && string[i] >= 'a' && string[i] <= 'z') {
                        operand[j++] = string[i++];
                    }
                    operand[j] = '\0';

                    size_t k = 0;
                    for ( ; k < number; ++k) {
                        if (!strcmp(operand, list[k]->name)) {
                            flag = true;
                            stack[stack_size++] = list[k]->value;
                            break;
                        }
                    }
                }

                --i;

                if (operand) {
                    free(operand);
                }

                if (!flag) {
                    if (stack) {
                        free(stack);
                    }

                    if (op) {
                        free(op);
                    }

                    return IN_ERROR;
                }
            }
        }
    }

    while (op_size != 0) {
        process(stack, &stack_size, op[--op_size]);
    }

    bool result = false;

    if (stack_size > 0) {
        result = stack[stack_size - 1];
    }

    if (stack) {
        free(stack);
    }

    if (op) {
        free(op);
    }

    return result;
}

int logical_parser(FILE *stream) {
    if (NULL == stream) {
        return IN_ERROR;
    }

    int result = 0;

    size_t str_number = 0;
    char **str_array = get_text(&str_number, stream);

    if (!str_array || !str_number) {
        if (str_array) {
            free_text(str_array, str_number);
        }
    } else {
        remove_extra_spaces_from_text(str_array, str_number);

        size_t list_number = 0;
        struct var_t **list = NULL;
        list = get_list(str_array, str_number, &list_number, &result);

        if (str_number == list_number && result >= 0) {
            result = 0;
        } else if (result >= 0 && result < str_number) {
            result = logical_calc(list, list_number, str_array[result]);
        }

        if (str_array) {
            free_text(str_array, str_number);
        }

        if (list) {
            free_list(list, list_number);
        }
    }

    return result;
}

void remove_extra_spaces_from_string(char *string) {
    if (NULL == string) {
        return;
    }

    bool flag = true;

    for (size_t i = 0; i < strlen(string) && ' ' == string[i]; ++i) {
        size_t j = i;
        for ( ; j < strlen(string); ++j) {
            string[j] = string[j + 1];
        }

        string[j] = '\0';
        --i;
    }

    for (size_t i = strlen(string) - 1; i >= 0 && ' ' == string[i]; --i) {
        string[i] = '\0';
    }

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
    if (NULL == buff) {
        return NULL;
    }

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

void print(const char *message, FILE *stream) {
    if (NULL == message || NULL == stream) {
        return;
    }

    fprintf(stream, "%s", message);
}

void free_text(char **str_array, const size_t number) {
    if (NULL == str_array) {
        return;
    }

    for (size_t i = 0; i < number; ++i) {
        free(str_array[i]);
    }

    free(str_array);
}
