#ifndef MYHEAP_H_INCLUDED
#define MYHEAP_H_INCLUDED

#include <stdlib.h>

void *myAlloc(size_t size);
void myFree(void const *ptr);
void heapDumpChunksConsole(void);
void heapDumpChunksBitmap(char const *filename);
void heapDumpDataBitmap(char const *filename);

#endif // MYHEAP_H_INCLUDED
