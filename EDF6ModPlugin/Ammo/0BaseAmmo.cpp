#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"
#include "0BaseAmmo.h"
// subfunction
#include "1CommonBullet.h"
#include "2SolidBullet01.h"

void __fastcall AmmoClass_HookFunction(PBYTE hmodDLL)
{
	AmmoClass_HookFunction_CommonBullet(hmodDLL);
	AmmoClass_HookFunction_SolidBullet01(hmodDLL);
}
