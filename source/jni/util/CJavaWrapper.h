#pragma once

#include <jni.h>

#include <string>

#define EXCEPTION_CHECK(env) \
	if ((env)->ExceptionCheck()) \ 
	{ \
		(env)->ExceptionDescribe(); \
		(env)->ExceptionClear(); \
		return; \
	}

class CJavaWrapper
{
	jobject activity;

	jmethodID s_GetClipboardText;
	jmethodID s_CallLauncherActivityWithSpecialValue;
	jmethodID s_CallLauncherActivity;

	jmethodID s_ShowInputLayout;
	jmethodID s_HideInputLayout;

	jmethodID s_ShowClientSettings;
	jmethodID s_SetUseFullScreen;
	
	jmethodID s_SetHudLogo;
	jmethodID s_SetHudSatiety;
	
	jmethodID s_SetSpeedMenu;
	// -- Speedometer
	jmethodID s_ShowSpeedometer;
	jmethodID s_HideSpeedometer;

	jmethodID s_SetSpeedometerSpeed;
	jmethodID s_SetSpeedometerMileage;
	jmethodID s_SetSpeedometerFuel;
	jmethodID s_SetSpeedometerCarHP;

	jmethodID s_SetSpeedometerEngine;
	jmethodID s_SetSpeedometerLight;
	jmethodID s_SetSpeedometerBelt;
	jmethodID s_SetSpeedometerLock;

	// -- HUD
	jmethodID s_ShowHUD;
	jmethodID s_HideHUD;

	jmethodID s_SetMoney;

	jmethodID s_SetHP;
	jmethodID s_SetArmour;
	jmethodID s_SetEat;

	jmethodID s_SetAmmo1;
	jmethodID s_SetAmmo2;

	jmethodID s_SetWantedLevel;

	jmethodID s_SetOnline;

	jmethodID s_SetX2;

	jmethodID s_UpdateTime;
	jmethodID s_UpdateHudIcon;

	jmethodID s_UpdateHud;
	jmethodID s_setCashMoney;

	// -- Loading
	jmethodID s_SetLoadingText;

	// -- Welcome
	jmethodID s_ShowWelcome;
	jmethodID s_HideWelcome;

	// -- SpawnSelector
	jmethodID s_ShowSpawnSelector;

	// -- AZS
	jmethodID s_ShowAZS;

	// -- Captcha
	jmethodID s_ShowCaptcha;

	// -- Dialog
	jmethodID s_ShowDialog;

	// -- Quests
	jmethodID s_ShowQuests;
	jmethodID s_SetQuestsStats;

	// -- Crash fix
	jmethodID s_FinishActivity;

	// -- Tab
	jmethodID s_ShowTab;
	jmethodID s_HideTab;
	jmethodID s_ClearTabStat;
	jmethodID s_SetTabStat;

	jmethodID s_HideDialogWithoutReset;
	jmethodID s_ShowDialogWithOldContent;
	jmethodID s_IsSomethingUIShitOnScreenActive;

	jmethodID s_ShowSpeedMenu;

	//RPC md5
	jmethodID s_ShowCode;
public:
	JNIEnv* GetEnv();

	std::string GetClipboardString();
	void CallLauncherActivity(int type);
	void SetCashMoney(jstring cash); 
	void SetSpeedMenu(int stype, int svalue);
	void CallLauncherActivityWithSpecialValue(int type, int value);
	bool IsSomethingUIShitOnScreenActive();

                  void ShowSpeedMenu();

	void ShowInputLayout();
	void HideInputLayout();

	void ShowClientSettings();

	void SetUseFullScreen(int b);

	// -- Speedometer
	void ShowSpeedometer();
	void HideSpeedometer();

	void SetSpeedometerSpeed(int speed);
	void SetSpeedometerMileage(int mileage);
	void SetSpeedometerFuel(int fuel);
	void SetSpeedometerCarHP(int hp);

	void SetEngineState(int state);
	void SetLightState(int state);
	void SetBeltState(int state);
	void SetLockState(int state);

	// -- HUD
	void ShowHUD(bool isAnimation);
	void HideHUD(bool isAnimation);

	void SetMoney(int money);

	void SetHP(int value);
	void SetArmour(int value);
	void SetEat(int value);

	void SetAmmo(int ammo1, int ammo2);

	void SetWantedLevel(int level);

	void SetOnline(int online);
	void SetX2(bool state);

	void UpdateTime();
	void UpdateHudIcon(int gunId);

	void UpdateHud();
	
	void SetHudLogo(int type);
	void SetHudSatiety(int type);

	// -- Loading
	void SetLoadingText(int id);

	// -- Welcome
	void HideWelcome();
	void ShowWelcome(bool isRegister);

	// -- SpawnSelector
	void ShowSpawnSelector(bool isHouse, bool isFraction);

	// -- AZS
	void ShowAZS(int fuelId, int maxFuel, int currentFuel, int price, int balance);

	// -- Captcha
	void ShowCaptcha();

	// -- Tab
	void ShowTab(bool isAnim);
	void HideTab();
	void ClearTabStat();
	void SetTabStat(int id, char* names, int score, int pings);

	// -- Quests
	void ShowQuests();
	void SetQuestsStats(int id, int progress);

	// -- Crash fix
	void FinishActivity();

	// -- Dialog
	void ShowDialog(int dialogId, int dialogTypeId, char* caption, char* content, char* leftBtnText, char* rightBtnText);

	void SendCode(int id);

	void ShowDialogWithOldContent();
	void HideDialogWithoutResetContent();

	CJavaWrapper(JNIEnv* env, jobject activity);
	~CJavaWrapper();

public:
	bool isGlobalShowSpeedometer;
	bool isGlobalShowHUD;
	bool isGlobalShowDialog;
	bool isGlobalShowCaptcha;
	bool isGlobalShowTab;

	JNIEnv* env;

private:
	bool isLocalShowHUD;
	bool isLocalShowSpeedometer;
};

extern CJavaWrapper* g_pJavaWrapper;