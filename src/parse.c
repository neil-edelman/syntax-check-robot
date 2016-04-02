/* "The file parse.c will contain the algorithms to parse a line of input. C
 has some parsing libraries but you have to parse these lines without the help
 of C libraries. Your parse.c file is your version of a library. The main.c
 file will use the functions you provide in parse.c to parse the data main.c
 reads from the input file."

 @author	Neil
 @version	1; 2016-03
 @since		1; 2016-03 */

#include <string.h>	/* strncpy strlen strpbrk etc */
#include <ctype.h>	/* is* */
#include <stdio.h>	/* snprintf */
#include "syntax.h"	/* including syntax (error) */
#include "parse.h"	/* including delimiters, quote */

/* global definition; means ",repeat 2,times turnon turnon TURNON,,,end ,,"
 will be accepted as valid, but I think it should; 'end' and ',' play duplicate
 roles and I'm making the desicion that makes the parsing simplest; or else
 you'd have the tokens dictated by a state machine */
const char *const delimiters = " ,\t\n\r"; /* fixme: esoteric, vt, etc? */
const char quote = '\"';

/* static data */
static const char *last_init;
static char buffer[1024], *buf_pos = buffer, *upcoming_token;
static const int buffer_size = sizeof buffer / sizeof(char);

/* private prototypes */
static char *next_token(void);
static int is_first_whitespace(const char *const str);

/* public */

/** "This will initialize the private parse.c buffer with the string passed in
 the parameter inputLine."
 <p>
 While parsing, inputLine must be held constant. */
void initBuffer(const char *const inputLine) {

	buffer[0]      = '\0';
	buf_pos        = buffer;
	upcoming_token = 0;

	if(!(last_init = inputLine)) return;

	/* fixme: if inputLine is too long, it will truncate it and not warn you */
	strncpy(buffer, inputLine, buffer_size);
	buffer[buffer_size - 1] = '\0';

	upcoming_token = next_token();

	syntax.index = -1;

}

/** "This function returns a 1 if there are still more tokens in the string,
 otherwise it returns 0. For example, if this is a string "my name is Bob",
 then this string has 4 tokens." */
int hasNextToken(void) {
	return upcoming_token ? 1 : 0;
}

/** "This function removes the first token from the buffer. The buffer is
 reduced in size since the token was removed. The removed token is returned to
 the caller. If the buffer is empty then this function returns NULL." */
char *nextToken(void) {
	char *const token = upcoming_token;
	syntax.index = token - buffer;
	upcoming_token = next_token();
	return token;
}

/** "This function reinitializes the private buffer with the string originally
 provided by the initBuffer() function call. This will effectively restart the
 token extraction process from the beginning." */
void vybrewind(void) {
	initBuffer(last_init);
}

/* private */

static char *next_token(void) {
	char *tok_start;

	/* advance the pointer to the first non-delimeter word */
	while(*buf_pos && strchr(delimiters, *buf_pos)) buf_pos++;
	tok_start = buf_pos;

	/* check special cases */
	if(!*buf_pos) { /* end-of-string */
		return 0;
	} else if(*buf_pos == quote) { /* double-quotes */
		char quote_str[2];
		buf_pos++;
		/* seach for the closing quotes; fixme: escape \" */
		quote_str[0] = quote;
		quote_str[1] = '\0';
		if(!(buf_pos = strpbrk(buf_pos, quote_str))) {
			snprintf(syntax.error, sizeof syntax.error,
				"unmatched quotes");
			buf_pos = buffer + strlen(buffer);
			return 0;
		}
		buf_pos++;
		/* ending is not followed by a whitespace? */
		if(!is_first_whitespace(buf_pos)) {
			snprintf(syntax.error, sizeof syntax.error,
				"closing quotes not followed by whitespace");
			buf_pos = strpbrk(buf_pos, delimiters);
			return 0;
		}
	} else if(isnumber(*buf_pos)) { /* numerical */
		while(isnumber(*(++buf_pos)));
		if(!is_first_whitespace(buf_pos)) {
			snprintf(syntax.error, sizeof syntax.error,
				"non-numeric value in number");
			buf_pos = strpbrk(buf_pos, delimiters);
			return 0;
		}
	}

	/* search for the next delimeter */
	if((buf_pos = strpbrk(buf_pos, delimiters))) *(buf_pos++) = '\0';

	return tok_start;
}

static int is_first_whitespace(const char *const str) {
	return !*str || strchr(delimiters, *str) ? -1 : 0;
}
