#pragma once

typedef struct EDFWString_t {
	wchar_t* text;
	void* pad8;
	size_t size;
	size_t capacity;
} *PEDFWString;

//
typedef struct EDF20B2988_t {
	void* vtable;
	BYTE pad0[0xE00];
	// this is number of boxes currently picked up.
	int AcquiredWeaponBox, AcquiredArmorBox;
	// this is number of boxes inherited from failed missions.
	int ExtraWeaponBox, ExtraArmorBox;
	BYTE padE20[0x40];
	// this is max number of boxes that can be obtained.
	int TotalWeaponBox, TotalArmorBox;
}*Pedf20B2988;
static_assert(offsetof(EDF20B2988_t, ExtraArmorBox) == 0xE14);
static_assert(offsetof(EDF20B2988_t, TotalArmorBox) == 0xE5C);
