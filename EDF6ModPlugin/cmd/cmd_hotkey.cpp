#include "../pch.h"
#include "../Base/g_system.h"
#include "../Base/g_object.h"
#include "../Base/SSE.hpp"
#include "../Base/CommonStructure.hpp"
#include "../utiliy.h"
#include "cmd_hotkey.h"

cmd_KeyState_t cmd_keyState;
PBYTE cmd_PvCameraMode;
void_fastcallNoParameter cmdFunc__unlock_stage;

void cmd_ModConsole_Initialize(PBYTE hmodDLL)
{
	ZeroMemory(&cmd_keyState, sizeof(cmd_keyState));
	// edf.dll+20B3418 is bool PvCameraMode
	cmd_PvCameraMode = (PBYTE)(hmodDLL + 0x20B3418);
	// edf.dll+AEEB0, find "unlock_stage"
	cmdFunc__unlock_stage = (void_fastcallNoParameter)(hmodDLL + 0xAEEB0);
}

void cmd_ModConsole_MonitorKeys()
{
	static bool state_cheatonce = false;
	// delayed startup by 10 seconds.
	Sleep(10000);

	while (true) {
		auto pDXGI = GetDXGISwapChainDescPointer();
		if (GetForegroundWindow() != pDXGI->OutputWindow)
		{
			ZeroMemory(&cmd_keyState, sizeof(cmd_keyState));
			goto gotoSleep;
		}

		cmd_ModConsole_SetKeyState();

		if (cmd_keyState.ctrl){
			if (cmd_keyState.U == 1) {
				cmd_ModConsole_SwitchPlayerHudShow();
			}

			if (cmd_keyState.Add == 1) {
				cmd_ModConsole_ScalePlayerCamera(0);
			}
			if (cmd_keyState.Subtract == 1) {
				cmd_ModConsole_ScalePlayerCamera(1);
			}
		}

		if (cmd_keyState.alt) {
			if (cmd_keyState.O == 1) {
				state_cheatonce = !state_cheatonce;
			}

			if (state_cheatonce) {
				// this is unlock mission
				if (cmd_keyState.M == 1) {
					cmdFunc__unlock_stage();
					state_cheatonce = false;
				}
			}
		}

		gotoSleep:
		Sleep(10);
	}
}

void cmd_ModConsole_SetKeyState()
{
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
		cmd_keyState.ctrl += 1;
	} else {
		cmd_keyState.ctrl = 0;
	}

	// alt!
	if (GetAsyncKeyState(VK_MENU) & 0x8000) {
		cmd_keyState.alt += 1;
	}
	else {
		cmd_keyState.alt = 0;
	}


	if (GetAsyncKeyState('U') & 0x8000) {
		cmd_keyState.U += 1;
	} else {
		cmd_keyState.U = 0;
	}

	if (GetAsyncKeyState('O') & 0x8000) {
		cmd_keyState.O += 1;
	} else {
		cmd_keyState.O = 0;
	}

	if (GetAsyncKeyState('M') & 0x8000) {
		cmd_keyState.M += 1;
	} else {
		cmd_keyState.M = 0;
	}

	if (GetAsyncKeyState(VK_ADD) & 0x8000) {
		cmd_keyState.Add += 1;
	} else {
		cmd_keyState.Add = 0;
	}

	if (GetAsyncKeyState(VK_SUBTRACT) & 0x8000) {
		cmd_keyState.Subtract += 1;
	} else {
		cmd_keyState.Subtract = 0;
	}
}

void cmd_ModConsole_SwitchPlayerHudShow()
{
	auto pSystem = XGS_GetXGSSystemPointer();
	auto pPlayer = XGS_GetSystemPlayerPointer(pSystem, 0);
	auto pCamera = pPlayer->pCamera;
	if (pCamera) {
		pCamera->bIsPlayerHudShow = !pCamera->bIsPlayerHudShow;

		bool oldPvCameraMode = *cmd_PvCameraMode;
		*cmd_PvCameraMode = oldPvCameraMode ? 0 : 1;
	}

}

void cmd_ModConsole_ScalePlayerCamera(int isShrink)
{
	auto pSystem = XGS_GetXGSSystemPointer();
	auto pPlayer = XGS_GetSystemPlayerPointer(pSystem, 0);
	auto pCamera = pPlayer->pCamera;
	auto pGameObject = (PXGS_Object)(pCamera->pGameObject);
	if (!pGameObject) {
		return;
	}

	__m128 scaleV;
	if (isShrink) {
		scaleV = _mm_setr_ps(0.5f, 0.5f, 0.5f, 1.0f);
	}
	else {
		scaleV = _mm_setr_ps(2.0f, 2.0f, 2.0f, 1.0f);
	}

	__m128 v_camera1 = _mm_loadu_ps(&pGameObject->camera[0][0]);
	__m128 v_camera2 = _mm_loadu_ps(&pGameObject->camera[1][0]);

	v_camera1 = _mm_mul_ps(v_camera1, scaleV);
	v_camera2 = _mm_mul_ps(v_camera2, scaleV);

	_mm_storeu_ps(&pGameObject->camera[0][0], v_camera1);
	_mm_storeu_ps(&pGameObject->camera[1][0], v_camera2);
}
