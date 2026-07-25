#include "../pch.h"
#include "../utiliy.h"

#include "g_criFS.h"

extern "C" {
	uintptr_t vedf20B28D0; // edf.dll+20B28D0
	uintptr_t CriFileSystem_OpenFile; // edf.dll+84E80
	uintptr_t CriFileSystem_ReleaseData; // edf.dll+B4E20
}

void __fastcall CriFileSystem_Initialize(PBYTE hmodDLL) {
	vedf20B28D0 = (uintptr_t)(hmodDLL + 0x20B28D0);
	CriFileSystem_OpenFile = (uintptr_t)(hmodDLL + 0x84E80);
	CriFileSystem_ReleaseData = (uintptr_t)(hmodDLL + 0xB4E20);
}

CriFileSystemGet_t* CriFileSystemGet_t::Open(const std::wstring& filename) {
	typedef PCriFileSystemGet(__fastcall* callFunc)(uintptr_t crifs, PCriFileSystemGet out, const std::wstring& filename, int r9d);
	callFunc func = (callFunc)CriFileSystem_OpenFile;
	auto ret = func(vedf20B28D0, this, filename, 0);
	return ret;
}

void CriFileSystemGet_t::AddRef() {
	if (!manager) return;

	auto cs = &manager->cs;

	EnterCriticalSection(cs);
	fs->counter++;
	LeaveCriticalSection(cs);
}

void CriFileSystemGet_t::Release() {
	if (!manager) return;

	auto cs = &manager->cs;

	EnterCriticalSection(cs);

	fs->counter -= 1;
	if(fs->counter == 0){
		typedef void(__fastcall* callFunc)(PCriFileSystemManager manager, PCriFileSystem fs);
		callFunc func = (callFunc)CriFileSystem_ReleaseData;
		func(manager, fs);
	}

	LeaveCriticalSection(cs);
}
