.data

extern ProtectEDFCreateWSting : proto
extern edfCreateWSting : qword

extern wstr_GameTitle : word
extern setGameTitleRetAddr : qword

extern updateOnlineRoomMissionNameRetAddr : qword

.code

ASMsetGameTitle proc

		call ProtectEDFCreateWSting
		;
		mov rax, setGameTitleRetAddr
		mov r8d, 34
		lea rdx, wstr_GameTitle
		lea rcx, [rbp-40h]
		push rax
		jmp edfCreateWSting
		int 3

ASMsetGameTitle ENDP

align 16

ASMupdateOnlineRoomMissionName proc

		cmp esi, [rax+4]
		jae NoMissionName
		add rdx, rax
		lea rcx, [rsi+rsi*2]
		lea rax, [rdx+rcx*4]
		movsxd rcx, dword ptr [rax+8]
		jmp updateOnlineRoomMissionNameRetAddr
	NoMissionName:
		mov rax, rdi
		mov rbx, [rsp+40h]
		mov rsi, [rsp+48h]
		add rsp, 30h
		pop rdi
		ret 
		int 3

ASMupdateOnlineRoomMissionName ENDP

END