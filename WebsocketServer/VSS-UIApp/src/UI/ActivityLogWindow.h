#pragma once

#define ACTIVITY_ROW_PER_PAGE	9
#define MAX_ACTIVITY_ROWS		100

class ActivityLogGrid : public GenericDataGrid
{
public:
	ActivityLogGrid();
	void ResetState();
private:
	static void CB_AsyncDataArived(int CurlErr, char* response, void* userData);
	uint64_t m_PrevValuesCRC;
};

class ActivityLogWindow : public GenericWindow
{
public:
	ActivityLogWindow();
	int DrawWindow();
	void ResetState();
	void DestructorCheckMemLeaks();
private:
	InputTextData m_SearchFilter;
	ActivityLogGrid m_GridActivity;
};