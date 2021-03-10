#ifndef PARSERDATA_H_INCLUDED
#define PARSERDATA_H_INCLUDED

struct Expression
{
    int ordering;
    int length;
    char *expression;
    int lineNumber;
};

/*
    Output from the lexer, including tokenised expressions
    and an association structure between line numbers and
    expressions.
*/
struct LexerOutputData
{
    struct Expression **expressionOutput;
    int **lineNumbers;
};

struct LoopSpecExpression
{
    struct Expression *initialiseLoop;
    struct Expression *limitLoop;
    struct Expression *incrementLoop;
};

union ConditionOrLoopSpec
{
    struct Expression *conditional;
    struct LoopSpecExpression *forLoopSpec;
    struct Expression *containedExpression;
};

struct Node
{
    int ordering;
    char constructCode;
    struct GenericStack *expressionGroup;
    union ConditionOrLoopSpec *conditional;
};

#endif // PARSERDATA_H_INCLUDED
