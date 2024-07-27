#ifndef S4TERM_ERROR_H
#define S4TERM_ERROR_H

void error_usage (void);
void error_fatal (const char *const, ...);

void error_check_ptr (const void *const);
void error_at_lexer (const char*, const char *const, unsigned short, unsigned short, ...);

#endif
