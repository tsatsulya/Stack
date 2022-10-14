#include <stdio.h>
#include "stack.h"
int main() {

    Stack my_stack;
    stack_init(&my_stack, 5);
    stack_push(&my_stack, 300);
    stack_push(&my_stack, 31);
    stack_push(&my_stack, 40);
    stack_push(&my_stack, 41);
    stack_push(&my_stack, 42);
    stack_push(&my_stack, 43);
    print_size_info(&my_stack);
    stack_data_print(&my_stack);
    stack_pop(&my_stack);
    stack_pop(&my_stack);
    stack_data_print(&my_stack);

    return 0;
}