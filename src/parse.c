/* "The file parse.c will contain the algorithms to parse a line of input. C
 has some parsing libraries but you have to parse these lines without the help
 of C libraries. Your parse.c file is your version of a library. The main.c
 file will use the functions you provide in parse.c to parse the data main.c
 reads from the input file."

 @author	Neil
 @version	1; 2016-03
 @since		1; 2016-03 */

#include <string.h>	/* strncpy strlen etc */
#include "parse.h"

/* keeps track of the position; THIS IS A GLOBAL */
unsigned global_parse_index;

/* constants */
static const char *const delimiters = " ,\t\n\r"; /* fixme */
static const char quote = '\"';

/* static data */
static const char *last_init;
static char buffer[1024], *buf_pos = buffer, *upcoming_token;
static const int buffer_size = sizeof buffer / sizeof(char);

/* private prototypes */
char *next_token(void);

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
	global_parse_index = token - buffer;
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

char *next_token(void) {
	char *tok_start;

	/* advance the pointer to the first non-delimeter word */
	while(*buf_pos && strchr(delimiters, *buf_pos)) buf_pos++;
	if(!*buf_pos) return 0;
	tok_start = buf_pos;

	/* search for the next delimeter */
	if((buf_pos = strpbrk(buf_pos, delimiters))) *(buf_pos++) = '\0';

	return tok_start;
}
