#ifndef STRINGSEARCHREGEX_H_INCLUDED
#define STRINGSEARCHREGEX_H_INCLUDED

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

// Generate an array of integers indicating the reduction in how far we can skip
// ahead when matching a string in some text
int *generatePatternSkipData(
    char *pattern,
    int sizeOfPattern
);

/*
  We can iterate backwards through the pattern skip reduction array to find
  if a prefix duplicate exists in the pattern
*/
int getAppropriateSkipReduction(
    int *patternSkipReductionData,
    int index
);

/*
  If there are no repetitions of a prefix to be found in the pattern then
  we can fully skip to the current index
  The only situation the necessitates not fully skipping is if there is a
  substring prefix all the way up to  j - 1 (but not including j as if we
  are in this clause then we know this doesn't match therefore that
  character of the prefix substring will not match when it first appears
  in the actual prefix not the duplicate)
*/
int searchText(
    char *text,
    char *pattern,
    int sizeOfText,
    int sizeOfPattern
);

/*
                           abcdefgouabcdefgouk
ilsdfisdlfiulsidufabcdefgouabcdefgoutfgrres
*/



/*
  Create a row with a particular rowIndex
*/
struct RowList *createRowList(
    int rowIndex
);

/*
  Create an initial row
*/
struct RowList *createRowListFirst();

/*
  Create a column entry with a particular columnIndex
*/
struct Row *createRow(
    int columnIndex
);

/*
  Create a column entry along with an entry in one go
*/
struct Row *createRowWithEntry(
    int columnIndex,
    int nextSymbol
);

/*
  Create an initial column entry
*/
struct Row *createRowFirst();

/*
  Inserting an item in a row at the correct position (based on column index and
  assuming a node does not exist for this columnIndex already)
*/
struct Row *insertItemAtAppropriateRowPosition(
    int nextSymbol,
    int columnIndex,
    struct Row *row
);

/*
  Insert an item into the table based on the desired indices
*/
struct RowList *insertAtPlace(
    int nextSymbol,
    int columnIndex,
    int rowIndex,
    struct RowList *table
);

/*
  Find a value in the specified place in the table
*/
int getItemAtPlace(
    int columnIndex,
    int rowIndex,
    struct RowList *table
);

/*
  Delete an item at specified position
*/
struct Row *deleteItemAtAppropriateRowPosition(
    int columnIndex,
    struct Row *row
);

/*
  Delete the entry at the specified row and column
*/
void deleteAtPlace(
    int columnIndex,
    int rowIndex,
    struct RowList *table
);

/*
  Find the first index of a matching string in the supplied string using the
  regular expression
*/
int findMatch(
    char *array,
    int startIndex,
    int endIndex,
    struct RegexData *regex
);

/*
  Find the last index of a matching string within another string, using the
  Regex data
*/
int findMatchEndIndex(
    char *array,
    int startIndex,
    int endIndex,
    struct RegexData *regex
);

/*
  Creates a basic character string regular expression matcher
*/
struct RegexData *createBasicAlphabetStringMatcher();

#endif // STRINGSEARCHREGEX_H_INCLUDED
