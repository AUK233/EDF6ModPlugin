#pragma once
#include <string>

// maybe size is 0x78
typedef struct CriFileSystem_t {
	char pad00[0x50];
	uint8_t* data;
	size_t data_capacity;
	size_t data_size;
	char pad68[8];
	int counter;
} *PCriFileSystem;
static_assert(offsetof(CriFileSystem_t, data) == 0x50);
static_assert(offsetof(CriFileSystem_t, counter) == 0x70);

typedef struct CriFileSystemManager_t {
	char pad00[0x28];
	CRITICAL_SECTION cs;
} *PCriFileSystemManager;
static_assert(offsetof(CriFileSystemManager_t, cs) == 0x28);

typedef struct CriFileSystemGet_t {
	PCriFileSystem fs = 0;
	PCriFileSystemManager manager = 0;

	CriFileSystemGet_t* Open(const std::wstring& filename);
	void AddRef();
	void Release();

	~CriFileSystemGet_t(){
		Release();
	}
} *PCriFileSystemGet;

void __fastcall CriFileSystem_Initialize(PBYTE hmodDLL);
