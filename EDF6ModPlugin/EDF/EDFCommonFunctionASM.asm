include Class/EDFSoldierClass.inc

.data

extern hudShowSupportSlotRetAddr : qword

.code

ASMhudShowSupportSlot proc

		cmp dword ptr [r13+ofs_SoldierBase_human_type], 1 ; check is fencer
		jne ofs82D980
		test edi, edi ; check is sub slot 0
		jnz ofs82D980
		mov [r13+ofs_HeavyArmor_pSubWeapon], rax ; set sub weapon
	ofs82D980:
		xor edx, edx
		mov [rbp-8], rdx
		xorps xmm0, xmm0
		movdqa [rbp], xmm0
		mov [rbp+10h], rdx
		jmp hudShowSupportSlotRetAddr 
		int 3

ASMhudShowSupportSlot ENDP

;align 16

END
