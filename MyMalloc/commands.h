#ifndef COMMANDS_H_INCLUDED
#define COMMANDS_H_INCLUDED

#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
    char const *const name;
    char const *const description;
    /// <summary>
    /// Whether the command expects an argument.
    /// </summary>
    bool const hasArgument;
} Command;

void showCommandMenu(Command const commandsArray[], size_t commandCount);
Command const *inputCommand(Command const commandsArray[], size_t commandCount, long long *argument);

// These functions must be defined by the caller
void *customAlloc(size_t size);
void customFree(void *ptr);

#endif // COMMANDS_H_INCLUDED
