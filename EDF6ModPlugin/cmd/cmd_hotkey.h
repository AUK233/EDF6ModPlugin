#pragma once

typedef struct cmd_KeyState_t {
	int ctrl, alt;
	int U, O;
	int M;
	int Add, Subtract;
} *Pcmd_KeyState;

void cmd_ModConsole_Initialize(PBYTE hmodDLL);

void cmd_ModConsole_MonitorKeys();
void cmd_ModConsole_SetKeyState();

void cmd_ModConsole_SwitchPlayerHudShow();
void cmd_ModConsole_ScalePlayerCamera(int isShrink);
