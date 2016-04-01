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
#include <ctype.h>	/* is* */
#include "parse.h"
#include "syntax.h"

/* would be really a fn, but we can't modify the prototypes; THIS IS A GLOBAL */
char global_syntax_error[256] = "no error";
extern const char *const delimiters;
extern const char quote;

/* static data */

/* everything */
enum Tokens { NOT_TOKEN, TAKEASTEP, LEFT, RIGHT, PICKUP, DROP, TURNON, TURNOFF,
	REPEAT, TIMES, END, WHILE, NOT, DETECTMARKER, DO, SAY,
	NUMBER, STRING, COMMAND, COMMANDS };

/* alphabetised and in caps -- there is only one predictate, NOT DETECTMARKER,
 so it's easier to mark as SYNTAX; in general, this would be more flexible as
 PREDICATE, but it's difficult by the assigment specifications */
static const struct Token {
	char *string;
	int id;
	char avatar;
} tokens[] = {
	{ "",			NUMBER,		'#' },
	{ "",			STRING,		'a' },
	{ "",			COMMAND,	'$' },
	{ "",			COMMANDS,	'%' },
	{"DETECTMARKER",DETECTMARKER,'d'},
	{ "DO",			DO,			'D' },
	{ "DROP",		DROP,		'$' },
	{ "END",		END,		'E' },
	{ "LEFT",		LEFT,		'$' },
	{ "NOT",		NOT,		'!' },
	{ "PICKUP",		PICKUP,		'$' },
	{ "REPEAT",		REPEAT,		'R' },
	{ "RIGHT",		RIGHT,		'$' },
	{ "SAY",		SAY,		'S' },
	{ "TAKEASTEP",	TAKEASTEP,	'$' },
	{ "TIMES",		TIMES,		'T' },
	{ "TURNOFF",	TURNOFF,	'$' },
	{ "TURNON",		TURNON,		'$' },
	{ "WHILE",		WHILE,		'W' }
};
static const int tokens_size = sizeof tokens / sizeof(struct Token);
static const struct Token *const tok_number   = tokens + 0;
static const struct Token *const tok_string   = tokens + 1;
static const struct Token *const tok_command  = tokens + 2;
static const struct Token *const tok_commands = tokens + 3;

/* for offering suggestions */
static const struct Reverse {
	char avatar;
	char *string;
} reverse[] = {
	{ '!', "NOT" },
	{ '#', "<number>" },
	{ '$', "<command>" },
	{ '%', "<commands-followed-by-END>" },
	{ 'D', "DO" },
	{ 'E', "END" },
	{ 'R', "REPEAT" },
	{ 'S', "SAY" },
	{ 'T', "TIMES" },
	{ 'W', "WHILE" },
	{ 'a', "<message>" },
	{ 'd', "DETECTMARKER" }
};
static const int reverse_size = sizeof reverse / sizeof(struct Reverse);

/* valid syntax expression -- work with strings as opposed to int[] because of
 the library support; also, familiar */
static const char *avatars[] = {
	"$",	/* COMMAND */
	"R#T%",	/* REPEAT NUMBER TIMES COMMANDS */
	"Sa",	/* SAY STRING */
	"W!dD%"	/* WHILE NOT DETECTMARKER DO COMMANDS */
};
static const int avatars_size = sizeof avatars / sizeof(char *);

/* private prototypes */
static const struct Token *match_token(const char *const string);
static int token_compare(const void *a, const void *b);
static int tokstrcmp(const char *a, const char *b);
static int match_expression(const char *const expression);
static int expression_compare(const void *a, const void *b);
static const char *suggest_expression(const char *const close_to);
static char *expand_expression(const char *const avatar);
static char *reverse_token(const char avatar);
static int reverse_compare(const void *a, const void *b);

/** "Returns 1 if the token is one of the valid robot commands, otherwise it
 returns 0."
 <p>
 It may or may not set global_synax_error, depending on wheather it's a valid
 token; expressions are also valid syntax, but are not commands. */
int isValidCommand(const char *const token) {
	const struct Token *t;
	return (t = match_token(token)) && (t->avatar == '$') ? 1 : 0;
}

/** "Returns 1 if the expression agrees with one of the legal robot expressions,
 otherwise it returns 0." This will check a line (the expression) vs one
 'expression.' It's much easier, less code duplication, less sharing, to do it
 here.
 <p>
 If it is not valid and expression is not null, guaranteed to set
 global_synax_error.
 <p>
 It uses parse.c to tokenise, so it will destroy any temp data that you have. */
int isValidExpression(const char *const expression) {
	const struct Token *token;
	int shift;
	char avatar[512] = "", *a, *b;

	/* check the arguments */
	if(!expression) return 0;

	/* parse expression into Tokens and put them into the avatar (expression
	 buffer or whatever) */
	initBuffer(expression);
	while(hasNextToken()) {
		if(!(token = match_token(nextToken()))) return 0;
		if(snprintf(avatar, sizeof avatar, "%s%c", avatar, token->avatar) >= (int)sizeof avatar) {
			snprintf(global_syntax_error, sizeof global_syntax_error,
					 "line too long; %u tokens", (int)sizeof avatar);
			return 0;
		}
	}
	printf("avatar before grouping <%s>\n", avatar);

	/* group tokens together; it could have been combined with the previous
	 step for greater effecacity, but more confusion; this takes steps which
	 are understandable */
	while((b = strrchr(avatar, 'E'))) {
		for(a = b - 1; a >= avatar && *a == '$'; a--); a++;
		/* snprintf(avatar, sizeof avatar, "%.*s%%%s", a - avatar, avatar,
		 b + 1); POSIX: "if copying takes place between objects that overlap
		 . . . the results are undefined." well, it's faster just to . . . */
		for(shift = b - a, *(a++) = '%'; (*a = *(a + shift)); a++);
	}
	printf("avatar after grouping <%s>\n", avatar);

	return match_expression(avatar) ? 1 : 0;
}

/* private */

/** sets global_syntax_error on syntax error */
static const struct Token *match_token(const char *const token) {
	struct Token *t;

	/* strings and numbers; we've already vetted them in parse.c */
	if(!token)           return 0;
	if(*token == quote)  return tok_string;
	if(isnumber(*token)) return tok_number;
	/* or else it's, maybe, a token */
	if(!(t = bsearch(token, tokens, tokens_size, sizeof(struct Token), &token_compare))) {
		snprintf(global_syntax_error, sizeof global_syntax_error,
			"\"%.8s%s\" is not a valid command", token,
			strlen(token) > 8 ? "..." : "");
		/************************ fix: add ************************/
	}
	return t;
}

static int token_compare(const void *a, const void *b) {
	const char *key = a;
	const struct Token *elem = b;
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
static int tokstrcmp(const char *a, const char *b) {
	for( ; upper[(unsigned char)*a] == *b; a++, b++) if(*a == '\0') return 0;
	return upper[(unsigned char)*a] - *b;
}

static char *suggest_token(const char *const close_to) {
	return "no";
}

static int match_expression(const char *const expression) {
	if(!bsearch(expression, avatars, avatars_size, sizeof(char *), &expression_compare)) {
		snprintf(global_syntax_error, sizeof global_syntax_error,
				 "\"%.64s%s\" is not a valid expression; nearest match \"%s\"",
				 expand_expression(expression), strlen(expression) > 64 ? "..." : "",
				 expand_expression(suggest_expression(expression)));
		return 0;
	}
	return -1;
}

static int expression_compare(const void *a, const void *b) {
	const char *key = a;
	const char *const*elem_ptr = b;
	const char *const elem = *elem_ptr;
	return strcmp(key, elem);
}

/* this crashes sometimes */
static const char *suggest_expression(const char *const avatar) {
	int lo = 0, hi = avatars_size - 1, n = strlen(avatar), i;

	for(i = 0; i < n; i++) {
		/* slightly inefficent */
		while(hi > 0                && avatar[i] < avatars[hi][i]) hi--;
		while(lo < avatars_size - 1 && avatar[i] > avatars[lo][i]) lo++;
		/*printf(" - hi %d lo %d avatar[%i] = %c\n", hi, lo, i, avatar[i]);*/
		if(lo == hi) break;
	}

	return avatars[lo];
	
	/****************** update global-pos ****************/
}

static char *expand_expression(const char *const avatar) {
	static char expand[2][1024];
	static int z;
	int y;
	const int n = strlen(avatar);
	int i;

	for(expand[z][0] = '\0', i = 0; i < n; i++) {
		snprintf(expand[z], sizeof expand[z], "%s%s%s", expand[z], i ? " " : "", reverse_token(avatar[i]));
	}
	y = z;
	z = (z + 1) & 1;
	return expand[y];
}

static char *reverse_token(const char avatar) {
	const struct Reverse *const r = bsearch(&avatar, reverse, reverse_size, sizeof(struct Reverse), &reverse_compare);
	return r ? r->string : "null";
}

static int reverse_compare(const void *a, const void *b) {
	const char key = *(const char *)a;
	const struct Reverse *elem = b;
	return key - elem->avatar;
}
