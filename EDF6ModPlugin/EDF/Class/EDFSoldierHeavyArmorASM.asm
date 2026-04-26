include EDFSoldierClass.inc

.data

; vft
extern eHeavyArmorFunc28 : qword
extern SetModelDisplayStatus6C04B0 : qword

extern eHeavyArmorFunc210 : qword
extern eHeavyArmorFunc220 : qword

.code

ASMeHeavyArmorInitialization proc

		xorps xmm0, xmm0
		movups [rdi+ofs_HeavyArmor_pSubWeapon], xmm0
	ofs565146:
		mov rax, rdi
		mov rbx, [rsp+2F0h]
		movaps xmm6, [rsp+290h]
		add rsp, 2A0h
		pop r15
		pop r14
		pop r13
		pop r12
		pop rdi
		pop rsi
		pop rbp
		ret 
		int 3

ASMeHeavyArmorInitialization ENDP

align 16

ASMeHeavyArmorFunc28StackSize = 20h
ASMeHeavyArmorFunc28 proc

		cmp qword ptr [rcx+ofs_HeavyArmor_pSubWeapon], 0 ; check sub weapon
		mov rax, eHeavyArmorFunc28
		jne newFunc
		jmp rax
	newFunc:
		mov [rsp+8], rbx
		push rdi
		sub rsp, ASMeHeavyArmorFunc28StackSize
		mov rbx, rcx
		call rax
		;
		mov rdi, [rbx+ofs_HeavyArmor_pSubWeapon]
		mov dl, 1
		cmp [rdi+78Ch], dl
		je checkJumpKey
		mov byte ptr [rdi+78Ch], dl
		lea rcx, [rdi+0F30h]
		call SetModelDisplayStatus6C04B0
		;
	checkJumpKey:
		test byte ptr [rbx+5D0h], 4
		jne goBack
		cmp byte ptr [rbx+ofs_SoldierBase_key_jump_holdon], 0 ; check hold on jump key
		mov eax, [rbx+ofs_HeavyArmor_SubWeaponButtonTimer]
		je checkJumpKeyStatus
		inc eax
		mov [rbx+ofs_HeavyArmor_SubWeaponButtonTimer], eax
		cmp eax, 20
		jbe goBack
		; to reload
		mov dword ptr [rbx+ofs_HeavyArmor_SubWeaponButtonTimer], 0
		mov dword ptr [rbx+ofs_HeavyArmor_SubWeaponButtonCD], 20
		mov rcx, rdi
		mov rax, [rdi]
		call qword ptr [rax+98h] ; Request to reload
		jmp goBack
	checkJumpKeyStatus:
		cmp dword ptr [rbx+ofs_HeavyArmor_SubWeaponButtonCD], 0
		jle SubWeaponFire
		dec dword ptr [rbx+ofs_HeavyArmor_SubWeaponButtonCD]
		jmp goBack
	SubWeaponFire:
		test eax, eax
		jz goBack
		mov dword ptr [rbx+ofs_HeavyArmor_SubWeaponButtonTimer], 0 ; clear timer
		mov byte ptr [rdi+139h], 1
		;
	goBack:
		mov rbx, [rsp+ASMeHeavyArmorFunc28StackSize+10h]
		add rsp, ASMeHeavyArmorFunc28StackSize
		pop rdi
		ret 
		int 3

ASMeHeavyArmorFunc28 ENDP

align 16

ASMeHeavyArmorFunc220 proc

		cmp qword ptr [rcx+ofs_HeavyArmor_pSubWeapon], 0 
		jne NoAction
		jmp eHeavyArmorFunc220
	NoAction:
		jmp eHeavyArmorFunc210
		int 3

ASMeHeavyArmorFunc220 ENDP

END
