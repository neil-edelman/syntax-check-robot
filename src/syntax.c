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
#include "syntax.h"	/* including syntax (error) */
#include "parse.h"	/* including delimiters, quote */

/* should be f'n but we can't modify the prototypes; global definition */
struct Error syntax = { "no error", -1 };

/* static const data */

static const int debug = 0;

/* every token */
enum Tokens { NOT_TOKEN, TAKEASTEP, LEFT, RIGHT, PICKUP, DROP, TURNON, TURNOFF,
	REPEAT, TIMES, END, WHILE, NOT, DETECTMARKER, DO, SAY,
	NUMBER, STRING, COMMAND, COMMANDS };

/* alphabetised and in caps */
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

/* for offering suggestions; this should be tokens, but ordered by avatar;
 the string does not (and cannot, it's not bijective) have to be tokens.string,
 it's just used for printing */
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

/* valid syntax expression translated to single-char by tokens[] -- work with
 strings as opposed to int[] because of the library support */
static const char *avatars[] = {
	"",		/* blank line should be ignored */
	"$",	/* COMMAND */
	"R#T%",	/* REPEAT NUMBER TIMES COMMANDS */
	"Sa",	/* SAY STRING */
	"W!dD%"	/* WHILE NOT DETECTMARKER DO COMMANDS */
};
static const int avatars_size = sizeof avatars / sizeof(char *);

/* private prototypes */
static const struct Token *match_token(const char *const string);
static int token_compare(const void *a, const void *b);
static const char *match_expression(const char *const avatar);
static int expression_compare(const void *a, const void *b);
static char *expand_expression(const char *const avatar);
static char *reverse_token(const char avatar);
static int reverse_compare(const void *a, const void *b);
static const char *suggest_token(const char *const);
static const char *suggest_expression(const char *const);
static int tokstrcmp(const char *a, const char *b);

/* public */

/** "Returns 1 if the token is one of the valid robot commands, otherwise it
 returns 0."
 <p>
 It may or may not set synax.error, depending on wheather it's a valid token;
 parts of expressions are also valid syntax, but are not commands; these will
 not set syntax.error but will return false. If the return value is false, you
 can't be sure whether it's a syntax error or it's a valid token, but it's just
 not a command. The functionality is a subset of {@see isValidExpression}. */
int isValidCommand(const char *const token) {
	const struct Token *t;
	return (t = match_token(token)) && (t->avatar == '$') ? 1 : 0;
}

/** "Returns 1 if the expression agrees with one of the legal robot expressions,
 otherwise it returns 0."
 <p>
 "Expression" is a line, not in individal token. It checks the individual
 tokens, and then it checks the syntax of the line. If it returns false, it's
 guaranteed to set synax.error.
 <p>
 It uses parse.c to tokenise, so it will destroy any temp data that you have. */
int isValidExpression(const char *const expression) {
	const struct Token *token;
	int shift;
	char avatar[512] = "", *a, *b;

	/* check the arguments */
	if(!expression) {
		snprintf(syntax.error, sizeof syntax.error, "null expression");
		syntax.index = -1;
		return 0;
	}

	/* parse expression into Tokens and put them into the avatar (expression
	 buffer or whatever) */
	initBuffer(expression);
	a = avatar;
	while(hasNextToken()) {
		if(!(token = match_token(nextToken()))) return 0;
		/* danger! (undefined behaviour)
		 snprintf(avatar, sizeof avatar, "%s%c", avatar, token->avatar) */
		if(a >= avatar + sizeof avatar / sizeof(char) - 1 /*null*/) {
			snprintf(syntax.error, sizeof syntax.error,
					 "line too long; %u tokens", (int)sizeof avatar);
			/* index is set in parse.c */
			return 0;
		}
		*(a++) = token->avatar;
		*a     = '\0';
	}
	if(debug) fprintf(stderr, "isValidExp avatar before <%s>\n", avatar);

	/* group tokens together; it could have been combined with the previous
	 step for greater effecacity, but more confusion */
	while((b = strrchr(avatar, 'E'))) {
		for(a = b - 1; a >= avatar && *a == '$'; a--); a++;
		for(shift = b - a, *(a++) = '%'; (*a = *(a + shift)); a++);
	}
	if(debug) fprintf(stderr, "isValidExp avatar grouping <%s>\n", avatar);

	return match_expression(avatar) ? 1 : 0;
}

/* private */

/** Converts a string into a const struct Token or returns null and sets
 sytax.error. */
static const struct Token *match_token(const char *const token) {
	struct Token *t;

	/* strings and numbers; we've already vetted them in parse.c */
	if(!token)           return 0;
	if(*token == quote)  return tok_string;
	if(isnumber(*token)) return tok_number;
	/* or else it's, maybe, a token */
	if(!(t = bsearch(token, tokens, tokens_size, sizeof(struct Token), &token_compare))) {
		snprintf(syntax.error, sizeof syntax.error,
			"[%.16s%s] is not a valid command; did you mean, [%s]?", token,
			strlen(token) > 16 ? "..." : "", suggest_token(token));
		/* index is set in parse */
		return 0;
	}
	return t;
}

/** This is used in {@see match_token}. */
static int token_compare(const void *a, const void *b) {
	const char *key = a;
	const struct Token *elem = b;
	return tokstrcmp(key, elem->string);
}

/** Takes an expression avatar and compares with the list of valid; it returns
 the avatars entry or null. */
static const char *match_expression(const char *const avatar) {
	const char *match;

	if(!(match = bsearch(avatar, avatars, avatars_size, sizeof(char *), &expression_compare))) {
		snprintf(syntax.error, sizeof syntax.error,
			"[%.64s%s] is not a valid expression; did you mean, [%s]?",
			expand_expression(avatar), strlen(avatar) > 64 ? "..." : "",
			expand_expression(suggest_expression(avatar)));
		/* set the index to a negative because we don't have info on where */
		syntax.index = -1;
		return 0;
	}
	return match;
}

/** {@see match_expression} bsearch. */
static int expression_compare(const void *a, const void *b) {
	const char *key = a;
	const char *const*elem_ptr = b;
	const char *const elem = *elem_ptr;
	return strcmp(key, elem);
}

/** Takes "Sa#D" and expands it into "SAY <string> <number> DO" in a static
 buffer. There are four of them, so you could call it a maximum of four times at
 once without overruning. */
static char *expand_expression(const char *const avatar) {
	static char expand[4][1024];
	static int z;
	int y;
	const int n = strlen(avatar);
	int i;

	for(expand[z][0] = '\0', i = 0; i < n; i++) {
		snprintf(expand[z], sizeof expand[z], "%s%s%s", expand[z], i ? " " : "", reverse_token(avatar[i]));
	}
	y = z;
	z = (z + 1) & 3;
	return expand[y];
}

/** Takes a char and returns the meaning according to reverse; used in
 {@see expand_expression}. */
static char *reverse_token(const char avatar) {
	const struct Reverse *const r = bsearch(&avatar, reverse, reverse_size, sizeof(struct Reverse), &reverse_compare);
	return r ? r->string : "null";
}

/** {@see reverse_token} bsearch. */
static int reverse_compare(const void *a, const void *b) {
	const char key = *(const char *)a;
	const struct Reverse *elem = b;
	return key - elem->avatar;
}

/** O(n), but simple. Suggest, based on an arbitry token string, an actual
 token string. */
static const char *suggest_token(const char *const token) {
	const int max = tokens_size - 1;
	int lo = 0, hi = max, n = strlen(token), i;

	for(i = 0; i < n && lo != hi; i++) {
		while(hi > 0   && tokstrcmp(token, tokens[hi].string) < 0) hi--;
		while(lo < max && tokstrcmp(token, tokens[lo].string) > 0) lo++;
	}

	return tokens[lo].string;
}

/** Suggest, based on an arbitry avatar expression, an actual avatar
 expression. */
static const char *suggest_expression(const char *const avatar) {
	const int max = avatars_size - 1;
	int lo = 0, hi = max, n = strlen(avatar), i;

	for(i = 0; i < n && lo != hi; i++) {
		while(hi > 0   && avatar[i] < avatars[hi][i]) hi--;
		while(lo < max && avatar[i] > avatars[lo][i]) lo++;
	}

	return avatars[lo];
}

/* ANSI C89 does NOT have strcasecmp, stricmp, strcmpi, etc; that's POSIX, C++,
 Windows, or an extension -- just make our own; used in {@see tokstrcmp}. */
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

/** This is used in {@see token_compare} and {@see suggest_token}. b is in
 uppercase. */
static int tokstrcmp(const char *a, const char *b) {
	for( ; upper[(unsigned char)*a] == *b; a++, b++) if(*a == '\0') return 0;
	return upper[(unsigned char)*a] - *b;
}
