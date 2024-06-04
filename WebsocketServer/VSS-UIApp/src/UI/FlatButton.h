#pragma once

#include "Util/ObjDescriptor.h"

typedef void (*ButtonPushCallback)();

class FlatButton : public GenericButton
{
public:
	REFLECT_TYPE(FlatButton);
	FlatButton();
	FlatButton(void* owner, ButtonCallback cb, ButtonIds id, LocalizationRssIds txtId);
	void SetDrawCentered(bool newVal) { m_bDrawWindowCentered = newVal;  }
	void DrawButton(ImGuiID Id = 0);
	void DrawButtonAsTextKnownSize();
	void ToggleUnderline(bool newVal) { m_bDrawUnderline = newVal; }
private:
	void Init();
	ImVec2 m_buttonRectMin;
	ImVec2 m_buttonRectMax;
	bool m_bDrawWindowCentered;
	float m_fCenterX;
	bool m_bDrawUnderline;
};