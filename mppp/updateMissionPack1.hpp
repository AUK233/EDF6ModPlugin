#pragma once

static void __fastcall WriteHookToProcess(void* addr, void* data, size_t len) {
	DWORD oldProtect;
	VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(addr, data, len);
	VirtualProtect(addr, len, oldProtect, &oldProtect);
}

static void __fastcall hookGameBlockWithInt3(void* targetAddr, uintptr_t dataAddr) {
	BYTE hookFunction[] = {
		0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,				// jmp
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // addr
		0xCC											// int3
	};
	memcpy(&hookFunction[6], &dataAddr, sizeof(dataAddr));

	WriteHookToProcess(targetAddr, hookFunction, sizeof(hookFunction));
}

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
	static const BYTE vb_nop5[] = { 0x0F, 0x1F, 0x44, 0x00, 0x00 };
	// find BFFFFFFFFF80BB9409000000747F8B939C0900008B8B98090000
	// edf.dll+902F99
	int i_onlineBlockPublicRoom = 0x902F99;
	hookGameBlockWithInt3((void*)(hmodDLL + i_onlineBlockPublicRoom), (uintptr_t)ASMonlineBlockPublicRoom);
	WriteHookToProcess((void*)(hmodDLL + i_onlineBlockPublicRoom + 15), (void*)&vb_nop5, 5U);
	onlineBlockPublicRoomRetAddr = (uintptr_t)(hmodDLL + i_onlineBlockPublicRoom + 20);
}
