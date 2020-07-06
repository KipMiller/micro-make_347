/*
 *  Author: Chris Miller
 *  Date: 10/15/2018
 *  CS347 target.c
 *  
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include "target.h"

/* Create Target 
 * A constructor for the target structure.  
 */
targ createTarget(){
    targ temp; 
    temp = malloc(sizeof(struct Target));
    temp->ruleList = createRule();
	
    temp->targetName = NULL; 
    temp->dependencies = NULL;
	
    temp->next = NULL;
    return temp;
}

/* Create Rule
 * A constructor for the rule structure.
 */
rule createRule(){
    rule temp;
    temp = malloc(sizeof(struct Rules));
    temp->next = NULL;
    temp->rulesList = NULL;
    return temp;
}

/* Is Target 
 * line     The current line from which to look for a target.
 * 
 * Iterates through the line until finding a ':' character, 
 * returns 1 if there is a target within the line, 0 if otherwise.
 */
int isTarget(char *line){
    int ans = 0;
    for(int i = 0; i < strlen(line); i++){
        if(!isspace(line[i]) && ans == 0){
            ans = 1;
        }
        if(line[i] == ':' && ans == 1){
            //ans = 2;
            return 1;
        }
    }
    return 0;
}

/* Add Target 
 * head 	A pointer to the start of the target linked list.
 * line 	The current line from which targets and dependencies
 * 		will be pulled from. 
 *
 * This function splits up a line (not starting with a tab character)
 * into a target (the first element preceding ':') and the dependencies
 * (anything after the ':'). Also accounts for if the target is preceded
 * or followed by any form of whitespace, as long as there is ':'.
 *
 * The function iterates to the end of the target list, allocates space
 * for a new node and then assigns variables to the new node, setting the 
 * next node in the list to be NULL.
 */
void addTarget(struct Target *head, char *line){
    struct Target *current = head;
    while(current-> next != NULL){
        current = current->next; 
    }
    current->next = malloc(sizeof(struct Target));
    current->next->targetName = malloc(strlen(line)+1);
    current->next->dependencies = malloc(strlen(line)+1);
    current->next->ruleList = createRule();
    
    for(int i = 0; i < strlen(line); i++){
        if(line[i] == ':'){
            line[i]  = '\0';
            int j = 0;
            int index = -1;
            while(j < i){
                if(!isspace(line[j]) && index == -1){
                    index = j;
                }
                if(isspace(line[j])){
                    line[j] = '\0';
                }
                j++; 
            }
            strcpy(current->next->targetName, &line[index]);
            strcpy(current->next->dependencies, &line[i+1]);
        }
    }   
    current->next->next = NULL;
}

/* Add Rules
 * targHead		A pointer to the first target in the target list.
 * line	 		The current line from which rules will be assigned.
 *
 * This function iterates to the end of the target list, then iterates
 * to the end of that target node's rule list. 
 *
 * Once at the end of the rule list, it makes room for a new rule variable, 
 * and then assigns it to the contents of line, also setting next to NULL.
 */
void addRules(struct Target *targHead, char *line){
    struct Target *currentTarg = targHead;
    while(currentTarg-> next != NULL){
        currentTarg = currentTarg->next; 
    }
    
    struct Rules *current = currentTarg->ruleList;
    while(current->next != NULL){
        current = current->next; 
    }
    current->next = malloc(sizeof(struct Rules));
    current->next->rulesList = malloc(strlen(line)+1);
    
    strcpy(current->next->rulesList, line);
    current->next->next = NULL;
}

/* Print Targets
 * head 	The start of the target linked list. 
 *
 * Function to display all of the elements in the target list. 
 */ 
void printTargets(struct Target *head){
    struct Target *current = head;
    int i = 1;
    while(current != NULL){
        printf("Target #%d: %s \n", i, current->targetName);
        printf("Dependencies: %s \n", current->dependencies);
        printRules(current->ruleList);		
        current = current->next; 
        i++;
    }
}

/* Print Rules
 * head 	The start of the target linked list. 
 *
 * Function to display all of the rules in a rule list,
 * (which are contained within a list of targets). 
 */ 
void printRules(struct Rules *head){
    struct Rules *current = head;
    int index = 1; 
    while(current != NULL){
        printf("#%d Rules: %s \n",index, current->rulesList);
        index++;
        current = current->next;
    }
}

/* Free All
 * head 	A pointer to the start of a target list
 *
 * This function simply frees up all of the rule objects
 * within each node of the given target list, ensuring 
 * that every space of memory has been returned.
 */
void freeAll(struct Target *head){
    struct Target *current = head;
    while(current-> next != NULL){
        free(current->ruleList);    
        current = current->next;
    }
    free(current);
}

