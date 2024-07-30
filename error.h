#ifndef S4TB_ERR_H
#define S4TB_ERR_H

void error_print_usage (void);
void error_bad_use (const char *const, ...);

void error_check_ptr (const void *const);
void error_at_lexer (const char *const, char*, const unsigned short, const unsigned short, ...);

#endif
