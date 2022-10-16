#include <stdio.h>
#include "stack.h"
int main() {

    Stack my_stack;
    stack_init(&my_stack, 5);

    print_size_info(&my_stack, stdout);

    stack_data_print(&my_stack);

    stack_pop(&my_stack);
    stack_pop(&my_stack);

    stack_data_print(&my_stack);

    return 0;
}