#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "StringSearchRegex.h"

int *generatePatternSkipData(char *pattern, int sizeOfPattern)
{
    int i = 0;
    int j = 1;
    int *patternSkipReductionData = malloc(sizeOfPattern * sizeof(int));
    patternSkipReductionData[0] = 0;
    while (j < sizeOfPattern)
    {
        if (pattern[i] != pattern[j]) i = 0;
        patternSkipReductionData[j++] = i++;
    }
    return patternSkipReductionData;
}

int getAppropriateSkipReduction(int *patternSkipReductionData, int index)
{
    int currentIndex = index;

    while (patternSkipReductionData[index] == 0 && index >= 0) index--;

    if (currentIndex - index == 1) return patternSkipReductionData[currentIndex - 1] + 1;

    return 0;
}

void advanceIndicesWhenMatchingFails(int *i,int *j, int *patternSkipReductionData)
{
    *j = (*j != 0) * getAppropriateSkipReduction(patternSkipReductionData, *j) + (*j == 0) * (*j);
    *i = (*j != 0) * (*i) + (*j == 0) * (*i + 1);
}

int searchText(char *text, char *pattern, int sizeOfText, int sizeOfPattern)
{
    int i = 0;
    int j = 0;
    int *patternSkipReductionData = generatePatternSkipData(pattern, sizeOfPattern);
    while (i < sizeOfText && j < sizeOfPattern)
    {
        if (text[i] != pattern[j])
        {
            advanceIndicesWhenMatchingFails(&i, &j, patternSkipReductionData);
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

struct RowList *createRowList(int rowIndex)
{
    struct RowList *first = malloc(sizeof(struct RowList));
    first->rowIndex = rowIndex;
    first->rowList = NULL;
    return first;
}

struct RowList *createRowListFirst()
{
    struct RowList *first = malloc(sizeof(struct RowList));
    first->rowIndex = 0;
    first->rowList = NULL;
    return first;
}

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

struct Row *createRowFirst()
{
    struct Row *first = malloc(sizeof(struct Row));
    first->columnIndex = 0;
    first->next = NULL;
    return first;
}

struct Row *insertItemAtAppropriateRowPosition(int nextSymbol, int columnIndex,
        struct Row *row)
{
    struct Row *previous = row;
    struct Row *tracker = row;

    if (row == NULL)
    {
        return createRowWithEntry(columnIndex, nextSymbol);
    }

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
