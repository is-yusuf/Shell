/*
  Yusuf Ismail
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

// The command buffer will need to have room to hold the
// command, the \n at the end of the command, and the \0.
// That's why the maximum command size is 2 less than the
// command buffer size.
#define COMMAND_BUFFER_SIZE 102
#define MAX_COMMAND_SIZE COMMAND_BUFFER_SIZE - 2

// Return values for get_command
#define COMMAND_INPUT_SUCCEEDED 0
#define COMMAND_INPUT_FAILED 1
#define COMMAND_END_OF_FILE 2
#define COMMAND_TOO_LONG 3

int get_command(char *command_buffer, int buffer_size);
void execute_command(char *command_line);
void redirect_output(char *name);
void get_argv(char *command_line, char **address);
int main()
{
    const char *prompt = "shell208> ";
    char command_line[COMMAND_BUFFER_SIZE];

    // The main infinite loop
    while (1)
    {
        printf("%s", prompt);
        fflush(stdout);

        int result = get_command(command_line, COMMAND_BUFFER_SIZE);
        if (result == COMMAND_END_OF_FILE)
        {
            // stdin has reached EOF, so it's time to be done. This often happens
            // when the user hits Ctrl-D.
            break;
        }
        else if (result == COMMAND_INPUT_FAILED)
        {
            fprintf(stderr, "There was a problem reading your command. Please try again.\n");
            // we could try to analyze the error using ferror and respond in different
            // ways depending on the error, but instead, let's just bail.
            break;
        }
        else if (result == COMMAND_TOO_LONG)
        {
            fprintf(stderr, "Commands are limited to length %d. Please try again.\n", MAX_COMMAND_SIZE);
        }
        else
        {
            execute_command(command_line);
        }
    }

    return 0;
}

/*
    Retrieves the next line of input from stdin (where, typically, the user
    has typed a command) and stores it in command_buffer.

    The newline character (\n, ASCII 10) character at the end of the input
    line will be read from stdin but not stored in command_buffer.

    The input stored in command_buffer will be \0-terminated.

    Returns:
        COMMAND_TOO_LONG if the number of chars in the input line
            (including the \n), is greater than or equal to buffer_size
        COMMAND_INPUT_FAILED if the input operation fails with an error
        COMMAND_END_OF_FILE if the input operation fails with feof(stdin) == true
        COMMAND_INPUT_SUCCEEDED otherwise

    Preconditions:
        - buffer_size > 0
        - command_buffer != NULL
        - command_buffer points to a buffer large enough for at least buffer_size chars
*/
int get_command(char *command_buffer, int buffer_size)
{
    assert(buffer_size > 0);
    assert(command_buffer != NULL);

    if (fgets(command_buffer, buffer_size, stdin) == NULL)
    {
        if (feof(stdin))
        {
            return COMMAND_END_OF_FILE;
        }
        else
        {
            return COMMAND_INPUT_FAILED;
        }
    }

    int command_length = strlen(command_buffer);
    if (command_buffer[command_length - 1] != '\n')
    {
        // If we get here, the input line hasn't been fully read yet.
        // We need to read the rest of the input line so the unread portion
        // of the line doesn't corrupt the next command the user types.
        char ch = getchar();
        while (ch != '\n' && ch != EOF)
        {
            ch = getchar();
        }

        return COMMAND_TOO_LONG;
    }

    // remove the newline character
    command_buffer[command_length - 1] = '\0';
    return COMMAND_INPUT_SUCCEEDED;
}

void execute_command(char *command_line)
{
    if (strcmp(command_line, "help") == 0)
    {
        printf("Insert a command to run\n");
        return;
    }
    pid_t pid = fork();

    if (pid != 0)
    {
        int status;
        pid = wait(&status);
        return;
    }
    else
    {
        int max_commands = 10;
        char *args[max_commands];
        get_argv(command_line, args);
        execvp(args[0], args);
        fflush(stdout);
        perror("exec failed");
        fflush(stdout);
        return;
    }
}

void get_argv(char *command_line, char **address)
{
    int argCount = 0;
    char *part = strtok(command_line, " ");
    while (part != NULL && argCount < 9)
    {
        if (strcmp(part, ">") == 0)
        {
            part = strtok(NULL, " ");
            redirect_output(part);
            break;
        }
        address[argCount++] = part;
        part = strtok(NULL, " ");
    }
    address[argCount] = NULL;
}

void redirect_output(char *name)
{
    FILE *file = freopen(name, "w", stdout);
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }
    if (dup2(fileno(file), STDOUT_FILENO) == -1)
    {
        perror("Error redirecting stdout");
        return;
    }
}
