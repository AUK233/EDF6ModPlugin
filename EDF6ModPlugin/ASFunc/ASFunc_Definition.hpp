#pragma once

// doc see angelscript 2.37.0
typedef void (*asFUNCTION_t)();
struct asSFuncPtr
{
	asSFuncPtr(BYTE f)
	{
		for (size_t n = 0; n < sizeof(ptr.dummy); n++)
			ptr.dummy[n] = 0;
		flag = f;
	}

	union
	{
		char dummy[25]; // largest known class method pointer
		struct { asFUNCTION_t func; char dummy[25 - sizeof(asFUNCTION_t)]; } f;
	} ptr;
	BYTE flag; // 1 = generic, 2 = global func
};

enum asECallConvTypes
{
	asCALL_CDECL = 0,
	asCALL_STDCALL = 1,
	asCALL_THISCALL_ASGLOBAL = 2,
	asCALL_THISCALL = 3,
	asCALL_CDECL_OBJLAST = 4,
	asCALL_CDECL_OBJFIRST = 5,
	asCALL_GENERIC = 6,
	asCALL_THISCALL_OBJLAST = 7,
	asCALL_THISCALL_OBJFIRST = 8
};

typedef int(__fastcall* asCScriptEngine_RegisterGlobalFunction)(void* asCScriptEngine, const char* declaration, const asSFuncPtr& funcPointer, DWORD callConv, void* auxiliary);
