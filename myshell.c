// C Program to design a shell in Linux
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>


  
#define MAXCOM 100 // max number of letters to be supported
#define MAXLIST 10 // max number of commands to be supported
  

#define MAX 10

char stringArray[MAX][100]; //string array for FIFO queue that is used for holding values for history custom commnd
int front = 0;
int rear = -1;
int itemCount = 0;




void insert(char* data) {

strcpy(stringArray[(++rear)%MAX], data);
if(itemCount!=MAX)
{
    itemCount++;
}
else
{
    front++;
}

   
}





void print_array()
{
    int temp_front= front;
    for (int i = 0; i < itemCount; i++)
    {
        printf("%d %s\n", i+1,stringArray[(temp_front++)%itemCount]);
    }
}  

  
// Function to take input
int takeInput(char* str)
{
    
    char* buf;
    buf = readline("myshell>");
    if (strlen(buf) != 0) {
        insert(buf); 
        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}
// custom change directory command
void cd(char** parsed)
{
    if(parsed[1] == NULL)
    {
        char* home = getenv("HOME");
        chdir(home);
        return;
    }
    chdir(parsed[1]);
}

void dir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
}
  
void history()
{
    //print 10 most recently entered commands in your shell
    print_array();
}

void bye()
{
    printf("Goodye:)\n");
    exit(0);
}


// Function where the system command is executed
void execArgs(char** parsed)
{
    // Forking a child
    pid_t pid = fork(); 
  
    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate
    
        wait(NULL); 
        
        return;
    }
}

void execArgsBackground(char** parsed)
{
    pid_t p1;
    
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }

  
    if (p1 == 0) 
    {
        // Child executing..
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(1);
        }
        exit(0);
    } else 
    {
        // Parent executing
        return;   
    }

}
// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int pipefd[2]; 
    pid_t p1, p2;
  
    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }

  
    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(STDOUT_FILENO);
        close(pipefd[0]);
        dup(pipefd[1]);
        close(pipefd[1]);
  
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
        exit(1);
    } else {
        // Parent executing
        p2 = fork();
  
        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }
  
        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            close(STDIN_FILENO); 
            close(pipefd[1]);
            dup(pipefd[0]);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
            exit(1);
        } else {
            // parent executing, waiting for two children

            close(pipefd[0]);
            close(pipefd[1]);
            
            wait(NULL);
            wait(NULL);
        }
    }
}
  

  
// Function to execute builtin commands
int ownCmdHandler(char** parsed)
{
    int NoOfOwnCmds = 4, i, switchOwnArg = 0;
    char* ListOfOwnCmds[NoOfOwnCmds];
    char* username;
  
    ListOfOwnCmds[0] = "cd";
    ListOfOwnCmds[1] = "dir";
    ListOfOwnCmds[2] = "history";
    ListOfOwnCmds[3] = "bye";
  
    for (i = 0; i < NoOfOwnCmds; i++) 
    {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) 
        {
            switchOwnArg = i + 1;
            break;
        }
    }
  
    switch (switchOwnArg) {
    case 1:
        cd(parsed);
        dir();
        return 1;
    case 2:
        dir();
        return 1;
    case 3:
        history();
        return 1;
    case 4:
        bye();
    default:
        break;
    }
  
    return 0;
}


int parseBackground(char* str, char** strbackground)
{
    int i;
    for (i = 0; i < 2; i++) {
        strbackground[i] = strsep(&str, "&");
        if (strbackground[i] == NULL)
            break;
    }
  
    if (strbackground[1] == NULL)
        return 0; // returns zero if no & is found.
    else {
        return 1;
    }
}

// function for finding pipe
int parsePipe(char* str, char** strpiped)
{
    int i;
    for (i = 0; i < 2; i++) {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }
  
    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
}
  
// function for parsing command words
void parseSpace(char* str, char** parsed)
{
    int i;
  
    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");
  
        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}
 
int processString(char* str, char** parsed, char** parsedpipe)
{
  
    char* strpiped[2];
    int piped = 0;

    char* strbackground[2];
    int background=0;
    background= parseBackground(str, strbackground);
  
    piped = parsePipe(str, strpiped);



  
    if (piped) 
    {
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedpipe);
 
  
    } 
    else if(background)
    {
        parseSpace(strbackground[0], parsed);
        return 3;
    }
    
    else {
  
        parseSpace(str, parsed);
    }
  
    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1 + piped;
}
  
int main()
{
    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    char* parsedArgsPiped[MAXLIST];
    int execFlag = 0;
    
  
    while (1) {
       

        // take input
        if (takeInput(inputString))
            continue;
        // process
        execFlag = processString(inputString,
        parsedArgs, parsedArgsPiped);
        // execflag returns zero if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command
        // 2 if it is including a pipe.
        // 3 if it includes &
        // execute
        if (execFlag == 1)
        {
            execArgs(parsedArgs);
        }
        if (execFlag == 2)
        {
            execArgsPiped(parsedArgs, parsedArgsPiped);
        }    
        if (execFlag == 3)
        {
            execArgsBackground(parsedArgs);
        }    

    }
    return 0;
}




// gcc myshell.c -lreadline
// ./a.out