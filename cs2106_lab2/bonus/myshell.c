/**
 * CS2106 AY22/23 Semester 2 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "myshell.h"

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct PCBTable PCBTableArr[MAX_PROCESSES]; // defined in myshell.h, set to 50
int PCBTableArrCounter;
bool interrputed;
bool stopped;
bool make_fg;

char ***extract_arguments(char **full_command);
char ***parse_command(size_t num_tokens, char **tokens);
int has_redirection(char **command);
int has_ampersand(char **tokens);
void create_PCBTable(int index, pid_t pid, int status);
void handle_background_child();
char **parse_redirection(char **command);

static void proc_update_status(pid_t pid, int status, int exit_code)
{

    /******* FILL IN THE CODE *******/

    // Call everytime you need to update status and exit code of a process in PCBTable

    // May use WIFEXITED, WEXITSTATUS, WIFSIGNALED, WTERMSIG, WIFSTOPPED

    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (PCBTableArr[i].pid == pid)
        {
            PCBTableArr[i].status = status;
            PCBTableArr[i].exitCode = exit_code;
            break;
        }
    }
}

/*******************************************************************************
 * Signal handler : ex4
 ******************************************************************************/

static void signal_handler(int signo)
{

    // Use the signo to identy ctrl-Z or ctrl-C and print “[PID] stopped or print “[PID] interrupted accordingly.
    // Update the status of the process in the PCB table
    if (signo == SIGINT)
    {
        printf("[%d] interrupted\n", PCBTableArr[PCBTableArrCounter - 1].pid);
        pid_t pid = PCBTableArr[PCBTableArrCounter - 1].pid;
        if (PCBTableArr[PCBTableArrCounter - 1].status == 2)
        {
            kill(pid, SIGINT);
            interrputed = true;
            proc_update_status(pid, 1, 2);
        }
    }
    if (signo == SIGTSTP)
    {

        printf("[%d] stopped\n", PCBTableArr[PCBTableArrCounter - 1].pid);
        // printf("before if 1\n");
        pid_t pid = PCBTableArr[PCBTableArrCounter - 1].pid;
        // printf("before if 2\n");
        // printf("status: %d\n", PCBTableArr[PCBTableArrCounter - 1].status);

        if (PCBTableArr[PCBTableArrCounter - 1].status == 2)
        {
            // printf("after if\n");
            kill(pid, SIGTSTP);
            stopped = true;
            proc_update_status(pid, 4, -1);
            // printf("after if status: %d\n", PCBTableArr[PCBTableArrCounter - 1].status);
        }
    }
}

/*******************************************************************************
 * Built-in Commands
 ******************************************************************************/

static void command_info(char **command)
{

    /******* FILL IN THE CODE *******/

    handle_background_child();

    if (command[1] == NULL)
    {
        fprintf(stderr, "Wrong command\n");
    }
    // info 0
    // Print details of all PCBTableArr in the order in which they were run.
    // You will need to print their process IDs, their current status (Exited, Running, Terminating, Stopped)
    // Status: 1 = Exited , 2 = Running , 3 = Terminating , 4 = Stopped
    else if (strcmp(command[1], "0") == 0)
    {
        for (int i = 0; i < PCBTableArrCounter; i++)
        {
            if (PCBTableArr[i].status == 1)
            {
                printf("[%d] Exited %d\n", PCBTableArr[i].pid, PCBTableArr[i].exitCode);
            }
            else if (PCBTableArr[i].status == 2)
            {
                printf("[%d] Running\n", PCBTableArr[i].pid);
            }
            else if (PCBTableArr[i].status == 3)
            {
                printf("[%d] Terminating\n", PCBTableArr[i].pid);
            }
            else if (PCBTableArr[i].status == 4)
            {
                printf("[%d] Stopped\n", PCBTableArr[i].pid);
            }
        }
    }
    // info 1
    //  Print the number of exited process
    else if (strcmp(command[1], "1") == 0)
    {
        int exited_processes = 0;
        for (int i = 0; i < PCBTableArrCounter; i++)
        {
            if (PCBTableArr[i].status == 1)
            {
                exited_processes++;
            }
        }
        printf("Total exited process: %d\n", exited_processes);
    }
    // info 2
    // Print the number of running process
    else if (strcmp(command[1], "2") == 0)
    {
        int running_processes = 0;
        for (int i = 0; i < PCBTableArrCounter; i++)
        {
            if (PCBTableArr[i].status == 2)
            {
                running_processes++;
            }
        }
        printf("Total running process: %d\n", running_processes);
    }
    else if (strcmp(command[1], "3") == 0)
    {
        int terminated_processes = 0;
        for (int i = 0; i < PCBTableArrCounter; i++)
        {
            if (PCBTableArr[i].status == 3)
            {
                terminated_processes++;
            }
        }
        printf("Total terminating process: %d\n", terminated_processes);
    }
    else if (strcmp(command[1], "4") == 0)
    {
        int stopped_processes = 0;
        for (int i = 0; i < PCBTableArrCounter; i++)
        {
            if (PCBTableArr[i].status == 4)
            {
                stopped_processes++;
            }
        }
        printf("Total stopped process: %d\n", stopped_processes);
    }

    else
    {
        printf("Wrong command\n");
    }
}

static void command_wait(char **command)
{

    /******* FILL IN THE CODE *******/

    // Find the {PID} in the PCBTable
    // If the process indicated by the process id is RUNNING, wait for it (can use waitpid()).
    // After the process terminate, update status and exit code (call proc_update_status())
    // Else, continue accepting user commands.

    pid_t child = atoi(command[1]);
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (PCBTableArr[i].pid == child)
        {
            if (PCBTableArr[i].status == 2)
            {
                int status;
                if (waitpid(child, &status, 0) > 0)
                {
                    int code = WEXITSTATUS(status);
                    proc_update_status(child, 1, code);
                }
            }
            break;
        }
    }
}

static void command_terminate(char **command)
{

    /******* FILL IN THE CODE *******/

    // Find the pid in the PCBTable
    // If {PID} is RUNNING:
    // Terminate it by using kill() to send SIGTERM
    // The state of {PID} should be “Terminating” until {PID} exits

    pid_t child = atoi(command[1]);
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (PCBTableArr[i].pid == child)
        {
            if (PCBTableArr[i].status == 2)
            {
                // If kill returns 0, it means the signal was sent successfully
                int code = kill(child, SIGTERM);
                proc_update_status(child, 3, code);
            }
            break;
        }
    }
}

static void command_fg(char **command)
{

    /******* FILL IN THE CODE *******/

    // if the {PID} status is stopped
    // Print “[PID] resumed”
    // Use kill() to send SIGCONT to {PID} to get it continue and wait for it
    // After the process terminate, update status and exit code (call proc_update_status())
    pid_t stopped = atoi(command[1]);
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (PCBTableArr[i].pid == stopped)
        {
            // If the process indicated by the process id is STOPPED, send SIGCONT to it.
            if (PCBTableArr[i].status == 4)
            {
                printf("[%d] resumed\n", stopped);
                kill(stopped, SIGCONT);
                proc_update_status(stopped, 2, -1);
                make_fg = true;
                // printf("status: %d\n", PCBTableArr[i].status);
            }
            break;
        }
    }
}

/*******************************************************************************
 * Program Execution
 ******************************************************************************/

static void command_exec(char **command)
{

    /******* FILL IN THE CODE *******/

    // check if program exists and is executable : use access()

    // fork a subprocess and execute the program
    char *program = command[0]; // the first index is the program
    if (access(command[0], X_OK) != 0 || access(program, R_OK) != 0)
    {
        // Print “{program} not found” to stderr
        fprintf(stderr, "%s not found\n", program);
        return;
    }

    pid_t pid;

    if ((pid = fork()) == 0)
    {
        // CHILD PROCESS

        // check file redirection operation is present : ex3

        // if < or > or 2> present:
        // use fopen/open file to open the file for reading/writing with  permission O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC, O_SYNC and 0644
        // use dup2 to redirect the stdin, stdout and stderr to the files
        // call execv() to execute the command in the child process

        // else : ex1, ex2
        // call execv() to execute the command in the child process

        // Exit the child

        // Execute {program} in a child process with the supplied arguments.

        // Handle redirection before execution

        // check file redirection operation is present
        if (has_redirection(command))
        {
            char **files = parse_redirection(command);
            if (files[0] != NULL)
            {
                // If there exists a file at {file}: {program} reads the contents of {file} as input.
                int file = open(files[0], O_RDONLY, O_SYNC);
                if (file == -1)
                {
                    // Else, print “{file} does not exist” to stderr
                    fprintf(stderr, "%s does not exist\n", files[0]);
                    // exit the child process with exit code 1
                    exit(1);
                }

                // Update the stdin file descriptor to allow redirect before closing stdin file
                dup2(file, STDIN_FILENO);
                close(file);
            }
            if (files[1] != NULL)
            {
                // If there does not exist a file at {file}, create it
                //{file} is opened in write mode
                int file = open(files[1], O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0644);
                // Redirect the standard output of {program} into {file}
                dup2(file, STDOUT_FILENO);
                close(file);
            }
            if (files[2] != NULL)
            {
                // If there does not exist a file at {file}, create it
                //{file} is opened in write mode
                int file = open(files[2], O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0644);
                // Redirect the standard error of {program} into {file}
                dup2(file, STDERR_FILENO);
                close(file);
            }
        }

        execv(program, extract_arguments(command));
    }
    else
    {
        // PARENT PROCESS
        // register the process in process table
        create_PCBTable(PCBTableArrCounter, pid, 2);
        PCBTableArrCounter++;

        // If  child process need to execute in the background  (if & is present at the end )
        // print Child [PID] in background
        if (has_ampersand(command) == 1)
        {
            printf("Child [%d] in background\n", pid);
            waitpid(pid, NULL, WNOHANG);
        }

        // else wait for the child process to exit

        // Use waitpid() with WNOHANG when not blocking during wait and  waitpid() with WUNTRACED when parent needs to block due to wait

        else
        {
            int child_process_status;
            waitpid(pid, &child_process_status, WUNTRACED);
            int code = WEXITSTATUS(child_process_status);
            if (interrputed)
            {
                // proc_update_status(pid, 1, 2);
                interrputed = false;
            }
            else if (stopped)
            {
                // proc_update_status(pid, 4, -1);
                stopped = false;
            }
            else if (make_fg)
            {
                // proc_update_status(pid, 2, -1);
                make_fg = false;
            }
            else
            {
                proc_update_status(pid, 1, code);
            }
            // Child process has finished, update PCBTable with status code 1: Stopped and new exitcode
        }
    }
    handle_background_child();
}

/*******************************************************************************
 * Command Processor
 ******************************************************************************/

// Takes in the command which is in the form of {keyword} [args]
static void command(char **command)
{

    /******* FILL IN THE CODE *******/

    // if command is "info" call command_info()             : ex1
    // if command is "wait" call command_wait()             : ex2
    // if command is "terminate" call command_terminate()   : ex2
    // if command is "fg" call command_fg()                 : ex4

    char **first_keyword = command[0];

    // printf("Command is : %s\n", first_keyword);

    // simple commands --> wait, terminate, info,
    if (strcmp(first_keyword, "info") == 0)
    {
        // printf("is info");
        command_info(command);
    }
    else if (strcmp(first_keyword, "wait") == 0)
    {
        // printf("is wait");
        command_wait(command);
    }
    else if (strcmp(first_keyword, "terminate") == 0)
    {
        // printf("is terminate");
        command_terminate(command);
    }
    else if (strcmp(first_keyword, "fg") == 0)
    {
        // printf("is fg");
        command_fg(command);
    }

    else
    {
        // printf("is a program");
        command_exec(command);
    }

    // call command_exec() for all other commands           : ex1, ex2, ex3
}

/*******************************************************************************
 * High-level Procedure
 ******************************************************************************/

void my_init(void)
{

    /******* FILL IN THE CODE *******/

    // use signal() with SIGTSTP to setup a signalhandler for ctrl+z : ex4
    // use signal() with SIGINT to setup a signalhandler for ctrl+c  : ex4
    signal(SIGTSTP, signal_handler);
    signal(SIGINT, signal_handler);
    interrputed = false;
    stopped = false;
    make_fg = false;

    // anything else you require
    PCBTableArrCounter = 0;
}

// -tokens is an array of tokens with the last element of the array being NULL
//        i.e. the original command separated by white space
// –size is the size of that array, including the NULL.
void my_process_command(size_t num_tokens, char **tokens)
{

    /******* FILL IN THE CODE *******/

    // Split tokens at NULL or ; to get a single command (ex1, ex2, ex3, ex4(fg command))

    // for example :  /bin/ls ; /bin/sleep 5 ; /bin/pwd
    // split the above line as first command : /bin/ls , second command: /bin/pwd  and third command:  /bin/pwd
    // Call command() and pass each individual command as arguements
    int command_number = 0;

    // generate the command array split by ";"
    char ***commands = parse_command(num_tokens, tokens);
    while (commands[command_number] != NULL)
    {
        command(commands[command_number]);
        command_number++;
    }
}

char ***parse_command(size_t num_tokens, char **tokens)
{
    char ***command_arr;

    // Count the number of commands in the tokens array by checking for ';' delimiters
    int num_of_cmds = 1;
    for (size_t i = 0; i < num_tokens - 1; i++)
    {
        if (strcmp(tokens[i], ";") == 0)
        {
            num_of_cmds++;
        }
    }

    // Allocate memory for the array of commands
    command_arr = malloc(sizeof(char **) * (num_of_cmds + 1));

    // Loop through the tokens array and split it into individual commands
    int cmds_index = 0;
    int args_index = 0;
    while (tokens[args_index] != NULL)
    {
        // Allocate memory for the current command and initialize number of arguments
        char **command = malloc(sizeof(char *) * num_tokens);
        int num_of_args = 0;

        // Copy tokens into the command array until a ';' delimiter is found or end of tokens array is reached
        while (tokens[args_index] != NULL && strcmp(tokens[args_index], ";") != 0)
        {
            command[num_of_args] = tokens[args_index];
            num_of_args++;
            args_index++;
        }

        // Add a NULL terminator to the end of the command array
        command[num_of_args] = NULL;

        // Skip over the ';' delimiter
        if (tokens[args_index] != NULL)
        {
            args_index++;
        }

        // Add the current command to the array of commands
        command_arr[cmds_index] = command;
        cmds_index++;
    }

    // Add a NULL terminator to the end of the array of commands
    command_arr[num_of_cmds] = NULL;

    // Return the array of commands
    return command_arr;
}

char **parse_redirection(char **command)
{
    // Parses the command and stores the value of each {file} (if present) into an array called files
    char **files = malloc(sizeof(char *) * 3);
    int i = 0;
    while (command[i] != NULL)
    {
        // Store input of {program} in files[0]
        if (strcmp(command[i], "<") == 0)
        {
            files[0] = command[i + 1];
        }
        // Store standard output of {program} in files[1]
        else if (strcmp(command[i], ">") == 0)
        {
            files[1] = command[i + 1];
        }
        // Store standard error of {program} in files[2]
        else if (strcmp(command[i], "2>") == 0)
        {
            files[2] = command[i + 1];
        }
        i++;
    }
    return files;
}

// reads the command until first occurance of ("&", "<", ">", or "2>") to get the arguments only
char ***extract_arguments(char **full_command)
{
    int num_args = 0;
    char **args_array = NULL;

    // Count number of arguments in input array
    while (full_command[num_args] != NULL &&
           strcmp(full_command[num_args], "&") != 0 && strcmp(full_command[num_args], "<") != 0 &&
           strcmp(full_command[num_args], ">") != 0 && strcmp(full_command[num_args], "2>") != 0)
    {
        num_args++;
    }

    // Allocate memory for output array
    args_array = calloc(num_args + 1, sizeof(char *));
    // Copy arguments from input array to output array
    for (int i = 0; i < num_args; i++)
    {
        args_array[i] = strdup(full_command[i]);
    }
    // Add terminating NULL pointer to output array
    args_array[num_args] = NULL;

    return args_array;
}

void handle_background_child()
{
    int status;
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (PCBTableArr[i].status == 3 || PCBTableArr[i].status == 2)
        {
            if (waitpid(PCBTableArr[i].pid, &status, WNOHANG) > 0)
            {
                int child = PCBTableArr[i].pid;
                if (WIFEXITED(status))
                {
                    proc_update_status(child, 1, WEXITSTATUS(status));
                }
                if (WIFSIGNALED(status))
                {
                    proc_update_status(child, 1, WTERMSIG(status));
                }
            }
        }
    }
}

void my_quit(void)
{

    /******* FILL IN THE CODE *******/
    // Kill every process in the PCB that is either stopped or running
    handle_background_child();

    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (PCBTableArr[i].status == 2 || PCBTableArr[i].status == 4)
        {
            if (PCBTableArr[i].status == 4)
            {
                kill(PCBTableArr[i].pid, SIGCONT);
            }
            printf("Killing [%d]\n", PCBTableArr[i].pid);
            kill(PCBTableArr[i].pid, SIGTERM);
        }
    }

    printf("\nGoodbye\n");
}

void create_PCBTable(int index, pid_t pid, int status)
{
    PCBTableArr[index].pid = pid;
    PCBTableArr[index].status = status;
    PCBTableArr[index].exitCode = -1;
}

// Return 0 if there is 0 redirection in command
// Else return 1 if there is at least one redirection in command
int has_redirection(char **command)
{
    int index = 0;
    while (command[index] != NULL)
    {
        if (strcmp(command[index], "<") == 0 || strcmp(command[index], ">") == 0 || strcmp(command[index], "2>") == 0)
        {
            return 1;
        }
        index++;
    }
    return 0;
}

// Return 0 if there is 0 ampersand in command
// Else return 1 if there is ampersand
int has_ampersand(char **command)
{
    int index = 0;
    while (command[index] != NULL)
    {
        if (strcmp(command[index], "&") == 0)
        {
            return 1;
        }
        index++;
    }
    return 0;
}