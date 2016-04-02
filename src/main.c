/** "Using the command-line, the user will provide your program with a file
 name. This file name is a string. The string will have no spaces. The file
 name can be just the name of a file or the name of a file with a path.
 "If the user enters anything else then the program terminates immediately
 displaying an error message saying why it stopped and showing the command-line
 syntax for how the user should properly use the program.
 "Your program will syntax check the file the user entered at the command-line.
 Similar to a compiler it will display line errors to the user. The errors will
 display on the screen. The displayed errors will each contain the following
 information: the line number of the offending line, echo the offending line,
 display a marker indicating where the error is located in the line (three
 asterixes before the offence), and it will display an error message describing
 what it the program expecting, on a new line. Like a compiler, it will do this
 for every line in the file. Then the program terminates."

 @author	Neil
 @version	1; 2016-03
 @since		1; 2016-03 */

#include <stdio.h>  /* fprintf, fgets */
#include <string.h>	/* strlen */
#include <stdlib.h>	/* EXIT_* */
#include "syntax.h"	/* including syntax (error) */

/* constants */
static const char *programme   = "q1";
static const char *year        = "2016";
static const int versionMajor  = 1;
static const int versionMinor  = 0;

static const char *lf = "\n\r"; /* fixme: vertical tab, etc */
static const int debug = 0;

/* private */
static void usage(void);

/** Entry point.
 @param argc	The number of arguments, starting with the programme name.
 @param argv	The arguments.
 @return		Either EXIT_SUCCESS or EXIT_FAILURE.
 @fixme			Should be "q1 < file" instead of "q1 file"; much simpler and
				more robust. */
int main(int argc, char **argv) {
	char line[1024];
	int buffer_len;
	FILE *fp = 0;
	enum Error { E_NO, E_SYNTAX, E_FILE, E_LINE } error = E_NO;
	int line_no = 0;
	char *fn = 0;

	/* try */ do {

		/* check that one arg is passed */
		if(argc != 2) { error = E_SYNTAX; break; }
		fn = argv[1];

		/* open the file */
		if(!(fp = fopen(fn, "r"))) { error = E_FILE; break; }

		/* syntax check */
		while(fgets(line, sizeof line, fp)) {
			line_no++;
			if(debug) fprintf(stderr, "LINE %u: %s", line_no, line);
			/* "Every command or expression terminates with a carriage return
			 and line feed." -- too restrictive (Windows gah,) but test at least
			 new lines of any kind; fixme: \n is different on different
			 systems, try it out */
			buffer_len = strlen(line);
			if(!buffer_len || !strchr(lf, line[buffer_len - 1])) { error = E_LINE; break; }

			/* check for syntax; isValidExpression checks if the line is a
			 valid expression, including all the commands and expressions;
			 Expression (line) != expression (token) */
			if(isValidExpression(line)) continue;

			/* syntax error message */
			if(syntax.index < 0) {
				printf("%.16s%s:%u: %s", fn, strlen(fn) > 16 ? "..." : "",
					   line_no, line);
			} else {
				printf("%.16s%s:%u: %.*s***%s", fn, strlen(fn) > 16 ? "..." :
					   "", line_no, syntax.index, line, line + syntax.index);
			}
			printf("syntax error: %s\n\n", syntax.error);
		}
		if(error) break;
		if(ferror(fp)) { error = E_FILE; break; };

	} while(0); /* finally */ {

		if(fp && fclose(fp) && !error) error = E_FILE;

	} /* catch */ if(error) {

		char msg[64];
		snprintf(msg, sizeof msg, "%s line %u", fn, line_no);
		switch(error) {
			case E_SYNTAX:  usage(); break;
			case E_FILE:	perror(msg); break;
			case E_LINE:	fprintf(stderr, "%s: too long or not followed by new line.\n", msg); break;
			case E_NO:		break; /* won't get here */
		}
		return EXIT_FAILURE;

	}

	return EXIT_SUCCESS;
}

/** Prints command-line help. */
static void usage(void) {
	fprintf(stderr, "Usage: %s <filename>\n", programme);
	fprintf(stderr, "Version %d.%d.\n\n", versionMajor, versionMinor);
	fprintf(stderr, "%s %s Neil Edelman\n\n", programme, year);
}
