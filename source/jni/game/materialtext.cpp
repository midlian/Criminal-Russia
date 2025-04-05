#include "../main.h"
#include "game.h"
#include "RW/RenderWare.h"
#include "materialtext.h"
#include "../util/armhook.h"
#include "../gui/UIUtils.h"

extern CGame *pGame;
extern CGUI *pGUI;

/* imgui_impl_renderware.h */
bool ImGui_ImplRenderWare_NewFrame();
void ImGui_ImplRenderWare_RenderDrawData(ImDrawData* draw_data);
bool ImGui_ImplRenderWare_CreateDeviceObjects();

CMaterialText::CMaterialText()
{
    m_camera = 0;
    m_zBuffer = 0;
    SetUpScene();
}

void CMaterialText::SetUpScene()
{
    // RwCameraCreate
	m_camera = ((uintptr_t(*)())(g_libGTASA + 0x1ADA1C + 1))();

	// RwFrameCreate
	m_zBuffer = ((uintptr_t(*)())(g_libGTASA + 0x1AE9E0 + 1))();

	if(m_camera && m_zBuffer)
	{
		// _rwObjectHasFrameSetFrame
		((void(*)(uintptr_t, uintptr_t))(g_libGTASA + 0x1B2988 + 1))(m_camera, m_zBuffer);

		// RwCameraSetFarClipPlane
		((void(*)(uintptr_t, float))(g_libGTASA + 0x1AD710 + 1))(m_camera, 300.0f);
	
		// RwCameraSetNearClipPlane
		((void(*)(uintptr_t, float))(g_libGTASA + 0x1AD6F4 + 1))(m_camera, 0.01f);
	
		// RwCameraSetViewWindow
		float view[2] = { 0.5f, 0.5f };
		((void(*)(uintptr_t, float*))(g_libGTASA + 0x1AD924 + 1))(m_camera, view);
	
		// RwCameraSetProjection
		((void(*)(uintptr_t, int))(g_libGTASA + 0x1AD8DC+ 1))(m_camera, 1);
	
		// RpWorldAddCamera
		uintptr_t pRwWorld = *(uintptr_t*)(g_libGTASA + 0x95B060);
		if (pRwWorld) {
			((void(*)(uintptr_t, uintptr_t))(g_libGTASA + 0x1EB118 + 1))(pRwWorld, m_camera);
		}
	}
}

static uint16_t sizes[14][2] = {
            { 32, 32 } , { 64, 32 }, { 64, 64 }, { 128, 32 }, { 128, 64 }, { 128,128 }, { 256, 32 },
            { 256, 64 } , { 256, 128 } , { 256, 256 } , { 512, 64 } , { 512,128 } , { 512,256 } , { 512,512 }
    };

RwTexture* CMaterialText::Generate(uint8_t matSize, const char* fontname, float fontSize, uint8_t bold, uint32_t fontcol, uint32_t backcol, uint8_t align, const char* szText)
{
    matSize = (matSize / 10) - 1;

    uintptr_t raster = (uintptr_t)RwRasterCreate(sizes[matSize][0] * 2, sizes[matSize][1] * 2, 32, rwRASTERFORMAT8888 | rwRASTERTYPECAMERATEXTURE);
	
	// Create Texture Buffer
	uintptr_t bufferTexture = ((uintptr_t(*)(uintptr_t))(g_libGTASA + 0x1B1B4C + 1))(raster);
    //RwTexture* bufferTexture = RwTextureCreate((RwRaster*)raster);
    if(!raster || !bufferTexture) return 0;

    // set camera frame buffer
	*(uintptr_t*)(m_camera + 0x60) = (uintptr_t)raster;

	// CVisibilityPlugins::SetRenderWareCamera
	((void(*)(uintptr_t))(g_libGTASA + 0x55CFA4 + 1))(m_camera);

	// RwCameraClear
	int b = (backcol) & 0xFF;
	int g = (backcol >> 8) & 0xFF;
	int r = (backcol >> 16) & 0xFF;
	int a = (backcol >> 24) & 0xFF;
	unsigned int dwBackgroundABGR = (r | (g << 8) | (b << 16) | (a << 24));
	
	((void(*)(uintptr_t, unsigned int*, int))(g_libGTASA + 0x1AD8A0 + 1))(m_camera, &dwBackgroundABGR, 3);
	
	RwCameraBeginUpdate((RwCamera*)m_camera);

	/*RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)true);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)true);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODENASHADEMODE);
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void*)0);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODENACULLMODE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)false);*/

    CallFunction<void>(g_libGTASA + 0x559008 + 1);

    int bgB = (backcol) & 0xFF;
    int bgG = (backcol >> 8) & 0xFF;
    int bgR = (backcol >> 16) & 0xFF;
    int bgA = (backcol >> 24) & 0xFF;

    int fontB = (fontcol) & 0xFF;
    int fontG = (fontcol >> 8) & 0xFF;
    int fontR = (fontcol >> 16) & 0xFF;
    int fontA = (fontcol >> 24) & 0xFF;

    SetUpMaterial(matSize, fontname, fontSize * 0.8f, bold, ImColor(fontR, fontG, fontB, fontA), ImColor(bgR, bgG, bgB, bgA), align, szText);

	RwCameraEndUpdate((RwCamera*)m_camera);
    return (RwTexture*)bufferTexture;
}

void CMaterialText::SetUpMaterial(uint8_t matSize, const char* fontname, float fontSize, uint8_t bold, ImColor fontcol, ImColor backcol, uint8_t align, const char* szText)
{
	ImGui_ImplRenderWare_NewFrame();
    ImGui::NewFrame();

    char utf8[2048];
    cp1251_to_utf8(utf8, szText, 2048);

    // text align
    ImVec2 vecPos;

    std::string strText = utf8;
    std::stringstream ssLine(strText);
    std::string tmpLine;
    int newLineCount = 0;

    Log("TEXT: %s", szText);

    switch(align)
    {
        case OBJECT_MATERIAL_TEXT_ALIGN_LEFT:
            while(std::getline(ssLine, tmpLine, '\n'))
            {
                if(tmpLine[0] != 0)
                {
                    vecPos.x = 0;
                    vecPos.y = (sizes[matSize][1] - fontSize) / 2;
                }
            }
            break;
        case OBJECT_MATERIAL_TEXT_ALIGN_CENTER:
            while(std::getline(ssLine, tmpLine, '\n'))
            {
                if(tmpLine[0] != 0)
                {
                    vecPos.x = (sizes[matSize][0] / 2) - (UIUtils::CalcTextSizeWithoutTags((char *)tmpLine.c_str(), fontSize).x / 2);
                    vecPos.y = (sizes[matSize][1] / 2) - (UIUtils::CalcTextSizeWithoutTags((char *)tmpLine.c_str(), fontSize).y / 2);
                }
            }
            break;
        case OBJECT_MATERIAL_TEXT_ALIGN_RIGHT:
            while(std::getline(ssLine, tmpLine, '\n'))
            {
                if(tmpLine[0] != 0)
                {
                    vecPos.x = (sizes[matSize][0] - UIUtils::CalcTextSizeWithoutTags((char *)tmpLine.c_str(), fontSize).x);
                    vecPos.y = (sizes[matSize][1] - fontSize);
                }
            }
            break;
    }

    if(backcol)
    {
        ImVec2 backpos = ImVec2(vecPos.x-pGUI->GetFontSize(), vecPos.y-pGUI->GetFontSize());
        ImVec2 backsize = ImVec2(fontSize+pGUI->GetFontSize(), fontSize+pGUI->GetFontSize());
        ImGui::GetBackgroundDrawList()->AddRectFilled(backpos, backsize, backcol);
    }
    vecPos.x *= 2;
    vecPos.y *= 2;
    UIUtils::drawText(vecPos, fontcol, utf8, fontSize * 2);

	ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplRenderWare_RenderDrawData(ImGui::GetDrawData());
}