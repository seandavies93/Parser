#ifndef GENERICSTACKFUNCTIONS_H_INCLUDED
#define GENERICSTACKFUNCTIONS_H_INCLUDED

struct LinkedString
{ // Collection of characters
    char curr;
    struct LinkedString *next;
};

struct GrammarStack
{ // Collections of grammar nodes
    struct Node *node;
    struct GrammarStack *next;
};

struct ExpressionStack
{ // Collection of expressions
    struct Expression *expression;
    struct ExpressionStack *next;
};

struct LineNumberStack
{ // Collection of line number to expression associations
    struct LineNumber *data;
    struct LineNumberStack *next;
};

struct GenericStack
{ // Stack structure with stack top and polymorphic push, pop and peek functions
    void *stackTop;
    void (*push)(void *, struct GenericStack *);
    void *(*pop)(struct GenericStack *);
    void *(*peek)(struct GenericStack *);
};

struct LinkedString *createStack();

struct LinkedString *push(
    char character,
    struct LinkedString *first
);

struct LinkedString *pop(
    struct LinkedString *first,
    char *addressForPoppedElement
);

struct GenericStack *createStackGrammar();

struct GenericStack *createStackExpression();

struct GenericStack *createStackLineNumber();

#endif // GENERICSTACKFUNCTIONS_H_INCLUDED
