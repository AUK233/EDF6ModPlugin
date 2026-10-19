#pragma once

typedef struct XGS_SGONodeFind_t {
	wchar_t* m_name;
	int m_index; // if = -1, means it not found
} *P_XGS_SGONodeFind;

typedef struct XGS_SGOGetNode_t {
	void* pNode[2];
	INT16 m_type; // return 1 or 2
} *P_SGOGetNode;

typedef struct XGS_SGOGetNodeInput_t {
	P_SGOGetNode pOut;
	int m_index;
} *P_SGOGetNodeInput;

typedef void(__fastcall* func_SGOData_FindNode)(void* pData, P_XGS_SGONodeFind pInfo);
typedef void(__fastcall* func_SGOData_GetNode)(void* pData, P_SGOGetNodeInput pInfo);
typedef void(__fastcall* func_SGONode_ReadValue_FP64)(void* pNode, double* out);


void __fastcall XGS_SGOData_Initialize(PBYTE hmodDLL);

inline INT16 XGS_SGOData_GetFnIndex(PBYTE pData);

extern "C" {
	// All fnIndex values must not be equal to -1!
	void __fastcall fn_SGOData_FindNode(void* pData, P_XGS_SGONodeFind pInfo, INT16 fnIndex);
	void __fastcall fn_SGOData_GetNode(void* pData, P_SGOGetNodeInput pInfo, INT16 fnIndex);
	void __fastcall fn_SGONode_ReadValue_FP64(void* pNode, double* out, INT16 fnIndex);
}