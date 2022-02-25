#ifndef _BOOLEAN_PARSER_H_
#define _BOOLEAN_PARSER_H_

#define BOOLEAN_PARSER_SYNTAX_ERROR -1 // result of parser if expression is malformed

/*
* Copy pasted from stackoverflow : https://stackoverflow.com/questions/26947117/parse-and-calculate-boolean-expression-in-c
*/
int calculate_boolean_expression(const char *str, const char* variables);
#endif