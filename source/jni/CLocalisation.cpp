#include "CLocalisation.h"

#include "main.h"
#include "gui/gui.h"
#include "game/game.h"
#include "net/netgame.h"
#include "game/RW/RenderWare.h"
#include "chatwindow.h"
#include "playertags.h"
#include "dialog.h"
#include "keyboard.h"
#include "CSettings.h"
#include "util/armhook.h"

extern CGUI* pGUI;
extern CChatWindow* pChatWindow;

char CLocalisation::m_szMessages[E_MSG::MSG_COUNT][MAX_LOCALISATION_LENGTH] = {
#ifdef _RULANG
		"Подключение к серверу...",
		"Вход в игру",
		"Сервер закрыл соединение, перезайдите",
		"Некоторые файлы были модифированы, переустановите клиент",
		"unused",
		"Вы были заблокированы сервером",
		"Потеряно соеденение с сервером, переподключение...",
		"Проблемы с сетью, переподключение...",
		"Сервер полон"
#else
		"Підключення до сервера...",
		"Вхід в гру",
		"Сервер закрив з'єднання, перезайдіть",
		"Деякі файли були модифіковані, перевстановіть клієнт",
		"unused",
		"Вас заблокував сервер",
		"Втрачено з'єднання з сервером, перепідключення...",
		"Проблеми з мережею, перепідключення...",
		"Сервер повний"
#endif
};

void CLocalisation::Initialise(const char* szFile)
{
#ifdef _RULANG
                  Log("Selected lang: RU");
#else
                  Log("Selected lang: UA");
#endif

	char buff[MAX_LOCALISATION_LENGTH];
	sprintf(&buff[0], "%sSAMP/%s", g_pszStorage, szFile);

	FILE* pFile = fopen(&buff[0], "r");
	if (!pFile)
	{
		return;
	}

	for (int i = 0; i < E_MSG::MSG_COUNT; i++)
	{
		memset(m_szMessages[i], 0, MAX_LOCALISATION_LENGTH);
	}
	uint32_t counter = 0;
	while (fgets(&buff[0], MAX_LOCALISATION_LENGTH, pFile) != NULL)
	{
		if (counter == E_MSG::MSG_COUNT)
		{
			break;
		}

		memcpy((void*)& m_szMessages[counter][0], (const void*)(&buff[0]), MAX_LOCALISATION_LENGTH);
		counter++;
	}
	fclose(pFile);
}

char* CLocalisation::GetMsg(E_MSG msg)
{
	return &m_szMessages[msg][0];
}
