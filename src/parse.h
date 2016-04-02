extern const char *const delimiters;
extern const char quote;

void initBuffer(const char *const inputLine);
int hasNextToken(void);
char *nextToken(void);
void vybrewind(void);
