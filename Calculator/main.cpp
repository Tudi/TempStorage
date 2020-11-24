#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <list>
#include "Logger.h"
#include "PosixToInfix.h"
#include "KeyWords.h"
#include "Expression.h"

/*
* Contains hardcoded strings to print out string.
* Same string contained in the readme file
*/
void PrintHelp(int Type=0)
{
#define HelpString	R"(ToptalCalculator.exe <mathematic expression> [er=num] [v] [-h]
ex : ToptalCalculator.exe "1+2*3"
<mathematic expression>   Syntactically valid mathematic expression
                          See list of keywords usable
er=num                    Expected result. Used for automatic testing
                          Generate a warning if calculated result does
                          not match expected result
v                         Enable verbosity. Used for manually checking
                          the steps of solving an equation/expression
p                         Postfix notation. If not specified, infix notation
                          is implied
k                         Print keyword list)"
#define KeywordString R"(Keywords
	+			Addition
	-			Substraction
	*			Multiplication
	/			Division
	ln			logarithm base e
	Log			logarithm base 10
	Log100		logarithm base 100
	sin			sinus
	cos			cosinus
	tan			tangent
	ctan		cotangent
	pi			3.14... constant
	e			2.7... constant
	(			left paranhesis
	)			right paranthesis
	=			equal sign
	x			variable for calculating equation
	y			variable for calculating equation)"
	if(Type==0)
		printf(HelpString);
	else 
		printf(KeywordString);
}

/*
* Main function. Parse input parameters and start the parsing of input expression
*/
int main(int argc, char* argv[])
{
	//seems like not enough parameters received
	if (argc <= 1)
	{
		PrintHelp();
		return 0;
	}

	//create keyword list that will be used to parse expression
	InitExprElemList();

	//enable verbosity for the application
	int Verbosity = 0;
	//signal if the input is posfix, else we presume it is infix
	int PostfixNotation = 0;
	//used for testing. We can verify if the calculator works correctly if we test our output to expected output
	double ExpectedResult = NOT_A_VALID_VALUE;

	//parse input paramaters to customize program behavior
	for (int i = 1; i < argc; i++)
	{
		//enable verbosity
		if (argv[i][0] == 'v')
			Verbosity = 1;
		//posfix notation 
		if (argv[i][0] == 'p')
			PostfixNotation = 1;
		//expected result
		if (argv[i][0] == 'e' && argv[i][1] == 'r' && argv[i][2] == '=')
			ExpectedResult = atof(&argv[i][3]);
		//print help
		if (argv[i][0] == 'k')
		{
			PrintHelp(1);
			return 0;
		}
	}

	//tell logger class the amount of verbosity we expect
	if(Verbosity)
		sLog.SetLogLevelFlags(LL_Info | LL_Warning | LL_ERROR | LL_Always);

	//do not touch input string, copy it to a new one
	char* expr = _strdup(argv[1]);

	//maybe we can early detect some bad format
	DetectBadExpression(expr);

	//our solver is based on infix notation. We need to convert posfix to infix
	if (PostfixNotation)
	{
		char *t = ConvertPosixToInfix(expr);
		free(expr);
		expr = t;
		if (expr == NULL)
			exit(1);
		sLog.Log(LL_Info, __FILE__, __LINE__, "Expr as infix : %s", expr);
	}

	//maybe we can early detect some bad format
	DetectBadExpression(expr);

	//convert from string to elements we can jugle around
	std::list< ExprElem*> *ExprList = ParseStringToExpr(expr);

	//chances are some * operators are missing
	InsertMissingMultiplierOperators(ExprList);

	//does this expression have unknown variables in it ?
	int HasEQ = 0, HasVar = 0;
	for(auto itr= ExprList->begin(); itr != ExprList->end(); itr++)
		if ((*itr)->ElemType == ET_EQ)
			HasEQ = 1;
		else if ((*itr)->ElemType == ET_Variable)
			HasVar = 1;

	//let's check if we parsed it correctly
	PrintExpression(ExprList, 0);

	//try to solve it
	double result;
	if(HasEQ && HasVar)
		result = SolveEQ(ExprList);
	else
		result = SolveExpression(ExprList);

	//we did our best, print out the remaining elements as result
	if (result != NOT_A_VALID_VALUE)
		sLog.Log(LL_Always, __FILE__, __LINE__, "Result of the expression : %f\n", result);
	else
		PrintExpression(ExprList, LL_Always);

	//werify if result is expected result
	if(ExpectedResult != NOT_A_VALID_VALUE && round(ExpectedResult) != round(result))
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Expected result does not match ! : %f != %f\n", ExpectedResult, result);

	//try to guess what went wrong
	if (ExpectedResult == NOT_A_VALID_VALUE)
		DetectFailureReason(ExprList);

	sLog.Log(LL_Info, __FILE__, __LINE__, "Done solving the expression\n");

	//destroy the list
	for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
		delete (*itr);
	ExprList->clear();
	//other resource cleanups
	free(expr);

	//end of the program
	return 0;
}