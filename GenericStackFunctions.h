#ifndef GENERICSTACKFUNCTIONS_H_INCLUDED
#define GENERICSTACKFUNCTIONS_H_INCLUDED

struct LinkedString
{
    char curr;
    struct LinkedString *next;
};

struct GrammarStack
{
    struct Node *node;
    struct GrammarStack *next;
};

struct ExpressionStack
{
    struct Expression *expression;
    struct ExpressionStack *next;
};

/*
    Collection of line number to expression associations
*/
struct LineNumberStack {
    struct LineNumber *data;
    struct LineNumberStack *next;
};

struct GenericStack
{
    void *stackTop;
    void (*push)(void *, struct GenericStack *);
    void *(*pop)(struct GenericStack *);
    void *(*peek)(struct GenericStack *);
};

struct LinkedString *createStack();

struct LinkedString *push(char character, struct LinkedString *first);

struct LinkedString *pop(struct LinkedString *first, char *addressForPoppedElement);

struct GenericStack *createStackGrammar();

struct GenericStack *createStackExpression();

struct GenericStack *createStackLineNumber();

#endif // GENERICSTACKFUNCTIONS_H_INCLUDED
