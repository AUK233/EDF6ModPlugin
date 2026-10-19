#include "../pch.h"
#include "../utiliy.h"
#include "../commonNOP.h"

#include "ASFunc_OriginalFunctions.h"
#include "ASFunc_MissionLink.h"

bool* pMissionLinkData;
constexpr int MissionLinkDataSize = 3.5 * 1024;

void __fastcall ASFunc_MissionLink_Initialize() {
	auto p = (bool*)_aligned_malloc(MissionLinkDataSize, 0x10);
	pMissionLinkData = p;
	if (p) {
		memset(p, 0, MissionLinkDataSize);
	}
	// end
}

namespace angelscriptFunc {
	enum MissionLinkDataType {
		MissionLinkDataType_Reset = 0,
		MissionLinkDataType_Set = 1,
		MissionLinkDataType_Get = 2,
	};

	bool MissionLinkDataInfo(int index, int type) {
		if(!pMissionLinkData) return false;

		if ((uint32_t)index >= MissionLinkDataSize) return false;

		if (_IsOnlineMode()) return false;

		switch (type) {
		case MissionLinkDataType_Reset:
			pMissionLinkData[index] = false;
			return false;
		case MissionLinkDataType_Set:
			pMissionLinkData[index] = true;
			return true;
		case MissionLinkDataType_Get:
			return pMissionLinkData[index];
		default: return false;
		}
	}
	// end
}
