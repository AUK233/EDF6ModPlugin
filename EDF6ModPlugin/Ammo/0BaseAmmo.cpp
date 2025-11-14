#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"
#include "0BaseAmmo.h"
// subfunction
#include "1CommonBullet.h"

void __fastcall AmmoClass_HookFunction(PBYTE hmodDLL)
{
	AmmoClass_HookFunction_CommonBullet(hmodDLL);
}
