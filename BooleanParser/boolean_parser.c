#define syntax_error -1 // result of parser if expression is malformed

#define reject_character  (*scan)-- // used to back up scan when lexical error encountered

// Forward declarations
int boolean_or_sequence(const char* expression, int* scan);
int boolean_and_sequence(const char* expression, int* scan);

int boolean_primitive(const char* expression, int *scan)
// returns result of a subexpression consisting of just variable names or constants
{
    switch (expression[(*scan)++])
    {
    case '0': // constant for "false"
        return 0;
    case '1': // constant for "true"
        return 1;
    default:
        return syntax_error;
    }
    return syntax_error;
}

int boolean_term(const char* expression, int* scan)
// returns result of expression involving NOT or (...)
{
    int subexpression_result;
    switch (expression[(*scan)++])
    {
    case '~': // not operator
        subexpression_result = boolean_primitive(expression, scan);
        if (subexpression_result == syntax_error)
        {
            return syntax_error;
        }
        return !subexpression_result;
    case '(': // nested expression
        subexpression_result = boolean_or_sequence(expression, scan);
        if (subexpression_result == syntax_error)
        {
            return syntax_error;
        }
        if (expression[(*scan)++] == ')')
        {
            return subexpression_result;
        }
        return syntax_error;
    default:
        reject_character;
        return boolean_primitive(expression, scan);
    }
    return syntax_error;
}

int boolean_and_sequence(const char* expression, int* scan)
// returns result of expression of form   s1 & s2 & ...
{
    int subexpression_result = boolean_term(expression, scan);
    if (subexpression_result == syntax_error)
    {
        return syntax_error;
    }
    while (expression[(*scan)++] == '&') // "and" operator?
    {
        int subexpression2_result = boolean_term(expression, scan);
        if (subexpression2_result == syntax_error)
        {
            return syntax_error;
        }
        subexpression_result &= subexpression2_result;
    }
    reject_character; // undo overscan for '&'
    return subexpression_result;
}

int boolean_or_sequence(const char* expression, int* scan)
// returns result of expression of form of s1 | s2 | ...
{
    int subexpression_result = boolean_and_sequence(expression, scan);
    if (subexpression_result == syntax_error)
    {
        return syntax_error;
    }
    while (expression[(*scan)++] == '|') // "or" operator?
    {
        int subexpression2_result = boolean_primitive(expression, scan);
        if (subexpression2_result == syntax_error)
        {
            return syntax_error;
        }
        subexpression_result |= subexpression2_result;
    }
    reject_character; // undo overscan for '|'
    return subexpression_result;
}

int calculate_boolean_expression(const char* expression_to_evaluate)
// returns int==0 for boolean false;
// int==1 for boolean true
// int==-1 for malformed expression
{
    int subexpression_result;
    int scan = 0;
    subexpression_result = boolean_or_sequence(expression_to_evaluate , &scan);
    if (subexpression_result == syntax_error)
    {
        return syntax_error;
    }
    if (expression_to_evaluate[scan] == 0) // expression ends without excess junk in string?
    {
        return subexpression_result;
    }
    else
    {
        return syntax_error;
    }
}
