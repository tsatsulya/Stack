#include "stack.h"

#define min(x, y) ((x) < (y)) ? (x) : (y)

#define max(x, y) ((x) > (y)) ? (x) : (y)

static Canary_data CANARY = 0x000DEAD000;
Stack_data INVALID_VALUE = 0x00FE00;





static size_t stack_get_buffer_size(size_t length) {

    size_t size = length*sizeof(Stack_data) + 2*sizeof(Canary_data);

    return size;
}

static char* stack_get_data_base_ptr(Stack* stack) {

     return stack->data + sizeof(Canary_data);
}  



static Stack_data* get_ptr_to_element(Stack* stack, long long int position) {

    char* data = stack_get_data_base_ptr(stack);
    
    char* ptr_to_element = (data + sizeof(Stack_data) * position);
    return (Stack_data*)ptr_to_element;
} 

static Stack_data get_element(Stack* stack, long long int position) {
    
    return *get_ptr_to_element(stack, position);
} 


static stack_status light_stack_check_status(Stack* stack) {

    int status = SUCCESS;

    if (stack->capacity < stack->element_count)
        status |= STACK_OVERFLOW;
    if (!stack)
        status |= STACK_NULL;
    if (!stack->data)
        status |= DATA_NULL;

    return status;
}

static unsigned long long int stack_hash(Stack* stack);

static stack_status stack_check_hash(Stack* stack) {

    if (!stack)
        return STACK_NULL;

    unsigned long int correct_stack_hash = stack->hash;
    unsigned long int actual_stack_hash = stack_hash(stack);

    return (actual_stack_hash == correct_stack_hash) ? SUCCESS : INVALID_HASH;
}

static stack_status stack_check_canaries(Stack* stack) {

    stack_status status = SUCCESS;
    
    if (*stack->left_canary_position != CANARY) status|=DAMAGED_LEFT_CANARY;
    if (*stack->right_canary_position != CANARY) status|=DAMAGED_RIGHT_CANARY;

    return status;
}

static stack_status stack_check_status(Stack *stack) {

    int status = SUCCESS;

    status |= light_stack_check_status(stack);
    if (status) return status;
    status |= stack_check_canaries(stack);
    status |= stack_check_hash(stack);

    return status;
}

void stack_dump(Stack* stack, const char* file, const char* func, int line, stack_status status);

static stack_status stack_assert_invariants(Stack* stack, const char* file, const char* func, int line) {

    stack_status status = SUCCESS;

    if (!stack)
        status = STACK_NULL;
    if (status) {
        stack_dump(stack, file, func, line, status);
        return status;
    }

    status = stack_check_status(stack);

    if (status) {
        stack_dump(stack, file, func, line, status);
        // print_error + dump
    }

    return status;
}

#define stack_assert_invariants(stack) stack_assert_invariants(stack, __FILE__, __func__, __LINE__)





static fullness get_stack_occupancy_status(Stack* stack) {

    if (stack->element_count == 0) return EMPTY;
    if (stack->element_count < stack->capacity/2) return UNDER_HALF;
    if (stack->element_count < stack->capacity) return OVER_HALF;
    return FULL;
}

static stack_status stack_change_length(Stack* stack, double multiplier) {

    size_t new_capacity = max(stack->min_capacity, stack->capacity * multiplier);
    size_t new_size = stack_get_buffer_size(new_capacity); 

    stack->capacity = new_capacity;
    stack->data = (char*) realloc(stack->data, new_size);

    return 0;
}

void print_size_info(Stack* stack, FILE* file) {
    fprintf(file, "capacity: %ld\n", stack->capacity);
    fprintf(file, "num of elements: %ld\n", stack->element_count);
}



static stack_status stack_length_up(Stack* stack) {
    stack_change_length(stack, 2);
    //
    return 0;
}

static stack_status stack_length_down(Stack* stack) {
    stack_change_length(stack, 1/2);
    //
    return 0;
}


static Canary_data* get_left_canary_position(Stack* stack) {
    return (Canary_data*)stack->data;
}

static Canary_data* get_right_canary_position(Stack* stack) {
    return (Canary_data*)(stack->data + sizeof(Stack_data) * stack->capacity + sizeof(Canary_data));
}

static void print_canaries(Stack* stack, FILE* file) {
    
    fprintf(file, "left_canary: %llx\n", *stack->left_canary_position);
    fprintf(file, "right_canary: %llx\n", *stack->right_canary_position);
}

static stack_status set_canaries(Stack* stack){

    *stack->left_canary_position = *stack->right_canary_position = CANARY;
    return SUCCESS;
}

static unsigned long int gnu_hash(void* data, size_t size) {

    char* hash_data = (char*) data;
    unsigned long int h = 0;
    
    for (unsigned long int c = 0; c < size; c++) {    
        h = h * 33  + *(hash_data+c);
    }

    return 0;
}

static unsigned long long int stack_hash(Stack* stack) {

    int data_hash = gnu_hash(stack_get_data_base_ptr(stack), stack->capacity * sizeof(Stack_data));
    int struct_hash = gnu_hash(stack, sizeof(Stack));

    return data_hash ^ struct_hash;
}

static int stack_rehash(Stack* stack) {

    stack->hash = stack_hash(stack);

    return SUCCESS;
}


void stack_data_print(Stack* stack) {

    char* data = stack_get_data_base_ptr(stack);

    for (int i = 0; i < (int)stack->element_count; i++) {
        printf("______elem #%d: %d\n", i+1, *(Stack_data*)(data + i*4));
    }
}


stack_status stack_init(Stack* stack, size_t length) {

    if (!stack) return STACK_NULL;
    *stack = (Stack){
        .capacity = length,
        .min_capacity = length,
        .element_count = 0,
        .hash = 0,
        .data = (char*)calloc(stack_get_buffer_size(length), sizeof(char)),
        .left_canary_position = NULL, 
        .right_canary_position = NULL
    };

   
    debug_msg(INFO_, printf("___init_stack: %p\n", stack));
    debug_msg(INFO_, printf("___init_data: %p\n", stack->data));
    


    stack->left_canary_position = get_left_canary_position(stack);
    stack->right_canary_position = get_right_canary_position(stack);
    set_canaries(stack);

    stack_rehash(stack);
    debug_msg(INFO_, print_canaries(stack, stdout));

    stack_assert_invariants(stack);
    return 0;
}

stack_status stack_push(Stack* stack, Stack_data element) {

    stack_resize(stack);

    *get_ptr_to_element(stack, stack->element_count++) = element;

    stack_rehash(stack);

    return 0;
}

Stack_data stack_pop(Stack* stack) {

    if (get_stack_occupancy_status(stack) == EMPTY) {
        stack_dump(stack, __FILE__, __func__, __LINE__, EMPTY_STACK);
        abort();
    }
    stack_assert_invariants(stack);

    Stack_data last_element = get_element(stack, stack->element_count--);
    
    stack_resize(stack);
    stack_rehash(stack);

    return last_element;
}

stack_status stack_resize(Stack* stack) {

    fullness status = get_stack_occupancy_status(stack);

    if (status >= FULL) {
        puts("length up!");
        stack_length_up(stack);
    }
    if (status <= UNDER_HALF) {
        puts("length down!");
        stack_length_down(stack);
    }
    //print_size_info(stack, stdout);
    set_canaries(stack);
    stack->left_canary_position = get_left_canary_position(stack);
    stack->right_canary_position = get_right_canary_position(stack);
    stack_rehash(stack);

    return 0;
}

stack_status stack_destruct(Stack* stack) {

    //
    free(stack->data);

    *stack = (Stack) {
        .capacity = 0,
        .min_capacity = 0,
        .element_count = 0,
        .hash = 0, 
        .data = NULL,};

    return SUCCESS;
}


void stack_dump(Stack* stack, const char* file, const char* func, int line, stack_status status) {


    debug_msg(INFO_, printf("DUMP was called from %s :: %s :: %d\n", file, func, line));

    FILE* dump_file = fopen("dump.txt", "w");
    assert(dump_file && "opening file error");

    fprintf(dump_file, "DUMP was called from %s :: %s :: %d\n", file, func, line);

    fprintf(dump_file, "The stack address: [%p]\n", stack);

    if (status & STACK_NULL) { 

        fprintf(dump_file, "Dump aborted, stack has NULL pointer");
        fclose(dump_file);
        abort();
    }

    fprintf(dump_file, "The data begining address: [%p]\n", stack->data);

    if (status & DATA_NULL) {

        fprintf(dump_file, "Dump aborted, data has NULL pointer\n");
        fclose(dump_file);
        abort();
    }


    if (status & EMPTY_STACK) {

        fprintf(dump_file, "Dump aborted, removing element from empty stack!\n");
        print_size_info(stack, dump_file);
    }

    if (status & STACK_OVERFLOW) {

        fprintf(dump_file, "StackOverflow!\n");
        print_size_info(stack, dump_file);
    }

    if (status & INVALID_HASH) {

        fprintf(dump_file, "Dump aborted, hash was damaged!\n");
        fprintf(dump_file, "Stack hash: %llu\n", stack->hash);
        fprintf(dump_file, "Actual hash: %llu\n", stack_hash(stack));
    }

    if (status & DAMAGED_LEFT_CANARY || status & DAMAGED_RIGHT_CANARY) {

        fprintf(dump_file, "Dump aborted, damaged either or both canaries!\n");
        print_canaries(stack, dump_file);
    }


    fclose(dump_file), dump_file = NULL;

    return;
}
