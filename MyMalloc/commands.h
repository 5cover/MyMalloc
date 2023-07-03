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

typedef struct 
{
    Command const *const array;
    size_t const count;
    size_t const maxNameLength;
} CommandGroup;

CommandGroup createCommandGroup(Command const commands[], size_t count);
void showCommandMenu(CommandGroup commands);
Command const *inputCommand(CommandGroup commands, long long *argument);

// These functions must be defined by the caller
void *customAlloc(size_t size);
void customFree(void *ptr);

#endif // COMMANDS_H_INCLUDED
