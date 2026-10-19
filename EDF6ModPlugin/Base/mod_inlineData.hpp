#pragma once

namespace MOD {
	inline UINT32 ReadUInt32Data(void* pData) {
		return *(UINT32*)pData;
	}

	inline void WriteInt32Data(void* pData, int value) {
		*(int*)pData = value;
	}

	inline void WriteFP32Data(void* pData, float value) {
		*(float*)pData = value;
	}

	inline void WritePTRData(void* pData, void* value) {
		*(uintptr_t*)pData = (uintptr_t)value;
	}
}
