#include "GenericStackFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct LinkedString *createStack()
{
    struct LinkedString *first = malloc(sizeof(struct LinkedString));
    first->next = NULL;
    return first;
}

struct LinkedString *push(char character, struct LinkedString *first)
{
    first->curr = character;
    struct LinkedString *newCharacter = malloc(sizeof(struct LinkedString));
    newCharacter->next = first;
    return newCharacter;
}

struct LinkedString *pop(struct LinkedString *first,
                         char *addressForPoppedElement)
{
    if (first->next != NULL)
    {
        *addressForPoppedElement = first->next->curr;
        struct LinkedString *result = first->next;
        free(first);
        first = NULL;
        return result;
    }
    *addressForPoppedElement = first->curr;
    return first;
}

void pushGrammar(void *node, struct GenericStack *stack)
{
    if (stack->stackTop == NULL)
    {
        stack->stackTop = malloc(sizeof(struct GrammarStack));
    }
    ((struct GrammarStack*)stack->stackTop)->node = node;
    struct GrammarStack *newNode = malloc(sizeof(struct GrammarStack));
    newNode->next = (struct GrammarStack*)stack->stackTop;
    stack->stackTop = (void*)newNode;
}

void *popGrammar(struct GenericStack *stack)
{
    struct GrammarStack *first = (struct GrammarStack*)stack->stackTop;
    struct Node *result;
    if (first == NULL) return NULL;
    if (first->next != NULL)
    {
        result = ((struct GrammarStack*)stack->stackTop)->next->node;
        stack->stackTop = (void*)first->next;
        free(first);

    }
    else
    {
        result = NULL;
        free(first);
        stack->stackTop = NULL;
    }
    return result;
}

void *peekGrammar(struct GenericStack *stack)
{
    struct GrammarStack *first = (struct GrammarStack*)stack->stackTop;
    if (first == NULL) return NULL;
    if (first->next != NULL)
    {
        return ((struct GrammarStack*)stack->stackTop)->next->node;
    }
    else
    {
        return NULL;
    }
}

struct GenericStack *createStackGrammar()
{
    struct GrammarStack *first = malloc(sizeof(struct GrammarStack));
    first->next = NULL;
    struct GenericStack *stack = malloc(sizeof(struct GenericStack));
    stack->stackTop = (void*)first;
    stack->pop = popGrammar;
    stack->push = pushGrammar;
    stack->peek = peekGrammar;
    return stack;
}

void pushExpression(void *expression,
                              struct GenericStack *stack)
{
    if (stack->stackTop == NULL)
    {
        stack->stackTop = malloc(sizeof(struct ExpressionStack));
    }
    ((struct ExpressionStack*) stack->stackTop)->expression = expression;
    struct ExpressionStack *newNode = malloc(sizeof(struct ExpressionStack));
    newNode->next = (struct ExpressionStack*) stack->stackTop;
    stack->stackTop = (void*) newNode;
}

void *popExpression(struct GenericStack *stack)
{
    struct ExpressionStack *first = (struct ExpressionStack*) stack->stackTop;
    struct Expression *result;
    if (first == NULL) return NULL;
    if (first->next != NULL)
    {
        result = ((struct ExpressionStack*) stack->stackTop)->next->expression;
        stack->stackTop = (void*)first->next;
        free(first);

    }
    else
    {
        result = NULL;
        free(first);
        stack->stackTop = NULL;
    }
    return result;
}

void *peekExpression(struct GenericStack *stack)
{
    struct ExpressionStack *first = (struct ExpressionStack*) stack->stackTop;
    if (first == NULL) return NULL;
    if (first->next != NULL)
    {
        return ((struct ExpressionStack*) stack->stackTop)->next->expression;
    }
    else
    {
        return NULL;
    }
}

struct GenericStack *createStackExpression()
{
    struct ExpressionStack *first = malloc(sizeof(struct ExpressionStack));
    first->next = NULL;
    struct GenericStack *stack = malloc(sizeof(struct GenericStack));
    stack->stackTop = (void*)first;
    stack->pop = popExpression;
    stack->push = pushExpression;
    stack->peek = peekExpression;
    return stack;
}

void pushLineNumber(void *data,
                              struct GenericStack *stack)
{
    if (stack->stackTop == NULL)
    {
        stack->stackTop = malloc(sizeof(struct LineNumberStack));
    }
    ((struct LineNumberStack*) stack->stackTop)->data = data;
    struct LineNumberStack *newNode = malloc(sizeof(struct LineNumberStack));
    newNode->next = (struct LineNumberStack*) stack->stackTop;
    stack->stackTop = (void*) newNode;
}

void *popLineNumber(struct GenericStack *stack)
{
    struct LineNumberStack *first = (struct LineNumberStack*) stack->stackTop;
    struct LineNumber *result;
    if (first == NULL) return NULL;
    if (first->next != NULL)
    {
        result = ((struct LineNumberStack*) stack->stackTop)->next->data;
        stack->stackTop = (void*)first->next;
        free(first);

    }
    else
    {
        result = NULL;
        free(first);
        stack->stackTop = NULL;
    }
    return result;
}

void *peekLineNumber(struct GenericStack *stack)
{
    struct LineNumberStack *first = (struct LineNumberStack*) stack->stackTop;
    if (first == NULL) return NULL;
    if (first->next != NULL)
    {
        return ((struct LineNumberStack*) stack->stackTop)->next->data;
    }
    else
    {
        return NULL;
    }
}

struct GenericStack *createStackLineNumber()
{
    struct LineNumberStack *first = malloc(sizeof(struct LineNumberStack));
    first->next = NULL;
    struct GenericStack *stack = malloc(sizeof(struct GenericStack));
    stack->stackTop = (void*)first;
    stack->pop = popLineNumber;
    stack->push = pushLineNumber;
    stack->peek = peekLineNumber;
    return stack;
}
