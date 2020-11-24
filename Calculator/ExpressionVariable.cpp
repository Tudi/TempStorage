#include "KeyWords.h"
#include "Expression.h"
#include <vector>

void SplitExprExtractBracket(std::list< ExprElem*>* ExprList, std::list< ExprElem*>::iterator AtInd, std::list< ExprElem*>* preb, std::list< ExprElem*>* br, std::list< ExprElem*>* postb)
{
	//count the depth of the bracket
	int BracketsEntered = 1;
	int BracketsExited = 0;
	auto itr = AtInd;
	itr++;
	for (; itr != ExprList->end(); itr++)
	{
		if ((*itr)->ElemType == ET_ParenthesisL)
			BracketsEntered++;
		if ((*itr)->ElemType == ET_ParenthesisR)
			BracketsExited++;
		if (BracketsEntered == BracketsExited)
		{
			itr++;
			break;
		}
	}

	preb->clear();
	for (auto itr2 = ExprList->begin(); itr2 != AtInd; itr2++)
		preb->push_back(*itr2);
	br->clear();
	for (auto itr2 = AtInd; itr2 != itr; itr2++)
		br->push_back(*itr2);
	postb->clear();
	for (auto itr2 = itr; itr2 != ExprList->end(); itr2++)
		postb->push_back(*itr2);
}

void ApplyOperandOnBracketElements(ExprElem* pkw, ExprElem* pop, std::list< ExprElem*>* ExprList)
{
	ExprElem* kwPrev = NULL;
	for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
	{
		ExprElem* kw = (*itr);
		int MultiplyThisBlock = 0;
		if (kw->ElemType == ET_2ParamOperator && (kw->GetKeyword()[0] == '+' || kw->GetKeyword()[0] == '-'))
			MultiplyThisBlock = 1;
		else if(kw->ElemType == ET_ParenthesisR)
			MultiplyThisBlock = 1;
		//!!Note that at this stage all functions have been eliminated
		if (MultiplyThisBlock && kwPrev != NULL && kwPrev->ElemType == ET_Value)
		{
			ExprElem* op = new ExprElem(pop->GetKeyword());
			pop->CopyTo(op);
			ExprElem* val = new ExprElem(NULL);
			pkw->CopyTo(val);
			ExprList->insert(itr, op);
			ExprList->insert(itr, val);
			kwPrev = NULL; // avoid inserting again
		}
		else
			kwPrev = kw;
	}
}

std::list< ExprElem*>* DuplicateList(std::list< ExprElem*>* l)
{
	std::list< ExprElem*>* ret = new std::list< ExprElem*>;
	for (auto itr = l->begin(); itr != l->end(); itr++)
	{
		ExprElem* kw = new ExprElem((*itr)->GetKeyword());
		(*itr)->CopyTo(kw);
		ret->push_back(kw);
	}
	return ret;
}

void ApplyBracketOntElements(std::list< ExprElem*>* prebr, std::list< ExprElem*>* br)
{
	std::list< ExprElem*> out;
	for (auto itr = br->begin(); itr != br->end(); itr++)
	{
		if ((*itr)->ElemType == ET_ParenthesisL || (*itr)->ElemType == ET_ParenthesisR)
		{
			out.push_back(*itr);
			continue;
		}
		std::list< ExprElem*>* pre = DuplicateList(prebr);
		for (auto itr2 = pre->begin(); itr2 != pre->end(); itr2++)
			out.push_back(*itr2);
		while (itr != br->end())
		{
			if ((*itr)->ElemType == ET_Value || (*itr)->ElemType == ET_Variable ||
				((*itr)->ElemType == ET_2ParamOperator && ((*itr)->GetKeyword()[0] == '*' || (*itr)->GetKeyword()[0] == '/')))
			{
				out.push_back(*itr);
				itr++;
			}
			else
				break;
		}
		if (itr != br->end())
			out.push_back(*itr);
	}
	//we no longer need preb
	for (auto itr = prebr->begin(); itr != prebr->end(); itr++)
		delete* itr;
	prebr->clear();
	//generate the ourput : replacement of the bracket with the expanded bracket
	br->clear();
	for (auto itr = out.begin(); itr != out.end(); itr++)
		br->push_back(*itr);
}

std::list< ExprElem*>::iterator GetMaxDepthBracketStart(std::list< ExprElem*>* ExprList, std::list< ExprElem*>::iterator StartAt)
{
	//get the deepest bracket
	int BracketsOpened = 0;
	int MaxDepth = 0;
	std::list< ExprElem*>::iterator MaxDepthAt = ExprList->end();
	for (auto itr = StartAt; itr != ExprList->end(); itr++)
	{
		if ((*itr)->ElemType == ET_ParenthesisL)
			BracketsOpened++;
		else if ((*itr)->ElemType == ET_ParenthesisR)
			BracketsOpened--;
		if (BracketsOpened > MaxDepth)
		{
			MaxDepth = BracketsOpened;
			MaxDepthAt = itr;
		}
	}
	return MaxDepthAt;
}

void MergeListsIntoOne(std::list< ExprElem*>* a, std::list< ExprElem*>* b, std::list< ExprElem*>* c, std::list< ExprElem*>* ExprList)
{
	ExprList->clear();
	for (auto itr2 = a->begin(); itr2 != a->end(); itr2++)
		ExprList->push_back(*itr2);
	for (auto itr2 = b->begin(); itr2 != b->end(); itr2++)
		ExprList->push_back(*itr2);
	for (auto itr2 = c->begin(); itr2 != c->end(); itr2++)
		ExprList->push_back(*itr2);
	PrintExpression(ExprList, 0);
}

int UnpackBracket(std::list< ExprElem*>* ExprList)
{
	std::list< ExprElem*>::iterator mditr = GetMaxDepthBracketStart(ExprList, ExprList->begin());
	std::list< ExprElem*> pre, br, post;

	while (1)
	{
		//found not brackets
		if (mditr == ExprList->end())
			return 0;

		//split the expr in 3 parts : prebracket, bracket, postBracket
		SplitExprExtractBracket(ExprList, mditr, &pre, &br, &post);

		//check if we can expand the bracket
		int CanUnpack = 1;
		if (pre.empty() == false)
		{
			auto LastOperator = pre.end();
			LastOperator--;
			if ((*LastOperator)->ElemType == ET_2ParamOperator && (*LastOperator)->GetKeyword()[0] == '/')
				CanUnpack = 0;
		}
		if (CanUnpack == 0)
		{
			//should pick another bracket
			mditr++;
			mditr = GetMaxDepthBracketStart(ExprList, mditr);
		}
		else
			break;
	}

	// get the list of multipliers before the bracket
	std::list< ExprElem*> pre1;
	auto itr2 = pre.end();
	while (itr2 != pre.begin())
	{
		itr2--;
		if (itr2 != pre.end() && ((*itr2)->GetKeyword()[0] == '*' || (*itr2)->GetKeyword()[0] == '/'
			|| (*itr2)->ElemType == ET_Value || (*itr2)->ElemType == ET_Variable))
		{
			pre1.push_front((*itr2));
			pre.erase(itr2);
			itr2 = pre.end();
			continue;
		}
		break;
	}

	//apply to this partial pre1, the content of the bracket
	if (pre1.empty() == false)
	{
		ApplyBracketOntElements(&pre1, &br);
		MergeListsIntoOne(&pre, &br, &post, ExprList);
		return 1;
	}

	//get the list of multipliers after the bracket
	int MadeChanges = 0;
	while (1)
	{
		auto itr2 = post.begin();
		if (itr2 != post.end() && ((*itr2)->GetKeyword()[0] == '*' || (*itr2)->GetKeyword()[0] == '/'))
		{
			auto itr3 = itr2;
			if (itr3 != post.end())
				itr3++;
			if ((*itr3)->ElemType == ET_Value)
			{
				//PrintExpression(&br, 0);
				ApplyOperandOnBracketElements((*itr3), (*itr2), &br);
				//PrintExpression(&br, 0);
				post.erase(itr2);
				post.erase(itr3);
				MadeChanges = 1;
				continue;
			}
		}
		break;
	}

	//if no operations were made on this bracket list, just remove the brackets
	if (MadeChanges == 0)
	{
		br.erase(br.begin());
		auto LastElem = br.end();
		LastElem--;
		br.erase(LastElem);
		MadeChanges = 1;
	}

	//reassemble into a new list
	MergeListsIntoOne(&pre, &br, &post, ExprList);

	//if we got here, we made changes for sure
	return MadeChanges;
}

void MoveVariableToEndOfMult(std::list< ExprElem*>* ExprList)
{
	int ChangesMade = 1;
	int TimeOut = 50; // might deadlock on badly formatted expression
	while (ChangesMade == 1 && TimeOut>0)
	{
		ChangesMade = 0;
		TimeOut--;
		for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
		{
			if ((*itr)->ElemType != ET_Variable)
				continue;
			//take a look at the next to 2 elements
			auto itr2 = itr;
			itr2++;
			if (itr2 == ExprList->end())
				break;
			if ((*itr2)->ElemType != ET_2ParamOperator
				|| (*itr2)->GetKeyword()[0] != '*')
				continue;
			itr2++;
			if (itr2 == ExprList->end()) //probably an error if it happens
				break;
			//swap the 2 elements to be able to simplify our expression ( if possible )
			std::swap(*itr, *itr2);
			ChangesMade = 1;
			break;
		}
	}
}

void MoveVariablesToRight(std::list< ExprElem*>* ExprList)
{
	int FlipSign = 1;
	std::list< ExprElem*> left, right;
	int ChangesMade = 1;
	while (ChangesMade)
	{
		ChangesMade = 0;
		for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
		{
			if ((*itr)->ElemType == ET_EQ)
				FlipSign = 0;
			//we want to move parts that contain variables
			if ((*itr)->ElemType != ET_Variable)
				continue;
			//check a streak of values multiplied or divided if contains a variable
			auto itr2 = itr;
			for (; itr2 != ExprList->begin(); itr2--)
				if ((*itr2)->IsPartOfStreak() == 0)
					break;
			if ((*itr2)->ElemType == ET_EQ && itr2!= ExprList->end())
				itr2++;
			//is the first 
			ExprElem* AddOperator = NULL;
			if (FlipSign)
			{
				if ((*itr2)->ElemType == ET_2ParamOperator && (*itr2)->GetKeyword()[0] == '-')
				{
					AddOperator = new ExprElem("+");
					CopyOperatorTo('+', AddOperator);
				}
				else 
					//if ((*itr2)->ElemType == ET_2ParamOperator && (*itr2)->GetKeyword()[0] == '+')
				{
					AddOperator = new ExprElem("-");
					CopyOperatorTo('-', AddOperator);
				}
			}
			else
			{
				if ((*itr2)->ElemType == ET_2ParamOperator)
				{
					AddOperator = new ExprElem((*itr2)->GetKeyword());
					(*itr2)->CopyTo(AddOperator);
				}
				else
				{
					AddOperator = new ExprElem("+");
					CopyOperatorTo('+', AddOperator);
				}
			}
			//add the "flipped" operator
			right.push_back(AddOperator);
			//push the stream to the right side
			for (auto itr3 = itr2; itr3 != itr;itr3++)
			{
				//skip adding the sign since we already flipped it
				if (itr3 != itr2 || (*itr2)->ElemType != ET_2ParamOperator)
					right.push_back(*itr3);
				else
					delete* itr3;
			}
			//remove them from the old list
			for (auto itr3 = itr2; itr3 != itr;)
			{
				auto itr4 = itr3;
				itr3++;
				ExprList->erase(itr4);
			}
			//also add the variable
			right.push_back(*itr);
			ExprList->erase(itr);
			ChangesMade = 1;
			break;
		}
	}

	//find the EQ 
	auto EQitr = ExprList->begin();
	for (; EQitr != ExprList->end(); EQitr++)
	{
		if ((*EQitr)->ElemType != ET_EQ)
			continue;
		break;
	}

	//flip all signs after EQ
	for (auto itr = EQitr; itr != ExprList->end(); itr++)
	{
		if ((*itr)->ElemType == ET_2ParamOperator && (*itr)->GetKeyword()[0] == '-')
		{
			ExprElem* AddOperator = new ExprElem("+");
			CopyOperatorTo('+', AddOperator);
			delete* itr;
			*itr = AddOperator;
		}
		else if ((*itr)->ElemType == ET_2ParamOperator && (*itr)->GetKeyword()[0] == '+')
		{
			ExprElem* AddOperator = new ExprElem("-");
			CopyOperatorTo('-', AddOperator);
			delete* itr;
			*itr = AddOperator;
		}
	}

	//are there any values ( at least 1 ) after EQ ?
	auto NextToEQ = EQitr;
	if (NextToEQ != ExprList->end())
		NextToEQ++;
	if (EQitr == ExprList->begin() || (*ExprList->begin())->ElemType == ET_2ParamOperator)
	{
		ExprElem* t = new ExprElem(NULL);
		t->ElemType = ET_Value;
		t->Result = 0;
		ExprList->push_front(t);
	}
	if(NextToEQ != ExprList->end())
	{
		ExprElem* AddOperator = new ExprElem("-");
		CopyOperatorTo('-', AddOperator);
		ExprList->insert(EQitr, AddOperator);

		//move EQ to the end
		ExprList->push_back(*EQitr);
		ExprList->erase(EQitr);
	}

	//remove first operator from the right side and move it inside "sign"
	auto itr = right.begin();
	if (itr != right.end())
	{
		if ((*itr)->ElemType == ET_2ParamOperator)
		{
			auto Next = itr;
			Next++;
			int remove = 1;
			if ((*itr)->GetKeyword()[0] == '-')
			{
				if((*Next)->ElemType == ET_Value)
					(*Next)->sign = -(*Next)->sign;
				else if ((*Next)->ElemType == ET_Variable)
				{
					ExprElem* t = new ExprElem(NULL);
					t->ElemType = ET_Value;
					t->Result = 0;
					right.push_front(t);
					remove = 0;
				}
			}
			if (remove)
			{
				delete* itr;
				right.erase(itr);
			}

		}
	}

	//append the variables
	for(auto itr=right.begin();itr!=right.end();itr++)
		ExprList->push_back(*itr);
}

int HasBrackets(std::list< ExprElem*>* ExprList)
{
	for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
		if ((*itr)->ElemType == ET_ParenthesisL || (*itr)->ElemType == ET_ParenthesisR)
			return 1;
	return 0;
}

void ExtractVariableOnLeft(std::list< ExprElem*>* ExprList)
{
	//is x a multiplier or divider
	int IsDivider = 0;
	for (auto itr = ExprList->begin(); itr != ExprList->end(); itr++)
	{
		if ((*itr)->ElemType == ET_Variable && itr != ExprList->begin())
		{
			auto itr2 = itr;
			itr2--;
			if ((*itr2)->ElemType == ET_2ParamOperator && (*itr2)->GetKeyword()[0] == '/')
				IsDivider = 1;
			break;
		}
	}
	//!we can't really handle divider in case it is still in a bracket !
	if (IsDivider == 0)
	{
		//find the EQ 
		auto EQitr = ExprList->begin();
		for (; EQitr != ExprList->end(); EQitr++)
		{
			if ((*EQitr)->ElemType != ET_EQ)
				continue;
			break;
		}
		//put 1 instead variables
		ExprElem* var = NULL;
		for (auto itr = EQitr; itr != ExprList->end();itr++)
		{
			if ((*itr)->ElemType != ET_Variable)
				continue;
			if (var == NULL)
				var = *itr;
			else
				delete* itr;
			ExprElem* t = new ExprElem(NULL);
			t->ElemType = ElementTypes::ET_Value;
			t->Result = 1;
			*itr = t;
		}
		//add bracket to beginning and end of the left side
		ExprElem* t = new ExprElem("(");
		CopyOperatorTo('(', t);
		ExprList->push_front(t);
		t = new ExprElem(")");
		CopyOperatorTo(')', t);
		ExprList->insert(EQitr, t);
		t = new ExprElem("/");
		CopyOperatorTo('/', t);
		ExprList->insert(EQitr, t);
		t = new ExprElem("(");
		CopyOperatorTo('(', t);
		ExprList->insert(EQitr, t);
		t = new ExprElem(")");
		CopyOperatorTo(')', t);
		ExprList->push_back(t);
		//move the EQ sign at the end
		t = *EQitr;
		ExprList->erase(EQitr);
		ExprList->push_back(t);
		//add the variable after EQ
		ExprList->push_back(var);
	}
}

double SolveEQ(std::list< ExprElem*>* ExprList)
{
	PrintExpression(ExprList, 0);
	int ChangesMade = 1;
	while (ChangesMade)
	{
		//if we can simplify it, than simplify it
		SolveExpression(ExprList);
		//if we have remaining nested brackets, unpack them
		ChangesMade = UnpackBracket(ExprList);
		//for chained multiplication, we might want to move the variable to the end of operation list
		MoveVariableToEndOfMult(ExprList);
	}
	//move known part to the left, variable to the right
	if (HasBrackets(ExprList) == 0)
	{
		MoveVariablesToRight(ExprList);
		PrintExpression(ExprList,0);
		//if we can simplify it, than simplify it
		SolveExpression(ExprList);
	}
	
	//extract variable
	ExtractVariableOnLeft(ExprList);
	
	//if we can simplify it, than simplify it
	SolveExpression(ExprList);
	
	//if there is only 1 value before EQ, chances are we solved it
	if (ExprList->size() == 3)
		return (*ExprList->begin())->Result;

	//somehting must have went wrong
	return NOT_A_VALID_VALUE;
}