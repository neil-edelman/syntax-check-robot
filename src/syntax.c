/* "Every token of data main.c reads will be validated by the file syntax.c.
 The file syntax.c will return true of false to main.c for each token of input
 main.c sends to syntax.c. If syntax.c returns a false then main.c displays an
 error message for the offending line, and then continues onto the next line.
 If an input line contains more than one error, your program only displays an
 error message for the first mistake it detects on that line. Then it discards
 the rest of the line and move on to the next input line from the file."

 @author	Neil
 @version	1; 2016-03
 @since		1; 2016-03 */

#include <stdlib.h> /* bsearch */
#include <stdio.h>  /* snprintf */
#include <string.h>	/* strlen */
#include "syntax.h"

/* would be really a fn, but we can't modify the prototypes; THIS IS A GLOBAL */
char global_syntax_error[128] = "no error";

/* static data */

/* everything */
enum Tokens { NOT_TOKEN, TAKEASTEP, LEFT, RIGHT, PICKUP, DROP, TURNON, TURNOFF,
	REPEAT, TIMES, END, WHILE, NOT, DETECTMARKER, DO, SAY,
	NUMBER, STRING, COMMAND, COMMANDS };

/* classifications of the above */
enum CommandType { PURE, SYNTAX, LITERAL };

/* alphabetised and in caps -- there is only one predictate, NOT DETECTMARKER,
 so it's easier to mark as SYNTAX; in general, this would be more flexible as
 PREDICATE, but it's not allowed by the assigment specifications */
static const struct Token {
	char *string;
	int id;
	enum CommandType type;
} tokens[] = {
	{ "",			NUMBER,		LITERAL },
	{ "",			STRING,		LITERAL },
	{"DETECTMARKER",DETECTMARKER,SYNTAX },
	{ "DO",			DO,			SYNTAX },
	{ "DROP",		DROP,		PURE },
	{ "END",		END,		SYNTAX },
	{ "LEFT",		LEFT,		PURE },
	{ "NOT",		NOT,		SYNTAX },
	{ "PICKUP",		PICKUP,		PURE },
	{ "REPEAT",		REPEAT,		SYNTAX },
	{ "RIGHT",		RIGHT,		PURE },
	{ "SAY",		SAY,		SYNTAX }, /* really CONSUMER */
	{ "TAKEASTEP",	TAKEASTEP,	PURE },
	{ "TIMES",		TIMES,		SYNTAX },
	{ "TURNOFF",	TURNOFF,	PURE },
	{ "TURNON",		TURNON,		PURE },
	{ "WHILE",		WHILE,		SYNTAX }
};
static const int tokens_size = sizeof tokens / sizeof(struct Token);

/* valid syntax */
static const enum Tokens expressions[][5] = {
	{ REPEAT,	NUMBER,	TIMES,			COMMANDS,	0 },
	{ WHILE,	NOT,	DETECTMARKER,	DO,			COMMANDS },
	{ SAY,		STRING,	0,				0,			0 },
	{ COMMAND,	0,		0,				0,			0 }
};

/* private prototypes */
int token_compare(const void *s1, const void *s2);
int tokstrcmp(const char *a, const char *b); /* not in ANSI C */

/** "Returns 1 if the token is one of the valid robot commands, otherwise it
 returns 0."
 <p>
 If it is not valid, guaranteed to set global_synax_error. */
int isValidCommand(const char *const token) {
	const struct Token *t = bsearch(token, tokens, tokens_size, sizeof(struct Token), &token_compare);
	if(!t || t->type != PURE) {
		snprintf(global_syntax_error, sizeof global_syntax_error,
				 "\"%.8s%s\" is not a valid command", token,
				 strlen(token) > 8 ? "..." : "");
		return 0;
	}
	return 1;
}

/** "Returns 1 if the expression agrees with one of the legal robot expressions,
 otherwise it returns 0."
 <p>
 If it is not valid, guaranteed to set global_synax_error.
 <p>
 Uhhh, don't do this! make it void and require . . . in fact . . . no. */
int isValidExpression(const char *const expression) {
	snprintf(global_syntax_error, sizeof global_syntax_error, "<%s>", "no");
	return 0;
}

/* private */

int token_compare(const void *s1, const void *s2) {
	const char *key = s1;
	const struct Token *elem = s2;
	return tokstrcmp(key, elem->string);
}

/* ANSI does NOT have strcasecmp, that's POSIX Issue 4, Version 2 and stricmp
 is Windows, C++, or an extension -- this is kind of overkill, but fast? */
static char upper[] = {
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,123,124,125,126,127,
	-0x80,-0x7f,-0x7e,-0x7d,-0x7c,-0x7b,-0x7a,-0x79,
	-0x78,-0x77,-0x76,-0x75,-0x74,-0x73,-0x72,-0x71,
	-0x70,-0x6f,-0x6e,-0x6d,-0x6c,-0x6b,-0x6a,-0x69,
	-0x68,-0x67,-0x66,-0x65,-0x64,-0x63,-0x62,-0x61,
	-0x60,-0x5f,-0x5e,-0x5d,-0x5c,-0x5b,-0x5a,-0x59,
	-0x58,-0x57,-0x56,-0x55,-0x54,-0x53,-0x52,-0x51,
	-0x50,-0x4f,-0x4e,-0x4d,-0x4c,-0x4b,-0x4a,-0x49,
	-0x48,-0x47,-0x46,-0x45,-0x44,-0x43,-0x42,-0x41,
	-0x40,-0x3f,-0x3e,-0x3d,-0x3c,-0x3b,-0x3a,-0x39,
	-0x38,-0x37,-0x36,-0x35,-0x34,-0x33,-0x32,-0x31,
	-0x30,-0x2f,-0x2e,-0x2d,-0x2c,-0x2b,-0x2a,-0x29,
	-0x28,-0x27,-0x26,-0x25,-0x24,-0x23,-0x22,-0x21,
	-0x20,-0x1f,-0x1e,-0x1d,-0x1c,-0x1b,-0x1a,-0x19,
	-0x18,-0x17,-0x16,-0x15,-0x14,-0x13,-0x12,-0x11,
	-0x10,-0x0f,-0x0e,-0x0d,-0x0c,-0x0b,-0x0a,-0x09,
	-0x08,-0x07,-0x06,-0x05,-0x04,-0x03,-0x02,-0x01
};
int tokstrcmp(const char *a, const char *b) {
	for( ; upper[(unsigned char)*a] == *b; a++, b++) if(*a == '\0') return 0;
	return upper[(unsigned char)*a] - *b;
}
