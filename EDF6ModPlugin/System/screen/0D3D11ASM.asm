.data

extern module_InitializeD3D11 : proto
extern dx11CreateDeviceRetAddr : qword

extern DLSS_Release : proto

extern DLSS_Draw : proto
extern RenderBufferToScreenBufferRetAddr : qword

.code

ASMdx11CreateDevice proc

	mov [rsp+20h], rax
	xor r9d, r9d
	xor r8d, r8d
	lea rcx, [rbp-49h] ; DXGI_SWAP_CHAIN_DESC
	call module_InitializeD3D11
	jmp dx11CreateDeviceRetAddr
	int 3

ASMdx11CreateDevice ENDP

align 16

ASMsysExitGame proc

	add rsp, 90h
	pop rdi
	pop rsi
	pop rbx
	jmp DLSS_Release
	int 3

ASMsysExitGame ENDP

align 16

ASMRenderBufferToScreenBuffer proc

	mov rdx, rax
	mov rcx, [rbx+8]
	call DLSS_Draw
	lea rcx, [rsp+40h]
	mov dword ptr [rsp+30h], 4
	mov rdx, rbx
	jmp RenderBufferToScreenBufferRetAddr
	int 3

ASMRenderBufferToScreenBuffer ENDP

END
