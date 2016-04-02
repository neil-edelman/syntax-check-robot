2016 Neil Edelman

neil dot edelman each mail dot mcgill dot ca

Version 1.0.

Usage:

bin/q1 <filename>

I have had several people ask me questions about Question 1 in Prof.
Joseph Vybihal's new assignment; it's actually fairly non-trivial
(and interesting!) for people who are taking their first course in
C. Let's do it! (of course, after the due date.) Here is the question,

"McGill University, COMP206 Software Systems, Assignment #3 March
201[6]

"Question 1: Syntax Checker Application [ . . . ]

"Your modular program must be made from these three files: main.c,
syntax.c, and parse.c. The files syntax.c and parse.c may have .h
files."

The assignment requires the following prototypes; all others will
be private.

parse.c
void initBuffer(char *inputLine);
BOOLEAN hasNextToken();
char *nextToken();
void rewind();

sytax.c
int isValidCommand(char *token);
int isValidExpression(char *expression);

"Using the command-line, the user will provide your program with a
file name. This file name is a string. The string will have no
spaces. The file name can be just the name of a file or the name
of a file with a path. [ . . . ]If the user enters anything else
then the program terminates immediately displaying an error message
saying why it stopped and showing the command-line syntax for how
the user should properly use the program.Your program will syntax
check the file the user entered at the command-line. Similar to a
compiler it will display line errors to the user. The errors will
display on the screen. The displayed errors will each contain the
following information: the line number of the offending line, echo
the offending line, display a marker indicating where the error is
located in the line (three asterixes before the offence), and it
will display an error message describing what it the program
expecting, on a new line. Like a compiler, it will do this for every
line in the file."

"The input file is a text file. It is the source file to a robot
programming language. This language has two elements: commands and
expressions. Our imaginary robot developer wants a program that
validates the syntax of their scripts. You are building this script
syntax validation program.  "A robot script is a series of commands
or expressions until the end of the source file. Commands and
expressions must fit within a single line. They cannot extend over
multiple lines. Every command or expression terminates with a
carriage return and line feed.  "A robot command is atomic. It is
a single word that directs the robot to do a specific self-contained
task. Legal commands are: TAKEASTEP, LEFT, RIGHT, PICKUP, DROP,
DETECTMARKER, TURNON, and TURNOFF. These commands are not case
sensitive, so Walk, walk, walK, and WALK are all valid.

"A robot expression is a statement that composes robot commands
into control structures. Legal expressions are: "REPEAT n TIMES
comma-separated-list-of-commands END -- The identifier n is any
integer number greater than zero. The expression "repeat" iterates
n times executing the comma-separated list of commands in the order
they are written. Note: you are only validating the syntax. You are
not executing thecode. Ex: repeat 4 time takeastep, left end

"WHILE NOT c DO comma-separated-list-of-commands END -- The identifier
c is any legal condition. In this assignment there is only one
condition and it is a robot command: DETECTMARKER. It returns a
true if the robot is standing on a "marker", otherwise it returns
false. Ex: while not DETECTMARKER do takeastep

"SAY "message" -- The robot speaks the words contained in the string
"message"."
