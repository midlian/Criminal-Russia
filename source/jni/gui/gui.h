#pragma once
#include "../vendor/imgui/imgui.h"
#include "..//vendor/raknet/SingleProducerConsumer.h"

enum eTouchType
{
	TOUCH_POP = 1,
	TOUCH_PUSH = 2,
	TOUCH_MOVE = 3
};

struct BUFFERED_COMMAND_TEXTDRAW
{
	uint16_t textdrawId;
};

class CGUI
{
public:
	CGUI();
	~CGUI();

	bool m_bMemDebugOn = false;

                  // -- fix widgets for carry
	bool m_bCarryStuff = false;

	void Render();
	void DrawTextureInfo(uintptr_t pEntity);

	float ScaleX(float x) { return m_vecScale.x * x; }
	float ScaleY(float y) { return m_vecScale.y * y; }
	ImFont* GetFont() { return m_pFont; }
	ImFont* GetSampFont() { return m_pSampFont; }
	float GetFontSize() { return m_fFontSize; }
                  ImFont* LoadFont(char *font, float fontsize);
	bool OnTouchEvent(int type, bool multi, int x, int y);
                  void AddText(ImFont* font, ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end, float font_size);
	void RenderText(ImVec2& pos, ImU32 col, bool bOutline, const char* text_begin, const char* text_end = nullptr);
	void RenderTextForChatWindow(ImVec2& pos, ImU32 col, bool bOutline, const char* text_begin, const char* text_end = nullptr);
                  void  RenderTextWithSize(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end, float font_size);
                  void RenderTextA(float font_size, ImVec2& posCur, ImU32 col, bool bOutline, ImU32 outlineCol, int outlineOffset, const char* text_begin, const char* text_end = nullptr);
	void PushToBufferedQueueTextDrawPressed(uint16_t textdrawId);
                  void RenderTextDeathMessage(ImVec2& pos, ImU32 col, bool bOutline, const char* text_begin, const char* text_end = nullptr, float font_size = 0.0f, ImFont *font = NULL, bool bOutlineUseTextColor = false);
                  ImFont* GetWeaponFont() { return m_pFontGTAWeap; }

//                  ImGuiWindow* m_imWindow = nullptr;
//	struct RwTexture* b_tab;
	struct RwTexture* b_l;
	struct RwTexture* b_h;
	struct RwTexture* b_r;
	struct RwTexture* b_alt;
//	struct RwTexture* b_g;
	struct RwTexture* b_n;
	struct RwTexture* b_y;
//	struct RwTexture* b_x;
	struct RwTexture* b_ctrl;
//	struct RwTexture* b_f;
	float m_fButWidth;
	float m_fButHeight;
private:
	void PreProcessInput();
	void PostProcessInput();
	void RenderRakNetStatistics();
	void RenderVersion();
	void ProcessPushedTextdraws();
public:
	ImFont* 	m_pFont;

private:
	DataStructures::SingleProducerConsumer<BUFFERED_COMMAND_TEXTDRAW> m_BufferedCommandTextdraws;

	struct RwTexture* m_pSplashTexture;
	ImFont*		m_pSampFont;
    ImFont*      m_pFontGTAWeap;
	ImVec2		m_vecScale;
	float 		m_fFontSize;

	bool		m_bNeedClearMousePos;
	bool		m_bMouseDown;
	ImVec2		m_vTouchPos;
	bool		m_bNextClear;

	bool 		m_bKeysStatus;
};