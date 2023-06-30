#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "MyHeap.h"
#include "BitmapFactory.h"

#define DUMP_BMP_HEIGHT 10
#define HEAP_SIZE 256

#define NOT_IMPLEMENTED do                            \
{                                                     \
    fprintf(stderr, "%s:%d: %s is not implemented\n", \
            __FILE__, __LINE__, __func__);            \
    abort();                                          \
} while(0)

// Design:
/*
What are chunks?
    -> Chunks represent some metadata over our pool of bytes. They indicate which memory is allocated.

Could we use an array of chunks instead?
Maybe that instead of having an array of bytes and chunk with pointers to elements of this array, we could have a single chunk represent our whole pool at the start.

No "unchunked" memory
All mem is covered by chunks.

Are size=0 chunks ok?
*/

typedef enum
{
    CHK_FREE,
    CHK_ALLOCATED,
} ChunkState;

typedef struct
{
    uint8_t *start;
    size_t size;
    ChunkState state;
} Chunk;

Chunk splitChunk(Chunk *source, size_t size, ChunkState newChunkState);

// Array of bytes representing the heap
static uint8_t gs_pool[HEAP_SIZE];

static size_t gs_chunkCount = 1;

// Array of chunks sorted by start pointer.
static Chunk gs_chunks[HEAP_SIZE / sizeof(void *)] =
{
    {
        .size = HEAP_SIZE,
        .state = CHK_FREE,
        .start = gs_pool,
    }
};

void *myAlloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }
    if (gs_chunkCount == ARRAYSIZE(gs_chunks))
    {
        fprintf(stderr, "Allocation failed: maximum number of allocations (%zu) reached.\n", ARRAYSIZE(gs_chunks));
        return NULL;
    }

    // For each chunk that is free and at least the size desired:
    // Find the index of smallest one
    // In c# this could be implemented with a simple .Where().Min()
    // I hate procedural code. It's hard to read, hard to write, and hard to understand. Errors are hidden and the intent isn't clear.
    size_t iSmallestSuitable = 0;
    bool suitableChunkFound = false;

    for (size_t i = 0; i < gs_chunkCount; ++i)
    {
        Chunk chunk = gs_chunks[i];
        if (chunk.state == CHK_FREE
            && chunk.size >= size
            && (!suitableChunkFound || chunk.size < gs_chunks[iSmallestSuitable].size))
        {
            iSmallestSuitable = i;
            suitableChunkFound = true;
        }
    }

    if (!suitableChunkFound)
    {
        fprintf(stderr, "Allocation failed: no suitable free chunk found.\n");
        return NULL;
    }

    // Split it and return the smaller one.
    Chunk newChunk = splitChunk(gs_chunks + iSmallestSuitable, size, CHK_ALLOCATED);

    // Add the chunk
    gs_chunks[gs_chunkCount++] = newChunk;

    // Returns its start address
    return newChunk.start;
}

void myFree(void const *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    // Find chunk with correct start pointer
    bool chunkFound = false;

    for (size_t i = 0; i < gs_chunkCount && !chunkFound; ++i)
    {
        if (gs_chunks[i].state == CHK_ALLOCATED && gs_chunks[i].start == ptr)
        {
            chunkFound = true;
            gs_chunks[i].state = CHK_FREE;
        }
    }

    if (!chunkFound)
    {
        fprintf(stderr, "Invalid pointer: %p", ptr);
        abort();
    }
}

void heapDumpChunksConsole(void)
{
    // Print chunk list
    printf("Chunks (%zu):\n| %-2s | %-16s | %-16s | %-16s |\n",
           gs_chunkCount, "#", "Start offset", "Size", "State");
    for (size_t i = 0; i < gs_chunkCount; ++i)
    {
        Chunk const chunk = gs_chunks[i];
        char const *stateRepr = "?";
        switch (chunk.state)
        {
        case CHK_ALLOCATED: stateRepr = "allocated"; break;
        case CHK_FREE: stateRepr = "free"; break;
        }
        printf("| %-2zu | %-16zu | %-16zu | %-16s |\n", i, chunk.start - gs_pool, chunk.size, stateRepr);
    }
}

void heapDumpChunksBitmap(char const *filename)
{
    uint8_t image[DUMP_BMP_HEIGHT][HEAP_SIZE][BYTES_PER_PIXEL];

    for (size_t i = 0; i < gs_chunkCount; ++i)
    {
        Chunk const chunk = gs_chunks[i];

        for (size_t y = 0; y < DUMP_BMP_HEIGHT; ++y)
        {
            // Draw sepearetor
            uint8_t *pxSep = image[y][chunk.start - gs_pool];
            switch (chunk.state)
            {
            case CHK_ALLOCATED:
                pxSep[I_R] = 128;
                pxSep[I_G] = 0;
                pxSep[I_B] = 0;
                break;
            case CHK_FREE:
                pxSep[I_R] = 0;
                pxSep[I_G] = 128;
                pxSep[I_B] = 0;
                break;
            }

            for (size_t iByte = 1; iByte < chunk.size; ++iByte)
            {
                uint8_t *pxRepr = image[y][chunk.start - gs_pool + iByte];
                switch (chunk.state)
                {
                case CHK_ALLOCATED:
                    pxRepr[I_R] = 255;
                    pxRepr[I_G] = 0;
                    pxRepr[I_B] = 0;
                    break;
                case CHK_FREE:
                    pxRepr[I_R] = 0;
                    pxRepr[I_G] = 255;
                    pxRepr[I_B] = 0;
                    break;
                }
            }
        }
    }

    generateBitmapImage((uint8_t const *)image, DUMP_BMP_HEIGHT, HEAP_SIZE, filename);
}

void heapDumpDataBitmap(char const *filename)
{
    uint8_t image[DUMP_BMP_HEIGHT][HEAP_SIZE][BYTES_PER_PIXEL];

    for (size_t i = 0; i < HEAP_SIZE; ++i)
    {
        uint8_t byte = gs_pool[i];

        for (size_t y = 0; y < DUMP_BMP_HEIGHT; ++y)
        {
            image[y][i][I_R] = byte;
            image[y][i][I_G] = byte;
            image[y][i][I_B] = byte;
        }
    }

    generateBitmapImage((uint8_t const *)image, DUMP_BMP_HEIGHT, HEAP_SIZE, filename);
}

Chunk splitChunk(Chunk *source, size_t size, ChunkState newChunkState)
{
    assert(size <= source->size);

    source->size -= size;

    return (Chunk)
    {
        .start = source->start + source->size,
        .size = size, .state = newChunkState,
    };
}