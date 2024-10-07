#include "../libs/my_lib.h"

struct Stack {
    u8 *memory;
    size_t pointer;
    size_t size; //in bytes
};

void pushToStack(Stack *stack, EasyToken token) {
    if((stack->pointer + sizeof(token)) <= stack->size) {
        EasyToken *t = (EasyToken *)(stack->memory + stack->pointer);
        *t = token;
        
        stack->pointer += sizeof(token);
    } else {
        printf("STACK FULL\n");
        assert(false);
    }

}

EasyToken popOffStack(Stack *stack) {
    EasyToken token = lexInitToken(TOKEN_UNINITIALISED, 0, 0, 0);
    if(stack->pointer > 0) {
        assert((stack->pointer % sizeof(EasyToken)) == 0);
        stack->pointer -= sizeof(EasyToken);
        token = *((EasyToken *)(stack->memory + stack->pointer));
    } else {
        printf("Nothing on stack\n");
        assert(false);
    }
    return token;
}

Stack initStack(Arena *arena, size_t size) {
    Stack stack;
    stack.size = size;
    stack.memory = (u8 *)pushSize(arena, stack.size);
    stack.pointer = 0;
    return stack;
}

int popOffInt(Stack *stack) {
    EasyToken t = popOffStack(stack);
    assert(t.type == TOKEN_INTEGER);
    return t.intVal;
}
void pushOnInt(Stack *stack, int a) {
    EasyToken token = lexInitToken(TOKEN_INTEGER, 0, 0, 0);
    token.intVal = a;
    pushToStack(stack, token);
}

void DEBUG_testing(Stack *stack) {
    EasyToken token = lexInitToken(TOKEN_UNINITIALISED, 0, 0, 0);
    pushToStack(stack, token);
    token.type = TOKEN_ASTRIX;
    pushToStack(stack, token);
    token.type = TOKEN_FORWARD_SLASH;
    pushToStack(stack, token);
    token.type = TOKEN_INTEGER;
    pushToStack(stack, token);

    EasyToken t = popOffStack(stack);
    assert(t.type == TOKEN_INTEGER);

    token.type = TOKEN_STRING;
    pushToStack(stack, token);

    t = popOffStack(stack);
    assert(t.type == TOKEN_STRING);

    t = popOffStack(stack);
    assert(t.type == TOKEN_FORWARD_SLASH);

    t = popOffStack(stack);
    assert(t.type == TOKEN_ASTRIX);

    t = popOffStack(stack);
    assert(t.type == TOKEN_UNINITIALISED);

    assert(stack->pointer == 0);
}

int main(int argc, char **args) {
    memory_setupArenas();

    const char *fileName = "./rpn_expression-5869168714501555882.txt";
    // const char *fileName = "./test0.txt";
    
    FileContents file = platformReadEntireFile((char *)fileName, true);
    assert(file.valid);

    EasyTokenizer tokenizer = lexBeginParsing(file.memory, EASY_LEX_OPTION_EAT_WHITE_SPACE);

    Stack stack = initStack(&globalLongTermArena, Megabytes(64));

    DEBUG_testing(&stack);

    bool parsing = true;
    while(parsing) {
        EasyToken t = lexGetNextToken(&tokenizer);

        switch(t.type) {
            case(TOKEN_NULL_TERMINATOR): {
                parsing = false;
            } break;
            case(TOKEN_INTEGER): {
                printf("INT\n");
                pushToStack(&stack, t);
            } break;
            case(TOKEN_PLUS): {
                printf("PLUS\n");
                int a = popOffInt(&stack);
                int b = popOffInt(&stack);

                int c = a + b;
                
                pushOnInt(&stack, c);
            } break;
            case(TOKEN_MINUS): {
                printf("MINUS\n");
                int b = popOffInt(&stack);
                int a = popOffInt(&stack);

                int c = a - b;
                
                pushOnInt(&stack, c);
            } break;
            case(TOKEN_ASTRIX): {
                printf("MULT\n");
                int a = popOffInt(&stack);
                int b = popOffInt(&stack);

                int c = a * b;
                
                pushOnInt(&stack, c);
            } break;
            case(TOKEN_FORWARD_SLASH): {
                printf("DIVIDE\n");
                int b = popOffInt(&stack);
                int a = popOffInt(&stack);

                assert(b != 0);

                int c = a / b;
                
                pushOnInt(&stack, c);
            } break;
            default: {
                printf("NO TOKEN FOUND\n");
            } break;
        }
    }

    int a = popOffInt(&stack);
    assert(stack.pointer == 0);
    printf("Your value was %d\n", a);

    return 0;
}