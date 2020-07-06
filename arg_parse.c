/*
 *  Author: Chris Miller
 *  Date: 10/15/2018
 *  CS347 arg_parse.c
 * 
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> 
#include <ctype.h>
#include "arg_parse.h"

/* Arg Count
 * Helper function for arg_pase, loops through 'line' counting the number of arguments and placing a NULL at the end of each word; 
 * so that process_line can correctly execute the parsed arguments. 
 * 
 * Arguments are any words seperated by a form of white space, and are then followed by a NULL character. 
 */
static int arg_count(char* line){
    int argCount = 0;
    int lineLen = strlen(line);
    
    for(int i = 0; i < lineLen; i++){
        if((!isspace(line[i]) && line[i] != '\0') && (isspace(line[i+1]) || line[i+1] == '\0')){
            argCount++;			
            line[i+1] = '\0';                 
        }
    }    
    return argCount;
}

/* Argument Parse 
 * Helper function for process_line, calls the arg_count function to help count and seperate the arguments in'line'. 
 * Then makes an array of pointers to the start of each word in line, making sure the final element in the array 
 * is a NULL character so that execvp can correctly execute. 
 *
 * Loops through 'line', marking the start of each argument as long as it is seperated by any form of white space (or NULL).
 */
char **arg_parse(char *line, int *argcp){
    int lineLen = strlen(line);
    *argcp = arg_count(line);
    
    char** args = malloc((*argcp+1)*sizeof(char *));
   
    int j = 0; 
    if(!isspace(line[j]) && line[j]!= '\0'){
        args[j] = &line[j];
        j++; 
    }
    for(int i = 0; i < lineLen; i++){	
        if((isspace(line[i]) || line[i] == '\0') && (!isspace(line[i+1])) && (line[i+1] != '\0')){
            args[j] = &line[i+1];
            j++;
        }
    }
    args[j] = NULL; 

	
    return args;
}

