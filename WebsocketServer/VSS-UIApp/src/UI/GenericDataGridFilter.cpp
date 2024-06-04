#include "stdafx.h"
#include "ResourceManager/AsyncTaskManager.h"

static char* stristr(const char* str1, const char* str2)
{
	const char* p1 = str1;
	const char* p2 = str2;
	const char* r = *p2 == 0 ? str1 : 0;

	while (*p1 != 0 && *p2 != 0) 
	{
		if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2)) 
		{
			if (r == 0) 
			{
				r = p1;
			}
			p2++;
		}
		else 
		{
			p2 = str2;
			if (r != 0) 
			{
				p1 = r + 1;
			}
			if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2)) 
			{
				r = p1;
				p2++;
			}
			else 
			{
				r = 0;
			}
		}
		p1++;
	}
	return *p2 == 0 ? const_cast<char*>(r) : 0;
}

GenericDataGrid::GenericDataGridFilter::GenericDataGridFilter(GenericDataGrid* parent)
{
	InitTypeInfo();
	if (parent == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeveritySever, LogSourceGroups::LogSourceDataGrids, 0, 0,
			"DataGrid:Filter parent should not be NULL");
	}
	m_Parent = parent;
	m_sFilterString = "";
	m_FilterState = GenericDataGrid::GenericDataGridFilter::FilterRefreshStates::FilterIdle;
	m_dFilterCounter = 0;
	m_pRowsSelected = &m_RowsSelected1;
}

bool isNumber(const char *s) 
{
	if (s[0] == 0)
	{
		return false;
	}

	// Allow leading sign
	size_t i = 0;
	if (s[i] == '-' || s[i] == '+') 
	{
		++i;
		// If all characters are sign, return false
		if (s[0] == 0)
		{
			return false;
		}
	}

	bool hasDecimal = false;
	bool hasDigit = false;

	for (; i < s[0] != 0; ++i)
	{
		if (std::isdigit(s[i])) 
		{
			hasDigit = true;
		}
		else if (s[i] == '.') 
		{
			// Allow only one decimal point
			if (hasDecimal)
			{
				return false;
			}
			hasDecimal = true;
		}
		else 
		{
			// If character is not a digit or decimal point
			return false;
		}
	}

	// If string contains at least one digit
	return hasDigit;
}

static int compareStrings(const char* str1, const char* str2) 
{
	if (isNumber(str1) && isNumber(str2)) 
	{
		return atoi(str1) - atoi(str2);
	}
	return strcmp(str1, str2);
}

void GenericDataGrid::GenericDataGridFilter::AsyncTask_FilterOrSort(void* params)
{
#ifdef _DEBUG
	uint64_t startStamp = GetTickCount64();
#endif
	GenericDataGrid::GenericDataGridFilter* self = typecheck_castL(GenericDataGridFilter, params);
	GenericDataGrid* m_Parent = self->m_Parent;
	uint64_t myCounter = ++self->m_dFilterCounter;

	self->m_FilterState = GenericDataGrid::GenericDataGridFilter::FilterRefreshStates::FilterProcessing;

	size_t maxRows = m_Parent->m_dRows;
	const size_t maxCols = m_Parent->m_dCols;

	unsigned short filteredSortedValues[MAX_GRIDDATE_ROWS];
	size_t resultValues = 0;
	if (maxRows >= MAX_GRIDDATE_ROWS)
	{
		maxRows = MAX_GRIDDATE_ROWS - 1;
	}

	// no need for filtering, just sort it
	if (self->m_sFilterString.empty() == true)
	{
		for (size_t i = 0; i < maxRows; i++)
		{
			filteredSortedValues[i] = (unsigned short)i;
		}
		resultValues = maxRows;
	}
	// should reapply both filtering and sorting
	else
	{
		// check for "contains" or "exact" filtering
		char* filterStr = (char*)self->m_sFilterString.c_str(); // hmm, what if this changes while we are already searching
		const size_t filterStrLen = self->m_sFilterString.length();

		bool bIsExactMatch = false;
		if (filterStr[0] == '"' && filterStr[filterStrLen - 1] == '"')
		{
			bIsExactMatch = true;
			// remove the enclosing '"'
			filterStr++;
			filterStr[filterStrLen - 2] = 0;
		}

		for (size_t rowNr = 0; rowNr < maxRows; rowNr++)
		{
			bool bRowSelected = false;
			for (size_t colNr = 0; colNr < maxCols; colNr++)
			{
				const char* val = m_Parent->GetCellData((uint32_t)rowNr, (uint32_t)colNr);
				if (val == NULL)
				{
					continue;
				}
				if (bIsExactMatch)
				{
					if (strcmp(val, filterStr) == 0)
					{
						bRowSelected = true;
						break;
					}
				}
				else if (stristr(val, filterStr))
				{
					bRowSelected = true;
					break;
				}
			}
			if (bRowSelected)
			{
				filteredSortedValues[resultValues++] = ((unsigned short)rowNr);
			}
		}

		// restore the enclosing '"'
		if (bIsExactMatch)
		{
			filterStr[filterStrLen - 2] = '"';
		}
	}

	// order by col
	if (resultValues > 0 &&
		m_Parent->m_dSortByCol < m_Parent->m_dCols &&
		m_Parent->m_dSortOrder != DataGridSortTypes::NotSorted)
	{
		const DataGridSortTypes sortOrder = m_Parent->m_dSortOrder;
		const uint32_t sortCol = m_Parent->m_dSortByCol;

		// check which row has the bigger / smaller value
		for (__int64 rowNr1 = resultValues - 1; rowNr1 >= 1; rowNr1--)
		{
			__int64 swapWithRow = -1;
			const char* val1 = m_Parent->GetCellData(filteredSortedValues[rowNr1], sortCol);
			if (val1 == NULL)
			{
				continue;
			}
			for (__int64 rowNr2 = rowNr1 - 1; rowNr2 >= 0; rowNr2--)
			{
				const char* val2 = m_Parent->GetCellData(filteredSortedValues[rowNr2], sortCol);
				int compRes = compareStrings(val1, val2);
				if ((compRes < 0 && sortOrder == DataGridSortTypes::Ascending) ||
					(compRes > 0 && sortOrder == DataGridSortTypes::Descending))
				{
					val1 = val2;
					swapWithRow = rowNr2;
				}
			}

			// looks like this row should be swapped out
			if (swapWithRow != -1)
			{
				unsigned short t = filteredSortedValues[rowNr1];
				filteredSortedValues[rowNr1] = filteredSortedValues[swapWithRow];
				filteredSortedValues[swapWithRow] = t;
			}
		}
	}

	std::vector<unsigned short>* m_pRowsSelected;
	if (self->m_pRowsSelected == &self->m_RowsSelected1)
	{
		m_pRowsSelected = &self->m_RowsSelected2;
	}
	else
	{
		m_pRowsSelected = &self->m_RowsSelected1;
	}

	bool bRowCountChanged = self->m_pRowsSelected->size() != resultValues;
	// only register the last filter
	if (self->m_FilterState == GenericDataGrid::GenericDataGridFilter::FilterRefreshStates::FilterProcessing
		&& myCounter == self->m_dFilterCounter)
	{
		if (m_pRowsSelected->capacity() < resultValues)
		{
			m_pRowsSelected->reserve(maxRows);
		}
		// allow rendering to use the new values. Must come before value copy or else they get reset
		m_pRowsSelected->resize(resultValues);
		// copy the new selected rows
		memcpy(m_pRowsSelected->data(), filteredSortedValues, resultValues * sizeof(filteredSortedValues[0]));
		// we are done editing the inactive vector, switch it to active vector
		self->m_pRowsSelected = m_pRowsSelected;
		self->m_FilterState = GenericDataGrid::GenericDataGridFilter::FilterRefreshStates::FilterIdle;
	}

	if (bRowCountChanged || m_Parent->m_dCurPage == -1)
	{
		// update disabled buttons
		m_Parent->OnActivePageChanged();
	}

#ifdef _DEBUG
	AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceDataGridFilter, 0, 0,
		"DataGridFilter:Filtering and sorting took %lld ms.", GetTickCount64() - startStamp);
#endif
}

void GenericDataGrid::GenericDataGridFilter::UpdateFilter(const char* newFilter, bool reasonDataChange)
{
	if (reasonDataChange == true) // ignore the new filter value and apply the old one
	{
	}
	else if (newFilter == NULL)
	{
		m_sFilterString = "";
	}
	else if (strcmp(m_sFilterString.c_str(), newFilter) != 0)
	{
		m_sFilterString = newFilter;
	}
	else
	{
		return; // not a data change and the new filter is the same as the old one
	}

	// no filter means we show everything
	if (m_FilterState != GenericDataGrid::GenericDataGridFilter::FilterRefreshStates::FilterQueued)
	{
		m_FilterState = GenericDataGrid::GenericDataGridFilter::FilterRefreshStates::FilterQueued;
		AddAsyncTask1(AsyncTaskPriorityRanks::Priority_1, AsyncTask_FilterOrSort, this, true);
	}
}