#pragma once

#include "Util/ObjDescriptor.h"
#include "Util/MonochromeImage.h"

typedef void (*ButtonPushCallback)();

class TransparentButton : public GenericButton
{
public:
	REFLECT_TYPE(TransparentButton);
	TransparentButton();
	void DrawButton();
	void LoadHitMap(const char *fileName);
protected:
	MonochromeImage m_Hitmap;
};

class HoverOnlyButton : public TransparentButton
{
public:
	REFLECT_TYPE(HoverOnlyButton);
	HoverOnlyButton() {
		InitTypeInfo(); 
		m_dHoverImgId = 0; 
	}
	void DrawButton();
	void SetHoverImageId(unsigned __int64 id) 
	{ 
		m_dHoverImgId = id;
	}
protected:
	unsigned __int64 m_dHoverImgId;
};

// radio buttons can have multiple states : inactive, hover, active, disabled
class MultiStateButton : public HoverOnlyButton
{
public:
	REFLECT_TYPE(MultiStateButton);
	MultiStateButton() { InitTypeInfo(); m_dActiveImgId = 0; m_dImgId = 0; }
	void DrawButton(bool IsActive = false);
	// in pressed down state. Used for radio groups
	void SetActiveImageId(unsigned __int64 id) 
	{ 
		m_dActiveImgId = id; 
	}
	// Default view of a button
	void SetImageId(unsigned __int64 id)
	{
		m_dImgId = id;
	}
	// Button can't be interacted with
	void SetDisabledImageId(unsigned __int64 id)
	{
		m_dDisabledImgId = id;
	}
protected:
	unsigned __int64 m_dActiveImgId; // activated button. Something like a radio button
	unsigned __int64 m_dImgId; // it's not hovered or activated. Default view of a button
	unsigned __int64 m_dDisabledImgId; // button is not usable. No hover / active / inactive states
};

class HoverPopupButton : public TransparentButton
{
public:
	REFLECT_TYPE(HoverPopupButton);
	HoverPopupButton() {
		InitTypeInfo();
		m_dHoverImgId = 0;
	}
	void DrawButton();
	void SetHoverImageId(unsigned __int64 id)
	{
		m_dHoverImgId = id;
	}
protected:
	unsigned __int64 m_dHoverImgId;
};