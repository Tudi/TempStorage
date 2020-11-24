#include <string>
#include "Expression.h"
#include "KeyWords.h"

/*
* push parts of a string into a stack of strings
*/
void AddToStack(std::list<char*>* ValueStack, const char* expr, int StartPos, int Position)
{
	//allocate memory to store part of the string
	int tLen = Position - StartPos;
	char* t = (char*)malloc(tLen + 2);
	for (int i = 0; i < tLen; i++)
		t[i] = expr[i + StartPos];
	t[tLen] = 0;
	//push the part of the string into the stack
	ValueStack->push_front(t);
}

/*
* Convert string from postfix to infix notation
*/
char* ConvertPosixToInfix(const char*expr)
{
	//parse from the end to the beggining
	size_t len = strlen(expr);
	int Position = 0;
	std::list<char*> ValueStack;
	//parse until we reach the end of the input string
	while (Position < (int)len)
	{
		int StartPos = Position;
		//we expect a value to come first
		double val = GetValueAtPosition(expr, &Position);
		if (val != NOT_A_VALID_VALUE)
		{
			AddToStack(&ValueStack, expr, StartPos, Position);
		}
		//after a value, chances are coems an operator or function
		else
		{
			//consider it an operator or a function
			ExprElem *exr = GetExprElemAtPosition(expr, &Position);
			//if it was not a value and not an operator than we have no idea what it is
			if (exr == NULL)
			{
				printf("Could not parse this expression\n");
				return NULL;
			}
			if(exr->ElemType == ET_0ParamFunction)
				AddToStack(&ValueStack, expr, StartPos, Position);
			else if (exr->ElemType == ET_1ParamFunction)
			{
				//pop 1 element from the stack
				char* val = *ValueStack.begin();
				ValueStack.pop_front();
				char NewElem[500];
				sprintf_s(NewElem, sizeof(NewElem), "%s(%s)", exr->GetKeyword(), val);
				free(val);
				ValueStack.push_front(_strdup(NewElem));
			}
			else if (exr->ElemType == ET_2ParamOperator)
			{
				//pop 1 element from the stack
				char* val1 = *ValueStack.begin();
				ValueStack.pop_front();
				char* val2 = *ValueStack.begin();
				ValueStack.pop_front();
				char NewElem[500];
				sprintf_s(NewElem, sizeof(NewElem), "(%s%s%s)", val2, exr->GetKeyword(), val1);
				free(val1);
				free(val2);
				ValueStack.push_front(_strdup(NewElem));
			}
		}
	}

	//something must have went wrong
	if (ValueStack.empty())
		return NULL;

	//at this point we should be having exactly 1 element in the valuestack
	return *ValueStack.begin();
}