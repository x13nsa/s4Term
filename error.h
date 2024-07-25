#ifndef S4TERM_ERR_H
#define S4TERM_ERR_H

void error_usage (void);
void error_fatal (const char*, ...);

void error_check_ptr (const void *const);

#endif
