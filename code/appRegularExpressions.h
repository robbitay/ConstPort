/*
File:   appRegularExpressions.h
Author: Taylor Robbins
Date:   08\13\2017
*/

#ifndef _REGULAR_EXPRESSIONS_H
#define _REGULAR_EXPRESSIONS_H

struct Regex_t
{
	LinkHeader_t header;
	
	char* name;
	char* expression;
};

struct RegexList_t
{
	MemoryArena_t* memArena;
	LinkedList_t list;
};

#endif // _REGULAR_EXPRESSIONS_H
