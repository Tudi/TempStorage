#pragma once

#define ImGUIUnderlinePrevItem(lowerByPixels, width, color)  {\
    ImVec2 textPos = ImGui::GetItemRectMin(); \
    ImVec2 textSize = ImGui::GetItemRectSize(); \
    float lineHeight = ImGui::GetTextLineHeight() + lowerByPixels; \
    ImVec2 lineStart(textPos.x + 3, textPos.y + lineHeight); \
    ImVec2 lineEnd(textPos.x + textSize.x - 3, textPos.y + lineHeight); \
    ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, color, width); \
}

class GenericButton;

class GenericWindow
{
public:
	virtual int DrawWindow() = 0;
	virtual void ResetState() = 0;	// check if we could shut down so that no memory is leaked. This is because managers are singletons
	virtual void DestructorCheckMemLeaks() {};
    virtual void OnWindowClosed() {};
	/// !! You should not do a lot of processing in events. Create a worker thread to avoid blocking UI !!
//	virtual void OnButtonHoverStart(GenericButton* btn) { btn; };
//	virtual void OnButtonHoverEnd(GenericButton* btn) { btn; };
private:
//	static virtual void OnButtonPush(GenericButton* btn, void *pParent) { btn; };
};
