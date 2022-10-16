#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "debug.h"
#include <assert.h>

typedef int Stack_data;
typedef long long int Canary_data;

typedef struct Stack {
    size_t capacity;
    size_t min_capacity;
    size_t element_count;
    long long int hash;
    char* data;
    Canary_data* left_canary_position;
    Canary_data* right_canary_position;

} Stack;

typedef enum stack_status {
    SUCCESS =               0b000000000, 
    STACK_NULL =            0b000000001,
    EMPTY_STACK =           0b000000010,
    STACK_OVERFLOW =        0b000000100,
    DATA_NULL =             0b000001000,
    DEAD_STACK =            0b000010000,
    INVALID_HASH =          0b000100000,
    DAMAGED_LEFT_CANARY =   0b001000000,
    DAMAGED_RIGHT_CANARY =  0b010000000,
    NULL_PTR =              0b100000000
} stack_status;

typedef enum fullness {
    EMPTY = 0,
    UNDER_HALF = 1,
    OVER_HALF = 2,
    FULL = 3,
}fullness;


stack_status stack_init(Stack* stack, size_t length);
stack_status stack_push(Stack* stack, Stack_data element);
Stack_data stack_pop(Stack* stack);
stack_status stack_resize(Stack* stack);
stack_status stack_destruct(Stack* stack);
void stack_data_print(Stack* stack);
void print_size_info(Stack* stack, FILE* file);