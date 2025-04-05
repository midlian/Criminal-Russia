#include "main.h"
#include "game/game.h"
#include "game/RW/RenderWare.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "Notification.h"
#include "CSettings.h"

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CGUI *pGUI;
extern CSettings *pSettings;

Notification::Notification(NotificationType type, NotificationId id, NotificationDuration duration, NotificationPosOnScreen vecPos, const char* text, float fFontSize, ImU32 textColor, bool bTextHasOutline, ImU32 textOutlineColor, int textOutlineOffset, bool bHasIcon, NotificationIconScaleOnScreen vecIconScale, uintptr_t pIconPointer) 
{
	this->g_type = type; 
	this->g_id = id; 
	this->g_dwDuration = duration;
	this->g_vecPos = vecPos;
	this->g_bTextHasOutline = bTextHasOutline;
	this->g_textColor = textColor;
	this->g_iTextOutlineOffset = textOutlineOffset;
	this->g_textOutlineColor = textOutlineColor;
	this->g_fFontSize = fFontSize;
	strcpy(this->g_text, text);

	if(bHasIcon) {
		this->g_bHasIcon = bHasIcon;
		this->g_vecIconScale = vecIconScale;
		this->g_pIconPointer = pIconPointer;
	} else {
		this->g_bHasIcon = false;
		this->g_vecIconScale = ImVec2(0, 0);
		this->g_pIconPointer = (uintptr_t)NULL;
	}

	this->g_dwStartTick = GetTickCount();
	
	
	SAMP_DUMP("Notification", "Created new notify -> type: %d, id: %d, duration = %d, text = %s, hasIcon = %s", type, id, duration, text ? text : "", bHasIcon ? "true" : "false");
}

Notification::~Notification() {
	SAMP_DUMP("Notification", "Notify destroyed -> id: %d", this->g_id);
	this->g_type = -1; 
	this->g_id = -1; 
	this->g_dwDuration = 0;
	this->g_fFontSize = 0.0f;
	memset(this->g_text, '\0', sizeof(this->g_text));
	this->g_bTextHasOutline = false;
	this->g_iTextOutlineOffset = 0;
	this->g_textOutlineColor = 0;
	this->g_textColor = 0;
	this->g_bHasIcon = false;
	this->g_pIconPointer = (uintptr_t)NULL;
	this->g_vecIconScale = ImVec2(0, 0);
	this->g_dwStartTick = 0;
	this->g_vecPos = ImVec2(0, 0);

}

void Notification::RenderIcon() 
{
	// icon with text
	if(STR_VALIDATE(this->getText())) 
	{
		int iconScaleX = this->g_vecIconScale.x;
		int iconScaleY = this->g_vecIconScale.y;

		ImVec2 pos = this->getPos();
		pos.x -= iconScaleX / 2 + (pGUI->GetFontSize() * 1.5f);
   	 	pos.y -= (iconScaleY / 2) - (ImGui::CalcTextSizeByFontSize(this->getTextFontSize(), this->getText()).y / 2);
		pos.x -= (this->getTextFontSize() / 2 + iconScaleX / 2) - this->getTextFontSize();

   		ImVec2 a = ImVec2(pos.x, pos.y);
   		ImVec2 b = ImVec2(pos.x + iconScaleX, pos.y + iconScaleY);
   		RwTexture* pTexture = (RwTexture*)this->getIconTexturePointer();
   		ImGui::GetBackgroundDrawList()->AddImage((ImTextureID)pTexture->raster, a, b);
   		return;
	}

	// icon without text
	int iconScaleX = this->g_vecIconScale.x;
	int iconScaleY = this->g_vecIconScale.y;
	ImVec2 pos = this->getPos();
	
   	ImVec2 a = ImVec2(pos.x, pos.y);
   	ImVec2 b = ImVec2(pos.x + iconScaleX, pos.y + iconScaleY);
   	RwTexture* pTexture = (RwTexture*)this->getIconTexturePointer();
   	ImGui::GetBackgroundDrawList()->AddImage((ImTextureID)pTexture->raster, a, b);
}

void Notification::Render() 
{
	if(!pGUI || this->g_type == -1 || this->g_id == -1) {
		return;
	}

	if(STR_VALIDATE(this->g_text)) {
		ImVec2 vecPos = this->getPos();
		pGUI->RenderTextA(this->getTextFontSize(), vecPos, this->getTextColor(), this->hasTextOutline(), this->getTextOutlineColor(), this->getTextOutlineOffset(), this->getText());
	}
	
	if(this->g_bHasIcon && this->g_pIconPointer != 0) {
		this->RenderIcon();
	}
}