#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*

Given that this is about parsing a stream of text
I might as well start writing the code for the
next thing here. The next thing being a program
to inspect the normal form of a boolean expression and finding a way to minimise
it.

The normal form expression could be fed in as text
and then parsed, hence the relation to this work.

the data structure that could be used is perhaps just an array,
as the functions will always be expressed as a sum of products and each element
of the array

could represent each constituent product.
This seems like the simplest structure which still captures the essence of the
problem.

it seems the array would keep track of simply
whether or not the i-th input is inverted or not in the product.

*/



/* STRUCTS SECTION ////////////////////*/

struct inversionStruct
{
    int isInverted;
};

struct linkedInversionArray
{
    struct inversionStruct inversion;
    struct linkedInversionArray *next;
};

struct termArray
{
    struct linkedInversionArray inversionArray;
    struct termArray *next;
};

/*
  Structure for a linked list of rows
*/
struct RowList
{
    int rowIndex;
    struct Row *row;
    struct RowList *rowList;
};

/*
  Struct for columns along the row
*/
struct Row
{
    int columnIndex;
    int nextSymbol;
    struct Row *next;
};

/*
  Packaged struct including the automaton table and the known final state for
  use in searching
*/
struct RegexData
{
    struct RowList *regexTable;
    int finalState;
};

struct LinkedString
{
    char curr;
    struct LinkedString *next;
};

struct Expression
{
    int ordering;
    int length;
    char *expression;
    int lineNumber;
};

struct GenericStack
{
    void *stackTop;
    void (*push)(void *, struct GenericStack *);
    void *(*pop)(struct GenericStack *);
    void *(*peek)(struct GenericStack *);
};

/*
    Possible memory efficient way of dealing with line numbers
    consider after trying the naive way
*/
struct LineNumber {
    int number;
    struct ExpressionStack *applicableExpressions;
};

/*
    Collection of line number to expression associations
*/
struct LineNumberStack {
    struct LineNumber *data;
    struct LineNumberStack *next;
};

/*
    Output from the lexer, including tokenised expressions
    and an association structure between line numbers and
    expressions.
*/
struct LexerOutputData {
    struct Expression **expressionOutput;
    struct LineNumberStack *first;
};

struct LinkedStack
{
    char character;
    struct Stack *nextPointer;
};

struct ExpressionStack
{
    struct Expression *expression;
    struct ExpressionStack *next;
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

struct GrammarStack
{
    struct Node *node;
    struct GrammarStack *next;
};

//////////////////// END ////////////////////
/* STACK FUNCTION SECTION ////////////////////*/
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

struct LineNumberStack *pushLN(struct LineNumber *data,
                              struct LineNumberStack *first)
{
    first->data = data;
    struct LineNumberStack *newNode = malloc(sizeof(struct LineNumberStack));
    newNode->next = first;
    return newNode;
}

struct LineNumberStack *popLN(struct LineNumberStack *first)
{
    if (first->next != NULL)
    {
        struct LineNumberStack *result = first->next;
        free(first);
        first = NULL;
        return result;
    }
    else
        return first;
}

struct LineNumberStack *createStackLN()
{
    struct LineNumberStack *first = malloc(sizeof(struct LineNumberStack));
    first->next = NULL;
    return first;
}
//////////////////// END ////////////////////
// Generate an array of integers indicating the reduction in how far we can skip
// ahead when matching a string in some text
int *generatePatternSkipData(char *pattern, int sizeOfPattern)
{
    int i = 0;
    int j = 1;
    int *patternSkipReductionData = malloc(sizeOfPattern * sizeof(int));
    patternSkipReductionData[0] = 0;
    while (j < sizeOfPattern)
    {
        if (pattern[i] != pattern[j])
        {
            i = 0;
        }
        patternSkipReductionData[j++] = i++;
    }
    return patternSkipReductionData;
}

/*
  We can iterate backwards through the pattern skip reduction array to find
  if a prefix duplicate exists in the pattern
*/
int getAppropriateSkipReduction(int *patternSkipReductionData, int index)
{
    int currentIndex = index;
    while (patternSkipReductionData[index] == 0 && index >= 0)
    {
        index--;
    }
    if (currentIndex - index == 1)
    {
        return patternSkipReductionData[currentIndex - 1] + 1;
    }
    return 0;
}

/*
  If there are no repetitions of a prefix to be found in the pattern then
  we can fully skip to the current index
  The only situation the necessitates not fully skipping is if there is a
  substring prefix all the way up to  j - 1 (but not including j as if we
  are in this clause then we know this doesn't match therefore that
  character of the prefix substring will not match when it first appears
  in the actual prefix not the duplicate)
*/
int searchText(char *text, char *pattern, int sizeOfText, int sizeOfPattern)
{
    int i = 0;
    int j = 0;
    int *patternSkipReductionData =
        generatePatternSkipData(pattern, sizeOfPattern);
    while ((i < sizeOfText) && (j < sizeOfPattern))
    {
        if (text[i] != pattern[j])
        {
            if (j != 0)
            {
                j = getAppropriateSkipReduction(patternSkipReductionData, j);
            }
            else
            {
                i++;
            }
        }
        else
        {
            j++;
            i++;
        }
    }
    if (i == sizeOfText && j != sizeOfPattern)
        return -1;
    return i - j;
}

/*
                           abcdefgouabcdefgouk
ilsdfisdlfiulsidufabcdefgouabcdefgoutfgrres
*/



/*
  Create a row with a particular rowIndex
*/
struct RowList *createRowList(int rowIndex)
{
    struct RowList *first = malloc(sizeof(struct RowList));
    first->rowIndex = rowIndex;
    first->rowList = NULL;
    return first;
}

/*
  Create an initial row
*/
struct RowList *createRowListFirst()
{
    struct RowList *first = malloc(sizeof(struct RowList));
    first->rowIndex = 0;
    first->rowList = NULL;
    return first;
}

/*
  Create a column entry with a particular columnIndex
*/
struct Row *createRow(int columnIndex)
{
    struct Row *first = malloc(sizeof(struct Row));
    first->columnIndex = columnIndex;
    first->next = NULL;
    return first;
}

struct Row *createRowWithEntry(int columnIndex, int nextSymbol) {
    struct Row *first = malloc(sizeof(struct Row));
    first->columnIndex = columnIndex;
    first->nextSymbol = nextSymbol;
    first->next = NULL;
    return first;
}

/*
  Create an initial column entry
*/
struct Row *createRowFirst()
{
    struct Row *first = malloc(sizeof(struct Row));
    first->columnIndex = 0;
    first->next = NULL;
    return first;
}

/*
  Inserting an item in a row at the correct position (based on column index and
  assuming a node does not exist for this columnIndex already)
*/
struct Row *insertItemAtAppropriateRowPosition(int nextSymbol, int columnIndex,
        struct Row *row)
{
    struct Row *previous = row;
    struct Row *tracker = row;

    if (row == NULL)
    {
        return createRowWithEntry(columnIndex, nextSymbol);
    }

    /*
     Iterate through keeping track of the current node and the previous
     stop as soon as the tracker's columnIndex stops being less then the
     supplied columnIndex
    */
    while (tracker != NULL && tracker->columnIndex < columnIndex)
    {
        previous = tracker;
        tracker = tracker->next;
    }

    struct Row *newRow = createRowWithEntry(columnIndex, nextSymbol);
    newRow->next = tracker;
    if (tracker != previous)
    {
        previous->next = newRow;
        return row;
    }
    return newRow;
}

/*
  Insert an item into the table based on the desired indices
*/
struct RowList *insertAtPlace(int nextSymbol, int columnIndex, int rowIndex,
                              struct RowList *table)
{
    // Maybe find a way of dealing with the situation where there is initially
    // no row within this function instead
    struct RowList *rowTracker = table;
    struct Row *columnTracker;
    if (table != NULL)
    {
        while (rowTracker != NULL)
        {
            columnTracker = rowTracker->row;
            while (columnTracker != NULL)
            {
                if (columnTracker->columnIndex == columnIndex &&
                        rowTracker->rowIndex == rowIndex)
                {
                    columnTracker->nextSymbol = nextSymbol; // Found the node with the right indices, set the value here
                    return table;
                }
                columnTracker = columnTracker->next;
            }
            rowTracker = rowTracker->rowList;
        }
        rowTracker = table;
        while (rowTracker != NULL)
        {
            if (rowTracker->rowIndex == rowIndex)
            {
                rowTracker->row = insertItemAtAppropriateRowPosition(
                                      nextSymbol, columnIndex, rowTracker->row);
                return table;
            }
            rowTracker = rowTracker->rowList;
        }
    }
    // Otherwise we have not found either a a row or column and so we need to
    // create these
    struct RowList *newRowList = createRowList(rowIndex);
    struct Row *newRow = createRow(columnIndex);
    newRowList->rowList = table;
    newRowList->row = newRow;
    newRow->nextSymbol = nextSymbol;
    return newRowList;
}

int getItemAtPlace(int columnIndex, int rowIndex, struct RowList *table)
{
    struct RowList *rowTracker = table;
    struct Row *columnTracker;
    if (table != NULL)
    {
        while (rowTracker != NULL)
        {
            if (rowTracker->rowIndex == rowIndex)
            {
                columnTracker = rowTracker->row;
                while (columnTracker != NULL)
                {
                    if (columnTracker->columnIndex == columnIndex)
                    {
                        return columnTracker->nextSymbol;
                    }
                    columnTracker = columnTracker->next;
                }
            }
            rowTracker = rowTracker->rowList;
        }
    }
    return -1;
}

struct Row *deleteItemAtAppropriateRowPosition(int columnIndex,
        struct Row *row)
{
    struct Row **pointerToRow = &row;

    if (row == NULL)
    {
        return row;
    }

    if (row->columnIndex == columnIndex)
    {
        struct Row *result = row->next;
        free(row);
        return result;
    }

    while (*pointerToRow != NULL && (*pointerToRow)->columnIndex != columnIndex)
    {
        pointerToRow = &((*pointerToRow)->next);
    }

    struct Row *toBeFreedRow = *pointerToRow;
    *pointerToRow = toBeFreedRow->next;
    free(toBeFreedRow);
    return row;
}

void deleteAtPlace(int columnIndex, int rowIndex,
                              struct RowList *table)
{
    struct RowList *rowTracker = table;
    if (table != NULL)
    {
        while (rowTracker != NULL)
        {
            if (rowTracker->rowIndex == rowIndex)
            {
                rowTracker->row = deleteItemAtAppropriateRowPosition(
                                      columnIndex, rowTracker->row);
            }
            rowTracker = rowTracker->rowList;
        }
    }
}

/*
  Find the first index of a matching string in the supplied string using the
  regex
*/
int findMatch(char *array, int startIndex, int endIndex,
              struct RegexData *regex)
{
    int offset = startIndex;
    int currentState = 0;
    int previousState = 0;
    int relativeIndex = 0;
    while ((offset + relativeIndex) < endIndex)
    {
        previousState = currentState;
        currentState = getItemAtPlace(
                           currentState, (int)array[offset + relativeIndex], regex->regexTable);
        if (currentState == regex->finalState)
            break;
        relativeIndex++;
        if (previousState != 0 && currentState == 0)
        {
            offset = offset + relativeIndex;
            relativeIndex = 0;
        }
    }
    return offset + relativeIndex - 1;
}

/*
  Find the last index of a matching string within another string, using the
  Regex data
*/
int findMatchEndIndex(char *array, int startIndex, int endIndex,
                      struct RegexData *regex)
{
    int trackingIndex = startIndex;
    int currentState = 0;
    while (trackingIndex < endIndex)
    {
        currentState = getItemAtPlace(currentState, (int)array[trackingIndex],
                                      regex->regexTable);
        if (currentState == regex->finalState)
            break;
        trackingIndex++;
    }
    return trackingIndex - 1;
}

/*
  Creates a basic character string regular expression matcher
*/
struct RegexData *createBasicAlphabetStringMatcher()
{
    int startSymbol = 32;
    int i = startSymbol;
    int startOfCapitals = 65;
    int endOfCapitals = 90;
    int startOfLowerCase = 97;
    int endOfLowerCase = 122;
    int finalSymbol = 127;
    struct RegexData *regexData = malloc(sizeof(struct RegexData));
    struct RowList *table = createRowListFirst();
    struct Row *row = createRowFirst();
    table->row = row;

    while (i < finalSymbol)
    {
        if ((i >= startOfCapitals && i <= endOfCapitals) ||
                (i >= startOfLowerCase && i <= endOfLowerCase))
        {
            table = insertAtPlace(1, 0, i, table);
        }
        else
        {
            table = insertAtPlace(0, 0, i, table);
        }
        i++;
    }
    i = startSymbol;
    while (i < finalSymbol)
    {
        if ((i >= startOfCapitals && i <= endOfCapitals) ||
                (i >= startOfLowerCase && i <= endOfLowerCase))
        {
            table = insertAtPlace(1, 1, i, table);
        }
        else
        {
            table = insertAtPlace(2, 1, i, table);
        }
        i++;
    }
    regexData->regexTable = table;
    regexData->finalState = 2;
    return regexData;
}

void deAllocateExpressionMemory(struct Expression *expression)
{
    free(expression->expression);
    expression->expression = NULL;
    free(expression);
    expression = NULL;
}

struct Expression **tokenizeNew(char *array, int size, int *numberTokens);
struct Node *parseCode(struct Expression **lexedContent, int numberOfTokens);

int main(int argc, char *argv[])
{
    FILE *ptr1;
    FILE *ptr2;
    ptr1 = fopen(argv[1], "r");
    ptr2 = fopen(argv[2], "r");
    char currentChar;
    int size1 = 0;
    int size2 = 0;

    struct LinkedString *current1 = createStack();
    while ((currentChar = fgetc(ptr1)) != EOF)
    {
        size1++;
        current1 = push(currentChar, current1);
    }

    struct LinkedString *current2 = createStack();
    while ((currentChar = fgetc(ptr2)) != EOF)
    {
        size2++;
        current2 = push(currentChar, current2);
    }

    fclose(ptr1);

    char *string1 = malloc(size1 * sizeof(char));
    for (int i = size1 - 1; i >= 0; i--)
    {
        current1 = pop(current1, &(string1[i]));
    }

    char *string2 = malloc(size2 * sizeof(char));
    for (int i = size2 - 1; i >= 0; i--)
    {
        current2 = pop(current2, &(string2[i]));
    }

    free(current1);
    current1 = NULL;
    free(current2);
    current2 = NULL;

    int result = searchText(string1, string2, size1, size2);
    printf("result %d\n", result);

    int *numberOfTokens = malloc(sizeof(int));
    struct Expression **arrayExp = tokenizeNew(string1, size1, numberOfTokens);
    // Need to first the individual pointers in this block, then the entire
    // block
    struct Node *parseTree = parseCode(arrayExp, *numberOfTokens);

    /*
      Note for now: all that matters from the perspective of the free function is
      that the pointer value matches one that it has assigned in its assignment table (might be how it works under the hood)
    */
    free(arrayExp);
    arrayExp = NULL;

    // Test out the sparse array
    struct RowList *table = createRowListFirst();
    table->row = createRowFirst();
    table = insertAtPlace(10, 1, 1, table);
    table = insertAtPlace(10, 1, 2, table);
    table = insertAtPlace(12, 1, 3, table);
    table = insertAtPlace(13, 2, 1, table);
    table = insertAtPlace(14, 2, 2, table);

    printf("item at 1 1: %d\n", getItemAtPlace(1, 1, table));
    printf("item at 1 2: %d\n", getItemAtPlace(1, 2, table));
    printf("item at 1 3: %d\n", getItemAtPlace(1, 3, table));
    printf("item at 2 1: %d\n", getItemAtPlace(2, 1, table));
    printf("item at 2 2: %d\n", getItemAtPlace(2, 2, table));

    // Testing regex stuff
    struct RegexData *regex = createBasicAlphabetStringMatcher();
    int index = 32;
    int matchStart = findMatch(string2, 2, size2 - 1, regex);

    free(string1);
    free(string2);
    free(numberOfTokens);
    string1 = NULL;
    string2 = NULL;
}

struct Expression *allocateSingleCharExpression(char item, int lineNumber)
{
    struct Expression *newExpression = malloc(sizeof(struct Expression));
    char *newChar = malloc(sizeof(char));
    *newChar = item;
    newExpression->expression = newChar;
    newExpression->length = 1;
    newExpression->lineNumber = lineNumber;
    return newExpression;
}

/*
    function for splitting a string into tokens
*/
struct Expression **tokenizeNew(char *array, int size, int *numberOfTokens)
{
    struct GenericStack *first = createStackExpression();
    struct LinkedString *firstCharacterOnStore = createStack();
    struct LineNumberStack *lineNumberList = createStackLN();
    int counter = 0;
    int tempCharacterCount = 0;
    int sizeOfExpressionArray = 0;
    int lineNumber = 0;
    struct LineNumber *lines = malloc(sizeof(struct LineNumber));
    lines->number = lineNumber;
    pushLN(lines, lineNumberList);
    while (counter < size)
    {
        printf("Reading char lexer: %c\n", array[counter]);
        if (array[counter] == '(' || array[counter] == ')' ||
                array[counter] == '}' || array[counter] == '{' ||
                (array[counter] == 'f' && array[counter + 1] == '(') ||
                (array[counter] == 'w' && array[counter + 1] == '('))
        {
            first->push(allocateSingleCharExpression(array[counter], lineNumber), first);
            sizeOfExpressionArray++;
        }
        else if (array[counter] == '\n')
        {
            lineNumber++;
            lines = malloc(sizeof(struct LineNumber));
            lines->number = lineNumber;
            pushLN(lines, lineNumberList);
        }
        else if (array[counter] != '(' && array[counter] != ')' &&
                 array[counter] != ';' && array[counter] != 'f' &&
                 array[counter] != 'w' && array[counter] != '+' &&
                 array[counter] != ',' && array[counter] != '=')
        {
            firstCharacterOnStore = push(array[counter], firstCharacterOnStore);
            tempCharacterCount++;
        }
        else if (array[counter] == ';' || array[counter] == '+' ||
                 array[counter] == '*' || array[counter] == '=' ||
                 array[counter] == ',')
        {
            struct Expression *newExp = malloc(sizeof(struct Expression));
            char *charBlock = malloc(tempCharacterCount * sizeof(char));
            newExp->length = tempCharacterCount;
            while (tempCharacterCount > 0)
            {
                firstCharacterOnStore =
                    pop(firstCharacterOnStore, charBlock + (tempCharacterCount - 1));
                tempCharacterCount--;
            }
            newExp->expression = charBlock;
            newExp->lineNumber = lineNumber;
            first->push(newExp, first);
            sizeOfExpressionArray++;
            // Separation within for loop is implicit with the splitting off into
            // different expressions
            if (array[counter] != ',')
            {
                first->push(allocateSingleCharExpression(array[counter], lineNumber), first);
                sizeOfExpressionArray++;
            }
        }
        counter++;
    }
    struct Expression **result =
        malloc(sizeOfExpressionArray * sizeof(struct Expression *));
    int j = sizeOfExpressionArray - 1;

    while (first->peek(first) != NULL)
    {
        result[j] = first->pop(first);
        j--;
    }
    free(firstCharacterOnStore);
    firstCharacterOnStore = NULL;
    free(first);
    first = NULL;
    *numberOfTokens = sizeOfExpressionArray;
    return result;
};

struct LoopSpecExpression *initialiseLoopSpecExpression()
{
    struct LoopSpecExpression *newSpec =
        malloc(sizeof(struct LoopSpecExpression));
    newSpec->incrementLoop = NULL;
    newSpec->limitLoop = NULL;
    newSpec->initialiseLoop = NULL;
    return newSpec;
}

void pushNodeIntoExpressionGroup(struct GenericStack *currentWorkingNode,
                                 struct Node *nodeToPush)
{
    struct Node *current = (struct Node*) currentWorkingNode->peek(currentWorkingNode);
    if (current->expressionGroup == NULL)
        current->expressionGroup = createStackGrammar();
    current->expressionGroup->
        push(nodeToPush, current->expressionGroup);
}

struct Node *initialiseNode(char currentState)
{
    struct Node *newChild = malloc(sizeof(struct Node));
    newChild->constructCode = currentState;
    newChild->expressionGroup = NULL;
    newChild->conditional = NULL;
    return newChild;
};

void addInitialLoopSpecToNode(struct Node *node)
{
    union ConditionOrLoopSpec *newConditionOrLoop =
            malloc(sizeof(union ConditionOrLoopSpec));
    struct LoopSpecExpression *newSpec = initialiseLoopSpecExpression();
    newConditionOrLoop->forLoopSpec = newSpec;
    node->conditional = newConditionOrLoop;
}

void addInitialWhileConditionToNode(struct Node *node)
{
    union ConditionOrLoopSpec *newConditionOrLoop =
            malloc(sizeof(union ConditionOrLoopSpec));
    struct Expression *newCondition = malloc(sizeof(struct Expression));
    newConditionOrLoop->conditional = newCondition;
    node->conditional = newConditionOrLoop;
}

void addPlainExpressionToNode(struct Node *node)
{
    union ConditionOrLoopSpec *newContainedExpression =
            malloc(sizeof(union ConditionOrLoopSpec));
    node->conditional = newContainedExpression;
}

char getCurrentConstructCode(struct GenericStack *stack)
{
    struct Node *current = (struct Node*) stack->peek(stack);
    return current->constructCode;
}

struct GenericStack *getCurrentExpressionGroup(struct GenericStack *stack)
{
    struct Node *current = (struct Node*) stack->peek(stack);
    return current->expressionGroup;
}

struct Node *getCurrentNode(struct GenericStack *stack)
{
    return (struct Node*) stack->peek(stack);
}

union ConditionOrLoopSpec *getCurrentConditional(struct GenericStack *stack)
{
    struct Node *current = (struct Node*) stack->peek(stack);
    return current->conditional;
}

void pushMathNodeWhenEncounteringPlusSymbol(struct GenericStack *currentWorkingNode,
                                       struct Expression *mathExp,
                                       int *depthOfMathStructure)
{
    struct Node *math;
    struct Node *newChild;
    if (getCurrentConstructCode(currentWorkingNode) == 't' ||
            getCurrentConstructCode(currentWorkingNode) == 'b')
    {
        newChild = initialiseNode('+');
        pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
        currentWorkingNode->push(newChild, currentWorkingNode);
        math = initialiseNode('m');
        addPlainExpressionToNode(math);
        math->conditional->containedExpression = mathExp;
        pushNodeIntoExpressionGroup(currentWorkingNode, math);
        (*depthOfMathStructure)++;
    }
    else if (getCurrentConstructCode(currentWorkingNode) == '*')
    {
        math = initialiseNode('m');
        addPlainExpressionToNode(math);
        math->conditional->containedExpression = mathExp;
        pushNodeIntoExpressionGroup(currentWorkingNode, math);
        currentWorkingNode->pop(currentWorkingNode);
        (*depthOfMathStructure)--;
    }
    else if (getCurrentConstructCode(currentWorkingNode) == '+')
    {
        math = initialiseNode('m');
        addPlainExpressionToNode(math);
        math->conditional->containedExpression = mathExp;
        pushNodeIntoExpressionGroup(currentWorkingNode, math);
    }
}

void pushMathNodeWhenEncounteringMultiplySymbol(
    struct GenericStack *currentWorkingNode, struct Expression *mathExp,
    int *depthOfMathStructure)
{
    struct Node *math;
    struct Node *newChild;
    if (getCurrentConstructCode(currentWorkingNode) == 't' ||
            getCurrentConstructCode(currentWorkingNode) == 'b')
    {
        // By default we have a simple two level tree for non-bracketed series
        // of multiplications and additions
        newChild = initialiseNode('+');
        pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
        currentWorkingNode->push(newChild, currentWorkingNode);
        newChild = initialiseNode('*');
        pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
        currentWorkingNode->push(newChild, currentWorkingNode);
        *depthOfMathStructure = (*depthOfMathStructure) + 2;
    }
    else if (getCurrentConstructCode(currentWorkingNode) == '+')
    {
        newChild = initialiseNode('*');
        pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
        currentWorkingNode->push(newChild, currentWorkingNode);
        (*depthOfMathStructure)++;
    }
    math = initialiseNode('m');
    addPlainExpressionToNode(math);
    math->conditional->containedExpression = mathExp;
    pushNodeIntoExpressionGroup(currentWorkingNode, math);
}

void pushWhileNode(struct GenericStack *currentWorkingNode)
{
    struct Node *newChild = initialiseNode('w');
    pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
    currentWorkingNode->push(newChild, currentWorkingNode);
    addInitialWhileConditionToNode(newChild);
}

void pushForNode(struct GenericStack *currentWorkingNode)
{
    struct Node *newChild = initialiseNode('f');
    pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
    currentWorkingNode->push(newChild, currentWorkingNode);
    addInitialLoopSpecToNode(newChild);
}

void pushAssignmentNode(struct GenericStack *currentWorkingNode,
                                        struct Expression *expression,
                                        int *depthOfMathStructure)
{
    struct Node *newChild = initialiseNode('=');
    addPlainExpressionToNode(newChild);
    newChild->conditional->containedExpression = expression;
    pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
    currentWorkingNode->push(newChild, currentWorkingNode);
    (*depthOfMathStructure)++;
}

void pushMainMathNode(struct GenericStack *currentWorkingNode,
                                      int *depthOfMathStructure)
{
    struct Node *newChild = initialiseNode('t');
    pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
    currentWorkingNode->push(newChild, currentWorkingNode);
    (*depthOfMathStructure)++;
}

void pushBracketNode(struct GenericStack *currentWorkingNode,
                                     int *depthOfMathStructure)
{
    struct Node *newChild = initialiseNode('b');
    pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
    currentWorkingNode->push(newChild, currentWorkingNode);
    (*depthOfMathStructure)++;
}

void deallocateConditionOrLoopSpec(char constructCode, union ConditionOrLoopSpec *condition)
{
    if (condition == NULL)
        return;
    if (constructCode == 'f')
    {
        if (condition->forLoopSpec->incrementLoop != NULL)
        {
            free(condition->forLoopSpec->incrementLoop->expression);
            condition->forLoopSpec->incrementLoop->expression = NULL;
        }
        if (condition->forLoopSpec->limitLoop != NULL)
        {
            free(condition->forLoopSpec->limitLoop->expression);
            condition->forLoopSpec->incrementLoop->expression = NULL;
        }
        if (condition->forLoopSpec->initialiseLoop != NULL)
        {
            free(condition->forLoopSpec->initialiseLoop->expression);
            condition->forLoopSpec->initialiseLoop->expression = NULL;
        }

        free(condition->forLoopSpec->incrementLoop);
        condition->forLoopSpec->incrementLoop = NULL;
        free(condition->forLoopSpec->incrementLoop);
        condition->forLoopSpec->limitLoop = NULL;
        free(condition->forLoopSpec->incrementLoop);
        condition->forLoopSpec->initialiseLoop = NULL;

        free(condition->forLoopSpec);
        condition->forLoopSpec = NULL;

        free(condition);

    }
    else if (constructCode == 'w')
    {
        free(condition->conditional->expression);
        condition->conditional->expression = NULL;

        free(condition->conditional);
        condition->conditional = NULL;

        free(condition);

    }
    else if (constructCode == '=' || constructCode == 'm')
    {
        free(condition->containedExpression->expression);
        condition->containedExpression->expression = NULL;

        free(condition->containedExpression);
        condition->containedExpression = NULL;

        free(condition);
    }

}

void deallocateParseTree(struct Node *root)
{
    struct GenericStack *currentWorkingNode = createStackGrammar();
    currentWorkingNode->push(root, currentWorkingNode);
    while(1)
    {
        if (getCurrentExpressionGroup(currentWorkingNode) == NULL)
        {
            if (getCurrentConditional(currentWorkingNode) != NULL)
            {
                deallocateConditionOrLoopSpec(getCurrentConstructCode(currentWorkingNode), getCurrentConditional(currentWorkingNode));
                getCurrentNode(currentWorkingNode)->conditional = NULL;
            }

            free(getCurrentNode(currentWorkingNode));
            currentWorkingNode->pop(currentWorkingNode);
            /*
             The following check terminates the loop because the last node has already been deallocated
             the last node, the root node. Due to the way the stack works, the top of the stack refers to
             the same node after the previous pop. Specifically when next is NULL, then the last pop action
             hasn't changed the effective top of the stack (based on how the stack utility I wrote works)
            */
            if (getCurrentNode(currentWorkingNode) == NULL)
                break;

            getCurrentNode(currentWorkingNode)->expressionGroup->
            pop(getCurrentNode(currentWorkingNode)->expressionGroup);
            // Similar thing applies here
            if (getCurrentNode(getCurrentExpressionGroup(currentWorkingNode)) == NULL)
            {
                free(getCurrentExpressionGroup(currentWorkingNode)->stackTop);
                free(getCurrentExpressionGroup(currentWorkingNode));
                getCurrentNode(currentWorkingNode)->expressionGroup = NULL;
            }
        }
        else
        {
            currentWorkingNode->push(getCurrentNode(getCurrentExpressionGroup(currentWorkingNode)), currentWorkingNode);
        }
    }
    free(currentWorkingNode->stackTop);
    free(currentWorkingNode);
};

enum ParserContext
{
    LEFT_ASSIGNMENT,
    NORMAL,
    STATEMENT_NORMAL,
    FOUND_INITIALISE_LOOP,
    FOUND_LIMIT_LOOP,
    FOUND_INCREMENT_LOOP,
    FOUND_LOOP
};

/*
  Basic parser using a Node stack instead of the implicit function call stack
  (can swap over as a refactoring challenge sometime)
*/
struct Node *parseCode(struct Expression **lexedContent, int numberOfTokens)
{
    struct Node *newChild;
    struct Node *initialNode = initialiseNode('r');
    initialNode->expressionGroup = createStackGrammar();
    struct GenericStack *currentWorkingNode = createStackGrammar();
    currentWorkingNode->push(initialNode, currentWorkingNode);
    int needToDeallocateCurrentExpression = 0;
    int encounteredInvalidToken = 0;
    int depthOfMathStructure = 0;
    int i = 0;
    char currentState = 'b';
    enum ParserContext currentContext = NORMAL;
    while (i < numberOfTokens)
    {
        printf("Item: %c\n", *(lexedContent[i]->expression));
        if (currentContext == NORMAL && currentState == 'f' &&
                *(lexedContent[i]->expression) != '(')
        {
            // Error state for when a bracket does not follow a for 'f' symbol as
            // expected
            // TODO: I forgot that the lexer might actually deal with this already
            encounteredInvalidToken = 1;
            break;
        }
        else if (currentContext == NORMAL && currentState == 'w' &&
                 *(lexedContent[i]->expression) != '(')
        {
            // Error state for when a bracket does not follow a while 'w' symbol as
            // expected
            encounteredInvalidToken = 1;
            break;
        }
        else if (currentContext == LEFT_ASSIGNMENT &&
                 *(lexedContent[i]->expression) == '{')
        {
            encounteredInvalidToken = 1;
            break;
        }
        else if (currentContext == LEFT_ASSIGNMENT &&
                 *(lexedContent[i]->expression) == ',')
        {
            encounteredInvalidToken = 1;
            break;
        }
        else if (currentContext == LEFT_ASSIGNMENT &&
                 *(lexedContent[i]->expression) == 'w')
        {
            encounteredInvalidToken = 1;
            break;
        }
        else if (currentContext == LEFT_ASSIGNMENT &&
                 *(lexedContent[i]->expression) == 'f')
        {
            encounteredInvalidToken = 1;
            break;
        }
        else if (currentContext == LEFT_ASSIGNMENT &&
                 *(lexedContent[i]->expression) == '}')
        {
            encounteredInvalidToken = 1;
            break;
        }
        else if (currentContext == FOUND_INITIALISE_LOOP &&
                 *(lexedContent[i]->expression) == ')')
        {
            encounteredInvalidToken = 1;
            break;
        }
        else if (currentContext == FOUND_LIMIT_LOOP &&
                 *(lexedContent[i]->expression) == ')')
        {
            encounteredInvalidToken = 1;
            break;
        }
        else if (*(lexedContent[i]->expression) == '{')
        {
            currentContext = STATEMENT_NORMAL;
            needToDeallocateCurrentExpression = 1;
        }
        else if (*(lexedContent[i]->expression) == ',')
        {
            needToDeallocateCurrentExpression = 1;
        }
        else if (*(lexedContent[i]->expression) == 'w')
        {
            currentState = 'w';
            currentContext = NORMAL;
            needToDeallocateCurrentExpression = 1;
        }
        else if (*(lexedContent[i]->expression) == 'f')
        {
            currentState = 'f';
            needToDeallocateCurrentExpression = 1;
        }
        else if (*(lexedContent[i]->expression) == 'e')
        {
            currentState = 'e';
            needToDeallocateCurrentExpression = 1;
        }
        else if (*(lexedContent[i]->expression) == '}')
        {
            currentWorkingNode->pop(currentWorkingNode);
            needToDeallocateCurrentExpression = 1;
        }
        else if (*(lexedContent[i]->expression) == '(' &&
                 currentContext == NORMAL && currentState == 'w')
        {
            pushWhileNode(currentWorkingNode);
            currentContext = FOUND_LOOP;
            needToDeallocateCurrentExpression = 1;
        }
        else if (currentContext == FOUND_LOOP &&
                 *(lexedContent[i]->expression) == ')')
        {
            currentContext = NORMAL;
            currentState = 'b';
            needToDeallocateCurrentExpression = 1;
        }
        else if (currentContext == FOUND_INCREMENT_LOOP &&
                 *(lexedContent[i]->expression) == ')')
        {
            currentContext = NORMAL;
            currentState = 'b';
            needToDeallocateCurrentExpression = 1;
        }
        else if (currentContext == FOUND_LOOP && currentState == 'f')
        {
            ((struct GrammarStack*)currentWorkingNode->stackTop)->next->node->conditional->forLoopSpec->initialiseLoop =
                lexedContent[i];
            currentContext = FOUND_INITIALISE_LOOP;
        }
        else if (currentContext == FOUND_INITIALISE_LOOP &&
                 currentState == 'f')
        {
            ((struct GrammarStack*)currentWorkingNode->stackTop)->next->node->conditional->forLoopSpec->limitLoop =
                lexedContent[i];
            currentContext = FOUND_LIMIT_LOOP;
        }
        else if (currentContext == FOUND_LIMIT_LOOP &&
                 currentState == 'f')
        {
            ((struct GrammarStack*)currentWorkingNode->stackTop)->next->node->conditional->forLoopSpec->incrementLoop =
                lexedContent[i];
            currentContext = FOUND_INCREMENT_LOOP;
        }
        else if (*(lexedContent[i]->expression) == '(' && currentState == 'f')
        {
            pushForNode(currentWorkingNode);
            currentContext = FOUND_LOOP;
        }
        else if (currentContext == STATEMENT_NORMAL &&
                 *(lexedContent[i]->expression) != '=')
        {
            pushAssignmentNode(currentWorkingNode, lexedContent[i], &depthOfMathStructure);
        }
        else if (currentContext == STATEMENT_NORMAL &&
                 *(lexedContent[i]->expression) == '=')
        {
            currentContext = LEFT_ASSIGNMENT;
            pushMainMathNode(currentWorkingNode, &depthOfMathStructure);
            needToDeallocateCurrentExpression = 1;
        }
        else if (currentContext == LEFT_ASSIGNMENT &&
                 (*(lexedContent[i + 1]->expression) == '+' ||
                  *(lexedContent[i + 1]->expression) == ';'))
        {
            pushMathNodeWhenEncounteringPlusSymbol(currentWorkingNode, lexedContent[i], &depthOfMathStructure);
        }
        else if (currentContext == LEFT_ASSIGNMENT &&
                 (*(lexedContent[i + 1]->expression) == '*' ||
                  *(lexedContent[i + 1]->expression) == ';'))
        {
            pushMathNodeWhenEncounteringMultiplySymbol(
                                     currentWorkingNode, lexedContent[i], &depthOfMathStructure);
        }
        else if (*(lexedContent[i]->expression) == ';')
        {
            while (depthOfMathStructure > 0)
            {
                currentWorkingNode->pop(currentWorkingNode);
                depthOfMathStructure--;
            }
            currentContext = STATEMENT_NORMAL;
            needToDeallocateCurrentExpression = 1;
        }
        else if (currentContext == LEFT_ASSIGNMENT &&
                 *(lexedContent[i]->expression) == '(')
        {
            // 'b' indicates a node for brackets
            pushBracketNode(currentWorkingNode, &depthOfMathStructure);
            needToDeallocateCurrentExpression = 1;
        }
        else if (currentContext == LEFT_ASSIGNMENT &&
                 *(lexedContent[i]->expression) == ')')
        {
            // 'b' indicates a node for brackets
            currentWorkingNode->pop(currentWorkingNode);
            depthOfMathStructure--;
            needToDeallocateCurrentExpression = 1;
        }
        else if (*(lexedContent[i]->expression) == '*')
        {
            currentState = '*';
            needToDeallocateCurrentExpression = 1;
        }
        else if (*(lexedContent[i]->expression) == '+')
        {
            currentState = '+';
            needToDeallocateCurrentExpression = 1;
        }
        // De-allocate expression structs that will not be maintained past this
        // phase
        if (needToDeallocateCurrentExpression)
        {
            deAllocateExpressionMemory(lexedContent[i]);
            needToDeallocateCurrentExpression = 0;
        }
        i++;
    }

    if (encounteredInvalidToken)
    {
        printf("HERE");
        // De allocate remaining tokens and submit an error in parsing
        // There may be instances where these two operations both try to deallocate lexedContent[i]
        deallocateParseTree(initialNode);
        while (i < numberOfTokens)
        {
            deAllocateExpressionMemory(lexedContent[i]);
            i++;
        }

    }

    printf("Root Node: %c\n",
           initialNode->constructCode);
   printf("Child node last appearing: %c\n",
   getCurrentConstructCode(initialNode->expressionGroup));
    printf("Next Child node last appearing: %c\n",
   getCurrentConstructCode(getCurrentExpressionGroup(initialNode->expressionGroup)));
   printf("Next Child node last appearing: %c\n",
   getCurrentConstructCode(getCurrentExpressionGroup(getCurrentExpressionGroup(initialNode->expressionGroup))));
/*
    printf("Root Node: %c\n",
           initialNode);
    printf("initialise %c\n",
           initialNode->expressionGroup->next->node->conditional->forLoopSpec
               ->initialiseLoop->expression[0]);
    printf("limit %c\n", initialNode->expressionGroup->next->node->conditional
                             ->forLoopSpec->limitLoop->expression[0]);
    printf("initial %c\n", initialNode->expressionGroup->next->node->conditional
                               ->forLoopSpec->incrementLoop->expression[0]);
    printf("Child Node While: %c\n",
           initialNode->expressionGroup->next->node->expressionGroup->next->node
               ->constructCode);
    printf("Next Child Node While: %c\n",
           initialNode->expressionGroup->next->node->expressionGroup->next->node
               ->expressionGroup->next->node->constructCode);
    printf("Next Child Node math: %c\n",
           *(initialNode->expressionGroup->next->node->expressionGroup->next->node
                 ->expressionGroup->next->next->node->expressionGroup->next->node
                 ->expressionGroup->next->node->expressionGroup->next->node
                 ->expressionGroup->next->node->conditional->containedExpression
                 ->expression));
    printf("Next Child Node Expression: %c\n",
           initialNode->expressionGroup->next->node->expressionGroup->next->node
               ->expressionGroup->next->next->node->conditional
               ->containedExpression->expression[0]);
    printf("Next Child Node for: %c\n",
           initialNode->expressionGroup->next->node->expressionGroup->next->node
               ->expressionGroup->next->node->expressionGroup->next->node
               ->constructCode);
    printf("Next Child Node while: %c\n",
           initialNode->expressionGroup->next->node->expressionGroup->next->node
               ->expressionGroup->next->node->expressionGroup->next->next->node
               ->constructCode);
*/
    return initialNode;
}
