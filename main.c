#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*

Given that this is about parsing a stream of text
I might as well start writing the code for the
next thing here. The next thing being a program
to inspect the normal form of a boolean expression and finding a way to minimise it.

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

struct inversionStruct {
  int isInverted;
};

struct linkedInversionArray {
  struct inversionStruct inversion;
  struct linkedInversionArray *next;
};

struct termArray {
  struct linkedInversionArray inversionArray;
  struct termArray *next;
};

// Generate an array of integers indicating the reduction in how far we can skip
// ahead when matching a string in some text
int *generatePatternSkipData(char *pattern, int sizeOfPattern) {
  int i = 0;
  int j = 1;
  int *patternSkipReductionData = malloc(sizeOfPattern * sizeof(int));
  patternSkipReductionData[0] = 0;
  while (j < sizeOfPattern) {
    if (pattern[i] != pattern[j]) {
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
int getAppropriateSkipReduction(int *patternSkipReductionData, int index) {
  while (patternSkipReductionData[index] == 0 && index >= 0) {
    index--;
  }
  return index;
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
int searchText(char *text, char *pattern, int sizeOfText, int sizeOfPattern) {
  int i = 0;
  int j = 0;
  int *patternSkipReductionData =
      generatePatternSkipData(pattern, sizeOfPattern);
  while ((i < sizeOfText) && (j < sizeOfPattern)) {
    if (text[i] != pattern[j]) {
      int lastIndexWithNonZeroSkipReduction =
          getAppropriateSkipReduction(patternSkipReductionData, j);
      if (patternSkipReductionData[j] != 0 ||
          j - lastIndexWithNonZeroSkipReduction > 1) {
        j = 0;
      } else if (j != 0) {
        j = patternSkipReductionData[j - 1] + 1;
      } else {
        i++;
      }
    } else {
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
  Structure for a linked list of rows
*/
struct RowList {
  int rowIndex;
  struct Row *row;
  struct RowList *rowList;
};

/*
  Struct for columns along the row
*/
struct Row {
  int columnIndex;
  int nextSymbol;
  struct Row *next;
};

/*
  Packaged struct including the automaton table and the known final state for
  use in searching
*/
struct RegexData {
  struct RowList *regexTable;
  int finalState;
};

struct LinkedString {
  char curr;
  struct LinkedString *next;
};

/*
  Create a row with a particular rowIndex
*/
struct RowList *createRowList(int rowIndex) {
  struct RowList *first = malloc(sizeof(struct RowList));
  first->rowIndex = rowIndex;
  first->rowList = NULL;
  return first;
}

/*
  Create an initial row
*/
struct RowList *createRowListFirst() {
  struct RowList *first = malloc(sizeof(struct RowList));
  first->rowIndex = 0;
  first->rowList = NULL;
  return first;
}

/*
  Create a column entry with a particular columnIndex
*/
struct Row *createRow(int columnIndex) {
  struct Row *first = malloc(sizeof(struct Row));
  first->columnIndex = columnIndex;
  first->next = NULL;
  return first;
}

/*
  Create an initial column entry
*/
struct Row *createRowFirst() {
  struct Row *first = malloc(sizeof(struct Row));
  first->columnIndex = 0;
  first->next = NULL;
  return first;
}

/*
  Inserting an item in a row at the correct position (based on column index and
  assuming a node does not exists for this columnIndex already)
*/
struct Row *insertItemAtAppropriateRowPosition(int nextSymbol, int columnIndex,
                                               struct Row *row) {
  struct Row *previous = row;
  struct Row *tracker = row;

  if (row == NULL) {
    struct Row *newRow = createRow(columnIndex);
    newRow->nextSymbol = nextSymbol;
    return newRow;
  }

  /*
   Iterate through keeping track of the current node and the previous
   stop as soon as the tracker's columnIndex stops being less then the
   supplied columnIndex
  */
  while (tracker != NULL && tracker->columnIndex < columnIndex) {
    previous = tracker;
    tracker = tracker->next;
  }

  /*
   We have now found the right place to insert, we need to create a new column
   entry were the previous should now point at the new entry, and the new
   entry should point where the previous was pointing before we inserted
  */
  struct Row *newRow = createRow(columnIndex);
  newRow->nextSymbol = nextSymbol;
  newRow->next = tracker;
  previous->next = newRow;
  return row;
}

/*
  Insert an item into the table based on the desired indices
*/
struct RowList *insertAtPlace(int nextSymbol, int columnIndex, int rowIndex,
                              struct RowList *table) {
  struct RowList *rowTracker = table;
  struct Row *columnTracker;
  if (table != NULL) {
    while (rowTracker != NULL) {
      // Maybe find a way of dealing with the situation where there is initially
      // no row within this function instead
      columnTracker = rowTracker->row;
      while (columnTracker != NULL) {
        if (columnTracker->columnIndex == columnIndex &&
            rowTracker->rowIndex == rowIndex) {
          // Found the node with the right indices, set the value here
          columnTracker->nextSymbol = nextSymbol;
          return table;
        }
        columnTracker = columnTracker->next;
      }
      rowTracker = rowTracker->rowList;
    }
    rowTracker = table;
    while (rowTracker != NULL) {
      if (rowTracker->rowIndex == rowIndex) {
        // Here we insert the value in sorted order since we have not found an
        // existing node for the row and column indices
        insertItemAtAppropriateRowPosition(nextSymbol, columnIndex,
                                           rowTracker->row);
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

int getItemAtPlace(int columnIndex, int rowIndex, struct RowList *table) {
  struct RowList *rowTracker = table;
  struct Row *columnTracker;
  if (table != NULL) {
    while (rowTracker != NULL) {
      if (rowTracker->rowIndex == rowIndex) {
        columnTracker = rowTracker->row;
        while (columnTracker != NULL) {
          if (columnTracker->columnIndex == columnIndex) {
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

struct LinkedString *push(char character, struct LinkedString *first) {
  first->curr = character;
  struct LinkedString *newCharacter = malloc(sizeof(struct LinkedString));
  newCharacter->next = first;
  return newCharacter;
}

struct LinkedString *pop(struct LinkedString *first,
                         char *addressForPoppedElement) {
  if (first->next != NULL) {
    *addressForPoppedElement = first->next->curr;
    struct LinkedString *result = first->next;
    free(first);
    first = NULL;
    return result;
  } else {
    *addressForPoppedElement = first->curr;
    return first;
  }
}

/*
  Find the first index of a matching string in the supplied string using the
  regex
*/
int findMatch(char *array, int startIndex, int endIndex,
              struct RegexData *regex) {
  int offset = startIndex;
  int currentState = 0;
  int previousState = 0;
  int relativeIndex = 0;

  while ((offset + relativeIndex) < endIndex) {
    previousState = currentState;
    currentState = getItemAtPlace(
        currentState, (int)array[offset + relativeIndex], regex->regexTable);
    if (currentState == regex->finalState)
      break;
    relativeIndex++;
    if (previousState != 0 && currentState == 0) {
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
                      struct RegexData *regex) {
  int trackingIndex = startIndex;
  int currentState = 0;
  while (trackingIndex < endIndex) {
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
struct RegexData *createBasicAlphabetStringMatcher() {
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

  while (i < finalSymbol) {
    if ((i >= startOfCapitals && i <= endOfCapitals) ||
        (i >= startOfLowerCase && i <= endOfLowerCase)) {
      table = insertAtPlace(1, 0, i, table);
    } else {
      table = insertAtPlace(0, 0, i, table);
    }
    i++;
  }
  i = startSymbol;
  while (i < finalSymbol) {
    if ((i >= startOfCapitals && i <= endOfCapitals) ||
        (i >= startOfLowerCase && i <= endOfLowerCase)) {
      table = insertAtPlace(1, 1, i, table);
    } else {
      table = insertAtPlace(2, 1, i, table);
    }
    i++;
  }
  regexData->regexTable = table;
  regexData->finalState = 2;
  return regexData;
}

struct LinkedString *createStack() {
  struct LinkedString *first = malloc(sizeof(struct LinkedString));
  first->next = NULL;
  return first;
}

struct Expression {
  int ordering;
  int length;
  char *expression;
};

void deAllocateExpressionMemory(struct Expression *expression) {
  free(expression->expression);
  free(expression);
  expression->expression = NULL;
  expression = NULL;
}

struct Expression **tokenizeNew(char *array, int size, int *numberTokens);
struct Node *parseCode(struct Expression **lexedContent, int numberOfTokens);

int main(int argc, char *argv[]) {
  FILE *ptr1;
  FILE *ptr2;
  ptr1 = fopen(argv[1], "r");
  ptr2 = fopen(argv[2], "r");
  char currentChar;
  int size1 = 0;
  int size2 = 0;

  struct LinkedString *current1 = createStack();
  while ((currentChar = fgetc(ptr1)) != EOF) {
    size1++;
    current1 = push(currentChar, current1);
  }

  struct LinkedString *current2 = createStack();
  while ((currentChar = fgetc(ptr2)) != EOF) {
    size2++;
    current2 = push(currentChar, current2);
  }

  fclose(ptr1);
  char *string1 = malloc(size1 * sizeof(char));
  for (int i = size1 - 1; i >= 0; i--) {
    current1 = pop(current1, &(string1[i]));
  }
  char *string2 = malloc(size2 * sizeof(char));
  for (int i = size2 - 1; i >= 0; i--) {
    current2 = pop(current2, &(string2[i]));
  }
  free(current1);
  current1 = NULL;
  free(current2);
  current2 = NULL;

  int result = searchText(string1, string2, size1, size2);
  printf("result %d\n", result);

  int numberOfTokens;
  struct Expression **arrayExp = tokenizeNew(string1, size1, &numberOfTokens);
  // Need to free first the individual pointers in this block, then the entire
  // block
  struct Node *parseTree = parseCode(arrayExp, numberOfTokens);

  /*
  Stackoverflow answer for later perusal:

  "Yes, they are equivalent.

  Quoting C11, chapter §7.22.3.3, (emphasis mine)

  The free function causes the space pointed to by ptr to be deallocated, that
  is, made available for further allocation. If ptr is a null pointer, no action
  occurs. Otherwise, if the argument does not match a pointer earlier returned
  by a memory management function, or if the space has been deallocated by a
  call to free or realloc, the behavior is undefined.

  So, as long as you pass the same ptr value (the pointer itself or a copy of
  it) which was earlier returned by malloc() or family, you're good to go."
  */
  free(arrayExp);
  arrayExp = NULL;

  // Test out the sparse array
  // I think the new way I'm deallocating things is affecting this code, which
  // is expected To use it I will need separately allocated data so the previous
  // stuff cannot interfere
  struct RowList *table = createRowListFirst();
  table->row = createRowFirst();
  table = insertAtPlace(10, 1, 1, table);
  table = insertAtPlace(10, 1, 2, table);
  table = insertAtPlace(12, 1, 3, table);
  table = insertAtPlace(13, 2, 1, table);
  table = insertAtPlace(14, 2, 2, table);

  // Testing regex stuff
  struct RegexData *regex = createBasicAlphabetStringMatcher();
  int index = 32;
  int matchStart = findMatch(string2, 2, size2 - 1, regex);

  free(string1);
  free(string2);
  string1 = NULL;
  string2 = NULL;
}

struct LinkedStack {
  char character;
  struct Stack *nextPointer;
};

struct ExpressionStack {
  struct Expression *expression;
  struct ExpressionStack *next;
};

struct LoopSpecExpression {
  struct Expression *initialiseLoop;
  struct Expression *limitLoop;
  struct Expression *incrementLoop;
};

union ConditionOrLoopSpec {
  struct Expression *conditional;
  struct LoopSpecExpression *forLoopSpec;
  struct Expression *containedExpression;
};

struct Node {
  int ordering;
  char constructCode;
  struct GrammarStack *expressionGroup;
  union ConditionOrLoopSpec *conditional;
};

struct GrammarStack {
  struct Node *node;
  struct GrammarStack *next;
};

struct GrammarStack *pushG(struct Node *node, struct GrammarStack *first) {
  first->node = node;
  struct GrammarStack *newNode = malloc(sizeof(struct GrammarStack));
  newNode->next = first;
  return newNode;
}

struct GrammarStack *popG(struct GrammarStack *first) {
  if (first->next != NULL) {
    struct GrammarStack *result = first->next;
    free(first);
    first = NULL;
    return result;
  } else
    return first;
}

struct GrammarStack *createStackG() {
  struct GrammarStack *first = malloc(sizeof(struct GrammarStack));
  first->next = NULL;
  return first;
}

struct ExpressionStack *pushE(struct Expression *expression,
                              struct ExpressionStack *first) {
  first->expression = expression;
  struct ExpressionStack *newNode = malloc(sizeof(struct ExpressionStack));
  newNode->next = first;
  return newNode;
}

struct ExpressionStack *popE(struct ExpressionStack *first) {
  if (first->next != NULL) {
    struct ExpressionStack *result = first->next;
    free(first);
    first = NULL;
    return result;
  } else
    return first;
}

struct ExpressionStack *createStackE() {
  struct ExpressionStack *first = malloc(sizeof(struct ExpressionStack));
  first->next = NULL;
  return first;
}

struct GenericStack {
  void *stackTop;
  void *(*push)(void *, void *);
  void *(*pop)(void*);
};

struct Expression *allocateSingleCharExpression(char item) {
  struct Expression *newExpression = malloc(sizeof(struct Expression));
  char *newChar = malloc(sizeof(char));
  *newChar = item;
  newExpression->expression = newChar;
  newExpression->length = 1;
  return newExpression;
}

/*
  function for splitting a string into tokens
*/
struct Expression **tokenizeNew(char *array, int size, int *numberOfTokens) {
  struct ExpressionStack *first = createStackE();
  struct LinkedString *firstCharacterOnStore = createStack();
  int counter = 0;
  int tempCharacterCount = 0;
  int sizeOfExpressionArray = 0;
  while (counter < size) {
    if (array[counter] == '(' || array[counter] == ')' ||
        array[counter] == '}' || array[counter] == '{' ||
        (array[counter] == 'f' && array[counter + 1] == '(') ||
        (array[counter] == 'w' && array[counter + 1] == '(')) {
      first = pushE(allocateSingleCharExpression(array[counter]), first);
      sizeOfExpressionArray++;
    } else if (array[counter] != '(' && array[counter] != ')' &&
               array[counter] != ';' && array[counter] != 'f' &&
               array[counter] != 'w' && array[counter] != '+' &&
               array[counter] != ',' && array[counter] != '=') {
      firstCharacterOnStore = push(array[counter], firstCharacterOnStore);
      tempCharacterCount++;
    } else if (array[counter] == ';' || array[counter] == '+' ||
               array[counter] == '*' || array[counter] == '=' ||
               array[counter] == ',') {
      struct Expression *newExp = malloc(sizeof(struct Expression));
      char *charBlock = malloc(tempCharacterCount * sizeof(char));
      newExp->length = tempCharacterCount;
      while (tempCharacterCount > 0) {
        firstCharacterOnStore =
            pop(firstCharacterOnStore, charBlock + (tempCharacterCount - 1));
        tempCharacterCount--;
      }
      newExp->expression = charBlock;
      first = pushE(newExp, first);
      sizeOfExpressionArray++;
      // Separation within for loop is implicit with the splitting off into different
      // expressions
      if (array[counter] != ',') {
        first = pushE(allocateSingleCharExpression(array[counter]), first);
        sizeOfExpressionArray++;
      }
    }
    counter++;
  }
  struct Expression **result =
      malloc(sizeOfExpressionArray * sizeof(struct Expression *));
  int j = sizeOfExpressionArray - 1;
  do {
    result[j] = first->next->expression;
    first = popE(first);
    j--;
  } while (first->next != NULL);
  free(firstCharacterOnStore);
  firstCharacterOnStore = NULL;
  free(first);
  first = NULL;
  *numberOfTokens = sizeOfExpressionArray;
  return result;
};

struct LoopSpecExpression *
initialiseLoopSpecExpression(
    struct Expression *expressionInitialiseLoop,
    struct Expression *expressionIncrementLoop,
    struct Expression *expressionLimitLoop
    ) {
  struct LoopSpecExpression *newSpec =
      malloc(sizeof(struct LoopSpecExpression));
  newSpec->initialiseLoop = expressionInitialiseLoop;
  newSpec->incrementLoop = expressionIncrementLoop;
  newSpec->limitLoop = expressionLimitLoop;
  return newSpec;
}

void pushNodeIntoExpressionGroup(struct GrammarStack *currentWorkingNode,
                                 struct Node *nodeToPush) {
  if (currentWorkingNode->next != NULL) {
    currentWorkingNode->next->node->expressionGroup =
        pushG(nodeToPush, currentWorkingNode->next->node->expressionGroup);
  } else {
    currentWorkingNode->node->expressionGroup =
        pushG(nodeToPush, currentWorkingNode->node->expressionGroup);
  }
}

struct Node *initialiseNode(char currentState) {
  struct Node *newChild = malloc(sizeof(struct Node));
  newChild->constructCode = currentState;
  newChild->expressionGroup = createStackG();
  return newChild;
};

void addInitialLoopSpecToNode(struct Node *node,
                              struct Expression **expression) {
  union ConditionOrLoopSpec *newConditionOrLoop =
      malloc(sizeof(union ConditionOrLoopSpec));
  struct LoopSpecExpression *newSpec =
      initialiseLoopSpecExpression(expression[1], expression[2], expression[3]);
  newConditionOrLoop->forLoopSpec = newSpec;
  node->conditional = newConditionOrLoop;
}

void addInitialWhileConditionToNode(struct Node *node) {
  union ConditionOrLoopSpec *newConditionOrLoop =
      malloc(sizeof(union ConditionOrLoopSpec));
  struct Expression *newCondition = malloc(sizeof(struct Expression));
  newConditionOrLoop->conditional = newCondition;
  node->conditional = newConditionOrLoop;
}

void addPlainExpressionToNode(struct Node *node) {
  union ConditionOrLoopSpec *newContainedExpression =
      malloc(sizeof(union ConditionOrLoopSpec));
  node->conditional = newContainedExpression;
}

char getCurrentConstructCode(struct GrammarStack *first) {
  if (first->next != NULL) {
    return first->next->node->constructCode;
  } else {
    return first->node->constructCode;
  }
}

struct GrammarStack *pushMathNodeWhenEncounteringPlusSymbol(struct GrammarStack *currentWorkingNode, struct Expression *mathExp, int *depthOfMathStructure) {
  struct Node *math;
  struct Node *newChild;
  if (getCurrentConstructCode(currentWorkingNode) == 'm' ||
      getCurrentConstructCode(currentWorkingNode) == 'b') {
    newChild = initialiseNode('+');
    pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
    currentWorkingNode = pushG(newChild, currentWorkingNode);
    math = initialiseNode('m');
    addPlainExpressionToNode(math);
    math->conditional->containedExpression = mathExp;
    pushNodeIntoExpressionGroup(currentWorkingNode, math);
    (*depthOfMathStructure)++;
  } else if (getCurrentConstructCode(currentWorkingNode) == '*') {
    math = initialiseNode('m');
    addPlainExpressionToNode(math);
    math->conditional->containedExpression = mathExp;
    pushNodeIntoExpressionGroup(currentWorkingNode, math);
    currentWorkingNode = popG(currentWorkingNode);
    (*depthOfMathStructure)--;
  } else if (getCurrentConstructCode(currentWorkingNode) == '+') {
    math = initialiseNode('m');
    addPlainExpressionToNode(math);
    math->conditional->containedExpression = mathExp;
    pushNodeIntoExpressionGroup(currentWorkingNode, math);
  }
  return currentWorkingNode;
}

struct GrammarStack *pushMathNodeWhenEncounteringMultiplySymbol(struct GrammarStack *currentWorkingNode, struct Expression *mathExp, int *depthOfMathStructure) {
  struct Node *math;
  struct Node *newChild;
  if (getCurrentConstructCode(currentWorkingNode) == 'm' ||
      getCurrentConstructCode(currentWorkingNode) == 'b') {
    // By default we have a simple two level tree for non-bracketed series
    // of multiplications and additions
    newChild = initialiseNode('+');
    pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
    currentWorkingNode = pushG(newChild, currentWorkingNode);
    newChild = initialiseNode('*');
    pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
    currentWorkingNode = pushG(newChild, currentWorkingNode);
    *depthOfMathStructure = (*depthOfMathStructure) + 2;
  } else if (getCurrentConstructCode(currentWorkingNode) == '+') {
    newChild = initialiseNode('*');
    pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
    currentWorkingNode = pushG(newChild, currentWorkingNode);
    (*depthOfMathStructure)++;
  }
  math = initialiseNode('m');
  addPlainExpressionToNode(math);
  math->conditional->containedExpression = mathExp;
  pushNodeIntoExpressionGroup(currentWorkingNode, math);
  return currentWorkingNode;
}

struct GrammarStack *pushWhileNode(struct GrammarStack *currentWorkingNode) {
  struct Node *newChild = initialiseNode('w');
  pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
  currentWorkingNode = pushG(newChild, currentWorkingNode);
  addInitialWhileConditionToNode(newChild);
  return currentWorkingNode;
}

struct GrammarStack *pushForNode(struct GrammarStack *currentWorkingNode, struct Expression **expressions) {
  struct Node *newChild = initialiseNode('f');
  pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
  currentWorkingNode = pushG(newChild, currentWorkingNode);
  addInitialLoopSpecToNode(newChild, expressions);
  return currentWorkingNode;
}

struct GrammarStack *pushAssignmentNode(struct GrammarStack *currentWorkingNode, struct Expression *expression, int *depthOfMathStructure) {
  struct Node *newChild = initialiseNode('=');
  addPlainExpressionToNode(newChild);
  newChild->conditional->containedExpression = expression;
  pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
  currentWorkingNode = pushG(newChild, currentWorkingNode);
  (*depthOfMathStructure)++;
  return currentWorkingNode;
}

struct GrammarStack *pushMainMathNode(struct GrammarStack *currentWorkingNode, int *depthOfMathStructure) {
  struct Node *newChild = initialiseNode('m');
  pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
  currentWorkingNode = pushG(newChild, currentWorkingNode);
  (*depthOfMathStructure)++;
  return currentWorkingNode;
}

struct GrammarStack *pushBracketNode(struct GrammarStack *currentWorkingNode, int *depthOfMathStructure) {
  struct Node *newChild = initialiseNode('b');
  pushNodeIntoExpressionGroup(currentWorkingNode, newChild);
  currentWorkingNode = pushG(newChild, currentWorkingNode);
  (*depthOfMathStructure)++;
  return currentWorkingNode;
}

/*
  Basic parser using a Node stack instead of the implicit function call stack
  (can swap over as a refactoring challenge sometime)
*/
struct Node *parseCode(struct Expression **lexedContent, int numberOfTokens) {
  struct Node *newChild;
  struct Node *initialNode = initialiseNode('r');
  struct GrammarStack *currentWorkingNode = createStackG();
  currentWorkingNode = pushG(initialNode, currentWorkingNode);
  int needToDeallocateCurrentExpression = 0;
  int depthOfMathStructure = 0;
  int i = 0;
  char currentState = 'b';
  char currentContext = 'n';
  while (i < numberOfTokens) {
    printf("%c", *(lexedContent[i]->expression));
    if (*(lexedContent[i]->expression) == '{') {
      currentContext = 'i';
      needToDeallocateCurrentExpression = 1;
    } else if (*(lexedContent[i]->expression) == ',') {
      needToDeallocateCurrentExpression = 1;
    } else if (*(lexedContent[i]->expression) == 'w') {
      currentState = 'w';
      currentContext = 'n';
      needToDeallocateCurrentExpression = 1;
    } else if (*(lexedContent[i]->expression) == 'f') {
      currentState = 'f';
      needToDeallocateCurrentExpression = 1;
    } else if (*(lexedContent[i]->expression) == 'e') {
      currentState = 'e';
      needToDeallocateCurrentExpression = 1;
    } else if (*(lexedContent[i]->expression) == '}') {
      currentWorkingNode = popG(currentWorkingNode);
      needToDeallocateCurrentExpression = 1;
    } else if (*(lexedContent[i]->expression) == '(' && currentContext == 'n' &&
               currentState == 'w') {
      currentWorkingNode = pushWhileNode(currentWorkingNode);
      currentContext = 'e';
      needToDeallocateCurrentExpression = 1;
    } else if (*(lexedContent[i]->expression) == ')' && currentContext == 'e') {
      currentContext = 'n';
      needToDeallocateCurrentExpression = 1;
    } else if (currentContext == 'e' && currentState == 'w') {
      (currentWorkingNode->next->node->conditional)->conditional =
          lexedContent[i];
    } else if (*(lexedContent[i]->expression) == '(' && currentState == 'f') {
      currentWorkingNode = pushForNode(currentWorkingNode, lexedContent + i);
      currentContext = 'e';
      i += 3;
      needToDeallocateCurrentExpression = 1;
    } else if (currentContext == 'i' && *(lexedContent[i]->expression) != '=') {
      currentWorkingNode = pushAssignmentNode(currentWorkingNode, lexedContent[i], &depthOfMathStructure);
    } else if (currentContext == 'i' && *(lexedContent[i]->expression) == '=') {
      currentContext = 'm';
      currentWorkingNode = pushMainMathNode(currentWorkingNode, &depthOfMathStructure);
      needToDeallocateCurrentExpression = 1;
    } else if (currentContext == 'm' &&
               (*(lexedContent[i + 1]->expression) == '+' ||
                *(lexedContent[i + 1]->expression) == ';')) {
      currentWorkingNode = pushMathNodeWhenEncounteringPlusSymbol(currentWorkingNode, lexedContent[i], &depthOfMathStructure);
    } else if (currentContext == 'm' &&
               (*(lexedContent[i + 1]->expression) == '*' ||
                *(lexedContent[i + 1]->expression) == ';')) {
      currentWorkingNode = pushMathNodeWhenEncounteringMultiplySymbol(currentWorkingNode, lexedContent[i], &depthOfMathStructure);
    } else if (*(lexedContent[i]->expression) == ';') {
      while (depthOfMathStructure > 0) {
        currentWorkingNode = popG(currentWorkingNode);
        depthOfMathStructure--;
      }
      currentContext = 'e';
      needToDeallocateCurrentExpression = 1;
    } else if (currentContext == 'm' && *(lexedContent[i]->expression) == '(') {
      // 'b' indicates a node for brackets
      currentWorkingNode = pushBracketNode(currentWorkingNode, &depthOfMathStructure);
      needToDeallocateCurrentExpression = 1;
    } else if (currentContext == 'm' && *(lexedContent[i]->expression) == ')') {
      // 'b' indicates a node for brackets
      currentWorkingNode = popG(currentWorkingNode);
      depthOfMathStructure--;
      needToDeallocateCurrentExpression = 1;
    } else if (*(lexedContent[i]->expression) == '*') {
      needToDeallocateCurrentExpression = 1;
    } else if (*(lexedContent[i]->expression) == '+') {
      needToDeallocateCurrentExpression = 1;
    }

    // De-allocate expression structs that will not be maintained past this phase
    if (needToDeallocateCurrentExpression) {
      free(lexedContent[i]->expression);
      free(lexedContent[i]);
      lexedContent[i] = NULL;
      needToDeallocateCurrentExpression = 0;
    }
    i++;
  }
  printf("Root Node: %c\n",
         initialNode->expressionGroup->next->node->constructCode);
  printf("initialise %c\n", initialNode->expressionGroup->next->node->conditional
                     ->forLoopSpec->initialiseLoop->expression[0]);
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

  return initialNode;
}
