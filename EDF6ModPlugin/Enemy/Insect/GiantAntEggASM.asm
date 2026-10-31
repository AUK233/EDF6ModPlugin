
.data

extern MODGlobalManager_CheckEggBreak : proto
extern _F_CreateObjectFromWcharPath_ : qword
extern GiantAntEggToBreakRetAddr : qword

.code

ASMGiantAntEggToBreak proc

		test rax, rax
		jz returnOriginal
		mov rdx, rax
		mov rcx, rbx
		call MODGlobalManager_CheckEggBreak
		test rax, rax
		jz returnOriginal
	ofs413AF6:
		movsxd r8, dword ptr [rax+8]
		add r8, rax
		lea r9, [rbp-50h]
		lea rdx, [r15+60h]
		mov rcx, rbx
		call _F_CreateObjectFromWcharPath_
	returnOriginal:
		mov r8, rax
		jmp GiantAntEggToBreakRetAddr
		int 3

ASMGiantAntEggToBreak ENDP

;align 16

END
