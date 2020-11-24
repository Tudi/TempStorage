#include "KeyWords.h"
#include <list>
#include "Logger.h"

std::list< ExprElem*> ExprElems;

void CreateExprElem(const char* Key, ElementTypes pElemType, ParamFuncDef F)
{
	//init a new ExprElem
	ExprElem* kw = new ExprElem(Key);
	kw->ElemType = pElemType;
	kw->F = F;
	//add to the list of ExprElems
	ExprElems.push_back(kw);
}

double Adder(double* p)
{
	return p[0] + p[1];
}

double Substractor(double* p)
{
	return p[0] - p[1];
}

double Mult(double* p)
{
	return p[0] * p[1];
}

double Divider(double* p)
{
	return p[0] / p[1];
}

double Log10(double* p)
{
	return log10(p[0]);
}

double Log100(double* p)
{
	return log(p[0])/log(100);
}

double Loge(double* p)
{
	return log(p[0]);
}

double sin_(double* p)
{
	return sin(p[0]);
}

double cos_(double* p)
{
	return cos(p[0]);
}

double tan_(double* p)
{
	return atan(p[0]);
}

double ctan_(double* p)
{
	return tan(p[0]);
}

double pi_(double* p)
{
	return 3.14159265359;
}

double e_(double* p)
{
	return 2.71828182846;
}

void InitExprElemList()
{
	CreateExprElem("+", ET_2ParamOperator, &Adder);
	CreateExprElem("-", ET_2ParamOperator, &Substractor);
	CreateExprElem("*", ET_2ParamOperator, &Mult);
	CreateExprElem("/", ET_2ParamOperator, &Divider);
	CreateExprElem("ln", ET_1ParamFunction, &Loge);
	CreateExprElem("Log100", ET_1ParamFunction, &Log100);
	CreateExprElem("Log", ET_1ParamFunction, &Log10);
	CreateExprElem("sin", ET_1ParamFunction, &sin_);
	CreateExprElem("cos", ET_1ParamFunction, &cos_);
	CreateExprElem("tan", ET_1ParamFunction, &tan_);
	CreateExprElem("ctan", ET_1ParamFunction, &ctan_);
	CreateExprElem("pi", ET_0ParamFunction, &pi_);
	CreateExprElem("e", ET_0ParamFunction, &e_);
	CreateExprElem("(", ET_ParenthesisL, NULL);
	CreateExprElem(")", ET_ParenthesisR, NULL);
	CreateExprElem("=", ET_EQ, NULL);
	CreateExprElem("x", ET_Variable, NULL);
	CreateExprElem("y", ET_Variable, NULL);
}


void AdvanceCursorIfSpace(const char* expr, int* Pos)
{
	while (expr[*Pos] == ' ')
		*Pos += 1;
}

ExprElem* GetExprElemAtPosition(const char* expr, int* pos)
{
	AdvanceCursorIfSpace(expr, pos);
	for (auto itr = ExprElems.begin(); itr != ExprElems.end(); itr++)
	{
		//check if ExprElem matches
		int MatchLen = (*itr)->IsKeyWordMatch(&expr[*pos]);
		if (MatchLen > 0)
		{
			*pos += MatchLen;
			ExprElem* ret = new ExprElem((*itr)->GetKeyword());
			(*itr)->ExprElem::CopyTo(ret);
			return ret;
		}
	}
	return NULL;
}

double GetValueAtPosition(const char* expr, int* pos)
{
	AdvanceCursorIfSpace(expr, pos);
	double ret = 0;
	int Pos = *pos;
	double Divider = 0;
	int IsNegative = 0;
	if (expr[Pos] == '-')
	{
		IsNegative = 1;
		Pos += 1;
	}
	int DigitExtractionStart = Pos;
	while ((expr[Pos] >= '0' && expr[Pos] <= '9') || expr[Pos] == '.')
	{
		if (expr[Pos] == '.')
		{
			Divider = 10;
			Pos++;
			continue;
		}
		double digit = expr[Pos] - '0';
		if (Divider != 0)
		{
			digit /= Divider;
			Divider *= 10;
			ret += digit;
		}
		else
			ret = ret * 10 + digit;
		Pos += 1;
	}
	if (Pos == DigitExtractionStart)
		return NOT_A_VALID_VALUE;
	if (IsNegative)
		ret = -ret;
	*pos = Pos;
	return ret;
}

void CopyOperatorTo(char op, ExprElem* to)
{
	for (auto itr = ExprElems.begin(); itr != ExprElems.end(); itr++)
	{
		if ((*itr)->GetKeyword()[0] != op)
			continue;
		(*itr)->ExprElem::CopyTo(to);
		return;
	}
}

std::list<ExprElem*>* ParseStringToExpr(const char* expr)
{
	std::list<ExprElem*>* ret = new std::list<ExprElem*>();

	//we presume it's infix
	int ParserPosition = 0;
	int ExprLen = (int)strlen(expr);
	ExprElem* Prevkw = NULL;
	//check what starts where. Might help us organize parenthesis and qual sign
	while (ParserPosition < ExprLen)
	{
		ExprElem* kw = NULL;
		double v[2];

		//expecting a constant value preferably if previous element was not a constant value
		if (Prevkw == NULL || Prevkw->ElemType == ET_2ParamOperator || Prevkw->ElemType == ET_ParenthesisL || Prevkw->ElemType == ET_EQ)
		{
			v[0] = GetValueAtPosition(expr, &ParserPosition);
			if (v[0] != NOT_A_VALID_VALUE)
			{
				kw = new ExprElem("");
				kw->ElemType = ET_Value;
				kw->Result = v[0];
			}
		}

		//try to extract keyword if it was not a constant value
		if (kw == NULL)
			kw = GetExprElemAtPosition(expr, &ParserPosition);

		//if it was not a prefered setup, maybe we indeed have a value with sign at this position
		if (kw == NULL)
		{
			v[0] = GetValueAtPosition(expr, &ParserPosition);
			if (v[0] != NOT_A_VALID_VALUE)
			{
				kw = new ExprElem("");
				kw->ElemType = ET_Value;
				kw->Result = v[0];
			}
		}

		//neither a constant or a recognized keyword ?
		if (kw == NULL)
		{
			sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error : unable to parse at : %s\n", &expr[ParserPosition]);
			exit(1);
		}

		ret->push_back(kw);
		Prevkw = kw;
	}

	return ret;
}

void PrintExpression(std::list< ExprElem*>* ExprList, int Always = 0)
{
	sLog.Log(LL_Info, __FILE__, __LINE__, "Expr now : ");
	for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
	{
		if ((*itr)->ElemType == ET_Value)
		{
			double result = (*itr)->Result * (*itr)->sign;
			if((int)result == result)
				sLog.Log(LL_Info | Always, __FILE__, __LINE__, "%d", (int)result);
			else
				sLog.Log(LL_Info | Always, __FILE__, __LINE__, "%f", result);
		}
		else
			sLog.Log(LL_Info | Always, __FILE__, __LINE__, "%s", (*itr)->GetKeyword());
	}
	sLog.Log(LL_Info | Always, __FILE__, __LINE__, "\n");
}

int SimplifyExpression(std::list< ExprElem*>* ExprList, int PriorityParse)
{
	for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
	{
		ExprElem* kw = (*itr);

		//we can solve this. It has 0 dependency
		if (kw->ElemType == ET_0ParamFunction)
		{
			kw->Result = kw->F(NULL) * kw->sign;
			kw->sign = 1;
			kw->ElemType = ET_Value;
			//start parsing the expression again
			return 1;
		}

		//if we have ( value ), simplify it to : value
		if (kw->ElemType == ET_ParenthesisL)
		{
			auto itr2 = itr;
			itr2++;
			if (itr2 == ExprList->end())
			{
				sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error in expression while looking to close parenthesis\n");
				exit(1);
			}
			auto itr3 = itr2;
			itr3++;
			if (itr3 == ExprList->end())
			{
				sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error in expression while looking to close parenthesis\n");
				exit(1);
			}
			if (((*itr2)->ElemType == ET_Value || (*itr2)->ElemType == ET_Variable) && (*itr3)->ElemType == ET_ParenthesisR)
			{
				delete (*itr);
				delete (*itr3);
				ExprList->erase(itr);
				ExprList->erase(itr3);
				//reparse the expression
				return 1;
			}
		}

		if (kw->ElemType == ET_1ParamFunction)
		{
			//check if we have value ready for this function
			auto itr2 = itr;
			itr2++;
			if (itr2 != ExprList->end() && (*itr2)->ElemType == ET_Value)
			{
				ExprElem* kw2 = (*itr2); // get the value for this function
				kw->Result = kw->F(&kw2->Result) * kw->sign;
				kw->ElemType = ET_Value;
				kw->sign = 1;
				sLog.Log(LL_Info, __FILE__, __LINE__, "solving : %s(%f) eq %f\n", kw->GetKeyword(), kw2->Result, kw->Result);
				//remove the used value 
				delete (*itr2);
				ExprList->erase(itr2);
				//reparse the expression
				return 1;
			}
		}

		//if we have : a + b, simplify it to : value
		if (kw->ElemType == ET_2ParamOperator
			&& ((PriorityParse == 1 && (kw->GetKeyword()[0] == '*' || kw->GetKeyword()[0] == '/'))
				|| (PriorityParse == 0 && (kw->GetKeyword()[0] == '+' || kw->GetKeyword()[0] == '-')))
			)
		{
			auto itr2 = itr;
			if(itr2 == ExprList->begin())
			{
				sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error in expression while looking for operator value\n");
				exit(1);
			}
			itr2--;
			if (itr2 == ExprList->end())
			{
				sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error in expression while looking for operator value\n");
				exit(1);
			}
			//special case of missing bracket. ex : x/b*c -> do not calculate b*c
			if (itr2 != ExprList->begin())
			{
				auto itr4 = itr2;
				itr4--;
				if (itr4 != ExprList->end() && (*itr4)->ElemType == ET_2ParamOperator && (*itr4)->GetKeyword()[0] == '/')
					continue;
			}
			auto itr3 = itr;
			itr3++;
			if (itr3 == ExprList->end())
			{
				sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error in expression while looking for operator value\n");
				exit(1);
			}
			//another case : a+b*c -> do not add a+b
			if (kw->GetKeyword()[0] == '+' || kw->GetKeyword()[0] == '-')
			{
				auto itr4 = itr3;
				itr4++;
				if (itr4 != ExprList->end())
				{
					if ((*itr4)->ElemType == ET_2ParamOperator && ((*itr4)->GetKeyword()[0] == '*' || (*itr4)->GetKeyword()[0] == '/'))
						continue;
				}
			}

			if ((*itr2)->ElemType == ET_Value && (*itr3)->ElemType == ET_Value)
			{
				ExprElem* kw2 = (*itr2); // get the value for this function
				ExprElem* kw3 = (*itr3); // get the value for this function
				double v[2];
				v[0] = kw2->Result * kw2->sign;
				v[1] = kw3->Result * kw3->sign;
				kw->Result = kw->F(v) * kw->sign;
				kw->ElemType = ET_Value;
				kw->sign = 1;
				sLog.Log(LL_Info, __FILE__, __LINE__, "solving : %f %s %f eq %f\n", kw2->Result, kw->GetKeyword(), kw3->Result, kw->Result);

				delete (*itr2);
				delete (*itr3);
				ExprList->erase(itr2);
				ExprList->erase(itr3);
				//reparse the expression
				return 1;
			}
		}
	}
	//was unable to simplify
	return 0;
}

double SolveExpression(std::list< ExprElem*>* ExprList)
{
	while (ExprList->size() > 1)
	{
		//try to solve at least part of the expression
		if (SimplifyExpression(ExprList, 1) == 0)
			if (SimplifyExpression(ExprList, 0) == 0)
			{
				if (ExprList->size() != 1)
					return NOT_A_VALID_VALUE;
			}
		//show how the expression evolved so far
		PrintExpression(ExprList);
	}

	if (ExprList->empty())
		return NOT_A_VALID_VALUE;

	ExprElem* kw = (*ExprList->begin());
	if (kw && kw->ElemType == ET_Value)
		return kw->Result;

	return NOT_A_VALID_VALUE;
}

void InsertMissingMultiplierOperators(std::list< ExprElem*>* ExprList)
{
	for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
	{
		auto itr2 = itr;
		itr2++;
		if (itr2 == ExprList->end())
			break;
		int AddMult = 0;
		if ((*itr)->ElemType == ET_Value)
		{
			//check the next value. If it's a function,variable,bracket.. chances are this is a multiplier
			if ((*itr2)->ElemType == ET_Variable
				|| (*itr2)->ElemType == ET_0ParamFunction
				|| (*itr2)->ElemType == ET_1ParamFunction
				|| (*itr2)->ElemType == ET_ParenthesisL
				)
				AddMult = 1;
		}
		if ((*itr)->ElemType == ET_ParenthesisR
			|| (*itr)->ElemType == ET_0ParamFunction
			|| (*itr)->ElemType == ET_Variable
			)
		{
			if ((*itr2)->ElemType == ET_Variable
				|| (*itr2)->ElemType == ET_0ParamFunction
				|| (*itr2)->ElemType == ET_1ParamFunction
				|| (*itr2)->ElemType == ET_ParenthesisL
				|| (*itr2)->ElemType == ET_Value
				)
				AddMult = 1;

		}
		if(AddMult)
		{
			ExprElem* Mult = new ExprElem("*");
			CopyOperatorTo('*', Mult);
			ExprList->insert(itr2, Mult);
			continue;
		}
	}
}

void DetectFailureReason(std::list< ExprElem*>* ExprList)
{
	//check if all brackets were closed
	int BracketsLeft = 0;
	int BracketsRight = 0;
	for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
	{
		if ((*itr)->ElemType == ET_ParenthesisL)
			BracketsLeft++;
		else if ((*itr)->ElemType == ET_ParenthesisR)
			BracketsRight++;
	}
	if (BracketsLeft != BracketsRight)
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error : Number of left and right bracket count does not match\n");

	//check if there are enough operands for operators
	int ValuesRequired = 0;
	int ValuesFound = 0;
	for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
	{
		if ((*itr)->ElemType == ET_2ParamOperator)
			ValuesRequired += 2;
		else if ((*itr)->ElemType == ET_1ParamFunction)
			ValuesRequired += 1;
		else if ((*itr)->ElemType == ET_Value)
			ValuesFound += 1;
	}
	if (ValuesRequired > ValuesFound)
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error : Not enough values found for operators\n");

	//check if function is missing a value
	for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
	{
		if ((*itr)->ElemType == ET_1ParamFunction)
		{
			auto itrNext = itr;
			itrNext++;
			if (itrNext == ExprList->end() || (*itrNext)->ElemType != ET_Value)
				sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error : function is missing input value\n");
		}
	}

	//check if it's an equation but missing EQ
	int HasVariable = 0;
	int HasEQ = 0;
	for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
	{
		if ((*itr)->ElemType == ET_EQ)
			HasEQ++;
		else if ((*itr)->ElemType == ET_Variable)
			HasVariable++;
	}
	if(HasEQ>1)
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error : more than one = operator \n");
	if(HasEQ && HasVariable == 0)
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error : missing variable \n");
	if (HasEQ == 0 && HasVariable > 0)
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error : missing = operator \n");
}

void DetectBadExpression(const char *exp)
{
	//we only support 1 type of variable
	int XCount = 0;
	int YCount = 0;
	int len = strlen(exp);
	for (int i = 0; i < len; i++)
		if (exp[i] == 'x')
			XCount++;
		else if(exp[i] == 'y')
			YCount++;
	if (XCount > 0 && YCount > 0)
	{
		sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error : only 1 type of variable is supported \n");
		exit(1);
	}

	//we only support liniar equation for calculating a variable
	int VarDividerCount = 0;
	int VarMultiplier = 0;
	for (int i = 0; i < len; i++)
	{
		if (exp[i] == '+' || exp[i] == '-')
		{
			for (int j = i + 1; j < len; j++)
			{
				if (exp[j] == '+' || exp[j] == '-' || exp[j] == '=' || j==len-1)
				{
					//count number of X/Y
					int VariableCount = 0;
					for (int l = i + 1; l < j; l++)
						if (exp[l] == 'x' || exp[l] == 'y')
							VariableCount++;
					if (VariableCount > 1)
					{
						sLog.Log(LL_ERROR, __FILE__, __LINE__, "Error : only liniar equation is supported \n");
						exit(1);
					}
					break;
				}
			}
		}
	}
}