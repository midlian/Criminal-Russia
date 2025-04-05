#pragma once

#include "Notification.h"


#define MAX_NOTIFIES 64

class NotificationPool
{
private:
	Notification* m_pNotify[MAX_NOTIFIES+1];
	bool m_bNotifySlotState[MAX_NOTIFIES+1];
	uintptr_t pCachedIconTextures[MAX_NOTIFIES+1];

public:
	NotificationPool();
	~NotificationPool();

	void AddMixed(uint16_t playerid, NotificationType type, NotificationDuration duration, NotificationPosOnScreen vecPos, const char* text, float fFontSize, ImU32 textColor, bool bTextHasOutline, ImU32 textOutlineColor, int textOutlineOffset, bool bHasIcon, NotificationIconScaleOnScreen vecIconScale, uintptr_t pIconPointer, bool bRecreate);
	void Add(NotificationType type, NotificationDuration duration, NotificationPosOnScreen vecPos, const char* text, float fFontSize, ImU32 textColor, bool bTextHasOutline, ImU32 textOutlineColor, int textOutlineOffset, bool bHasIcon, NotificationIconScaleOnScreen vecIconScale, uintptr_t pIconPointer, bool bRecreate);
	void Add(NotificationType type, NotificationId id, NotificationDuration duration, NotificationPosOnScreen vecPos, const char* text, float fFontSize, ImU32 textColor, bool bTextHasOutline, ImU32 textOutlineColor, int textOutlineOffset, bool bHasIcon, NotificationIconScaleOnScreen vecIconScale, uintptr_t pIconPointer, bool bRecreate);
	void Remove(NotificationId id);
	void Process();	

	bool GetSlotState(NotificationId id) {
		return this->m_bNotifySlotState[id];
	};

	Notification* GetAt(NotificationId id) {
		if(this->GetSlotState(id)) {
			return this->m_pNotify[id];
		}

		return (Notification*)nullptr;
	};

	// todo: make texture cache class
	void CacheIconTexture(NotificationId, const char*, const char*);
	uintptr_t GetCachedIconTexture(NotificationId id) {
		return pCachedIconTextures[id];
	};

	void Render();
};