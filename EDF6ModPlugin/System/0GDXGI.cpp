#include "pch.h"
#include "utiliy.h"
#include "commonNOP.h"

#include "0GDXGI.h"

extern "C" {
	uintptr_t _F_GameDXGIRender; // edf.dll+2136FF0
	uintptr_t _F_GameApplication;  // edf.dll+2139B90
}

void DXGI_Initialize(PBYTE hmodEXE) {
	_F_GameDXGIRender = (uintptr_t)(hmodEXE + 0x2136FF0);
	_F_GameApplication = (uintptr_t)(hmodEXE + 0x2139B90);
}

PGameDXGIRender __fastcall DXGI_GetGameDXGIRender() {
	return (PGameDXGIRender)_F_GameDXGIRender;
}

PG_Application __fastcall DXGI_GetApplication(){
	return *(PG_Application*)_F_GameApplication;
}
