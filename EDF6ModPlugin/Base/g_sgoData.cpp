#include "pch.h"
#include "utiliy.h"

#include "g_sgoData.h"

extern "C" {
	func_SGOData_FindNode* SGOData_FindNode; // edf.dll+179EBA8
	func_SGOData_GetNode* SGOData_GetNode; // edf.dll+179EAF0
	func_SGONode_ReadValue_INT32* SGONode_ReadValue_INT32; // edf.dll+179EA90
	func_SGONode_ReadValue_FP64* SGONode_ReadValue_FP64; // edf.dll+179EAA8
}

void __fastcall XGS_SGOData_Initialize(PBYTE hmodDLL) {
	SGOData_FindNode = (func_SGOData_FindNode*)(hmodDLL + 0x179EBA8);
	SGOData_GetNode = (func_SGOData_GetNode*)(hmodDLL + 0x179EAF0);
	SGONode_ReadValue_INT32 = (func_SGONode_ReadValue_INT32*)(hmodDLL + 0x179EA90);
	SGONode_ReadValue_FP64 = (func_SGONode_ReadValue_FP64*)(hmodDLL + 0x179EAA8);
}

inline UINT16 XGS_SGOData_GetFnIndex(PBYTE pData) {
	return *(UINT16*)(pData + 0x10);
}

void __fastcall fn_SGOData_FindNode(void* pData, P_XGS_SGONodeFind pInfo, UINT16 fnIndex) {
	auto callFunc = SGOData_FindNode[fnIndex];
	callFunc(pData, pInfo);
}

void __fastcall fn_SGOData_GetNode(void* pData, P_SGOGetNodeInput pInfo, UINT16 fnIndex) {
	auto callFunc = SGOData_GetNode[fnIndex];
	callFunc(pData, pInfo);
}

void __fastcall fn_SGONode_ReadValue_INT32(P_SGOGetNode pNode, int* out, UINT16 fnIndex){
	if (fnIndex == -1) {
		*out = 115114;
		return;
	}

	auto callFunc = SGONode_ReadValue_INT32[fnIndex];
	callFunc(pNode, out);
}

void __fastcall fn_SGONode_ReadValue_FP64(P_SGOGetNode pNode, double* out, UINT16 fnIndex) {
	if (fnIndex == -1) {
		*out = 115114.0;
		return;
	}

	auto callFunc = SGONode_ReadValue_FP64[fnIndex];
	callFunc(pNode, out);
}

bool __fastcall XGS_SGOData_ReadValue_INT32(P_XGS_SceneObject pIn, const wchar_t* name, int* out) {
	auto fnIndex = pIn->SGOData_fnIndex;
	if (fnIndex == 0xFFFF) return false;

	auto pData = &pIn->SGOData[0];
	XGS_SGONodeFind_t info;
	info.m_name = name;
	info.m_index = -1;
	fn_SGOData_FindNode(pData, &info, fnIndex);
	if (info.m_index == -1) return false;

	XGS_SGOGetNode_t node;
	node.m_type = -1;
	XGS_SGOGetNodeInput_t input;
	input.pOut = &node;
	input.m_index = info.m_index;
	fn_SGOData_GetNode(pData, &input, fnIndex);
	if (node.m_type == -1) return false;

	fn_SGONode_ReadValue_INT32(&node, out, node.m_type);
	return true;
}
