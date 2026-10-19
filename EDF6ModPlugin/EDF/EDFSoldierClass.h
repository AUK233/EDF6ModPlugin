#pragma once
#include "Base/base_baseObject.h"

// edf.dll+56A5C0
typedef struct XGS_HumanBase_t : XGS_GameObjectBase_t {
	char pad5C0[0x20];
	G_Matrix3D_t m_human_transform; // copy from m_transform
	char pad620[0xBA4];
	float m_baseMoveSpeed, m_baseJumpHeight, pad11CC;
	G_Vector4_t m_inputControl; // +x is left, +z is front, [-1,1]
	G_Vector4_t m_relativeMotionVelocity; // yeah, without direction
	G_Vector4_t pad11F0;
	G_Vector4_t m_motionVelocity1200, m_motionVelocity1210;
	float m_velocityFactor1220; char pad1224[0xC];
	G_Vector4_t m_orientation1230, m_orientation1240; // radian, +x is upward, y needs to be inverted
	char pad1250[0x6F0];
} *P_XGS_HumanBase;
#if 1
static_assert(offsetof(XGS_HumanBase_t, m_human_transform) == 0x5E0);
static_assert(offsetof(XGS_HumanBase_t, m_baseMoveSpeed) == 0x11C4);
static_assert(offsetof(XGS_HumanBase_t, m_inputControl) == 0x11D0);
static_assert(offsetof(XGS_HumanBase_t, m_relativeMotionVelocity) == 0x11E0);
static_assert(offsetof(XGS_HumanBase_t, m_motionVelocity1200) == 0x1200);
static_assert(offsetof(XGS_HumanBase_t, m_orientation1240) == 0x1240);
static_assert(sizeof(XGS_HumanBase_t) == 0x1940);
#endif

// edf.dll+58D020
typedef struct XGS_SoldierBase_t : XGS_HumanBase_t {
	void* vft_SoldierBase;
	char pad1948[0x5A8];
} *P_XGS_SoldierBase;
#if 1
static_assert(sizeof(XGS_SoldierBase_t) == 0x1EF0);
#endif

typedef struct XGS_HumanBase_ActionRDX_t {
	uintptr_t pFunc;
	int state;
} *P_XGS_HumanBase_ActionRDX;
