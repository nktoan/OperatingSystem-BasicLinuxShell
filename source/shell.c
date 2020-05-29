#include<stdio.h>
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h>
#include<stdbool.h>
#include<fcntl.h>
#include<stddef.h>
  
#define MAX_LINE 100 /* The maximum length command */
#define MAX_ARGV 51 /* The maximum number of argvs  */
#define FLAG_NORMAL_COMMAND_WAIT 1
#define FLAG_NORMAL_COMMAND_NOT_WAIT 2
#define FLAG_PIPE_COMMAND 3

typedef struct History{
    char* lineCommand;
} History;

char* buffer;

void initHistory(History* hist){
    hist->lineCommand=NULL;
}

bool checkIfHistory(char* line){
    if (strlen(line)==2)
        return (line[0]=='!' && line[1]=='!');
    return false;
}

void addToHistory(History* hist, char* line){
    if (hist->lineCommand!=NULL)
        strcpy(hist->lineCommand,line);
    else {
        int sz = strlen(line);
        hist->lineCommand = (char *)malloc(++sz * sizeof(char)); //allocate memory
        for (int i=0;i<sz;++i)
            hist->lineCommand[i]=line[i];
    }
}

char* getInput(History* hist){
    fgets(buffer,MAX_LINE,stdin);
    int size = strlen(buffer);
    if (buffer[size-1]=='\n'){
        buffer[size-1]='\0';
        --size;
    }
    while (buffer[size-1]==' '){
        buffer[size-1]='\0';
        --size;
    }
    if (checkIfHistory(buffer))
        return hist->lineCommand;
    else 
        addToHistory(hist,buffer);
    return buffer;
}

int openFileIn_Out(char *file, char *opt){
    int fd;
    if (strcmp(">", opt) == 0) {
        fd = open(file, O_CREAT | O_TRUNC | O_WRONLY);
        if (fd < 0)
            return -1;
        dup2(fd, STDOUT_FILENO);
        close(fd);
        return 1;
    }
    if (strcmp("<", opt) == 0) {
        fd = open(file, O_RDONLY);
        if (fd < 0)
            return -1;
        dup2(fd, STDIN_FILENO);
        close(fd);
        return 1;
    }
    return 0;
}

int checkOperator(char **args, char *operator, int i){
    int flag;
    if (strstr(args[i], operator) != NULL) {
        if (strcmp(args[i], operator) == 0) {
            if (args[i + 1] == NULL)
                return -1;
            flag = openFileIn_Out(args[i + 1], operator);
            return flag;
        }
    }
    return 0;
}

char** handlerIfIO(char *argv[]){
    int i = 0, position = 0;
    int InFile, OutFile;
    char **new_argv = NULL, **ptr_tmp;

    new_argv = malloc((MAX_ARGV) * sizeof(char*));
    if (!new_argv)
        fprintf(stderr, "ERROR: Allocation error\n");
  

    while (argv[i] != NULL)
    {
        OutFile = checkOperator(argv, ">", i);
        InFile = checkOperator(argv, "<", i);
        if ((OutFile > 0) || (InFile > 0)){
            i+=2;
            continue;
        }
        if ((OutFile == 0) && (InFile == 0))
        {
            new_argv[position] = malloc((strlen(argv[i])+1) * sizeof(char));
    		if (!new_argv[position])
    			fprintf(stderr, "ERROR: Allocation error\n");

            strcpy(new_argv[position], argv[i]);
            position++;
            if (position >= MAX_ARGV) {
                
                int num = 2 * MAX_ARGV;
                ptr_tmp = new_argv;
                
                new_argv = realloc(new_argv, num * sizeof(char*));
                if (!new_argv) {
                	free(ptr_tmp);
                	fprintf(stderr, "ERROR: Allocation error\n");
                	new_argv = NULL;
                }
            }
        }
        i++;
    }
    new_argv[position] = NULL;
    return new_argv;
}

void execute(char** argv, int flag_wait){
    pid_t pid = fork();
    int status;
    if (pid<0){
        printf("ERROR: forking child process failed \n");
        exit(1);
    }
    else if (pid==0){
        argv = handlerIfIO(argv);
        if (flag_wait==FLAG_NORMAL_COMMAND_NOT_WAIT)
            sleep(1); //-> for testing parent process wait for their child or not
        if (execvp(*argv,argv)<0){
           printf("ERROR: exec failed \n");
           exit(1);
        }
    }
    else {
        if (flag_wait==FLAG_NORMAL_COMMAND_WAIT){
            while (wait(&status) != pid);
        }
        return;
    }
}
void executePiped(char** argv, char** argvPiped){
    int fd[2];
    pid_t p1,p2;
    //int status;

    if (pipe(fd)<0){
        printf("ERROR: Pipe could not be initialized \n");
        return;
    }
    p1=fork();
    if (p1<0){
        printf("ERROR: Exec command 1 failed\n");
        return;
    }
    else if (p1==0){
        close(fd[0]);
        dup2(fd[1],STDOUT_FILENO);
        if (execvp(argv[0],argv)<0){
            printf("ERROR: Could not execute command 1\n");
            exit(1);
        }
    }
    p2=fork();
    if (p2<0){
        printf("ERROR: Exec command 2 failed\n");
        return;
    }
    else if (p2==0){
        close(fd[1]);
        dup2(fd[0],STDIN_FILENO);
        if (execvp(argvPiped[0],argvPiped)<0){
            printf("ERROR: Could not execute command 2\n");
            exit(1);
        }
    }
    close(fd[1]);
    close(fd[0]);
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
    
    //wait(NULL);
    //wait(NULL);
    //while (wait(&status) != p2);
    //while (wait(&status) != p1);
}
int parseInputString(char* line, char** argv, char cutChar[]){
    int j=0;
    char* token = strtok(line, cutChar);
    while(token != NULL){
        argv[j++]=token;
        token = strtok(NULL, cutChar);
    }
    argv[j]=NULL;
    return j;
}

int parsePipedLine(char* line,char** argv, char** argvPiped){
    char *temp_argvPiped[MAX_ARGV];
    parseInputString(line,temp_argvPiped, "|");
    if(temp_argvPiped[1] != NULL){
        parseInputString(temp_argvPiped[0], argv, " ");
        parseInputString(temp_argvPiped[1], argvPiped, " ");
        return 1;
    }
    return 0;
}

int handleTypesOfCommand(char* line, char** argv, char** argv_pipe){
    int isPipe = parsePipedLine(line, argv, argv_pipe);
    if (isPipe)
        return FLAG_PIPE_COMMAND;
    else {
        //printf("(%s)",line); - explained? 
        //--ok cuz in line 230 if command not is piped-type, then it would not affects line.
        int NumOfCmd = parseInputString(line, argv, " ");
        if (strcmp(argv[NumOfCmd-1],"&")==0){
            argv[--NumOfCmd]=NULL;
            return FLAG_NORMAL_COMMAND_NOT_WAIT;   
        }
        return FLAG_NORMAL_COMMAND_WAIT;
    }
}
void debug(char** myString){
    int i=0;
    while(myString[i]!=NULL)
        printf("(%s)",myString[i++]);
}
void clearMemoryAllocate(char** myString){
    int i=0;
    while(i<MAX_ARGV)
        if (myString[i]!=NULL) free(myString[i++]);
    free(myString);
}

int main(void)
{
    /* Initialize The Program */
    buffer = (char *)malloc(MAX_LINE * sizeof(char));
    char* line = NULL;
    char *argv[MAX_ARGV] = {NULL},*argv_pipe[MAX_ARGV] = {NULL};
    History* hist = (History*) malloc(sizeof(History));
    initHistory(hist);

    /* Run The Shell */
    int should_run = 1;
    while (should_run) {
        printf("osh>");

        line = getInput(hist);
        if (line==NULL){
            printf("No commands in history\n");
            continue;
        }
        int handleType = handleTypesOfCommand(line,argv,argv_pipe);
        if (strcmp(argv[0],"exit") == 0)
                exit(0);
        if (handleType != FLAG_PIPE_COMMAND){
            if (handleType == FLAG_NORMAL_COMMAND_WAIT)
                execute(argv,FLAG_NORMAL_COMMAND_WAIT);
            else 
                execute(argv,FLAG_NORMAL_COMMAND_NOT_WAIT);
        }
        else if (handleType == FLAG_PIPE_COMMAND) 
            executePiped(argv,argv_pipe);
        fflush(stdout);
    }

    /* Clear Memory Allocated */
    if (hist->lineCommand)  free(hist->lineCommand);
    free(hist);
    free(buffer);
    clearMemoryAllocate(argv);
    clearMemoryAllocate(argv_pipe);
    return 0;
}