#include "CJavaWrapper.h"

#include "../main.h"
#include "../scoreboard.h"
#include "../dialog.h"

#include "../game/game.h"

#include "../net/netgame.h"
#include "../net/CustomPacketEnumeration.h"

extern "C" JavaVM* javaVM;

#include "../keyboard.h"
#include "../chatwindow.h"
#include "../CSettings.h"
#include "../KillList.h"
extern CGame* pGame;
extern CNetGame* pNetGame;
extern CKeyBoard* pKeyBoard;
extern CChatWindow* pChatWindow;
extern CSettings* pSettings;
extern CScoreBoard* pScoreBoard;
extern CDialogWindow* pDialogWindow;
extern KillList *pKillList;

JNIEnv* CJavaWrapper::GetEnv()
{
	JNIEnv* env = nullptr;
	int getEnvStat = javaVM->GetEnv((void**)& env, JNI_VERSION_1_4);

	if (getEnvStat == JNI_EDETACHED)
	{
		Log("GetEnv: not attached");
		if (javaVM->AttachCurrentThread(&env, nullptr) != 0)
		{
			Log("Failed to attach");
			return nullptr;
		}
	}

	if (getEnvStat == JNI_EVERSION)
	{
		Log("GetEnv: version not supported");
		return nullptr;
	}

	if (getEnvStat == JNI_ERR)
	{
		Log("GetEnv: JNI_ERR");
		return nullptr;
	}

	return env;
}

std::string CJavaWrapper::GetClipboardString()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return std::string("");
	}

	jbyteArray retn = (jbyteArray)env->CallObjectMethod(activity, s_GetClipboardText);

	if ((env)->ExceptionCheck())
	{
		(env)->ExceptionDescribe();
		(env)->ExceptionClear();
		return std::string("");
	}

	if (!retn)
	{
		return std::string("");
	}

	jboolean isCopy = true;

	jbyte* pText = env->GetByteArrayElements(retn, &isCopy);
	jsize length = env->GetArrayLength(retn);

	std::string str((char*)pText, length);

	env->ReleaseByteArrayElements(retn, pText, JNI_ABORT);
	env->DeleteLocalRef(retn);
	return str;
}

void CJavaWrapper::CallLauncherActivityWithSpecialValue(int type, int value) {
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	// SAMP_DUMP("dbg5001", "CallLauncherActivityWithSpecialValue: id -> %d", type);

	env->CallVoidMethod(activity, s_CallLauncherActivityWithSpecialValue, type, value);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::CallLauncherActivity(int type)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	// SAMP_DUMP("dbg5001", "CallLauncherActivity: id -> %d", type);

	env->CallVoidMethod(activity, s_CallLauncherActivity, type);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetHudLogo(int type)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetHudLogo, type);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetHudSatiety(int type)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetHudSatiety, type);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetSpeedMenu(int stype, int svalue)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetSpeedMenu, stype, svalue);

	EXCEPTION_CHECK(env);
}


void CJavaWrapper::SetCashMoney(jstring cash)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_setCashMoney, cash);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ShowInputLayout()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_ShowInputLayout);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::HideInputLayout()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_HideInputLayout);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ShowClientSettings()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_ShowClientSettings);

	EXCEPTION_CHECK(env);
}

bool CJavaWrapper::IsSomethingUIShitOnScreenActive()
{
	JNIEnv* env = GetEnv();
	bool bIsActiveOnScreen = true;

	if (!env)
	{
		Log("No env");
		return bIsActiveOnScreen;
	}

	jboolean IsActiveOnScreen = env->CallBooleanMethod(activity, s_IsSomethingUIShitOnScreenActive);
	bIsActiveOnScreen = (bool)(IsActiveOnScreen == JNI_TRUE) ? true : false;
	// SAMP_DUMP("dbg5001", "bIsActiveOnScreen = %s", bIsActiveOnScreen ? "true" : "false");
	
	if ((env)->ExceptionCheck()) 
	{ 
		(env)->ExceptionDescribe(); 
		(env)->ExceptionClear(); 
		return bIsActiveOnScreen;
	}

	return bIsActiveOnScreen;
}

void CJavaWrapper::SetUseFullScreen(int b)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetUseFullScreen, b);

	EXCEPTION_CHECK(env);
}
extern int g_iStatusDriftChanged;
#include "..//CDebugInfo.h"
extern "C"
{
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onInputEnd(JNIEnv* pEnv, jobject thiz, jbyteArray str)
	{
		if (pKeyBoard)
		{
			pKeyBoard->OnNewKeyboardInput(pEnv, thiz, str);
		}
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onEventBackPressed(JNIEnv* pEnv, jobject thiz)
	{
		if (pKeyBoard)
		{
			if (pKeyBoard->IsOpen())
			{
				// Log("Closing keyboard");
				pKeyBoard->Close();
			}
		}
		
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onNativeHeightChanged(JNIEnv* pEnv, jobject thiz, jint orientation, jint height)
	{
		if (pChatWindow)
		{
			pChatWindow->SetLowerBound(height);
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeCutoutSettings(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iCutout = b;
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeKeyboardSettings(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iAndroidKeyboard = b;
		}

		if (pKeyBoard && b) {
			pKeyBoard->EnableNewKeyboard();
		} else if(pKeyBoard) {
			pKeyBoard->EnableOldKeyboard();
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeFpsCounterSettings(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iFPSCounter = b;
		}

		CDebugInfo::SetDrawFPS(b);
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeHpArmourText(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			if (!pSettings->GetWrite().iHPArmourText && b)
			{
				if (CAdjustableHudColors::IsUsingHudColor(HUD_HP_TEXT) == false)
				{
					CAdjustableHudColors::SetHudColorFromRGBA(HUD_HP_TEXT, 255, 0, 0, 255);
				}
				if (CAdjustableHudPosition::GetElementPosition(HUD_HP_TEXT).X == -1 || CAdjustableHudPosition::GetElementPosition(HUD_HP_TEXT).Y == -1)
				{
					CAdjustableHudPosition::SetElementPosition(HUD_HP_TEXT, 500, 500);
				}
				if (CAdjustableHudScale::GetElementScale(HUD_HP_TEXT).X == -1 || CAdjustableHudScale::GetElementScale(HUD_HP_TEXT).Y == -1)
				{
					CAdjustableHudScale::SetElementScale(HUD_HP_TEXT, 400, 400);
				}

				if (CAdjustableHudColors::IsUsingHudColor(HUD_ARMOR_TEXT) == false)
				{
					CAdjustableHudColors::SetHudColorFromRGBA(HUD_ARMOR_TEXT, 255, 0, 0, 255);
				}
				if (CAdjustableHudPosition::GetElementPosition(HUD_ARMOR_TEXT).X == -1 || CAdjustableHudPosition::GetElementPosition(HUD_ARMOR_TEXT).Y == -1)
				{
					CAdjustableHudPosition::SetElementPosition(HUD_ARMOR_TEXT, 300, 500);
				}
				if (CAdjustableHudScale::GetElementScale(HUD_ARMOR_TEXT).X == -1 || CAdjustableHudScale::GetElementScale(HUD_ARMOR_TEXT).Y == -1)
				{
					CAdjustableHudScale::SetElementScale(HUD_ARMOR_TEXT, 400, 400);
				}
			}

			pSettings->GetWrite().iHPArmourText = b;
		}

		CInfoBarText::SetEnabled(b);
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeOutfitGunsSettings(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iOutfitGuns = b;

			CWeaponsOutFit::SetEnabled(b);
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativePcMoney(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iPCMoney = b;
		}

		CGame::SetEnabledPCMoney(b);
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeRadarrect(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iRadarRect = b;

			CRadarRect::SetEnabled(b);
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeFX(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iFX = b;
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeTimeInHud(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
			pSettings->GetWrite().iTimeInHud = b;

		g_pJavaWrapper->UpdateHud();
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeOnlineInHud(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
			pSettings->GetWrite().iOnlineInHud = b;

		g_pJavaWrapper->UpdateHud();
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeNewDialogs(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
			pSettings->GetWrite().iNewDialogs = b;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeNewHud(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
			pSettings->GetWrite().iNewHud = b;

		*(uint8_t*)(SA_ADDR(0x7165E8)) = !b;

		if(b) {
			g_pJavaWrapper->ShowHUD(true);
                                    } else {
                                                      g_pJavaWrapper->HideHUD(true);
                                    }

		g_pJavaWrapper->isGlobalShowHUD = b;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeChatHidden(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
			pSettings->GetWrite().iChatHidden = b;

                                    if (pChatWindow)
			pChatWindow->SetChatDissappearTimeout(80, 0);

	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeSkyBox(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iSkyBox = b;
			g_iStatusDriftChanged = 1;
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setTogleHud(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if(b == true) {
			pGame->ToggleHUDElement(HUD_ELEMENT_CHAT, true);
			pGame->ToggleHUDElement(HUD_ELEMENT_MAP, true);
			pGame->ToggleHUDElement(HUD_ELEMENT_BUTTONS, true);
			pGame->HandleChangedHUDStatus();
			Log("setTogleHud true");
		} else {
			pGame->ToggleHUDElement(HUD_ELEMENT_CHAT, false);
			pGame->ToggleHUDElement(HUD_ELEMENT_MAP, false);
			pGame->ToggleHUDElement(HUD_ELEMENT_BUTTONS, false);
			pGame->HandleChangedHUDStatus();
			Log("setTogleHud false");
		}
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeKillList(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iKillList = b;
			if(!b) {
				pKillList->ClearList();
			}
		}
	}
	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeCutoutSettings(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iCutout;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeKeyboardSettings(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iAndroidKeyboard;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeFpsCounterSettings(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iFPSCounter;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeHpArmourText(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iHPArmourText;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeOutfitGunsSettings(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iOutfitGuns;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativePcMoney(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iPCMoney;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeRadarrect(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iRadarRect;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeFX(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
			return pSettings->GetReadOnly().iFX;

		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeTimeInHud(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
			return pSettings->GetReadOnly().iTimeInHud;

		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeOnlineInHud(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
			return pSettings->GetReadOnly().iOnlineInHud;

		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeNewDialogs(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
			return pSettings->GetReadOnly().iNewDialogs;

		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeNewHud(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
			return pSettings->GetReadOnly().iNewHud;

		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeChatHidden(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
			return pSettings->GetReadOnly().iChatHidden;

		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeSkyBox(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iSkyBox;
		}
		return 0;
	}
	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeKillList(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iKillList;
		}
		return 0;
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onSettingsWindowSave(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			pSettings->Save();
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onSettingsWindowDefaults(JNIEnv* pEnv, jobject thiz, jint category)
	{
		if (pSettings)
		{
			pSettings->ToDefaults(category);
			if (pChatWindow)
			{
				pChatWindow->m_bPendingReInit = true;
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeHudElementColor(JNIEnv* pEnv, jobject thiz, jint id, jint a, jint r, jint g, jint b)
	{
		CAdjustableHudColors::SetHudColorFromRGBA((E_HUD_ELEMENT)id, r, g, b, a);
	}

	JNIEXPORT jbyteArray JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeHudElementColor(JNIEnv* pEnv, jobject thiz, jint id)
	{
		char pTemp[9];
		jbyteArray color = pEnv->NewByteArray(sizeof(pTemp));

		if (!color)
		{
			return nullptr;
		}

		pEnv->SetByteArrayRegion(color, 0, sizeof(pTemp), (const jbyte*)CAdjustableHudColors::GetHudColorString((E_HUD_ELEMENT)id).c_str());

		return color;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeHudElementPosition(JNIEnv* pEnv, jobject thiz, jint id, jint x, jint y)
	{
		if (id == 7)
		{
			if (pSettings)
			{
				pSettings->GetWrite().fChatPosX = x;
				pSettings->GetWrite().fChatPosY = y;
				if (pChatWindow)
				{
					pChatWindow->m_bPendingReInit = true;
				}
				return;
			}
			return;
		}
		if (id == HUD_SNOW)
		{
			if (pSettings)
			{
				pSettings->GetWrite().iSnow = x;
			}
			CSnow::SetCurrentSnow(pSettings->GetReadOnly().iSnow);
			return;
		}
		CAdjustableHudPosition::SetElementPosition((E_HUD_ELEMENT)id, x, y);

		if (id >= HUD_WEAPONSPOS && id <= HUD_WEAPONSROT)
		{
			CWeaponsOutFit::OnUpdateOffsets();
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeHudElementScale(JNIEnv* pEnv, jobject thiz, jint id, jint x, jint y)
	{
		CAdjustableHudScale::SetElementScale((E_HUD_ELEMENT)id, x, y);

		if (id >= HUD_WEAPONSPOS && id <= HUD_WEAPONSROT)
		{
			CWeaponsOutFit::OnUpdateOffsets();
		}
	}

	JNIEXPORT jintArray JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeHudElementScale(JNIEnv* pEnv, jobject thiz, jint id)
	{
		jintArray color = pEnv->NewIntArray(2);

		if (!color)
		{
			return nullptr;
		}
		int arr[2];
		arr[0] = CAdjustableHudScale::GetElementScale((E_HUD_ELEMENT)id).X;
		arr[1] = CAdjustableHudScale::GetElementScale((E_HUD_ELEMENT)id).Y;
		pEnv->SetIntArrayRegion(color, 0, 2, (const jint*)& arr[0]);

		return color;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeWidgetPositionAndScale(JNIEnv* pEnv, jobject thiz, jint id, jint x, jint y, jint scale)
	{
		if (id == 0)
		{
			if (pSettings)
			{
				pSettings->GetWrite().fButtonMicrophoneX = x;
				pSettings->GetWrite().fButtonMicrophoneY = y;
				pSettings->GetWrite().fButtonMicrophoneSize = scale;
			}

			if (g_pWidgetManager)
			{
				if (g_pWidgetManager->GetSlotState(WIDGET_MICROPHONE))
				{
					g_pWidgetManager->GetWidget(WIDGET_MICROPHONE)->SetPos(x, y);
					g_pWidgetManager->GetWidget(WIDGET_MICROPHONE)->SetHeight(scale);
					g_pWidgetManager->GetWidget(WIDGET_MICROPHONE)->SetWidth(scale);
				}
			}
		}

		if (id == 2)
		{
			if (pSettings)
			{
				pSettings->GetWrite().fButtonCameraCycleX = x;
				pSettings->GetWrite().fButtonCameraCycleY = y;
				pSettings->GetWrite().fButtonCameraCycleSize = scale;
			}

			if (g_pWidgetManager)
			{
				if (g_pWidgetManager->GetSlotState(WIDGET_CAMERA_CYCLE))
				{
					g_pWidgetManager->GetWidget(WIDGET_CAMERA_CYCLE)->SetPos(x, y);
					g_pWidgetManager->GetWidget(WIDGET_CAMERA_CYCLE)->SetHeight(scale);
					g_pWidgetManager->GetWidget(WIDGET_CAMERA_CYCLE)->SetWidth(scale);
				}
			}
		}

		if (id == 3)
		{
			if (pSettings)
			{
				pSettings->GetWrite().fButtonPassengerCycleX = x;
				pSettings->GetWrite().fButtonPassengerCycleY = y;
				pSettings->GetWrite().fButtonPassengerCycleSize = scale;
			}

			if (g_pWidgetManager)
			{
				if (g_pWidgetManager->GetSlotState(WIDGET_PASSENGER))
				{
					g_pWidgetManager->GetWidget(WIDGET_PASSENGER)->SetPos(x, y);
					g_pWidgetManager->GetWidget(WIDGET_PASSENGER)->SetHeight(scale);
					g_pWidgetManager->GetWidget(WIDGET_PASSENGER)->SetWidth(scale);
				}
			}
		}
	}

	JNIEXPORT jintArray JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeHudElementPosition(JNIEnv* pEnv, jobject thiz, jint id)
	{
		jintArray color = pEnv->NewIntArray(2);

		if (!color)
		{
			return nullptr;
		}
		int arr[2];

		if (id == 7 && pSettings)
		{
			arr[0] = pSettings->GetReadOnly().fChatPosX;
			arr[1] = pSettings->GetReadOnly().fChatPosY;
		}
		else if (id == HUD_SNOW && pSettings)
		{
			arr[0] = CSnow::GetCurrentSnow();
			arr[1] = CSnow::GetCurrentSnow();
		}
		else
		{
			arr[0] = CAdjustableHudPosition::GetElementPosition((E_HUD_ELEMENT)id).X;
			arr[1] = CAdjustableHudPosition::GetElementPosition((E_HUD_ELEMENT)id).Y;
		}

		pEnv->SetIntArrayRegion(color, 0, 2, (const jint*)&arr[0]);

		return color;
	}

	JNIEXPORT jintArray JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeWidgetPositionAndScale(JNIEnv* pEnv, jobject thiz, jint id)
	{
		jintArray color = pEnv->NewIntArray(3);

		if (!color)
			return nullptr;

		int arr[3] = { -1, -1, -1 };

		if (pSettings)
		{
			if (id == 0)
			{
				arr[0] = pSettings->GetWrite().fButtonMicrophoneX;
				arr[1] = pSettings->GetWrite().fButtonMicrophoneY;
				arr[2] = pSettings->GetWrite().fButtonMicrophoneSize;
			}

			if (id == 1)
			{
				arr[0] = pSettings->GetWrite().fButtonEnterPassengerX;
				arr[1] = pSettings->GetWrite().fButtonEnterPassengerY;
				arr[2] = pSettings->GetWrite().fButtonEnterPassengerSize;
			}

			if (id == 2)
			{
				arr[0] = pSettings->GetWrite().fButtonCameraCycleX;
				arr[1] = pSettings->GetWrite().fButtonCameraCycleY;
				arr[2] = pSettings->GetWrite().fButtonCameraCycleSize;
			}

			if (id == 3)
			{
				arr[0] = pSettings->GetWrite().fButtonPassengerCycleX;
				arr[1] = pSettings->GetWrite().fButtonPassengerCycleY;
				arr[2] = pSettings->GetWrite().fButtonPassengerCycleSize;
			}
		}

		pEnv->SetIntArrayRegion(color, 0, 3, (const jint*)& arr[0]);

		return color;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_switchWeapon(JNIEnv* pEnv, jobject thiz)
	{
		auto pPlayerPed = pGame->FindPlayerPed();
		if (pPlayerPed)
			pPlayerPed->SwitchWeapon();
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_spawnPlayer(JNIEnv* pEnv, jobject thiz, jint spawnId)
	{
		RakNet::BitStream bs;
		bs.Write((uint8_t)ID_CUSTOM_PACKET_SYSTEM);
		bs.Write((uint8_t)0x8588);
		bs.Write((uint8_t)spawnId);

		if (pNetGame)
			pNetGame->GetRakClient()->Send(&bs, HIGH_PRIORITY, RELIABLE, 0);
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onClickPlayInWelcomeWindow(JNIEnv* pEnv, jobject thiz, jint spawnId)
	{
		RakNet::BitStream bs;
		bs.Write((uint8_t)ID_CUSTOM_PACKET_SYSTEM);
		bs.Write((uint8_t)ID_SHOW_AUTH_DIALOG);

		if (pNetGame)
			pNetGame->GetRakClient()->Send(&bs, HIGH_PRIORITY, RELIABLE, 0);
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_refuelTheCar(JNIEnv* pEnv, jobject thiz, jint fuel, jint price)
	{
		RakNet::BitStream bs;
		bs.Write((uint8_t)ID_CUSTOM_PACKET_SYSTEM);
		bs.Write((uint8_t)ID_REFUEL_THE_CAR);
		bs.Write((uint16_t)fuel);
		bs.Write((uint16_t)price);

		if (pNetGame)
			pNetGame->GetRakClient()->Send(&bs, HIGH_PRIORITY, RELIABLE, 0);
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_sendDialogResponse(JNIEnv *env, jobject thiz, jint btnId, jint dialogId, jint listBoxItem, jstring inputtext)
	{
		if (pNetGame)
			pNetGame->SendDialogResponse(dialogId, btnId, listBoxItem, (char*)jstring2string(g_pJavaWrapper->GetEnv(), inputtext).c_str());

		if (g_pJavaWrapper->isGlobalShowHUD)
			g_pJavaWrapper->ShowHUD(true);

		if(pGame)
			pGame->FindPlayerPed()->TogglePlayerControllable(true);

		g_pJavaWrapper->isGlobalShowDialog = false;
	}


	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_sendClick(JNIEnv *env, jobject thiz, jstring inputtext)
	{
		if (pNetGame)
			pNetGame->SendChatCommand((char*)jstring2string(g_pJavaWrapper->GetEnv(), inputtext).c_str());
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_inputHash(JNIEnv *env, jobject thiz, jstring inputtext)
	{
		if (pNetGame)
			pNetGame->SendHash((char*)jstring2string(g_pJavaWrapper->GetEnv(), inputtext).c_str());
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_showTab(JNIEnv *env, jobject thiz)
	{
		// pScoreBoard->Toggle();

		CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
		if (pPlayerPool)
	                  {
			std::vector<PLAYER_SCORE_INFO> players;

		 	PLAYER_SCORE_INFO info;

		 	info.dwID = pPlayerPool->GetLocalPlayerID();
		 	info.szName = pPlayerPool->GetLocalPlayerName();
		 	info.iScore = pPlayerPool->GetLocalPlayerScore();
		 	info.dwPing = pPlayerPool->GetLocalPlayerPing();

		 	players.push_back(info);

		 	for (PLAYERID x = 0; x < MAX_PLAYERS; x++)
		 	{
		 		if (!pPlayerPool->GetSlotState(x))
					continue;
	
		 		info.dwID = x;
		 		info.szName = pPlayerPool->GetPlayerName(x);
		 		info.iScore = pPlayerPool->GetRemotePlayerScore(x);
		 		info.dwPing = pPlayerPool->GetRemotePlayerPing(x);

		 		players.push_back(info);
		 	}

			for (int j = 0; j < players.size(); j++)
			{
				g_pJavaWrapper->SetTabStat(players.at(j).dwID, players.at(j).szName, players.at(j).iScore, players.at(j).dwPing);
		 	}

			g_pJavaWrapper->ShowTab(true);
		 }
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_hideCaptcha(JNIEnv *env, jobject thiz)
	{
		if (g_pJavaWrapper->isGlobalShowHUD)
			g_pJavaWrapper->ShowHUD(true);

		if(pGame)
			pGame->FindPlayerPed()->TogglePlayerControllable(true);

		g_pJavaWrapper->isGlobalShowCaptcha = false;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_applyQuest(JNIEnv *env, jobject thiz, jint questId)
	{
		RakNet::BitStream bs;
		bs.Write((uint8_t)ID_CUSTOM_PACKET_SYSTEM);
		bs.Write((uint8_t)ID_APPLY_QUEST);
		bs.Write((uint16_t)questId);

		if (pNetGame)
			pNetGame->GetRakClient()->Send(&bs, HIGH_PRIORITY, RELIABLE, 0);
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onTabClose(JNIEnv *env, jobject thiz, jint questId)
	{
		g_pJavaWrapper->isGlobalShowTab = false;

		pGame->ToggleHUDElement(HUD_ELEMENT_CHAT, true);
		pGame->ToggleHUDElement(HUD_ELEMENT_BUTTONS, true);
		pGame->ToggleHUDElement(HUD_ELEMENT_MAP, true);
	}
}

void CJavaWrapper::SetLoadingText(int id)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(this->activity, this->s_SetLoadingText, id);
	EXCEPTION_CHECK(env);
}

CJavaWrapper::CJavaWrapper(JNIEnv* env, jobject activity)
{
	this->env = env;
	this->activity = env->NewGlobalRef(activity);

	jclass nvEventClass = env->GetObjectClass(activity);
	if (!nvEventClass)
	{
		Log("nvEventClass null");
		return;
	}

	s_CallLauncherActivity = env->GetMethodID(nvEventClass, "callLauncherActivity", "(I)V");
	s_CallLauncherActivityWithSpecialValue = env->GetMethodID(nvEventClass, "callLauncherActivityWithSpecialValue", "(II)V");
	s_IsSomethingUIShitOnScreenActive = env->GetMethodID(nvEventClass, "isSomethingUIShitOnScreenActive", "()Z");

	s_setCashMoney = env->GetMethodID(nvEventClass, "setCashMoney", "(Ljava/lang/String;)V");
	s_GetClipboardText = env->GetMethodID(nvEventClass, "getClipboardText", "()[B");
 
	s_ShowInputLayout = env->GetMethodID(nvEventClass, "showInputLayout", "()V");
	s_HideInputLayout = env->GetMethodID(nvEventClass, "hideInputLayout", "()V");

	s_ShowClientSettings = env->GetMethodID(nvEventClass, "showClientSettings", "()V");
	s_SetUseFullScreen = env->GetMethodID(nvEventClass, "setUseFullscreen", "(I)V");

	s_SetHudLogo = env->GetMethodID(nvEventClass, "setHudLogo", "(I)V");
	s_SetHudSatiety= env->GetMethodID(nvEventClass, "setHudSatiety", "(I)V");
	s_SetSpeedMenu = env->GetMethodID(nvEventClass, "setSpeedMenu", "(II)V");

	// -- Speedometer
	s_ShowSpeedometer = env->GetMethodID(nvEventClass, "showSpeedometer", "()V");
	s_HideSpeedometer = env->GetMethodID(nvEventClass, "hideSpeedometer", "()V");

	s_SetSpeedometerSpeed = env->GetMethodID(nvEventClass, "setSpeedometerSpeed", "(I)V");
	s_SetSpeedometerMileage = env->GetMethodID(nvEventClass, "setSpeedometerMileage", "(I)V");
	s_SetSpeedometerFuel = env->GetMethodID(nvEventClass, "setSpeedometerFuel", "(I)V");
	s_SetSpeedometerCarHP = env->GetMethodID(nvEventClass, "setSpeedometerCarHP", "(I)V");

	s_SetSpeedometerEngine = env->GetMethodID(nvEventClass, "setEngineState", "(I)V");
	s_SetSpeedometerLight = env->GetMethodID(nvEventClass, "setLightState", "(I)V");
	s_SetSpeedometerBelt = env->GetMethodID(nvEventClass, "setBeltState", "(I)V");
	s_SetSpeedometerLock = env->GetMethodID(nvEventClass, "setLockState", "(I)V");

	// -- HUD
	s_ShowHUD = env->GetMethodID(nvEventClass, "showHud", "(Z)V");
	s_HideHUD = env->GetMethodID(nvEventClass, "hideHud", "(Z)V");

	s_SetMoney = env->GetMethodID(nvEventClass, "setMoney", "(I)V");

	s_SetHP = env->GetMethodID(nvEventClass, "setHP", "(I)V");
	s_SetArmour = env->GetMethodID(nvEventClass, "setArmour", "(I)V");
	s_SetEat = env->GetMethodID(nvEventClass, "setEat", "(I)V");

	s_SetAmmo1 = env->GetMethodID(nvEventClass, "setAmmo1", "(I)V");
	s_SetAmmo2 = env->GetMethodID(nvEventClass, "setAmmo2", "(I)V");

	s_SetWantedLevel = env->GetMethodID(nvEventClass, "setWantedLevel", "(I)V");

	s_SetOnline = env->GetMethodID(nvEventClass, "setOnline", "(I)V");

	s_SetX2 = env->GetMethodID(nvEventClass, "setX2", "(Z)V");

	s_UpdateTime = env->GetMethodID(nvEventClass, "updateTime", "()V");
	s_UpdateHudIcon = env->GetMethodID(nvEventClass, "updateHudIcon", "(I)V");

	s_UpdateHud = env->GetMethodID(nvEventClass, "updateHud", "()V");

	// -- Loading
	s_SetLoadingText = env->GetMethodID(nvEventClass, "setLoadingText", "(I)V");

	// -- Welcome
	s_ShowWelcome = env->GetMethodID(nvEventClass, "showWelcome", "(Z)V");
	s_HideWelcome = env->GetMethodID(nvEventClass, "hideWelcome", "()V");

	// -- SpawnSelector
	s_ShowSpawnSelector = env->GetMethodID(nvEventClass, "showSpawnSelector", "(ZZ)V");

	// -- AZS
	s_ShowAZS = env->GetMethodID(nvEventClass, "showAZS", "(IIIII)V");

	// -- Captcha
	s_ShowCaptcha = env->GetMethodID(nvEventClass, "showCaptcha", "()V");

	// -- Quests
	s_ShowQuests = env->GetMethodID(nvEventClass, "showQuests", "()V");
	s_SetQuestsStats = env->GetMethodID(nvEventClass, "setQuestInfo", "(II)V");

	// -- Crash fix
	s_FinishActivity = env->GetMethodID(nvEventClass, "finishGameActivity", "()V");

	// -- Tab
	s_ShowTab = env->GetMethodID(nvEventClass, "showTabWindow", "(Z)V");
	s_HideTab = env->GetMethodID(nvEventClass, "hideTabWindow", "()V");
	s_ClearTabStat = env->GetMethodID(nvEventClass, "clearTabStat", "()V");
	s_SetTabStat = env->GetMethodID(nvEventClass, "setTabStat", "(I[BII)V");

	// -- Dialog
	s_ShowDialog = env->GetMethodID(nvEventClass, "showDialog", "(II[B[B[B[B)V");
	s_ShowDialogWithOldContent = env->GetMethodID(nvEventClass, "showDialogWithOldContent", "()V");
	s_HideDialogWithoutReset = env->GetMethodID(nvEventClass, "hideDialogWithoutReset", "()V");

	s_ShowCode = env->GetMethodID(nvEventClass, "showCode", "(ILjava/lang/String;)V");

                  s_ShowSpeedMenu = env->GetMethodID(nvEventClass, "showSpeedMenu", "()V");

	env->DeleteLocalRef(nvEventClass);

	isGlobalShowSpeedometer = false;
	isLocalShowSpeedometer = false;

	isGlobalShowHUD = false;
	isLocalShowHUD = false;

	isGlobalShowDialog = false;
	isGlobalShowCaptcha = false;

	isGlobalShowTab = false;
}

CJavaWrapper::~CJavaWrapper()
{
	JNIEnv* pEnv = GetEnv();
	if (pEnv)
	{
		pEnv->DeleteGlobalRef(this->activity);
	}
}

void CJavaWrapper::ShowSpeedMenu()
{
	if (pKeyBoard->IsOpen())
	 	return;

	JNIEnv* env = GetEnv();

	if (!env)
	{
	 	Log("No env");
	 	return;
	}

                  env->CallVoidMethod(activity, s_ShowSpeedMenu);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ShowSpeedometer()
{
	if (isLocalShowSpeedometer || pKeyBoard->IsOpen())
	 	return;

	JNIEnv* env = GetEnv();

	if (!env)
	{
	 	Log("No env");
	 	return;
	}

                  env->CallVoidMethod(activity, s_ShowSpeedometer);
	isLocalShowSpeedometer = true;

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::HideSpeedometer()
{
	if (!isLocalShowSpeedometer)
		return;

	JNIEnv* env = GetEnv();

	if (!env)
	{
	 	Log("No env");
	 	return;
	}

	env->CallVoidMethod(activity, s_HideSpeedometer);
	isLocalShowSpeedometer = false;

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetSpeedometerSpeed(int speed)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
	 	Log("No env");
	 	return;
	}

	env->CallVoidMethod(activity, s_SetSpeedometerSpeed, speed);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetSpeedometerMileage(int mileage)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
	 	return;
                  }

	env->CallVoidMethod(activity, s_SetSpeedometerMileage, mileage);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetSpeedometerFuel(int fuel)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
	 	Log("No env");
	 	return;
	}

	env->CallVoidMethod(activity, s_SetSpeedometerFuel, fuel);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetSpeedometerCarHP(int hp)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
	 	Log("No env");
	 	return;
	}

	env->CallVoidMethod(activity, s_SetSpeedometerCarHP, hp / 10);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetEngineState(int state)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
	 	Log("No env");
	 	return;
	}

	env->CallVoidMethod(activity, s_SetSpeedometerEngine, state);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetLightState(int state)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
	 	Log("No env");
	 	return;
	}

	env->CallVoidMethod(activity, s_SetSpeedometerLight, state);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetBeltState(int state)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
	 	Log("No env");
	 	return;
	}

	env->CallVoidMethod(activity, s_SetSpeedometerBelt, state);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetLockState(int state)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
	 	Log("No env");
	 	return;
	}

	env->CallVoidMethod(activity, s_SetSpeedometerLock, state);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ShowHUD(bool isAnimation)
{
	if (!pSettings->GetReadOnly().iNewHud)
		return;

	if (isLocalShowHUD)
		return;

	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_ShowHUD, isAnimation);
	pGame->ToggleHUDElement(HUD_ELEMENT_HUD, true);
	isLocalShowHUD = true;

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::HideHUD(bool isAnimation)
{
	if (!isLocalShowHUD)
		return;

	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_HideHUD, isAnimation);
	isLocalShowHUD = false;

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetMoney(int money)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetMoney, money);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetHP(int value)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetHP, value);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetArmour(int value)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetArmour, value);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetEat(int value)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetEat, value);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetAmmo(int ammo1, int ammo2)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetAmmo1, ammo1);
	env->CallVoidMethod(activity, s_SetAmmo2, ammo2);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetWantedLevel(int level)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetWantedLevel, level);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetOnline(int online)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetOnline, online);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::UpdateTime()
{
                  // -- ПЕРЕНЕСЕНО В ДЖАВУ
}

void CJavaWrapper::UpdateHudIcon(int gunId)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_UpdateHudIcon, gunId);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::UpdateHud()
{
                   // MOVED TO JAVA && HOOKS
}

void CJavaWrapper::ShowWelcome(bool isRegister)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_ShowWelcome, isRegister);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::HideWelcome()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_HideWelcome);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ShowSpawnSelector(bool isHouse, bool isFraction)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_ShowSpawnSelector, isHouse, isFraction);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ShowAZS(int fuelId, int maxFuel, int currentFuel, int price, int balance)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_ShowAZS, fuelId, maxFuel, currentFuel, price, balance);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ShowDialog(int dialogId, int dialogTypeId, char* caption, char* content, char* leftBtnText, char* rightBtnText)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	if(pGame)
		pGame->FindPlayerPed()->TogglePlayerControllable(false);

	g_pJavaWrapper->isGlobalShowDialog = true;

    	jbyteArray bytes = env->NewByteArray(strlen(caption));
    	env->SetByteArrayRegion(bytes, 0, strlen(caption), (jbyte*)caption);
  
    	jbyteArray bytes1 = env->NewByteArray(strlen(content));
    	env->SetByteArrayRegion(bytes1, 0, strlen(content), (jbyte*)content);
    	
    	jbyteArray bytes2 = env->NewByteArray(strlen(leftBtnText));
    	env->SetByteArrayRegion(bytes2, 0, strlen(leftBtnText), (jbyte*)leftBtnText);
    	
    	jbyteArray bytes3 = env->NewByteArray(strlen(rightBtnText));
	env->SetByteArrayRegion(bytes3, 0, strlen(rightBtnText), (jbyte*)rightBtnText);

	env->CallVoidMethod(activity, s_ShowDialog, dialogId, dialogTypeId, bytes, bytes1, bytes2, bytes3);
	env->DeleteLocalRef(bytes);
	env->DeleteLocalRef(bytes1);
	env->DeleteLocalRef(bytes2);
	env->DeleteLocalRef(bytes3);
	EXCEPTION_CHECK(env);
}


void CJavaWrapper::SendCode(int id)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
	int idd = pNetGame->GetPlayerPool()->GetLocalPlayerID();
	jstring jCaptionStr = (*g_pJavaWrapper->GetEnv()).NewStringUTF(pNetGame->GetPlayerPool()->GetPlayerName(idd));
	env->CallVoidMethod(activity, s_ShowCode, idd, jCaptionStr);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ShowDialogWithOldContent()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_ShowDialogWithOldContent);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::HideDialogWithoutResetContent()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_HideDialogWithoutReset);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetX2(bool state)
{
                  // MOVED TO JAVA 
}

void CJavaWrapper::ShowCaptcha()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_ShowCaptcha);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::FinishActivity()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_FinishActivity);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ShowQuests()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_ShowQuests);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetQuestsStats(int id, int progress)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_SetQuestsStats, id, progress);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetTabStat(int id, char* names, int score, int pings) {
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

                  jbyteArray bytes = env->NewByteArray(strlen(names));
    	env->SetByteArrayRegion(bytes, 0, strlen(names), (jbyte*)names);
	
	env->CallVoidMethod(activity, s_SetTabStat, id, bytes, score, pings);
	env->DeleteLocalRef(bytes);
	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ShowTab(bool isAnim) {
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	g_pJavaWrapper->isGlobalShowTab = true;

	pGame->ToggleHUDElement(HUD_ELEMENT_CHAT, false);
	pGame->ToggleHUDElement(HUD_ELEMENT_BUTTONS, false);
	pGame->ToggleHUDElement(HUD_ELEMENT_MAP, false);

	env->CallVoidMethod(activity, s_ShowTab, isAnim);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::HideTab() {
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_HideTab);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ClearTabStat() {
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_ClearTabStat);

	EXCEPTION_CHECK(env);
}

CJavaWrapper* g_pJavaWrapper = nullptr;