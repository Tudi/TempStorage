#pragma once

#include <list>
class ExprElem;

//initialize list of known keywords into expression elements
void InitExprElemList();

//parse an infix expression string into expression storage
std::list<ExprElem*>* ParseStringToExpr(const char *expr);

//print out expression into a humanly readable format
void PrintExpression(std::list< ExprElem*>* ExprList,int Always);

//try to solve an expression
double SolveExpression(std::list< ExprElem*>* ExprList);

//check if it's a parsable value at a specific position
double GetValueAtPosition(const char* expr, int* pos);
void AdvanceCursorIfSpace(const char* expr, int* Pos);
ExprElem* GetExprElemAtPosition(const char* expr, int* pos);

//this expression is an equation. Try to solve it
//note that this is a very limited solver. Functions can not take a variable. Variable can not have a power ( ex x^2)
//can have one single variable either x or y
double SolveEQ(std::list< ExprElem*>* ExprList);

void CopyOperatorTo(char op, ExprElem* to);
//extern std::list<ExprElem*> ExprElems;

//expression might have missing multiplication operators
void InsertMissingMultiplierOperators(std::list< ExprElem*>* ExprList);

//detect failure reason
void DetectFailureReason(std::list< ExprElem*>* ExprList);
void DetectBadExpression(const char*);