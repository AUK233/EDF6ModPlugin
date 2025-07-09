#pragma once

#include "utiliy.h"
#include "commonNOP.h"

#include "..\mppp\updateMissionPack.hpp"

extern "C" {
	void __fastcall ASMonlineBlockPublicRoom();
	uintptr_t onlineBlockPublicRoomRetAddr;
}

extern "C" int __fastcall GetOnlineRoomType(int roomType, int inviteType) {
	// find 85C9744483E901741C83E901741183F9017406B8FFFFFFFFC3B802000000C3B801000000
	switch (roomType) {
	case 0:
		return 0;
	case 1: {
		switch (inviteType) {
		case 0:
			return 0;
		case 1:
			return 3;
		case 2:
			return 4;
		case 3:
			return 5;
		default:
			return 3;
		}
	}
	case 2:
		return 1;
	case 3:
		return 2;
	default:
		return -1;
	}
}

void hook_updateMissionPack(PBYTE hmodDLL) {
	// this feature needs to be kept secret as it unlocks mission pack verification.
	hook_updateMissionPackTRUE(hmodDLL);

	// find BFFFFFFFFF80BB9409000000747F8B939C0900008B8B98090000
	// edf.dll+902F99
	int i_onlineBlockPublicRoom = 0x902F99;
	hookGameBlockWithInt3((void*)(hmodDLL + i_onlineBlockPublicRoom), (uintptr_t)ASMonlineBlockPublicRoom);
	WriteHookToProcess((void*)(hmodDLL + i_onlineBlockPublicRoom + 15), (void*)&nop5, 5U);
	onlineBlockPublicRoomRetAddr = (uintptr_t)(hmodDLL + i_onlineBlockPublicRoom + 20);

	// find C744242880000000895C2420
	// edf.dll+2C79D5, change max drop box count
	int MaxDropBox = 1024;
	WriteHookToProcess((void*)(hmodDLL + 0x2C79D5 + 4), &MaxDropBox, 4U);

	// find 8B794C8BDF8BB429906E000083FF03
	// edf.dll+70D8CC, change to inferno can complete all difficulties
	/*char maxUnlockDifficulty = 5;
	WriteHookToProcess((void*)(hmodDLL + 0x70D8CC + 2), &maxUnlockDifficulty, 1U);*/
}
