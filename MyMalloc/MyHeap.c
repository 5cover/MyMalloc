#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "myHeap.h"
#include "bitmapFactory.h"
#include "macros.h"

#define DUMP_BMP_HEIGHT 10
#define HEAP_SIZE 256
#define CHUNKS_LENGTH (HEAP_SIZE / sizeof(void*))

// Pointer to the first byte of the heap.
#define HEAP_START_PTR ((intptr_t)gs_pool)

// Alignement strategy.
// myAlloc will align allocations on the nearest multiple of the value evaluated.
// size represents the requested size in bytes.
#define ALIGN 1

// Possible optimizations:
// - Keep gs_chunks sorted

typedef struct
{
    intptr_t start;
    size_t size;
} Chunk;

bool rangesOverlap(intptr_t x1, intptr_t x2, intptr_t y1, intptr_t y2);
bool isAreaAllocated(intptr_t start, size_t size);
void removeChunkAt(size_t index);

// Array of bytes representing the heap
static uint8_t gs_pool[HEAP_SIZE];

static size_t gs_chunkCount = 0;

// Array of allocated memory chunks.
// It's length represents the maximum number of simulatenous allocations.
static Chunk gs_chunks[CHUNKS_LENGTH];

void *myAlloc(size_t size)
{
    if (size == 0)
    {
        // malloc(0) is unspecified behavior
        // We can either return an unique pointer or NULL.
        return NULL;
    }
    if (gs_chunkCount == ARRAYLENGTH(gs_chunks))
    {
        fprintf(stderr, "Allocation failed: maximum number of allocations (%zu) reached.\n", ARRAYLENGTH(gs_chunks));
        return NULL;
    }

    // nearest multiple of ALIGN greater than HEAP_START_PTR
    intptr_t const alignedStart = HEAP_START_PTR + HEAP_START_PTR % ALIGN;

    // Complexity:
    // -> O(HEAP_SIZE / size * gs_chunkCount)
    // Best: O(1)
    // Worst: O(n²)
    for (intptr_t start = alignedStart; start + (intptr_t)size <= HEAP_START_PTR + HEAP_SIZE; start += ALIGN)
    {
        if (!isAreaAllocated(start, size))
        {
            gs_chunks[gs_chunkCount++] = (Chunk) {
                .start = start,
                .size = size,
            };
            return (void*)start;
        }
    }

    fprintf(stderr, "Allocation failed: heap too small.\n");
    return NULL;
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
        if (gs_chunks[i].start == (intptr_t)ptr)
        {
            chunkFound = true;
            removeChunkAt(i);
        }
    }

    if (!chunkFound)
    {
        // Freeing an invalid pointer is undefined behavior as per the C standard, so we can do whatever we want here.

        fprintf(stderr, "Tried to free an invalid pointer: %p", ptr);
        // We could ignore the error, but it's probably unsafe to continue, so fail-fast.
        abort();
    }
}

// Checks if at least one chunk occupies at least one byte of the specified memory area.
bool isAreaAllocated(intptr_t start, size_t size)
{
    foreach(Chunk const, chunk, gs_chunks, gs_chunkCount)
    {
        if (rangesOverlap(chunk->start, chunk->start + chunk->size - 1,
                          start, start + size - 1))
        {
            return true;
        }
    }
    return false;
}

void removeChunkAt(size_t index)
{
    for (size_t i = index; i < gs_chunkCount; ++i)
    {
        gs_chunks[i] = gs_chunks[i + 1];
    }
    --gs_chunkCount;
}

bool rangesOverlap(intptr_t x1, intptr_t x2, intptr_t y1, intptr_t y2)
{
    assert(x1 <= x2 && y1 <= y2);
    return x1 <= y2 && y1 <= x2;
}


void heapDumpChunksConsole(void)
{
    // Print chunk list
    printf("Chunks (%zu/%zu):\n\n| %-2s | %-16s | %-16s |\n",
           gs_chunkCount, CHUNKS_LENGTH, "#", "Start offset", "Size");
    size_t totalSize = 0;
    for (size_t i = 0; i < gs_chunkCount; ++i)
    {
        Chunk const chunk = gs_chunks[i];
        printf("| %-2zu | %-16Id | %-16zu |\n", i, chunk.start - HEAP_START_PTR, chunk.size);
        totalSize += chunk.size;
    }
    printf("\n%zu/%zu bytes allocated\n", totalSize, (size_t)HEAP_SIZE);
}

void heapDumpChunksBitmap(char const *filename)
{
    uint8_t image[DUMP_BMP_HEIGHT][HEAP_SIZE][BYTES_PER_PIXEL] = { 0 };

    // Draw the whole bar in green
    for (size_t i = 0; i < HEAP_SIZE; ++i)
    {
        for (size_t y = 0; y < DUMP_BMP_HEIGHT; ++y)
        {
            uint8_t *px = image[y][i];
            px[I_R] = 0;
            px[I_G] = 255;
            px[I_B] = 0;
        }
    }

    // Draw chunks individually
    foreach(Chunk const, chunk, gs_chunks, gs_chunkCount)
    {
        for (size_t y = 0; y < DUMP_BMP_HEIGHT; ++y)
        {
            // Draw sepearator
            uint8_t *const pxSep = image[y][chunk->start - HEAP_START_PTR];
            pxSep[I_R] = 128;
            pxSep[I_G] = 0;
            pxSep[I_B] = 0;

            for (size_t i = 1; i < chunk->size; ++i)
            {
                uint8_t *const pxRepr = image[y][chunk->start - HEAP_START_PTR + i];
                pxRepr[I_R] = 255;
                pxRepr[I_G] = 0;
                pxRepr[I_B] = 0;
            }
        }
    }

    generateBitmapImage((uint8_t const *)image, DUMP_BMP_HEIGHT, HEAP_SIZE, filename);
}

void heapDumpDataBitmap(char const *filename)
{
    uint8_t image[DUMP_BMP_HEIGHT][HEAP_SIZE][BYTES_PER_PIXEL] = { 0 };

    for (size_t i = 0; i < HEAP_SIZE; ++i)
    {
        uint8_t const byte = gs_pool[i];

        for (size_t y = 0; y < DUMP_BMP_HEIGHT; ++y)
        {
            image[y][i][I_R] = byte;
            image[y][i][I_G] = byte;
            image[y][i][I_B] = byte;
        }
    }

    generateBitmapImage((uint8_t const *)image, DUMP_BMP_HEIGHT, HEAP_SIZE, filename);
}