/* CSCI 347 micro-make
 * 
 * 09 AUG 2017, Aran Clauson
 *
 * 10/05/2018 Chris Miller
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> 
#include <ctype.h>
#include "arg_parse.h"
#include "target.h"

#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

/* CONSTANTS */

#define BUFFER 1024

/* PROTOTYPES */

/* IO Redirection 
 * args     A parsed line of rules to be passed into execvp
 * 
 * The ioRedirection function takes in a args and parses through it, 
 * looking for any characters that indicate a need to redirect I/O. 
 * 
 * Does nothing if there is no need to redirect I/O.
 */
void ioRedirection(char **args);

/* Check Time 
 * name     The name of the target we are going to compare 
 * head     The current node of a target linked list
 * 
 * checkTime compares the original target's last modified time to 
 * the current dependencie's last modified time and returns 0 if 
 * the target is newer than the dependencies and 1 if at least 
 * one of the dependencies is newer than the target.
 * 
 * Returns 0 if a target has no dependencies and is up to date. 
 */
int checkTime(char *name, struct Target *head);

/* Execute Dependencies 
 * head     The current node of a target linked list
 * 
 * execDepends recursively calls itself until the current target's
 * dependencies either don't match any given targets, or the current
 * target has no dependencies. 
 * 
 * It then works its way back up to the original target, calling 
 * each dependency target in reverse order.
 */ 
void execDepends(struct Target *head);

/* Expand
 * orig    	The input string that may contain variables to be expanded
 * new     	An output buffer that will contain a copy of orig with all 
 *         	variables expanded
 * newsize 	The size of the buffer pointed to by new.
 * 
 * Expand searches through a string for a variable to be expanded (starting
 * with ${ and ending with }). It then searches the environment for that 
 * expanded variable, then replaces the current with the new.
 *
 * Example: "Hello, ${PLACE}" will expand to "Hello, World" when the environment
 * variable PLACE="World". 
 */
int expand(char* orig, char* new, int newsize);

/* Execute Rules 
 * argc    A count of command-line arguments 
 * argv    The command-line argument valus
 * head    The first element in the Target linked list
 * 
 * This function takes in linked list of targets and their corresponding 
 * rules, then it iterates through target and compares them with the user 
 * input provided by argv. 
 * 
 * If the input matches the target name, the target's rules are then 
 * executed. Otherwise, no matching input will move onto the next user
 * input. (Currently no dependence integration)
 */
void executeRules(int argc, const char* argv[], struct Target *head);

/* Process Line
 * line    The command line to execute.
 * 
 * This function interprets line as a command line.  It creates a new child
 * process to execute the line and waits for that process to complete. 
 */
void processline(char* line);

/* Main entry point.
 * argc    A count of command-line arguments 
 * argv    The command-line argument valus
 *
 * Micro-make (umake) reads from the uMakefile in the current working
 * directory.  The file is read one line at a time.  Lines with a leading tab
 * character ('\t') are interpreted as a command and passed to processline minus
 * the leading tab.
 */
int main(int argc, const char* argv[]) {

  FILE* makefile = fopen("./uMakefile", "r");
  if(makefile == NULL){
        fprintf(stderr, "ERROR: Could not find uMakefile.\n");
        exit(1);
  }
  
  size_t  bufsize = 0;
  char*   line    = NULL;
  ssize_t linelen = getline(&line, &bufsize, makefile);

  struct Target *targets = createTarget();
  
  while(-1 != linelen) {

    if(line[linelen-1]=='\n') {
      linelen -= 1;
      line[linelen] = '\0';
    }

    for(int i = 0; i < strlen(line); i++){
        if(line[i] == '#'){
            line[i] = '\0';
        }
    }
	
    if(isTarget(line) == 1 && line[0] != '\0'){
        addTarget(targets, &line[0]);
    } else if (isTarget(line) != 1 && line[0] != '\t'){
        char* name = malloc(strlen(line));
        char* value = malloc(strlen(line));
        for(int i = 0; i < strlen(line); i++){
            if(line[i] == '='){
                line[i] = '\0';
                strcpy(name, line);
                strcpy(value, &line[i+1]);
                setenv(name, value, 1);
            }
        }
        free(name);
        free(value);
    } else if(line[0] == '\t'){
        addRules(targets, &line[0]);
    } 
	
    linelen = getline(&line, &bufsize, makefile);
  }
  executeRules(argc, argv, targets->next);
  
  freeAll(targets);
  free(targets);
  free(line);
  
  return EXIT_SUCCESS;
}

/* Process Linestat(name, &targStat) != 0
 * Creates a child process that calls arg_parse in order to split up the arguments in 'line', 
 * then uses execvp to execute the new child process, also calls the ioRedirection function in
 * the case that I/O needs to be redirected based on the rules of the target.
 */
void processline (char* line) {
  int count = 0;
  char new[BUFFER];
  char** args;
  
  if(expand(line, new, BUFFER) == 1){
	args = arg_parse(new, &count);
  }  
  else{ 
	args = arg_parse(line, &count);
  } 
 
  if(count != 0){
    const pid_t cpid = fork();
    switch(cpid) {
        
        case -1: {
            perror("fork");
            free(args);
            break;
        }

        case 0: {
            ioRedirection(args);
     
            execvp(*args, args);
            perror("execvp");
            free(args);
            exit(EXIT_FAILURE);
            break;
        }

        default: {
            int   status;
            const pid_t pid = wait(&status);
            if(-1 == pid) {
            perror("wait");
            }
            else if (pid != cpid) {
                fprintf(stderr, "wait: expected process %d, but waited for process %d",
                    cpid, pid);
            }
            break;
        }
    }

  }
  free(args);
}

/* IO Redirection 
 * args     A parsed line of rules to be passed into execvp
 * 
 * The ioRedirection function takes in a args and parses through it, 
 * looking for any characters that indicate a need to redirect I/O. 
 * 
 * If one of the IO characters is seen (>, >>, <), then uses open() 
 * in conjunction with dup2() to handle redirection of the I/O streams, 
 * lastly calling execvp() before exiting. 
 * 
 * Does nothing if there is no need to redirect I/O.
 */
void ioRedirection(char **args){
    for(int i = 0; args[i]!= NULL; i++){
        if(strcmp(args[i], ">") == 0){// Truncate 
            int output = open(args[i+1], O_TRUNC | O_WRONLY | O_CREAT, 0644); 
            args[i] = NULL;
            
            dup2(output, 1); 
            close(output);
            
            execvp(*args, args);
            perror("execvp");
            free(args);
            exit(EXIT_FAILURE);
            break;
        }

        if(strcmp(args[i], "<") == 0){// Input
            int input = open(args[i+1], O_RDONLY);
            args[i] = NULL;
            dup2(input, 0);
            
            for(int j = i+1; args[j] != NULL; j++){
                if(strcmp(args[j], ">") == 0){// Input and Truncate 
                    int output = open(args[j+1], O_TRUNC | O_WRONLY | O_CREAT, 0644); 
                    args[j] = NULL;
                    
                    dup2(output, 1);
                    close(output);
                    close(input);
                    
                    execvp(*args, args);
                    perror("execvp");
                    free(args);
                    exit(EXIT_FAILURE);
                    break;
                }
                if(strcmp(args[j], ">>") == 0){// Input and Append 
                    int output = open(args[j+1], O_WRONLY | O_APPEND | O_CREAT, 0644);
                    args[i] = NULL;
                    
                    dup2(output, 1); 
                    close(output);
                    close(input);
                    
                    execvp(*args, args);
                    perror("execvp");
                    free(args);
                    exit(EXIT_FAILURE);
                    break;
                }
            }	
            close(input);
            
            execvp(*args, args);
            perror("execvp");
            free(args);
            exit(EXIT_FAILURE);
            break;
        }

        if(strcmp(args[i], ">>") == 0){//Append
            int output = open(args[i+1], O_WRONLY | O_APPEND | O_CREAT, 0644);
            args[i] = NULL;
            
            dup2(output, 1);
            close(output);

            execvp(*args, args);
            perror("execvp");
            free(args);
            exit(EXIT_FAILURE);
            break;
        }
    }
}

/* Execute Rules
 * argc		The number of arguments in the command line.
 * argv[] 	The arguments entered in the command line.
 * head		The start of a target linked list. 
 *
 * Iterates through the target linked list, comparing the target name to 
 * the user input. 
 * 
 * If the two match, then the given target's rules are executed. If the
 * current user input does not match any targets in the list, move onto
 * the next user input.
 */
void executeRules(int argc, const char* argv[], struct Target *head){
    int i = 1; 
    struct Target *targList = head;
    while(i < argc){  
        if(strcmp(argv[i],targList->targetName) == 0){
            execDepends(targList);
            int flag = checkTime(targList->targetName, targList);
            if(flag == 1){
                struct Rules *current = targList->ruleList;
                while(current != NULL){
                    if(current->rulesList != NULL){
                        char *line = malloc(strlen(current->rulesList)+1);
                        strcpy(line, current->rulesList);
                        processline(line);
                        free(line);
                    }
                    current = current->next;
                }
                i++;
                targList = head;
            }
        }
        if(targList->next == NULL){
            i++;
            targList = head;
        }	
        targList = targList->next;
    }
}

/* Expand
 * orig    	The input string that may contain variables to be expanded
 * new     	An output buffer that will contain a copy of orig with all 
 *         	variables expanded
 * newsize 	The size of the buffer pointed to by new.
 *
 * Expand returns 1 upon a successfull expand or 0 upon failure. 
 */ 
int expand(char* orig, char* new, int newsize){
    char temp[newsize]; 
    char restOf[newsize];
    char tempOrig[newsize];
    strcpy(tempOrig, orig);
  
    int j = 0;
    int start = 0; 
    int end = 0;
    int endOfIndex = 0;
    int flag = 0;
    for(int i = 0; tempOrig[i] != '\0'; i++){
        if(tempOrig[i] == '$' && tempOrig[i+1] == '{'){
            i+=2;
            start = i;            
            while(tempOrig[i] != '}' && tempOrig[i] != '\0'){
                i++;
                j++;
            }
            if(tempOrig[i] == '}'){
                flag = 1;
                end = j;
                endOfIndex = i+1;             
                strncpy(temp, &tempOrig[start], end);
                temp[end] = '\0';
				
                strcpy(restOf, &tempOrig[endOfIndex]);
                char *expanded = getenv(temp);
                if(expanded != NULL) {
                    if(strlen(expanded) > BUFFER){
                        fprintf(stderr, "ERROR: Buffer Overflow \n");
                        exit(0);
                    }
                    strncpy(new, tempOrig, start-2);
                    new[start-2] = '\0';
                    strcpy(&new[start-2], expanded);
                    new[strlen(new)+1] = '\0';

                    strcpy(&new[strlen(new)], restOf); 
                    new[strlen(new)+1] = '\0';              
                    strcpy(tempOrig, new);
                } else if(expanded == NULL){
                    strncpy(new, tempOrig, start-2); 
                    new[start-2] = '\0';
                    
                    strcpy(&new[strlen(new)], restOf);
                    strcpy(tempOrig, new);
                    i = start-1; 
                }
                j = 0;
                start = 0; 
                end = 0;
                endOfIndex = 0;
            }			
        }   
    }
    if(start != 0 && flag == 0){
        fprintf(stderr, "ERROR: Mismatched braces \n");
        exit(0); 
    } else if(start == 0 && flag == 0){
        return 0;
    }
    return 1;
}

/* Execute Dependencies 
 * head     The current node of a target linked list
 * 
 * execDepends recursively calls itself until the current target's
 * dependencies either don't match any given targets, or the current
 * target has no dependencies. 
 * 
 * It then works its way back up to the original target, calling 
 * each dependency target in reverse order.
 */ 
void execDepends(struct Target *head){
    struct Target *targList = head;
    int dependCount = 0;
    char string[strlen(targList->dependencies)];
    strcpy(string, targList->dependencies);
	
    char** depen = arg_parse(string, &dependCount);
	
    for(int i = 0; i < dependCount; i++){
        struct Target *tempList = head; 
        while(tempList != NULL){
            if(strcmp(depen[i], tempList->targetName) == 0){
                struct Rules *tempRules = tempList->ruleList; 
                if(tempList->dependencies != NULL){
                    execDepends(tempList);
                }
                while(tempRules != NULL){
                    if(tempRules->rulesList != NULL){
                        int flag = checkTime(tempList->targetName, tempList);
                        if(flag == 1){
                            char *line = malloc(strlen(tempRules->rulesList)+1);
                            strcpy(line, tempRules->rulesList);
                            processline(line);
                            free(line);
                        }
                    }   
                    tempRules = tempRules->next;
                }       
            }
            tempList = tempList->next;
        }
    }
}

/* Check Time 
 * name     The name of the target we are going to compare 
 * head     The current node of a target linked list
 * 
 * checkTime compares the original target's last modified time to 
 * the current dependencie's last modified time and returns 0 if 
 * the target is newer than the dependencies and 1 if at least 
 * one of the dependencies is newer than the target.
 * 
 * If a target has no dependencies and it is up to date, 
 * 0 is returned. 
 */
int checkTime(char *name, struct Target *head){
    struct Target *targList = head;
    int dependCount = 0;
    char string[strlen(targList->dependencies)];
    strcpy(string, targList->dependencies);
    char** depen = arg_parse(string, &dependCount); 
	
    struct stat targStat; 
    stat(name, &targStat);
    time_t time1 = targStat.st_mtime;
    
    if(dependCount == 0 && stat(name, &targStat) == 0){
        return 0;
    } else if(stat(name, &targStat) != 0){
        return 1;
    }
    for(int i = 0; depen[i] != NULL; i++){		
        struct stat depenStat;	
        stat(depen[i], &depenStat);

        time_t time2 = depenStat.st_mtime;
        if(difftime(time1, time2) < 0){
            return 1;
        }
    }	
    return 0;
}
