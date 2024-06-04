#include "stdafx.h"
#include "stb/stb_image.h"
#include "ResourceManager/AsyncTaskManager.h"

TransparentButton::TransparentButton()
{
	InitTypeInfo();
    m_forcedMinSize = { 0.0f, 0.0f };
    m_bDisabled = false;
}

void TransparentButton::DrawButton()
{
    ImVec2 topleft = ImGui::GetCursorPos();
#ifdef _DEBUG
    ImVec2 botRight = m_forcedMinSize;
    botRight.x += topleft.x; botRight.y += topleft.y;
    ImGui::GetWindowDrawList()->AddRect(topleft, botRight, IM_COL32(255, 0, 0, 255));
#endif
    if(isMouseClickedInArea_Size(topleft, m_forcedMinSize))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;
        if (m_Hitmap.getPixel((int)(mousePos.x - topleft.x), (int)(mousePos.y - topleft.y)))
        {
            OnPush();
        }
    }
}

struct AsyncTaskLoadHitmapParams
{
    REFLECT_TYPE(AsyncTaskLoadHitmapParams);
    const char* fileName;
    MonochromeImage* m_Hitmap;
};
void AsyncTaskLoadHitmap(void* params)
{
    AsyncTaskLoadHitmapParams* par = typecheck_castL(AsyncTaskLoadHitmapParams, params);
    int Width = 0;
    int Height = 0;
    unsigned char* image_data = stbi_load(par->fileName, &Width, &Height, 0, 4);

    if (image_data == NULL || Width == 0 || Height == 0)
    {
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, LogSourceGroups::LogSourceTransparentButton, 0, 0,
            "TransparentButton:Failed to load image %s", par->fileName);
        return;
    }

    par->m_Hitmap->Init(Width, Height);
    par->m_Hitmap->ConstructFromSTBImage(image_data, Width, Height);

    InternalFree(params);
}

void TransparentButton::LoadHitMap(const char *fileName)
{
    AsyncTaskLoadHitmapParams* params = (AsyncTaskLoadHitmapParams *)InternalMalloc(sizeof(AsyncTaskLoadHitmapParams));
    params->InitTypeInfo();
    params->fileName = fileName;
    params->m_Hitmap = &m_Hitmap;
    AddAsyncTask1(AsyncTaskPriorityRanks::Priority_2, AsyncTaskLoadHitmap, params, false);
}

void HoverOnlyButton::DrawButton()
{
    ImVec2 topleft = ImGui::GetCursorPos();
    if (isMouseHoveredInArea_Size(topleft, m_forcedMinSize))
    {
        if (m_dHoverImgId)
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddImage(m_dHoverImgId,
                ImVec2(topleft.x, topleft.y),
                ImVec2(topleft.x + m_forcedMinSize.x, topleft.y + m_forcedMinSize.y));
        }
        if (ImGui::IsMouseClicked(0))
        {
            ImGuiIO& io = ImGui::GetIO();
            ImVec2 mousePos = io.MousePos;
            if (m_Hitmap.getPixel((int)(mousePos.x - topleft.x), (int)(mousePos.y - topleft.y)))
            {
                OnPush();
            }
        }
    }
    else
    {
#ifdef _DEBUG
        ImVec2 botRight = m_forcedMinSize;
        botRight.x += topleft.x; botRight.y += topleft.y;
        ImGui::GetWindowDrawList()->AddRect(topleft, botRight, IM_COL32(255, 0, 0, 255));
#endif
    }
}

void MultiStateButton::DrawButton(bool IsActive)
{
    ImVec2 topleft = ImGui::GetCursorPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    if (m_bDisabled == true)
    {
        if (m_dDisabledImgId)
        {
            drawList->AddImage(m_dDisabledImgId,
                ImVec2(topleft.x, topleft.y),
                ImVec2(topleft.x + m_forcedMinSize.x, topleft.y + m_forcedMinSize.y));
        }
    }
    else if (isMouseHoveredInArea_Size(topleft, m_forcedMinSize) && 
        m_Hitmap.getPixel((int)(io.MousePos.x - topleft.x), (int)(io.MousePos.y - topleft.y)))
    {
        if (IsActive == true && m_dActiveImgId != 0)
        {
            drawList->AddImage(m_dActiveImgId,
                ImVec2(topleft.x, topleft.y),
                ImVec2(topleft.x + m_forcedMinSize.x, topleft.y + m_forcedMinSize.y));
        }
        else if (m_dHoverImgId != 0)
        {
            drawList->AddImage(m_dHoverImgId,
                ImVec2(topleft.x, topleft.y),
                ImVec2(topleft.x + m_forcedMinSize.x, topleft.y  + m_forcedMinSize.y));
        }
        else if(m_dImgId != 0)
        {
            drawList->AddImage(m_dImgId,
                ImVec2(topleft.x, topleft.y),
                ImVec2(topleft.x + m_forcedMinSize.x, topleft.y + m_forcedMinSize.y));
        }
        if (ImGui::IsMouseClicked(0))
        {
            OnPush();
        }
    }
    else if(IsActive == true && m_dActiveImgId != 0)
    {
        drawList->AddImage(m_dActiveImgId,
            ImVec2(topleft.x, topleft.y),
            ImVec2(topleft.x + m_forcedMinSize.x, topleft.y + m_forcedMinSize.y));
    }
    else if (IsActive == false && m_dImgId != 0)
    {
        drawList->AddImage(m_dImgId,
            ImVec2(topleft.x, topleft.y),
            ImVec2(topleft.x + m_forcedMinSize.x, topleft.y + m_forcedMinSize.y));
    }
#ifdef _DEBUG
    ImVec2 botRight = m_forcedMinSize;
    botRight.x += topleft.x; botRight.y += topleft.y;
    ImGui::GetWindowDrawList()->AddRect(topleft, botRight, IM_COL32(255, 0, 0, 255));
#endif
}

void HoverPopupButton::DrawButton()
{
    ImVec2 topleft = ImGui::GetCursorPos();
    ImGuiIO& io = ImGui::GetIO();
    if (isMouseHoveredInArea_Size(topleft, m_forcedMinSize) &&
        m_Hitmap.getPixel((int)(io.MousePos.x - topleft.x), (int)(io.MousePos.y - topleft.y)))
    {
        if (m_dHoverImgId)
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddImage(m_dHoverImgId,
                ImVec2(topleft.x, topleft.y),
                ImVec2(topleft.x + m_forcedMinSize.x, topleft.y + m_forcedMinSize.y));
        }
    }
    else
    {
#ifdef _DEBUG
        ImVec2 botRight = m_forcedMinSize;
        botRight.x += topleft.x; botRight.y += topleft.y;
        ImGui::GetWindowDrawList()->AddRect(topleft, botRight, IM_COL32(255, 0, 0, 255));
#endif
    }
}