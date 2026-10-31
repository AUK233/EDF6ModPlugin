#pragma once
#include "../SSE.hpp"
#include "Base/g_system.h"


class MODGlobalManager_t {
public:
	SRWLOCK m_eggBreakLock; UINT m_eggBreakTime, m_eggBreakCount;

	void InitLock();
	void Reset();
};
typedef MODGlobalManager_t* PMODGlobalManager;

void __fastcall MODGlobalManager_Initialize();

extern "C" {
	void* __fastcall MODGlobalManager_CheckEggBreak(PXGS_System pSys, void* pData);
}
