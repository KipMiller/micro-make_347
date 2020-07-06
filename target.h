#ifndef __TARGET__H__
#define __TARGET__H__
/*
 *  Author: Chris Miller
 *  Date: 10/15/2018
 *  CS347 target.h
 * 
 */ 

/* Target Structure
 *
 * A linked list used to hold each target and its
 * corresponding dependencies in sequential order. 
 *
 * Also contains a Rules object, which is a linked
 * list of rules associated with the given target.
 */
 struct Target {
    char *targetName;
    char *dependencies; 

    struct Rules *ruleList;
    struct Target *next; 
};

/* Rules Structure
 *
 * A linked list used to hold rules associated
 * with a given target in sequential order. 
 */
struct Rules {
    char *rulesList;
    struct Rules *next;
};

//Type definitions of the two structures.
typedef struct Target *targ;
typedef struct Rules *rule;

/* Create Target 
 * A constructor for the target structure.  
 */
targ createTarget();

/* Create Rule
 * A constructor for the rule structure.
 */
rule createRule();

/* Is Target 
 * Function that will determine if the current line contains 
 * a target, a target is any character followed by a ':'.
 */
int isTarget(char *line);

/* Add Target 
 * head 	A pointer to the start of the target linked list.
 * line 	The current line from wich targets and dependencies
 * 		will be pulled from. 
 *
 * This function splits up a line (not starting with a tab character)
 * into a target (the first element preceding ':') and the dependencies
 * (anything after the ':'). 
 *
 * The function iterates to the end of the target list, allocates space
 * for a new node and then assigns variables to the new node, setting the 
 * next node in the list to be NULL.
 */
void addTarget(struct Target *head, char *line);

/* Add Rules
 * targHead	A pointer to the first target in the target list.
 * line	 	The current line from which rules will be assigned.
 *
 * This function iterates to the end of the target list, then iterates
 * to the end of that target node's rule list. 
 *
 * Once at the end of the rule list, it makes room for a new rule variable, 
 * and then assigns it to the contents of line, also setting next to NULL.
 */
void addRules(struct Target *targHead, char *line);

/* Print Targets
 * head 	The start of the target linked list. 
 *
 * Function to display all of the elements in the target list. 
 */ 
void printTargets(struct Target *head);

/* Print Rules
 * head 	The start of the target linked list. 
 * 
 * Function to display all of the rules in a rule list,
 * (which are contained within a list of targets). 
 */ 
void printRules(struct Rules *head);

/* Free All
 * head 	A pointer to the start of a target list
 *
 * This function simply frees up all of the rule objects
 * within each node of the given target list, ensuring 
 * that every space of memory has been returned.
 */
void freeAll(struct Target *head);

#endif
