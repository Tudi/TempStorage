#pragma once

//"any" value we use to signal that result of an operation is not valid
// smart guys use events to signal a bad result
#define NOT_A_VALID_VALUE	((double)0xFEEDFEED)

//expression can consist of these types of elements
//anything else will not be recognized
enum ElementTypes
{
	ET_0ParamFunction = 0,
	ET_1ParamFunction = 1,
	ET_2ParamOperator = 2,
	ET_Value = 10,
	ET_ParenthesisL,
	ET_ParenthesisR,
	ET_EQ,
	ET_Variable,
};

//operators/functions/constants will have functions assgined to them that will do the actual
//mathematic operations
typedef double (*ParamFuncDef)(double*);

//string input will be broken down to elements
//elements can be moved around easier than strings
class ExprElem
{
public:
	ExprElem(const char* pKey);
	~ExprElem();
	//check if part of a string matches to our internal "name"
	int IsKeyWordMatch(const char* expr);
	//list of values multiplied / divided can be considered part of a streak
	int IsPartOfStreak();
	//when we want to clone this element into another one
	void CopyTo(ExprElem* dst);
	ElementTypes ElemType;
	ParamFuncDef F; //function that expects 1 param
	double Result;
	//string representation of an operator/function/constant
	const char* GetKeyword() { return Key; }
	//when we move values from left to right, their sign will change
	//right now functions/constants do not have a sign
	double sign;
private:
	//store internally the string representation of an operator/constant/function
	char* Key;
};
