#include "AUTOMOBILE_COLLISION_result.h"
#include "FIRST_PROTECT_result.h"
#include "protect_common.h"

#include <stdint.h>
#include "../util/armhook.h"
#include <string>
#include <sys/mman.h>
#include "../main.h"
#include "game/RW/common.h"

extern "C" void sub_6758 (uintptr_t pc)
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

	for (uintptr_t start = pc; start != pc + g_SizeStart_AUTOMOBILE_COLLISION; start++)
	{
		if (!memcmp((void*)start, (void*)g_Start_AUTOMOBILE_COLLISION, g_SizeStart_AUTOMOBILE_COLLISION))
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

	addr += g_SizeStart_AUTOMOBILE_COLLISION;

	makeNOP(addr, g_SizeCode_AUTOMOBILE_COLLISION / 2);

	FuckCode(pc);
	FuckCode(addr);
}