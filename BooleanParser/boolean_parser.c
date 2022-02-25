#include <stddef.h>
#include <boolean_parser.h>

#define reject_character  (*scan)-- // used to back up scan when lexical error encountered

// Forward declarations
int boolean_or_sequence(const char* expression, int* scan, const char* variables);
int boolean_and_sequence(const char* expression, int* scan, const char* variables);

int boolean_primitive(const char* expression, int *scan, const char* variables)
// returns result of a subexpression consisting of just variable names or constants
{
    int caseVal = expression[(*scan)++];
    if (caseVal >= 'a' && caseVal <= 'z')
    {
        if (variables == NULL)
        {
            return BOOLEAN_PARSER_SYNTAX_ERROR;
        }
        return variables[caseVal - 'a'];
    }
    switch (caseVal)
    {
    case '0': // constant for "false"
        return 0;
    case '1': // constant for "true"
        return 1;
    default:
        return BOOLEAN_PARSER_SYNTAX_ERROR;
    }
    return BOOLEAN_PARSER_SYNTAX_ERROR;
}

int boolean_term(const char* expression, int* scan, const char* variables)
// returns result of expression involving NOT or (...)
{
    int subexpression_result;
    switch (expression[(*scan)++])
    {
    case '~': // not operator
    case '!': // not operator
        subexpression_result = boolean_primitive(expression, scan, variables);
        if (subexpression_result == BOOLEAN_PARSER_SYNTAX_ERROR)
        {
			reject_character;
			subexpression_result = boolean_term(expression, scan, variables);
			if (subexpression_result == BOOLEAN_PARSER_SYNTAX_ERROR)
			{
				return BOOLEAN_PARSER_SYNTAX_ERROR;
			}
        }
        return !subexpression_result;
    case '(': // nested expression
        subexpression_result = boolean_or_sequence(expression, scan, variables);
        if (subexpression_result == BOOLEAN_PARSER_SYNTAX_ERROR)
        {
            return BOOLEAN_PARSER_SYNTAX_ERROR;
        }
        if (expression[(*scan)++] == ')')
        {
            return subexpression_result;
        }
        return BOOLEAN_PARSER_SYNTAX_ERROR;
    default:
        reject_character;
        return boolean_primitive(expression, scan, variables);
    }
    return BOOLEAN_PARSER_SYNTAX_ERROR;
}

int boolean_and_sequence(const char* expression, int* scan, const char* variables)
// returns result of expression of form   s1 & s2 & ...
{
    int subexpression_result = boolean_term(expression, scan, variables);
    if (subexpression_result == BOOLEAN_PARSER_SYNTAX_ERROR)
    {
        return BOOLEAN_PARSER_SYNTAX_ERROR;
    }
    while (expression[(*scan)++] == '&') // "and" operator?
    {
        int subexpression2_result = boolean_term(expression, scan, variables);
        if (subexpression2_result == BOOLEAN_PARSER_SYNTAX_ERROR)
        {
            return BOOLEAN_PARSER_SYNTAX_ERROR;
        }
        subexpression_result &= subexpression2_result;
    }
    reject_character; // undo overscan for '&'
    return subexpression_result;
}

int boolean_or_sequence(const char* expression, int* scan, const char* variables)
// returns result of expression of form of s1 | s2 | ...
{
    int subexpression_result = boolean_and_sequence(expression, scan, variables);
    if (subexpression_result == BOOLEAN_PARSER_SYNTAX_ERROR)
    {
        return BOOLEAN_PARSER_SYNTAX_ERROR;
    }
    char bOperator = expression[(*scan)++];
    while (bOperator == '|' || bOperator == '&') // "or" operator?
    {
        int subexpression2_result = boolean_primitive(expression, scan, variables);
        if (subexpression2_result == BOOLEAN_PARSER_SYNTAX_ERROR)
        {
            return BOOLEAN_PARSER_SYNTAX_ERROR;
        }
        if (bOperator == '|')
        {
            subexpression_result |= subexpression2_result;
        }
        else
        {
            subexpression_result &= subexpression2_result;
        }
        bOperator = expression[(*scan)++];
    }
    reject_character; // undo overscan for '|'
    return subexpression_result;
}

int calculate_boolean_expression(const char* expression_to_evaluate, const char *variables)
// returns int==0 for boolean false;
// int==1 for boolean true
// int==-1 for malformed expression
{
    int subexpression_result;
    int scan = 0;
    subexpression_result = boolean_or_sequence(expression_to_evaluate, &scan, variables);
    if (subexpression_result == BOOLEAN_PARSER_SYNTAX_ERROR)
    {
        return BOOLEAN_PARSER_SYNTAX_ERROR;
    }
    if (expression_to_evaluate[scan] == 0) // expression ends without excess junk in string?
    {
        return subexpression_result;
    }
    else
    {
        return BOOLEAN_PARSER_SYNTAX_ERROR;
    }
}
