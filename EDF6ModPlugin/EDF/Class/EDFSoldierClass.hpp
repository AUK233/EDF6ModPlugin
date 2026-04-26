#pragma once

// ==================================================
// Fencer
typedef struct G_HeavyArmor_t {
	char pad0000[0x20F0];
	// new
	int IsDashToBoost, IsBoostToDash, BoostRecoveryBaseTime, DashRecoveryBaseTime;
	void* pSubWeapon; int SubWeaponButtonTimer, SubWeaponButtonCD;
} *PG_HeavyArmor;
#if 1
static_assert(offsetof(G_HeavyArmor_t, IsDashToBoost) == 0x20F0);
static_assert(offsetof(G_HeavyArmor_t, pSubWeapon) == 0x2100);
static_assert(offsetof(G_HeavyArmor_t, SubWeaponButtonTimer) == 0x2108);
#endif
// ==================================================
