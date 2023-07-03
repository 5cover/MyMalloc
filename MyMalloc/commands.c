#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "macros.h"

typedef struct {
    char const *name;
    long long argument;
    bool hasName;
    bool hasArgument;
} CommandParseResult;

char *allocateString(size_t length);
Command const *getCommandFromName(CommandGroup commands, char const *commandName);
Command const *getCommandFromParsedResult(CommandGroup commands, CommandParseResult parsedCommand);
CommandParseResult parseCommand(char *input);
unsigned digitCount(long long n);

CommandGroup createCommandGroup(Command const commands[], size_t count)
{
    size_t maxNameLength = 0;

    foreach(Command const, command, commands, count)
    {
        size_t nameLength = strlen(command->name);
        if (nameLength > maxNameLength)
        {
            maxNameLength = nameLength;
        }
    }

    return (CommandGroup) {
        .array = commands,
        .count = count,
        .maxNameLength = maxNameLength
    };
}

Command const *inputCommand(CommandGroup commands, long long *argument)
{
    size_t const inputLength = commands.maxNameLength + 1 + digitCount(LLONG_MAX);

    char *const input = allocateString(inputLength);

    while (true)
    {
        printf("\n> ");

        if (gets_s(input, inputLength) == NULL) // get_s failed
        {
            continue;
        }

        CommandParseResult const parsedCommand = parseCommand(input);

        Command const *command = getCommandFromParsedResult(commands, parsedCommand);

        if (command == NULL)
        {
            continue;
        }

        if (parsedCommand.hasArgument)
        {
            *argument = parsedCommand.argument;
        }

        customFree(input);
        return command;
    }
}

void showCommandMenu(CommandGroup commands)
{
    foreach(Command const, command, commands.array, commands.count)
    {
        printf("%*s %s\n", -(int)min(INT_MAX, commands.maxNameLength), command->name, command->description);
    }
}

Command const *getCommandFromParsedResult(CommandGroup commands, CommandParseResult parsedCommand)
{
    Command const *command = getCommandFromName(commands, parsedCommand.name);

    if (command == NULL)
    {
        printf("Unknown command");
        return NULL;
    }

    if (command->hasArgument && !parsedCommand.hasArgument)
    {
        printf("Argument missing");
        return NULL;
    }

    if (!command->hasArgument && parsedCommand.hasArgument)
    {
        printf("No argument was expected");
        return NULL;
    }

    return command;
}

Command const *getCommandFromName(CommandGroup commands, char const *commandName)
{
    if (commandName != NULL)
    {
        foreach(Command const, command, commands.array, commands.count)
        {
            if (streqn(command->name, commandName, commands.maxNameLength))
            {
                return command;
            }
        }
    }

    return NULL;
}

CommandParseResult parseCommand(char *input)
{
    CommandParseResult result = { 
        .name = NULL,
        .argument = 0,
        .hasName = false,
        .hasArgument = false,
    };

    // If available, set value, otherwise return NULL.

    char *context = NULL;
    char const *const separators = " \f\n\r\t\v";

    // Get first token (command name)
    char *token = strtok_s(input, separators, &context);
    if (token == NULL) // input is empty
    {
        return result;
    }

    // Command name is available from here
    result.name = token;
    result.hasName = true;

    // Get next token (integer)
    token = strtok_s(NULL, separators, &context);
    if (token == NULL)
    {
        return result;
    }

    // Convert argument
    char *endptr;
    errno = 0;
    unsigned long long arg = strtoull(token, &endptr, 0);
    if (errno == ERANGE) // entered number out of range
    {
        printf("Argument must be in range [%lld ; %lld]", LLONG_MIN, LLONG_MAX);
    }
    else if (*endptr != '\0') // conversion failed
    {
        printf("Argument is not a number");
    }
    else
    {
        // Store argument
        result.argument = arg;
        result.hasArgument = true;
    }

    return result;
}

char *allocateString(size_t length)
{
    size_t size = length * sizeof(char) + 1;
    void *const ptr = customAlloc(size);
    if (ptr == NULL)
    {
        fprintf(stderr, "Allocating %zu bytes failed.", size);
        abort();
    }
    return ptr;
}

unsigned digitCount(long long n)
{
    return n == 0 ? 1 : (unsigned)log10((double)llabs(n)) + 1;
}
