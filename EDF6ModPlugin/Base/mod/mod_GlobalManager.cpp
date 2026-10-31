#include "pch.h"
#include "utiliy.h"
#include "commonNOP.h"

#include "mod_GlobalManager.h"

PMODGlobalManager g_MODGlobalManager = 0;

void __fastcall MODGlobalManager_Initialize() {
	auto p = (PMODGlobalManager)_aligned_malloc(sizeof(MODGlobalManager_t), 16);
	if (p) {
		p->InitLock();
		p->Reset();
		g_MODGlobalManager = p;
	}
}

void* __fastcall MODGlobalManager_CheckEggBreak(PXGS_System pSys, void* pData) {
	auto p = g_MODGlobalManager;
	if (!p) return pData;

	UINT time = pSys->m_gameTime;

	AcquireSRWLockExclusive(&p->m_eggBreakLock);

	if (p->m_eggBreakTime != time) {
		p->m_eggBreakTime = time;
		p->m_eggBreakCount = 1;
		ReleaseSRWLockExclusive(&p->m_eggBreakLock);
		return pData;
	} else {
		auto count = ++p->m_eggBreakCount;
		ReleaseSRWLockExclusive(&p->m_eggBreakLock);
		if (count > 30) {
			return nullptr;
		}
		return pData;
	}

	/*
	LONG time = pSys->m_gameTime;
	if (p->m_eggBreakTime != time) {
		InterlockedExchange(&p->m_eggBreakTime, time);
		InterlockedExchange(&p->m_eggBreakCount, 1);
		return pData;
	} else {
		auto count = InterlockedIncrement(&p->m_eggBreakCount);
		if (count > 10) {
			return nullptr;
		} else {
			return pData;
		}
	}*/
	// end
}

void MODGlobalManager_t::InitLock() {
	InitializeSRWLock(&m_eggBreakLock);
}

void MODGlobalManager_t::Reset() {
	//ZeroMemory(this, sizeof(MODGlobalManager_t));
	m_eggBreakTime = 0;
	m_eggBreakCount = 0;
}
