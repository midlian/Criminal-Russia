#pragma once

#include <stdio.h>
#include <string.h>

#include "../vendor/imgui/imgui.h"

typedef uint8_t NotificationType;
typedef uint16_t NotificationId;
typedef uint32_t NotificationDuration;
typedef ImVec2 NotificationPosOnScreen;
typedef ImVec2 NotificationIconScaleOnScreen;

class Notification
{
private:
	NotificationType g_type; 
	NotificationId g_id; 
	char g_text[256+1] = { '\0' };
	bool g_bHasIcon = false;
	uintptr_t g_pIconPointer = 0;
	NotificationDuration g_dwDuration;
	NotificationDuration g_dwStartTick;
	NotificationPosOnScreen g_vecPos;
	float g_fFontSize = 30.0f;
	NotificationIconScaleOnScreen g_vecIconScale;
	ImU32 g_textColor, g_textOutlineColor;
	bool g_bTextHasOutline = true;
	int g_iTextOutlineOffset = 2;

public:
	Notification(NotificationType type, NotificationId id, NotificationDuration duration, NotificationPosOnScreen vecPos = ImVec2(0, 0), const char* text = nullptr, float fFontSize = 30.0f, ImU32 textColor = 0xFFFFFFFF, bool bTextHasOutline = true, ImU32 textOutlineColor = 0xFF000000, int textOutlineOffset = 2, bool bHasIcon = false, NotificationIconScaleOnScreen vecIconScale = ImVec2(0, 0), uintptr_t pIconPointer = 0);
	~Notification();

	void setDuration(NotificationDuration duration) {
		this->g_dwDuration = duration;
	};

	void setType(NotificationType type) {
		this->g_type = type;
	};

	void setId(NotificationId id) {
		this->g_id = id;
	};

	void setText(const char* text) {
		strcpy(this->g_text, text);
	};

	void setHasIcon(bool bState) {
		this->g_bHasIcon = bState; 
	};

	void setIconTexturePointer(uintptr_t pTexture) {
		this->g_pIconPointer = pTexture;
	};

	void setIconTextureScale(NotificationIconScaleOnScreen scale) {
		this->g_vecIconScale = scale;
	};

	void setTextFontSize(float fFontSize) {
		this->g_fFontSize = fFontSize;
	};

	void setTextHasOutline(bool bHasOutline = true) {
		this->g_bTextHasOutline = bHasOutline;
	};

	void setTextColor(ImU32 col) {
		this->g_textColor = col;
	};

	void setTextOutlineColor(ImU32 col) {
		this->g_textOutlineColor = col;
	};

	void setTextOutlineOffset(int offset) {
		this->g_iTextOutlineOffset = offset;
	};
	
	NotificationType getType() { return this->g_type; };
	NotificationId getId() { return this->g_id; };
	const char* getText() { return this->g_text; };
	bool hasIcon() { return this->g_bHasIcon; };
	uintptr_t getIconTexturePointer() { return this->g_pIconPointer; };
	NotificationDuration getDuration() { return this->g_dwDuration; };
	NotificationDuration getStartTick() { return this->g_dwStartTick; };
	NotificationPosOnScreen getPos() { return this->g_vecPos; };
	NotificationIconScaleOnScreen getIconTextureScale() { return this->g_vecIconScale; };
	float getTextFontSize() { return this->g_fFontSize; };
	bool hasTextOutline() { return this->g_bTextHasOutline; };
	ImU32 getTextColor() { return this->g_textColor; };
	ImU32 getTextOutlineColor() { return this->g_textOutlineColor; };
	int getTextOutlineOffset() { return this->g_iTextOutlineOffset; };

	void Render();
	void RenderIcon();

	bool IsRenderOver() {
		return this->g_type == -1 || this->g_id == -1;
	}

	void RenderOver() {
		this->g_type = -1;
		this->g_id = -1;
	}
};