#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "macros.h"
#include "myHeap.h"

#define CHUNKS_DUMP_FILENAME "heap_chunks_dump.bmp"
#define DATA_DUMP_FILENAME "heap_data_dump.bmp"
typedef struct
{
    size_t size;
    void const *address;
} Allocation;

void printAllocations(Allocation const allocations[], size_t allocationCount);
void removeAt(Allocation array[], size_t *length, size_t index);

void *customAlloc(size_t size)
{
    void *ptr = malloc(size);
    //printf("[Allocated %zu bytes at %p]\n", size, ptr);
    //heapDumpChunksBitmap(CHUNKS_DUMP_FILENAME);
    return ptr;
}
void customFree(void *ptr)
{
    free(ptr);
    //printf("[Freed %p]\n", ptr);
    //heapDumpChunksBitmap(CHUNKS_DUMP_FILENAME);
}

int main(void)
{
    Allocation allocations[100] = { 0 };
    size_t allocationCount = 0;

    Command const commandsArray[] = {
        (Command) {
            .name = "alloc",
            .description = "Allocate an amount of bytes.",
            .hasArgument = true,
        },
        (Command) {
            .name = "free",
            .description = "Free the specified allocation.",
            .hasArgument = true,
        },
        (Command) {
            .name = "list",
            .description = "List all allocations and chunks.",
            .hasArgument = false,
        },
        (Command) {
            .name = "view",
            .description = "Open the system editor for the chunks dump bitmap.",
            .hasArgument = false,
        },
        (Command) {
            .name = "data",
            .description = "Open the system editor for the data dump bitmap.",
            .hasArgument = false,
        },
        (Command) {
            .name = "help",
            .description = "Show this help menu.",
            .hasArgument = false,
        },
        (Command) {
            .name = "cls",
            .description = "Clear the screen.",
            .hasArgument = false,
        },
        (Command) {
            .name = "exit",
            .description = "Exit the program.",
            .hasArgument = false,
        },
    };
    CommandGroup const commands = createCommandGroup(commandsArray, ARRAYLENGTH(commandsArray));

    showCommandMenu(commands);

    while (true)
    {
        long long argument = 0;

        const Command *command = inputCommand(commands, &argument);

        if (streq(command->name, "alloc"))
        {
            if (allocationCount == ARRAYLENGTH(allocations))
            {
                printf("Maximum number of allocations (%zu) reached.\n", ARRAYLENGTH(allocations));
            }
            else if (argument < 0)
            {
                printf("Size ('%lld') must be greater than or equal to 0.\n", argument);
            }
            else
            {
                size_t size = (size_t)argument;

                Allocation newAllocation = { .address = myAlloc(size), .size = size, };

                printf("Added allocation of size %zu at %p.\n",
                       newAllocation.size, newAllocation.address);

                allocations[allocationCount++] = newAllocation;

                heapDumpChunksBitmap(CHUNKS_DUMP_FILENAME);
            }

        }
        else if (streq(command->name, "free"))
        {
            if (allocationCount == 0)
            {
                printf("No allocations are defined.\n");
            }
            else if (argument < 1 || (size_t)argument > allocationCount)
            {
                printf("Invalid allocation ID.\n");
            }
            else
            {
                size_t iRemovedAllocation = (size_t)(argument - 1);

                Allocation removedAllocation = allocations[iRemovedAllocation];

                myFree(removedAllocation.address);

                printf("Freed allocation of size %zu at %p (%zu remaining).\n",
                       removedAllocation.size, removedAllocation.address, allocationCount - 1);

                removeAt(allocations, &allocationCount, iRemovedAllocation);

                heapDumpChunksBitmap(CHUNKS_DUMP_FILENAME);
            }

        }
        else if (streq(command->name, "list"))
        {
            heapDumpChunksConsole();
            printf("\n");
            printAllocations(allocations, allocationCount);
        }
        else if (streq(command->name, "view"))
        {
            heapDumpChunksBitmap(CHUNKS_DUMP_FILENAME);
            system(CHUNKS_DUMP_FILENAME);
        }
        else if (streq(command->name, "data"))
        {
            heapDumpDataBitmap(DATA_DUMP_FILENAME);
            system(DATA_DUMP_FILENAME);
        }
        else if (streq(command->name, "help"))
        {
            showCommandMenu(commands);
        }
        else if (streq(command->name, "cls"))
        {
            system("cls");
        }
        else if (streq(command->name, "exit"))
        {
            break;
        }
        else
        {
            assert(false && "Failed to handle command");
        }
    }

    return 0;
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
        printf("| %-2zu | %-#16p | %-16zu |\n", i + 1, allocations[i].address, allocations[i].size);
    }
}

void removeAt(Allocation array[], size_t *count, size_t index)
{
    for (size_t i = index; i < *count - 1; ++i)
    {
        array[i] = array[i + 1];
    }
    --(*count);
}