.data

extern pOutRenderColor2139A78 : qword

.code

Get_g_xgl_draw_utility_color_textrure proc

		mov rdx, pOutRenderColor2139A78
		mov rcx, [rdx]
		mov rax, [rcx+8] ; maybe is pGameRenderer
		mov rdx, [rax+588h]
		mov rax, [rcx+0F98h]
		mov rcx, [rdx+0C8h]
		lea r9, [rax+rax*2]
		mov rdx, [rcx+r9*8+10h]
		test rdx, rdx
		jz ReturnZero
		mov rcx, [rdx+10h]
		test rcx, rcx
		jz ReturnZero
		mov rax, [rcx+10h]
		ret
	ReturnZero:
		xor eax, eax
		ret
		int 3

Get_g_xgl_draw_utility_color_textrure ENDP

END