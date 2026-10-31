#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "Insect/GiantAntEgg.h"

#include "0BaseEnemy.h"

void __fastcall HookFunction_EnemySeries(PBYTE hmodDLL) {
	Enemy_HookFunction_GiantAntEgg(hmodDLL);
}
