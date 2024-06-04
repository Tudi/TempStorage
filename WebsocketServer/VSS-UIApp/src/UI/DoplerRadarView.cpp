#include "stdafx.h"
#include <math.h>

DoplerModuleView::DoplerModuleView()
{	
#ifdef DRAW_DEMO_ALERT_BUTTON
	m_ButtonTriggerAlert.SetText("Trigger Alert");
	m_ButtonTriggerAlert.SetId(ButtonIds::BI_DOPLERMODULE_TRIGGERALERT);
	m_ButtonTriggerAlert.SetCallback(this, OnButtonClick);
#endif
#ifdef USE_POLLING_DEMO_DATA
	m_lldPrevModuleUpdateTriggerStamp = 0;
#endif
	m_ModulePOI.reserve(20);
	m_dModuleId = 0;
}

void DoplerModuleView::DestructorCheckMemLeaks()
{
}

void DoplerModuleView::ResetState()
{
}

void DrawPersonInfo(const float cx, const float cy, const DoplerModulePointInfo* poi)
{
	ImGui::SetNextWindowPos(ImVec2(cx + PERSON_SIZE_PIXEL_HALF * 2, cy), ImGuiCond_Always);
	if (ImGui::BeginChild(GetImGuiID("##PersonDetails"), ImVec2(270, 100), false, ImGuiWindowFlags_NoDecoration))
	{
		const float WindowLeftBorder = 10.0f;
		const float TextRowSpacing = 0.0f;
		ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 20.0f); // row spacing
		ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
		ImGui::Text("Type: ");
		ImGui::SameLine();
		ImGui::Text(poi->Tags.c_str());
		ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing

		ImGui::SetCursorPosX(ImGui::GetCursorPos().x + WindowLeftBorder); // move text to the right
		ImGui::Text("Speed: ");
		ImGui::SameLine();
		float xd = poi->xp - poi->x;
		float yd = poi->yp - poi->y;
		float speed = sqrt(xd * xd + yd * yd);
		speed = speed / 75.0f;
		char szSpeed[50];
		sprintf_s(szSpeed, "%.02f", speed);
		ImGui::Text(szSpeed);
	}
	ImGui::EndChild();
}

static void CleanupOutdatedObjects(std::vector<DoplerModulePointInfo> &m_ModulePOI)
{
	unsigned __int64 timeLimit = GetTickCount64() - 2 * 60 * 1000;
	for (auto itr = m_ModulePOI.begin(); itr != m_ModulePOI.end(); itr)
	{
		if (itr->m_lldLastUpdated < timeLimit)
		{
			itr = m_ModulePOI.erase(itr);
		}
		else
		{
			itr++;
		}
	}
}

int DoplerModuleView::DrawWindow()
{
	int display_w, display_h;
	GetDrawAreaSize(display_w, display_h, true);

	float minSize = 675.0f;
	ImVec2 imageSize(minSize, minSize);
	ImVec2 imagePosition = ImVec2(display_w - minSize - 200.0f, 90.0f);

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddImage(sImageManager.GetImage(ImageIds::II_DoplerBackgroundImage),
		imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y));

	// only draw the temp Module feed for wallmart
	if (m_dModuleId != 1)
	{
		return WindowManagerErrorCodes::WM_NO_ERROR;
	}

	// render Module POI
	{
		std::unique_lock<std::mutex> lock(m_POIListMutex);

		// flush objects that have not been updated for a while now
		CleanupOutdatedObjects(m_ModulePOI);

		float XOffset = imagePosition.x + imageSize.x / 2;
		float YOffset = imagePosition.y + imageSize.y / 2;
		unsigned __int64 stampNow = GetTickCount64();
		for (auto itr = m_ModulePOI.begin(); itr != m_ModulePOI.end(); itr++)
		{
			// this value poped up recently and we should wait for an additional value
			if (itr->xp == 0)
			{
				continue;
			}
			if (itr->m_lldPrevModuleUpdatePrevStamp == 0)
			{
				continue;
			}

			unsigned __int64 timePassed = stampNow - itr->m_lldPrevModuleUpdateStamp;

			// we replayed this movement smoothly
			// at this point we could interpolate future movement 
			float progressCoef;
			if (itr->m_lldPrevModuleUpdatePrevStamp + timePassed >= itr->m_lldPrevModuleUpdateStamp)
			{
				progressCoef = 1.0f;
			}
			else
			{
				unsigned __int64 timeFrame = itr->m_lldPrevModuleUpdateStamp - itr->m_lldPrevModuleUpdatePrevStamp;
				progressCoef = (float)timePassed / (float)timeFrame;
				if (progressCoef > 1.0f)
				{
					progressCoef = 1.0f;
				}
			}
			float inverseCoef = 1.0f - progressCoef;

			float tx = (itr->x * progressCoef + itr->xp * inverseCoef) / 1.0f;
			float ty = (itr->y * progressCoef + itr->yp * inverseCoef) / 1.0f;
			float cx = XOffset + tx;
			float cy = YOffset + ty;
			float dx = tx;
			float dy = ty;
			float dist = sqrt(dx * dx + dy * dy);
			dist = dist * 15.0f / (imageSize.x / 2); // because around 335 pixels will equal to 15m
			ImU32 color;
			ImageIds ImgId;
			if (dist > 15.0f)
			{
				itr->isClicked = false;
				continue;
			}
			if (dist > 10.0f)
			{
				color = IM_COL32(95, 232, 195, 255);
				ImgId = ImageIds::II_GreenPerson;
			}
			else if (dist > 5.0f)
			{
				color = IM_COL32(255, 190, 30, 255);
				ImgId = ImageIds::II_YellowPerson;
			}
			else
			{
				color = IM_COL32(237, 113, 157, 255);
				ImgId = ImageIds::II_RedPerson;
			}
/*			{
	//			drawList->AddCircleFilled(ImVec2(cx, cy), 15.f, color);
	//			drawList->AddRectFilled(ImVec2(cx + (float)itr->xi[0], cy + (float)itr->yi[0]), 
	//				ImVec2(cx + (float)itr->xi[3], cy + (float)itr->yi[3]), color);

				ImVec2 p1 = { cx + (float)(itr->xi[0] + itr->xi[2]) / 2, cy + (float)(itr->yi[0] + itr->yi[2]) / 2 };
				ImVec2 p2 = { cx + (float)itr->xi[1], cy + (float)itr->yi[1] };
				ImVec2 p3 = { cx + (float)itr->xi[3], cy + (float)itr->yi[3] };
				drawList->AddTriangleFilled(p1, p2, p3, color);
			}/**/
			{
				ImVec2 p1 = { cx + (float)itr->xi[0], cy + (float)itr->yi[0] };
				ImVec2 p2 = { cx + (float)itr->xi[1], cy + (float)itr->yi[1] };
				ImVec2 p3 = { cx + (float)itr->xi[2], cy + (float)itr->yi[2] };
				ImVec2 p4 = { cx + (float)itr->xi[3], cy + (float)itr->yi[3] };
				drawList->AddImageQuad(sImageManager.GetImage(ImgId),
					p3, p1, p2, p4);
				if (ImGui::IsMouseReleased(0))
				{
					float minX = MIN(MIN(p1.x, p2.x), MIN(p3.x, p4.x));
					float minY = MIN(MIN(p1.y, p2.y), MIN(p3.y, p4.y));
					float maxX = MAX(MAX(p1.x, p2.x), MAX(p3.x, p4.x));
					float maxY = MAX(MAX(p1.y, p2.y), MAX(p3.y, p4.y));
					if (ImGui::IsMouseHoveringRect(ImVec2(minX,minY), ImVec2(maxX, maxY), true))
					{
						itr->isClicked = !itr->isClicked;
					}
				}
				if (itr->isClicked)
				{
					DrawPersonInfo(cx , cy, &(*itr));
				}
			}
		}
	}

#ifdef DRAW_DEMO_ALERT_BUTTON
	// TEMP CODE TO TEST ALERTS !
	ImGui::SetCursorPosY(500.0f);
	ImGui::SetCursorPosX(30.0f);
	m_ButtonTriggerAlert.DrawButton();
#endif
	
#ifdef USE_POLLING_DEMO_DATA
	// periodically update dummy Module feed. This is to exercise fetching data from real Module
	if (m_lldPrevModuleUpdateTriggerStamp + Module_FEED_UPDATE_PERIOD < GetTickCount64())
	{
		m_lldPrevModuleUpdateTriggerStamp = GetTickCount64();
		WebApi_GetDemoRadarDataAsync(CB_AsyncDataArived_DemoData, this);
	}
#endif

	return WindowManagerErrorCodes::WM_NO_ERROR;
}

#ifdef DRAW_DEMO_ALERT_BUTTON
void AlertCreated(int CurlErr, char* response, void* userData)
{
	userData;
	yyJSON(yydoc);
	if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceModule, "") != WebApiErrorCodes::WAE_NoError)
	{
		return;
	}

	yyjson_val* root = yyjson_doc_get_root(yydoc);
	yyjson_val* yyNewAlertId = yyjson_obj_get(root, "id");
	if (yyNewAlertId == NULL)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceModule, 0, 0,
			"Doppler:Alert create response did not contain an ID");
		return;
	}
	int NewAlertId = yyjson_get_int(yyNewAlertId);

	// now send an SMS for the Alert
	WebApi_CreateSMSAlert(NewAlertId, NULL, NULL);
	WebApi_CreateEmailAlert(NewAlertId, NULL, NULL);
}
#endif

void DoplerModuleView::OnButtonClick(GenericButton* pBtn, void* pParent)
{
	if (pBtn == NULL || pParent == NULL)
	{
		return;
	}
#ifdef DRAW_DEMO_ALERT_BUTTON
	if (pBtn->GetId() == ButtonIds::BI_DOPLERMODULE_TRIGGERALERT)
	{
		WebApi_CreateAlert(sUserSession.GetOrganizationId(), sUserSession.GetUserId(), 1, AlertCreated, NULL);
	}
#endif
}

double calculate_angle(double x1, double y1, double x2, double y2) {
	double angle_radians = atan2(y2 - y1, x2 - x1);
	return angle_radians;
}

void rotate_point(double* x, double* y, double angle) {
	double original_x = *x;
	*x = original_x * cos(angle) - (*y) * sin(angle);
	*y = original_x * sin(angle) + (*y) * cos(angle);
}

#ifdef USE_POLLING_DEMO_DATA
bool CB_DoneReadingPerson(int rowIndex, ExtractDBColumnToBinary* rowColDataArr)
{
	rowIndex;
	DoplerModulePointInfo* ahd = (DoplerModulePointInfo*)rowColDataArr[0].cbDRF_userData1;
	DoplerModuleView* wnd = (DoplerModuleView*)rowColDataArr[0].cbDRF_userData2;
	std::vector<DoplerModulePointInfo>& POI = wnd->GetPOI();

	for (auto itr = POI.begin(); itr != POI.end(); itr++)
	{
		if (itr->id != ahd->id)
		{
			continue;
		}
		itr->xp = itr->x;
		itr->yp = itr->y;

		itr->x = ahd->x;
		itr->y = ahd->y;

		// if it bounced, reset the clicked state
		if (itr->xp * itr->x < 0 || itr->yp * itr->y < 0)
		{
//			itr->isClicked = 0;
		}
		
		itr->UpdateIconDrawPosition();

		return true;
	}

	// create a new entry
	ahd->isClicked = 0;
	ahd->angle = 0;
	memset(ahd->xi, 0, sizeof(ahd->xi));
	memset(ahd->yi, 0, sizeof(ahd->yi));
	ahd->xp = ahd->yp = 0;
	POI.push_back(*ahd);

	return true;
}

void DoplerModuleView::CB_AsyncDataArived_DemoData(int CurlErr, char* response, void* userData)
{
	yyJSON(yydoc);
	if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceModule, "") != WebApiErrorCodes::WAE_NoError)
	{
		return;
	}

	DoplerModulePointInfo rowData;
	ExtractDBColumnToBinary extractColumns[] = {
		{"ID", (void*)&rowData.id, sizeof(rowData.id), StrToInt32},
		{"x", (void*)&rowData.x, sizeof(rowData.x), StrToCustomObjFloat},
		{"y", (void*)&rowData.y, sizeof(rowData.y), StrToCustomObjFloat},
		{"Tag", (void*)&rowData.Tags, sizeof(rowData.Tags), StrToStdStr},
		{NULL} };
	extractColumns[0].SetDataRowFinishedFunc(CB_DoneReadingPerson, &rowData, userData);

	ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, "people", extractColumns, LogSourceGroups::LogSourceModule);

	DoplerModuleView* wnd = (DoplerModuleView*)userData;
	wnd->m_lldPrevModuleUpdatePrevStamp = wnd->m_lldPrevModuleUpdateStamp;
	wnd->m_lldPrevModuleUpdateStamp = GetTickCount64();
}
#endif

void DoplerModuleView::OnModulePositionDataArrived(__int64 ModuleId, unsigned __int64 Timestamp, __int64 ObjectId, float x, float y)
{
	if (m_dModuleId != ModuleId)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected,
			LogSourceGroups::LogSourceLocationViewWindow, 0, 0,
			"DoplerModuleView:Module %lld data arived, but we are watching %lld", ModuleId, m_dModuleId);
		return;
	}
	std::unique_lock<std::mutex> lock(m_POIListMutex);
	DoplerModulePointInfo* obj = NULL;
	bool bIsNew = false;
	// does it have the object ?
	for (auto itr = m_ModulePOI.begin(); itr != m_ModulePOI.end(); itr++)
	{
		if (itr->id == ObjectId)
		{
			obj = &(*itr);
			break;
		}
	}
	if (obj == NULL)
	{
		obj = &(m_ModulePOI.emplace_back());
		obj->m_lldPrevModuleUpdateStamp = 0;
		bIsNew = true;
	}

	// do not process out of order UDP packets
	if (Timestamp > obj->m_lldPrevModuleUpdateStamp)
	{
		obj->id = ObjectId;
		obj->xp = obj->x;
		obj->yp = obj->y;
		obj->x = x;
		obj->y = y;
		obj->m_lldPrevModuleUpdatePrevStamp = obj->m_lldPrevModuleUpdateStamp;
		obj->m_lldPrevModuleUpdateStamp = Timestamp;
		obj->m_lldLastUpdated = GetTickCount64();

		if (bIsNew == false)
		{
			// these are oneshot values
			obj->UpdateIconDrawPosition();
		}
	}
}

void DoplerModulePointInfo::UpdateIconDrawPosition()
{
	if (x != xp || y != yp)
	{
		// these are oneshot values
		angle = calculate_angle(x, y, xp, yp);

		xi[0] = -PERSON_SIZE_PIXEL_HALF; yi[0] = -PERSON_SIZE_PIXEL_HALF;
		rotate_point(&xi[0], &yi[0], angle);

		xi[1] = PERSON_SIZE_PIXEL_HALF; yi[1] = -PERSON_SIZE_PIXEL_HALF;
		rotate_point(&xi[1], &yi[1], angle);

		xi[2] = -PERSON_SIZE_PIXEL_HALF; yi[2] = PERSON_SIZE_PIXEL_HALF;
		rotate_point(&xi[2], &yi[2], angle);

		xi[3] = PERSON_SIZE_PIXEL_HALF; yi[3] = PERSON_SIZE_PIXEL_HALF;
		rotate_point(&xi[3], &yi[3], angle);
	}
}