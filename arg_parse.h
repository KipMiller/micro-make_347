#ifndef __ARG_PARSE__H__
#define __ARG_PARSE__H__
/*
 *  Author: Chris Miller
 *  Date: 10/15/2018
 *  CS347 arg_parse.h
 * 
 */ 

/* Arg Parse 
 * line    The command line that will be parsed out.
 *
 * This function returns an array of pointers to characters in line.
 * Used to split up one string into several strings.
 */
char **arg_parse(char* line, int *argcp);

#endif