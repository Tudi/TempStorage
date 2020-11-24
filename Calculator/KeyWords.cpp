#include "KeyWords.h"
#include <string>

//costructor
ExprElem::ExprElem(const char* pKey)
{
	if(pKey != NULL)
		Key = _strdup(pKey);
	else
	{
		Key = (char*)malloc(1);
		Key[0] = 0;
	}
	Result = NOT_A_VALID_VALUE;
	sign = 1;
}

//destructor to free up resources
ExprElem::~ExprElem()
{
	if(Key != NULL)
		free(Key);
	Key = NULL;
}

//strcmp with limited length
int ExprElem::IsKeyWordMatch(const char* expr)
{
	int i;
	for (i = 0; Key[i] != 0 && expr[i] != 0 && Key[i] == expr[i]; i++);
	if (Key[i] == 0 && Key[i - 1] == expr[i - 1])
		return i;
	return 0;
}

//clone state into another element
void ExprElem::CopyTo(ExprElem* dst)
{
	dst->ElemType = this->ElemType;
	dst->F = this->F;
	dst->Result = this->Result;
	dst->sign = this->sign;
}

//streak is a list of elements that multiply/divide.. and need to be moved around as 1 chunk
//functions are not checked, because we expect them to be converted to values before we inspect streaks
int ExprElem::IsPartOfStreak()
{
	if (ElemType == ET_Value)
		return 1;
	if (ElemType == ET_Variable)
		return 1;
	if (ElemType == ET_2ParamOperator && Key[0] == '*')
		return 1;
	if (ElemType == ET_2ParamOperator && Key[0] == '/')
		return 1;
	return 0;
}