#pragma once
#include "base_baseObject.h"

typedef struct XGS_SGONodeFind_t {
	const wchar_t* m_name;
	int m_index; // if = -1, means it not found
} *P_XGS_SGONodeFind;

typedef struct XGS_SGOGetNode_t {
	void* pNode[2];
	UINT16 m_type; // return 1 or 2
} *P_SGOGetNode;

typedef struct XGS_SGOGetNodeInput_t {
	P_SGOGetNode pOut;
	int m_index;
} *P_SGOGetNodeInput;

typedef void(__fastcall* func_SGOData_FindNode)(void* pData, P_XGS_SGONodeFind pInfo);
typedef void(__fastcall* func_SGOData_GetNode)(void* pData, P_SGOGetNodeInput pInfo);
typedef void(__fastcall* func_SGONode_ReadValue_INT32)(void* pNode, int* out);
typedef void(__fastcall* func_SGONode_ReadValue_FP64)(void* pNode, double* out);


void __fastcall XGS_SGOData_Initialize(PBYTE hmodDLL);

inline UINT16 XGS_SGOData_GetFnIndex(PBYTE pData);

extern "C" {
	// All fnIndex values must not be equal to -1!
	void __fastcall fn_SGOData_FindNode(void* pData, P_XGS_SGONodeFind pInfo, UINT16 fnIndex);
	void __fastcall fn_SGOData_GetNode(void* pData, P_SGOGetNodeInput pInfo, UINT16 fnIndex);
	void __fastcall fn_SGONode_ReadValue_INT32(P_SGOGetNode pNode, int* out, UINT16 fnIndex);
	void __fastcall fn_SGONode_ReadValue_FP64(P_SGOGetNode pNode, double* out, UINT16 fnIndex);

	//
	bool __fastcall XGS_SGOData_ReadValue_INT32(P_XGS_SceneObject pIn, const wchar_t* name, int* out);
}