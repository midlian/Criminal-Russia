#include "GAME_RESOURCES5_result.h"
#include "FIRST_PROTECT_result.h"
#include "protect_common.h"

#include <stdint.h>
#include "../util/armhook.h"
#include <string>
#include <sys/mman.h>
#include "../main.h"
#include "game/RW/common.h"

extern "C" void sub_2639 (uintptr_t pc)
{
	static bool once = false;
	if (once)
	{
		return;
	}
	once = true;

	std::lock_guard<std::mutex> lock(g_MiscProtectMutex);

	#ifdef USE_SANTROPE_BULLSHIT
		PROTECT_CODE_FIRST_PROTECT;
	#endif

	unProtect(pc);

	uintptr_t addr = 0;

	for (uintptr_t start = pc; start != pc + g_SizeStart_GAME_RESOURCES5; start++)
	{
		if (!memcmp((void*)start, (void*)g_Start_GAME_RESOURCES5, g_SizeStart_GAME_RESOURCES5))
		{
			addr = start;
			break;
		}
	}

	if (!addr)
	{
		FuckCode(pc);
		return;
	}

	addr += g_SizeStart_GAME_RESOURCES5;

	makeNOP(addr, g_SizeCode_GAME_RESOURCES5 / 2);

	FuckCode(pc);
	FuckCode(addr);
}