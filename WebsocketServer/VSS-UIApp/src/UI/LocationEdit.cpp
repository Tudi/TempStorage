#include "stdafx.h"

LocationEditWindow::LocationEditWindow() :
    m_ButtonSave(this, OnWindowButtonClick, ButtonIds::BI_LOCATIONEDIT_SAVE, LocalizationRssIds::LocationEdit_Save_Btn_Text),
    m_ButtonCancel(this, OnWindowButtonClick, ButtonIds::BI_LOCATIONEDIT_CANCEL, LocalizationRssIds::LocationEdit_Cancel_Btn_Text)
{
    InitTypeInfo();
    m_ButtonCancel.SetColor(GenericButtonColors::GBCT_TextHover, sStyles.GetUIntValue(StyleIds::Btn_Cancel_TextHover_Color));
    m_ButtonCancel.SetColor(GenericButtonColors::GBCT_UnderlineHover, sStyles.GetUIntValue(StyleIds::Btn_Cancel_TextHover_Color));

    m_sName.SetBadValText("Can't be empty");
    m_sName.SetCheckValFunc(InputTextData::CheckStrIsEmpty);
    m_sAddress1.SetBadValText("Can't be empty");
    m_sAddress1.SetCheckValFunc(InputTextData::CheckStrIsEmpty);
    m_sCity.SetBadValText("Can't be empty");
    m_sCity.SetCheckValFunc(InputTextData::CheckStrIsEmpty);
    m_sState.SetBadValText("Can't be empty");
    m_sState.SetCheckValFunc(InputTextData::CheckStrIsEmpty);
    m_sCountry.SetBadValText("Can't be empty");
    m_sCountry.SetCheckValFunc(InputTextData::CheckStrIsEmpty);
    m_sLatitude.SetBadValText("Can't be empty");
    m_sLatitude.SetCheckValFunc(InputTextData::CheckStrIsEmpty);
    m_sLongitude.SetBadValText("Can't be empty");
    m_sLongitude.SetCheckValFunc(InputTextData::CheckStrIsEmpty);

    ResetState(0);
}

void LocationEditWindow::DestructorCheckMemLeaks()
{
    m_ButtonSave.DestructorCheckMemLeaks();
    m_ButtonCancel.DestructorCheckMemLeaks();
}

void LocationEditWindow::ResetState(int id)
{
    // we will still refresh the data from DB though
    // id = 0xFFFFFF means we are closing the window
    // id = 0 means we are creating a new location
    if (id != 0xFFFFFF && (m_uEditedID != id) || id == 0)
    {
        m_uEditedID = id;
        m_sName.ResetState(NULL);
        m_sDescription.ResetState(NULL);
        m_sAddress1.ResetState(NULL);
        m_sAddress2.ResetState(NULL);
        m_sCity.ResetState(NULL);
        m_sState.ResetState(NULL);
        m_sCountryRegion.ResetState(NULL);
        m_sCountry.ResetState(NULL);
        m_sLatitude.ResetState(NULL);
        m_sLongitude.ResetState(NULL);
        m_sElevation.ResetState(NULL);
        sprintf_s(m_sElevation.GetInputBuff(), m_sElevation.GetInputSize(), "0");
    }
    m_bSetDefaultFocus = true;
    // this will not be show ? Set value to 0
    // refresh location data from DB
    if (id != 0 && id < 0xFFFFFF)
    {
        m_ButtonSave.SetText(sLocalization.GetString(LocalizationRssIds::LocationEdit_Save_Btn_Text));
        WebApi_GetLocationsAsync(id, 0, LocationEditWindow::CB_AsyncDataArived, this);
    }
    else
    {
        m_ButtonSave.SetText(sLocalization.GetString(LocalizationRssIds::LocationEdit_SaveNew_Btn_Text));
    }
}

void LocationEditWindow::CB_AsyncDataArived(int CurlErr, char* response, void* userData)
{
    yyJSON(yydoc);
    if (ExtractDBColumnToBinary::DBH_APIResultValid(CurlErr, response, yydoc, LogSourceGroups::LogSourceLocationEditWindow,"") != WebApiErrorCodes::WAE_NoError)
    {
        return;
    }

    const char* arrayName = "Locations";

    LocationEditWindow* wnd = (LocationEditWindow*)userData;
    ExtractDBColumnToBinary extractColumns[] = {
        {"LocationName", &wnd->m_sName},
        {"LocationDescription", &wnd->m_sDescription},
        {"LocationAddressLine1", &wnd->m_sAddress1},
        {"LocationAddressLine2", &wnd->m_sAddress2},
        {"LocationCity", &wnd->m_sCity},
        {"LocationState", &wnd->m_sState},
        {"LocationCountyOrRegion", &wnd->m_sCountryRegion},
        {"LocationCountry", &wnd->m_sCountry},
        {"LocationX", &wnd->m_sLatitude},
        {"LocationY", &wnd->m_sLongitude},
        {NULL, (InputTextData*)NULL} };

    ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yydoc, arrayName, extractColumns, LogSourceGroups::LogSourceLocationEditWindow);
}

int LocationEditWindow::DrawWindow()
{
    int display_w, display_h;
    GetDrawAreaSize(display_w, display_h, true);
    const int inputTextboxWidth = 1000;
    const float LeftBorder = 100;
    const float TopBorder = 100;
    float TextRowSpacing = ImGui::GetTextLineHeightWithSpacing() - 2;
    float WindowWidth = (float)display_w - LeftBorder - 15;
    float WindowHeight = (float)display_h - TopBorder - 15;
    const float RaiseInputField = 3;

    ImGui::SetNextWindowPos(ImVec2(LeftBorder, TopBorder), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)WindowWidth - LeftBorder, (float)WindowHeight - TopBorder), ImGuiCond_Always);

    if (ImGui::BeginChild(GetImGuiID("##LocationEdit"), ImVec2(WindowWidth, WindowHeight), false, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
    {
        ImGui::PushFont(sFontManager.GetFont(FontIds::FI_Medium));
        ImGui::PushStyleColor(ImGuiCol_Text, sStyles.GetUIntValue(StyleIds::Title_Location_Text_Color));
        if (m_uEditedID == 0)
        {
            ImGui::Text("Add Location");
        }
        else
        {
            ImGui::Text("Edit Location");
        }
        ImGui::PopStyleColor(1);
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
        if (m_uEditedID == 0)
        {
            ImGui::Text("Add a new location to the system then sync security modules to the location to begin real-time monitoring.");
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing

        if (m_bSetDefaultFocus == true)
        {
            ImGui::SetKeyboardFocusHere();
            m_bSetDefaultFocus = false;
        }
        FORM_FLAT_TXTI_FIELD("Name* ", m_sName);
        FORM_FLAT_TXTI_FIELD("Description ", m_sDescription);
        FORM_FLAT_TXTI_FIELD("Street Adress* ", m_sAddress1);
        FORM_FLAT_TXTI_FIELD("Address 2 (Unit, Suite) ", m_sAddress2);
        FORM_FLAT_TXTI_FIELD("City* ", m_sCity);
        FORM_FLAT_TXTI_FIELD("State* ", m_sState);
        FORM_FLAT_TXTI_FIELD("County / Region ", m_sCountryRegion);
        FORM_FLAT_TXTI_FIELD("Country* ", m_sCountry);
        FORM_FLAT_TXTI_FIELD("Lat (Decimal Form)* ", m_sLatitude);
        FORM_FLAT_TXTI_FIELD("Lon (Decimal Form)* ", m_sLongitude);
//            FORM_FLAT_TXTI_FIELD("Elev (Decimal Form)* ", m_sElevation);

        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
        m_ButtonSave.DrawButton();
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + TextRowSpacing); // row spacing
        m_ButtonCancel.DrawButton();
    }
    ImGui::EndChild();
    return WindowManagerErrorCodes::WM_NO_ERROR;
}

void LocationEditWindow::OnWindowButtonClick(GenericButton* btn, void* pParent)
{
    pParent;
    if (btn == NULL)
    {
        return;
    }

    if (btn->GetId() == ButtonIds::BI_LOCATIONEDIT_CANCEL)
    {
        sWindowManager.SetLocationEditWindowVisible(false, 0);
    }

    if (btn->GetId() == ButtonIds::BI_LOCATIONEDIT_SAVE)
    {
        LocationEditWindow* wnd = typecheck_castL(LocationEditWindow, pParent);

        // check if required fields have value
        InputTextData *MustHaveData[] = { &wnd->m_sName, &wnd->m_sAddress1, &wnd->m_sCity,
            &wnd->m_sState, &wnd->m_sCountry, &wnd->m_sLatitude, &wnd->m_sLongitude };
        bool AllRequiredHaveValue = true;
        for (int i = 0; i < _countof(MustHaveData); i++)
        {
            if(MustHaveData[i]->CheckNewValOk(NULL) == false)
            {
                AllRequiredHaveValue = false;
            }
        }
        if (AllRequiredHaveValue == false)
        {
            return;
        }

        // queue an async data update
        WebApi_UpdateLocationAsync(wnd->m_uEditedID, wnd->m_sName.GetInputBuff(), 
            wnd->m_sDescription.GetInputBuff(), wnd->m_sAddress1.GetInputBuff(), wnd->m_sAddress2.GetInputBuff(),
            wnd->m_sCity.GetInputBuff(), wnd->m_sState.GetInputBuff(), wnd->m_sCountryRegion.GetInputBuff(),
            wnd->m_sCountry.GetInputBuff(), wnd->m_sLatitude.GetInputBuff(), wnd->m_sLongitude.GetInputBuff(), 
            wnd->m_sElevation.GetInputBuff(), NULL, NULL);

        // update recent window name in case we edited the name
        // Note that if the update fails, the recent window will show incorrent location name 
        sLocationRecentManager.OnLocationUpdated(wnd->m_uEditedID, wnd->m_sName.GetInputBuff());

        // hide the currently open window. Will show us the splash screen
        sWindowManager.SetLocationEditWindowVisible(false, 0);
    }
}

void LocationEditWindow::SetLocationData(const char* name, const char* desc, const char* addr1, const char* addr2,
    const char* city, const char* state, const char* countryRegion, const char* country)
{
    m_sName.ResetState(name);
    m_sDescription.ResetState(desc);
    m_sAddress1.ResetState(addr1);
    m_sAddress2.ResetState(addr2);
    m_sCity.ResetState(city);
    m_sState.ResetState(state);
    m_sCountryRegion.ResetState(countryRegion);
    m_sCountry.ResetState(country);
}