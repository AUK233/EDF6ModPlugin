.data

extern GetPlayerD3DBufferNewSize : proto
extern setPlayerGraphicsSizeRetAddr : qword

extern GetPlayerNewSightGlobalResolution : proto
extern setPlayerSightGlobalResolutionRetAddr : qword

extern GetPlayerNewLockonGlobalResolution : proto
extern setPlayerLockonGlobalResolutionRetAddr : qword

.code

ASMsetPlayerGraphicsSize proc

		lea r9, [rbp+80h]
		mov r8d, [r12]
		mov edx, [r12+4]
		mov rcx, [rcx+10h]
		call GetPlayerD3DBufferNewSize
	ofs11C70F:
		mov r9d, [r12]
		xor edx, edx
		jmp setPlayerGraphicsSizeRetAddr
		int 3

ASMsetPlayerGraphicsSize ENDP

align 16

ASMsetPlayerSightGlobalResolution proc

		mov r14, rcx
		mov rbx, rdx
		; rdx is input
		add rcx, 190h
		call GetPlayerNewSightGlobalResolution
		movaps xmm8, xmm0
		movaps xmm3, xmm0
		movss xmm1, dword ptr [r14+28h]
		movss xmm2, dword ptr [r14+24h]
		lea rdx, [r14+220h]
		lea rcx, [r14+1A0h]
		jmp setPlayerSightGlobalResolutionRetAddr
		int 3

ASMsetPlayerSightGlobalResolution ENDP

align 16

ASMsetPlayerLockonGlobalResolution proc

		lea rdx, [rcx+rdi+504h]
		lea rcx, [rbx+5Ch]
		call GetPlayerNewLockonGlobalResolution
		lea rax, [rsp+30h]
		jmp setPlayerLockonGlobalResolutionRetAddr
		int 3

ASMsetPlayerLockonGlobalResolution ENDP

END
