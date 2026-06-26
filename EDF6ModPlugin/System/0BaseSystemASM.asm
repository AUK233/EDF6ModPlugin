.data

extern GameSetFont_RetAddr : qword
extern GameSetFont_GetNewFontSize : proto
extern LoadNewVoiceFilesCPP : proto

.code

ASMGameSetFont proc

		xorps xmm6, xmm6
		movzx edx, byte ptr [r15+40h]
		mov rcx, rbx
		call GameSetFont_GetNewFontSize
		movaps xmm6, xmm0
		movss dword ptr [r14+14h], xmm6
	ofs1180EBE:
		jmp GameSetFont_RetAddr
		int 3

ASMGameSetFont ENDP

align 16

ASMGameLoadVoiceFiles proc

		mov rcx, rsi
		mov rbp, [rsp+38h]
		mov rsi, [rsp+40h]
		add rsp, 20h
		pop rdi
		jmp LoadNewVoiceFilesCPP
		int 3

ASMGameLoadVoiceFiles ENDP

END