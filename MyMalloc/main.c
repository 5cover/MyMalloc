#ifndef __STDC_LIB_EXT1__
#error f
#endif
#define __STDC_WANT_LIB_EXT1__ 1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdarg.h>

#include "MyHeap.h"

#define CHUNKS_DUMP_FILENAME "heap_chunks_dump.bmp"
#define DATA_DUMP_FILENAME "heap_data_dump.bmp"

typedef struct { size_t size; void *address; } Allocation;

bool tryInputInt(long *result);
long inputInt(void);
void printAllocations(Allocation const allocations[], size_t allocationCount);
void removeAt(Allocation array[], size_t *length, size_t index);

long menu(size_t choiceCount, char const *choices[]);

#define MENU(choices) menu(ARRAYSIZE(choices), (choices))

int main(void)
{
    Allocation allocations[100] = { 0 };
    size_t allocationCount = 0;

    bool keepGoing = true;
    while (keepGoing)
    {
        printAllocations(allocations, allocationCount);
        puts("");
        switch (MENU(((char const *[])
        {
            "Allocate...",
            "Free...",
            "Update data...",
            "View chunks table",
            "Open chunks bitmap",
            "Open raw data bitmap",
            "Quit",
        })))
        {
        case 0:
        {
            if (allocationCount == ARRAYSIZE(allocations))
            {
                printf("Maximum number of allocations (%zu) reached.\n", ARRAYSIZE(allocations));
                break;
            }

            puts("Size in bytes (>=0):");
            long input;
            while (!tryInputInt(&input) || input < 0);
            size_t size = (size_t)input;

            Allocation newAllocation = { .address = myAlloc(size), .size = size, };

            printf("Added allocation of size %zu at %p.\n",
                   newAllocation.size, newAllocation.address);

            allocations[allocationCount++] = newAllocation;

            heapDumpChunksBitmap(CHUNKS_DUMP_FILENAME);
        }
        break;
        case 1:
        {
            if (allocationCount == 0)
            {
                puts("No allocations are defined.");
                break;
            }

            printf("Allocation # [1;%zu]:\n", allocationCount);
            long input;
            while (!tryInputInt(&input) || input <= 0 || input > (long)allocationCount);

            size_t iRemovedAllocation = (size_t)(input - 1);
            Allocation removedAllocation = allocations[iRemovedAllocation];

            myFree(removedAllocation.address);

            printf("Freed allocation of size %zu at %p (%zu remaining).\n",
                   removedAllocation.size, removedAllocation.address, allocationCount - 1);

            removeAt(allocations, &allocationCount, iRemovedAllocation);

            heapDumpChunksBitmap(CHUNKS_DUMP_FILENAME);
        }
        break;
        case 2:
        {
            // Update data...

            heapDumpDataBitmap(DATA_DUMP_FILENAME);
        }
        break;
        case 3:
        {
            heapDumpChunksConsole();
            puts("");
        }
        break;
        case 4:
        {
            heapDumpChunksBitmap(CHUNKS_DUMP_FILENAME);
            system(CHUNKS_DUMP_FILENAME);
        }
        break;
        case 5:
        {
            heapDumpDataBitmap(DATA_DUMP_FILENAME);
            system(DATA_DUMP_FILENAME);
        }
        break;
        case 6:
        {
            keepGoing = false;
        }
        break;
        }
    }

    return 0;
}

void removeAt(Allocation array[], size_t *count, size_t index)
{
    for (size_t i = index; i < *count - 1; ++i)
    {
        array[i] = array[i + 1];
    }
    --(*count);
}

void printAllocations(Allocation const allocations[], size_t allocationCount)
{
    if (allocationCount == 0)
    {
        return;
    }

    printf("Allocations (%zu):\n| %-2s | %-16s | %-16s |\n",
           allocationCount, "#", "Address", "Size");
    for (size_t i = 0; i < allocationCount; ++i)
    {
        printf("| %-2zu | %-16p | %-16zu |\n", i + 1, allocations[i].address, allocations[i].size);
    }
}

long menu(size_t choiceCount, char const *choices[])
{
    for (size_t i = 0; i < choiceCount; ++i)
    {
        printf("\t%zu. %s\n", i + 1, choices[i]);
    }

    long choice = 0;
    do
    {
        printf("> ");
    } while (!tryInputInt(&choice) || 1 > choice || choice > (long)choiceCount);

    return choice - 1;
}

long inputInt(void)
{
    long result = 0;
    do
    {
        printf("> ");
    } while (!tryInputInt(&result));
    return result;
}

// Determines the lengths of a integer litteral.
#define STR(X) #X
#define LEN(x) (sizeof(STR(x)) / sizeof(char) - 1)

bool tryInputInt(long *result)
{
    char buf[LEN(LONG_MAX)] = { 0 };

    // Yes, the L suffix causes 1 useless character, but that's fine

    if (fgets(buf, LEN(LONG_MAX), stdin) == NULL)
    {
        return false;
    }

    // have some input, convert it to integer:
    char *endptr;
    errno = 0;
    long converted = strtol(buf, &endptr, 0);

    if (errno == ERANGE)
    {
        printf("Out of range\n");
        return false;
    }
    if (endptr == buf)
    {
        // empty or invalid input
        return false;
    }

    *result = converted;
    return true;
}
#undef LEN