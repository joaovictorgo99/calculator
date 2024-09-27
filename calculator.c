// João Víctor Gomes de Oliveira - 2024
// A simple calculator in C programming language.
// How to use: enter the command line "./calculator 2*3+12/4"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// DEFINES
#define OPERAND 0
#define OPERATOR 1

// ENUMERATIONS
typedef enum {
    MISSING_ARGUMENT_ERROR,
    MEMORY_ALLOCATION_ERROR,
    UNKNOWN_OPERAND_OPERATOR_ERROR,
    EXPRESSION_START_END_ERROR,
    TWO_MORE_OPERATORS_BETWEEN_OPERANDS_ERROR,
    DIVISION_BY_ZERO_ERROR
} error_flag;

// STRUCTS
typedef struct Node {
    int type, data;
    struct Node *next;
} node;

typedef struct Queue {
    node *front;
    node *rear;
} queue;

typedef struct Stack {
    node *top;
} stack;

// CALCULATOR FUNCTIONS
void calculator(char *expression);
queue infix_to_postfix(char *expression);
int calculate(queue *postfix_expression);

// QUEUE FUNCTIONS
void initialize_queue(queue *queue);
int is_empty_queue(queue *queue);
node * peek_queue(queue *queue);
void enqueue(queue *queue, int type_value, int value);
void dequeue(queue *queue);

// STACK FUNCTIONS
void initialize_stack(stack *stack);
int is_empty_stack(stack *stack);
node * peek_stack(stack *stack);
void push(stack *stack, int type_value, int value);
void pop(stack *stack);

// MATHEMATICAL FUNCTIONS
int add(int first_value, int second_value);
int subtract(int first_value, int second_value);
int multiply(int first_value, int second_value);
int divide(int first_value, int second_value, queue *postfix_expression, 
            stack operand);

// ERROR FUNCTIONS
void call_error(int error_number);

// MAIN
int main(int argc, char *argv[])
{
    if (argv[1] == NULL) {  // Waits for an expression, otherwise calls error
        call_error(MISSING_ARGUMENT_ERROR);
    }

    calculator(argv[1]);

    return 0;
}
// MAIN

// CALCULATOR FUNCTIONS
void calculator(char *expression) {
    int result;
    queue postfix_expression;

    postfix_expression = infix_to_postfix(expression);
    result = calculate(&postfix_expression);

    printf("%d\n", result);
}

queue infix_to_postfix(char *infix_expression) {
    // Converts an infix expression to a postfix expression
    char temporary_string[256];
    int i, j;
    queue postfix_expression;
    stack operator;

    initialize_queue(&postfix_expression);
    initialize_stack(&operator);
    
    if (!isdigit(infix_expression[0]) 
            || !isdigit(infix_expression[strlen(infix_expression)-1])) {
        call_error(EXPRESSION_START_END_ERROR);
    }

    for (i = 0, j = 0; i <= strlen(infix_expression); i++, j++) {
        if (infix_expression[i] >= '0' && infix_expression[i] <= '9') {
            temporary_string[j] = infix_expression[i];  // Appends numbers to make a operand
        }
        else if (infix_expression[i] == '+' 
                    || infix_expression[i] == '-' 
                    || infix_expression[i] == '*' 
                    || infix_expression[i] == '/' 
                    || infix_expression[i] == '\0') {
            temporary_string[j] = '\0';
            enqueue(&postfix_expression, OPERAND, atoi(temporary_string));  // Converts a string to a integer (operand) and enqueue it

            if (infix_expression[i] != '\0') {
                if (infix_expression[i+1] == '+' 
                        || infix_expression[i+1] == '-' 
                        || infix_expression[i+1] == '*' 
                        || infix_expression[i+1] == '/') {
                    while (!is_empty_queue(&postfix_expression)) {
                        dequeue(&postfix_expression);
                    }
                    while (!is_empty_stack(&operator)) {
                        pop(&operator);
                    }

                    call_error(TWO_MORE_OPERATORS_BETWEEN_OPERANDS_ERROR);
                }

                if (!is_empty_stack(&operator)) {  // If the stack is empty, jump this condition and push the first operator
                    /*
                    Rule to operators in postfix expression: 
                    if precedence of the operator of the top of the stack >= precedence of the current operator, 
                    then enqueue the top of the stack into the expression and push the current operator onto the operator stack.
                    */
                    if (peek_stack(&operator)->data == '+' 
                            || peek_stack(&operator)->data == '-') {  // Top of the stack is '+' or '-'
                        if (infix_expression[i] == '+' 
                                || infix_expression[i] == '-') {  // Current operator is '+' or '-'
                            // Enqueue top of the stack into the expression and push the current operator onto the stack
                            // If the current operator is '*' or '/', only push the operator onto the stack
                            enqueue(&postfix_expression, OPERATOR, 
                                    peek_stack(&operator)->data);
                            pop(&operator);
                        }
                    }
                    else {  // Top of the stack is '*' or '/'
                        // Enqueue top of the stack into the expression and push the current operator onto the stack
                        enqueue(&postfix_expression, OPERATOR, 
                                peek_stack(&operator)->data);
                        pop(&operator);
                   }
                }

                push(&operator, OPERATOR, infix_expression[i]);
            }
            else {
                while (!is_empty_stack(&operator)) {  // Enqueue the remaining operators into the expression
                    enqueue(&postfix_expression, OPERATOR, 
                            peek_stack(&operator)->data);
                    pop(&operator);
                }
            }

            j = -1;
        }
        else {
            while (!is_empty_queue(&postfix_expression)) {
                dequeue(&postfix_expression);
            }
            while (!is_empty_stack(&operator)) {
                pop(&operator);
            }
            
            call_error(UNKNOWN_OPERAND_OPERATOR_ERROR);
        }
    }

    return postfix_expression;
}

int calculate(queue *postfix_expression) {
    // Calculates the result through the postfix expression
    int first_value, second_value, result;
    node * temporary_node;
    stack operand;

    initialize_stack(&operand);

    while (!is_empty_queue(postfix_expression)) {  // Runs through the expression
        temporary_node = peek_queue(postfix_expression);

        if (temporary_node->type == OPERAND) {  // Operand, push onto the stack
            push(&operand, OPERAND, temporary_node->data);
        }
        else if (temporary_node->type == OPERATOR) {  // Operator, do the calculation
            // Takes the second value first because of the stack structure, so for example a division operation becomes a/b instead of b/a
            second_value = peek_stack(&operand)->data;
            pop(&operand);
            first_value = peek_stack(&operand)->data;
            pop(&operand);

            switch (temporary_node->data) {
                case '+':
                    result = add(first_value, second_value);
                    break;
                case '-':
                    result = subtract(first_value, second_value);
                    break;
                case '*':
                    result = multiply(first_value, second_value);
                    break;
                case '/':
                    result = divide(first_value, second_value, 
                                    postfix_expression, operand);
                    break;
            }

            push(&operand, OPERAND, result);  // Push the result (operand) onto the stack
        }

        dequeue(postfix_expression);
    }

    result = peek_stack(&operand)->data;  // Calculated result is the last operand on the stack
    pop(&operand);

    return result;
}
// CALCULATOR FUNCTIONS

// QUEUE FUNCTIONS
void initialize_queue(queue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
}

int is_empty_queue(queue *queue) {
    return queue->front == NULL; 
}

node * peek_queue(queue *queue) {
    return queue->front;
}

void enqueue(queue *queue, int type_value, int value) {
    node *new_node;

    new_node = (node *) malloc(sizeof(node));

    if (new_node == NULL) {
        call_error(MEMORY_ALLOCATION_ERROR);
    }

    new_node->type = type_value;
    new_node->data = value;
    new_node->next = NULL;

    if (is_empty_queue(queue)) {  // Queue is empty, assign a node to the front and rear of the queue
        queue->front = new_node;
        queue->rear = new_node;
    }
    else {  // Queue isn't empty, assign a node to the rear of the queue
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
}

void dequeue(queue *queue) {
    node *temporary_node;

    temporary_node = queue->front;
    queue->front = queue->front->next;

    if (is_empty_queue(queue)) {  // Queue is empty, point rear node to NULL
        queue->rear = NULL;
    }

    free(temporary_node);
}
// QUEUE FUNCTIONS

// STACK FUNCTIONS
void initialize_stack(stack *stack) {
    stack->top = NULL;
}

int is_empty_stack(stack *stack) {
    return stack->top == NULL;
}

node * peek_stack(stack *stack) {
    return stack->top;
}

void push(stack *stack, int type_value, int value) {
    node *new_node;

    new_node = (node *) malloc(sizeof(node));

    if (new_node == NULL) {
        call_error(MEMORY_ALLOCATION_ERROR); 
    }

    new_node->type = type_value;
    new_node->data = value;
    new_node->next = stack->top;

    stack->top = new_node;  // New node is the new top of the stack
}

void pop(stack *stack) {
    node *temporary_node;

    temporary_node = stack->top;
    stack->top = stack->top->next;  // New top of the stack is the next node from the old top of the stack

    free(temporary_node);
}
// STACK FUNCTIONS

// MATHEMATICAL FUNCTIONS
int add(int first_value, int second_value) {
    return first_value+second_value;
}

int subtract(int first_value, int second_value) {
    return first_value-second_value;
}

int multiply(int first_value, int second_value) {
    return first_value*second_value;
}

int divide(int first_value, int second_value, queue *postfix_expression, 
            stack operand) {
    if (second_value == 0) {
        while (!is_empty_queue(postfix_expression)) {
            dequeue(postfix_expression);
        }

        while (!is_empty_stack(&operand)) {
            pop(&operand);
        }

        call_error(DIVISION_BY_ZERO_ERROR);
    }

    return first_value/second_value;
}
// MATHEMATICAL FUNCTIONS

// ERROR FUNCTIONS
void call_error(int error_number) {
    // Prints the error called and closes the program
    switch (error_number) {
        case MISSING_ARGUMENT_ERROR:
            printf("Error: missing arguments\n");
            break;
        case MEMORY_ALLOCATION_ERROR:
            printf("Error: memory allocation\n");
            break;
        case UNKNOWN_OPERAND_OPERATOR_ERROR:
            printf("Error: unknown operand and/or operator\n");
            break;
        case EXPRESSION_START_END_ERROR:
            printf("Error: expression must start "
                    "and end with a valid number\n");
            break;
        case TWO_MORE_OPERATORS_BETWEEN_OPERANDS_ERROR:
            printf("Error: two or more operators between the operands\n");
            break;
        case DIVISION_BY_ZERO_ERROR:
            printf("Error: division by zero\n");
            break;
    }
    
    exit(1);
}
// ERROR FUNCTIONS
