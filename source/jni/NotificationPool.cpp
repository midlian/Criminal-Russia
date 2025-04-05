#include "main.h"
#include "game/game.h"
#include "game/RW/RenderWare.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "CSettings.h"

extern CGUI *pGUI;

NotificationPool::NotificationPool() {
	for(int i = 0; i < ARRAY_SIZE(this->m_pNotify); i++) {
		this->m_pNotify[i] = (Notification*)nullptr;
		this->m_bNotifySlotState[i] = false;
	}

	//defs
	NotificationPool::CacheIconTexture(0, "samp", "voice_button2");
}

NotificationPool::~NotificationPool() {
	for(int i = 0; i < ARRAY_SIZE(this->m_pNotify); i++) {
		this->Remove(i);
	}
}

void NotificationPool::Add(NotificationType type, NotificationDuration duration, NotificationPosOnScreen vecPos, const char* text, float fFontSize, ImU32 textColor, bool bTextHasOutline, ImU32 textOutlineColor, int textOutlineOffset, bool bHasIcon, NotificationIconScaleOnScreen vecIconScale, uintptr_t pIconPointer, bool bRecreate) 
{
	NotificationId freeSlotID = -1;

	for(int i = 0; i < ARRAY_SIZE(m_pNotify); i++) 
	{
		if(!this->m_pNotify[i] && !this->m_bNotifySlotState[i]) {
			freeSlotID = i;
			break;
		}
	}

	if(freeSlotID == -1) {
		return;
	}

	this->Add(type, freeSlotID, duration, vecPos, text, fFontSize, textColor, bTextHasOutline, textOutlineColor, textOutlineOffset, bHasIcon, vecIconScale, pIconPointer, bRecreate); 
}

void NotificationPool::Add(NotificationType type, NotificationId id, NotificationDuration duration, NotificationPosOnScreen vecPos, const char* text, float fFontSize, ImU32 textColor, bool bTextHasOutline, ImU32 textOutlineColor, int textOutlineOffset, bool bHasIcon, NotificationIconScaleOnScreen vecIconScale, uintptr_t pIconPointer, bool bRecreate) 
{
	if(this->GetAt(id) && bRecreate) {
		SAFE_DELETE(this->m_pNotify[id]);
		this->m_bNotifySlotState[id] = false;
	}

	if(!this->GetSlotState(id) && !this->GetAt(id)) 
	{
		this->m_pNotify[id] = new Notification(type, id, duration, vecPos, text, fFontSize, textColor, bTextHasOutline, textOutlineColor, textOutlineOffset, bHasIcon, vecIconScale, pIconPointer);
		this->m_bNotifySlotState[id] = true;
	}
}

void NotificationPool::Remove(NotificationId id) {
	if(this->GetAt(id)) {
		SAFE_DELETE(this->m_pNotify[id]);
		this->m_bNotifySlotState[id] = false;
	}
}

void NotificationPool::CacheIconTexture(NotificationId id, const char* db, const char* tex) {
	if(!ARRAY_VALIDATE(pCachedIconTextures, id)) return;
	uintptr_t texture = LoadTextureFromDB(db, tex);
	pCachedIconTextures[id] = texture;
}

void NotificationPool::Process() 
{
	for(int i = 0; i < ARRAY_SIZE(m_pNotify); i++) 
	{
		if(!this->GetSlotState(i)) {
			continue;
		}

		if(Notification* pNotify = this->GetAt(i)) {
			if(pNotify->getDuration() > 0) {
				if(GetTickCount() - pNotify->getStartTick() >= pNotify->getDuration()) {
					this->Remove(i);
				}
			}
		}
	}
}	 

void NotificationPool::Render() {
	for(int i = 0; i < ARRAY_SIZE(m_pNotify); i++) 
	{
		if(!this->GetSlotState(i)) {
			continue;
		}

		if(Notification* pNotify = this->GetAt(i)) {
			pNotify->Render();
		}
	}
}