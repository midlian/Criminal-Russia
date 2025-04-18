#include "../main.h"
#include "game.h"
#include "../net/netgame.h"
#include <cmath>
#include "../gui/gui.h"
extern CGUI* pGUI;
extern CGame *pGame;
extern CNetGame *pNetGame;

CWidget::CWidget(const char* szTexture)
{
	m_dwLastTickClicked = 0;
	m_pSprite = new CSprite2d();
	m_pSprite->m_pRwTexture = (RwTexture*)LoadTextureFromDB(OBFUSCATE("mobile"), szTexture);
	m_bShouldBeDrawn = false;
	m_bColoured = false;
}

CWidget::~CWidget()
{
	delete m_pSprite;
}

void CWidget::SetDrawState(bool bState)
{
	m_bShouldBeDrawn = bState;
}

void CWidget::SetWidth(float w)
{
	this->w = pGUI->ScaleX(w);
}

void CWidget::SetHeight(float h)
{
	this->h = pGUI->ScaleX(h);
}

void CWidget::SetPos(float x, float y)
{
	this->x = pGUI->ScaleX(x);
	this->y = pGUI->ScaleY(y);

}

void CWidget::SetPosWithoutScale(float x, float y)
{
	this->x = x;
	this->y = y;
}

void CWidget::SetTexture(const char* szDB, const char* szTexture)
{
	m_pSprite->m_pRwTexture = (RwTexture*)LoadTextureFromDB(szDB, szTexture);
}
#include "..//chatwindow.h"
extern CChatWindow* pChatWindow;
void CWidget::Draw()
{
	if (!m_pSprite) return;
	if (!m_bShouldBeDrawn) return;
	CRGBA col;
	col.A = 255;
	col.B = 255;
	col.G = 255;
	col.R = 255;
	if (m_iState == 2)
	{
		if (GetTickCount() - m_dwLastTickClicked >= 75)
		{
			m_iState = 0;
		}
	}

	if (m_iState == 1)
	{
		//9CCF9C
		col.A = 255;
		col.B = 0x9C;
		col.G = 0xCF;
		col.R = 0x9C;
	}
	if (m_bColoured)
	{
		m_pSprite->Draw(x, y, w, h, color);
		m_bShouldBeDrawn = false;
		return;
	}
	m_pSprite->Draw(x, y, w, h, col);
	m_bShouldBeDrawn = false;
}

bool CWidget::IsXYInRect(float x, float y)
{
	return (x >= this->x && x <= this->x + w &&
		y >= this->y && y <= this->y + h);
}

void CWidget::SetColor(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	color.A = a;
	color.R = r;
	color.G = g;
	color.B = b;
	m_bColoured = true;
}

void CWidget::ResetColor()
{
	m_bColoured = false;
}


void CWidget::StatusMicro(bool status)
{
	m_bVoiceStatus = status;
	/*if(status){
		ImGuiIO &io = ImGui::GetIO();
		this->SetTexture("samp", "voice_button2");
		char bufferNotify[0xFF+1] = { '\0' };
		pChatWindow->AddDebugMessage("TOUCH_ON");
		sprintf(bufferNotify, "[%d] %s", pNetGame->GetPlayerPool()->GetLocalPlayerID(), pNetGame->GetPlayerPool()->GetLocalPlayerName());
		pNetGame->GetNotificationPool()->Add(0, 0, 0, ImVec2(pGUI->GetFontSize() * 5.5f, io.DisplaySize.y * 0.58f), bufferNotify, pGUI->GetFontSize(), 0xFFFAFAFA, true, 0xFF000000, 2, true, ImVec2(32, 32), pNetGame->GetNotificationPool()->GetCachedIconTexture(0), false);		
	
	}
	else
	{
			this->SetTexture("samp", "voice_button");
			pNetGame->GetNotificationPool()->Remove(0);
			pChatWindow->AddDebugMessage("m_iState=== 12 12");
			ResetColor();
	}*/
}

void CWidget::SetState(int iState)
{
	if (iState == 2 || iState == 12)
	{
		m_dwLastTickClicked = GetTickCount();
	}
	m_iState = iState;
}

int CWidget::GetState() const
{
	return m_iState;
}
