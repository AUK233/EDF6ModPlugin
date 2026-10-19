#include "pch.h"
#include "utiliy.h"

#include "g_sgoData.h"

extern "C" {
	func_SGOData_FindNode* SGOData_FindNode; // edf.dll+179EBA8
	func_SGOData_GetNode* SGOData_GetNode; // edf.dll+179EAF0
	func_SGONode_ReadValue_FP64* SGONode_ReadValue_FP64; // edf.dll+179EAA8
}

void __fastcall XGS_SGOData_Initialize(PBYTE hmodDLL) {
	SGOData_FindNode = (func_SGOData_FindNode*)(hmodDLL + 0x179EBA8);
	SGOData_GetNode = (func_SGOData_GetNode*)(hmodDLL + 0x179EAF0);
	SGONode_ReadValue_FP64 = (func_SGONode_ReadValue_FP64*)(hmodDLL + 0x179EAA8);
}

inline INT16 XGS_SGOData_GetFnIndex(PBYTE pData) {
	return *(INT16*)(pData + 0x10);
}

void __fastcall fn_SGOData_FindNode(void* pData, P_XGS_SGONodeFind pInfo, INT16 fnIndex) {
	auto callFunc = SGOData_FindNode[fnIndex];
	callFunc(pData, pInfo);
}

void __fastcall fn_SGOData_GetNode(void* pData, P_SGOGetNodeInput pInfo, INT16 fnIndex) {
	auto callFunc = SGOData_GetNode[fnIndex];
	callFunc(pData, pInfo);
}

void __fastcall fn_SGONode_ReadValue_FP64(void* pNode, double* out, INT16 fnIndex) {
	if (fnIndex == -1) {
		*out = 115114.0;
		return;
	}

	auto callFunc = SGONode_ReadValue_FP64[fnIndex];
	callFunc(pNode, out);
}
