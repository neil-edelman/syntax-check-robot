extern struct Error {
	char error[256];
	int index;
} syntax;

int isValidCommand(const char *const token);
int isValidExpression(const char *const expression);
