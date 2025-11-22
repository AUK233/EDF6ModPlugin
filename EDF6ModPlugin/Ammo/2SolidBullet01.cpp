#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"
#include "2SolidBullet01.h"

extern "C" {
	void __fastcall ASMammo_SolidBullet01HitFX();
	uintptr_t ammo_SolidBullet01HitFXRetAddr;
}

void __fastcall AmmoClass_HookFunction_SolidBullet01(PBYTE hmodDLL)
{
	// edf.dll+28B420, is vft+28's function
	// try to make penetrating bullet disappear when it hits a debris
	int i_ammo_SolidBullet01HitFX = 0x28B420;
	hookGameBlockWithInt3((void*)(hmodDLL + i_ammo_SolidBullet01HitFX), (uintptr_t)ASMammo_SolidBullet01HitFX);
	ammo_SolidBullet01HitFXRetAddr = (uintptr_t)(hmodDLL + i_ammo_SolidBullet01HitFX + 15);
}
