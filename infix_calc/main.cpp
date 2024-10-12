#include "../libs/my_lib.h"

struct Stack {
    u8 *memory;
    size_t pointer;
    size_t size; //in bytes
};

int getStackCount(Stack *stack) {
    return stack->pointer / sizeof(EasyToken);
}

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

int getPrecendence(EasyTokenType type) {
    int result = 0;
    if(type == TOKEN_PLUS || type == TOKEN_MINUS) {
        result = 0;
    } else {
        result = 1;
    }
    return result;
}

void evaluateStack(Stack *operatorStack, Stack *numberStack, int precedence) {
    bool evaluting = true;
    while(getStackCount(operatorStack) > 0 && evaluting) {
        EasyToken t = popOffStack(operatorStack);
        assert(t.type != TOKEN_INTEGER);
        if(TOKEN_OPEN_PARENTHESIS == t.type || getPrecendence(t.type) < precedence) {
            //NOTE: Put operators back on 
            pushToStack(operatorStack, t);
            evaluting = false;
        } else {
            if(getStackCount(numberStack) < 2) {
                printf("NOT ENOUGH NUMBERS ON STACK");
            }
            
            EasyToken t0 = popOffStack(numberStack);
            assert(t0.type == TOKEN_INTEGER);

            EasyToken t1 = popOffStack(numberStack);
            assert(t1.type == TOKEN_INTEGER);

            float value = 0;

            switch(t.type) {
                case TOKEN_ASTRIX: {
                    value = t1.intVal * t0.intVal;
                } break;
                case TOKEN_FORWARD_SLASH: {
                    value = t1.intVal / t0.intVal;
                } break;
                case TOKEN_PLUS: {
                    value = t1.intVal + t0.intVal;
                } break;
                case TOKEN_MINUS: {
                    value = t1.intVal - t0.intVal;
                } break;
                default: {
                    assert(false);
                }
            } 


            EasyToken token = lexInitToken(TOKEN_INTEGER, 0, 0, 0);
            token.intVal = round(value);
            pushToStack(numberStack, token);
        }
    }
}

int main(int argc, char **args) {
    memory_setupArenas();

    const char *fileName = "./ifn-expression.txt";
    // const char *fileName = "./test0.txt";
    
    FileContents file = platformReadEntireFile((char *)fileName, true);
    assert(file.valid);

    EasyTokenizer tokenizer = lexBeginParsing(file.memory, EASY_LEX_OPTION_EAT_WHITE_SPACE);

    Stack numberStack = initStack(&globalLongTermArena, Megabytes(64));
    Stack operatorStack = initStack(&globalLongTermArena, Megabytes(64));

    // DEBUG_testing(&numberStack);

    bool parsing = true;
    while(parsing) {
        EasyToken t = lexGetNextToken(&tokenizer);

        switch(t.type) {
            case(TOKEN_NULL_TERMINATOR): {
                evaluateStack(&operatorStack, &numberStack, 0);
                parsing = false;
            } break;
            case(TOKEN_OPEN_PARENTHESIS): {
                pushToStack(&operatorStack, t);
            } break;
            case(TOKEN_CLOSE_PARENTHESIS): {
                evaluateStack(&operatorStack, &numberStack, 0);
                t = popOffStack(&operatorStack);
                assert(t.type == TOKEN_OPEN_PARENTHESIS);
            } break;
            case(TOKEN_INTEGER): {
                pushToStack(&numberStack, t);
            } break;
            case(TOKEN_MINUS):
            case(TOKEN_PLUS): {
                evaluateStack(&operatorStack, &numberStack, 0);
                pushToStack(&operatorStack, t);
            } break;
            case(TOKEN_FORWARD_SLASH):
            case(TOKEN_ASTRIX): {
                evaluateStack(&operatorStack, &numberStack, 1);
                pushToStack(&operatorStack, t);
            } break;
            default: {
                printf("NO TOKEN FOUND\n");
            } break;
        }
    }

    int a = popOffInt(&numberStack);
    assert(numberStack.pointer == 0);
    printf("Your value was %d\n", a);

    return 0;
}