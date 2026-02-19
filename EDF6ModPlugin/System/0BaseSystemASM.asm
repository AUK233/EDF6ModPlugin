.data

extern GameSetFont_RetAddr : qword
extern GameSetFont_GetNewFontSize : proto

.code

ASMGameSetFont proc

		xorps xmm6, xmm6
		movzx edx, byte ptr [r15+40h]
		mov rcx, rbx
		call GameSetFont_GetNewFontSize
		movaps xmm6, xmm0
		movss dword ptr [r14+14h], xmm6
	ofs:
		jmp GameSetFont_RetAddr

ASMGameSetFont ENDP

END