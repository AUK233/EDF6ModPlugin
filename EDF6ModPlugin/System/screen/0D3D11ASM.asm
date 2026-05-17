.data

extern module_InitializeD3D11 : proto
extern dx11CreateDeviceRetAddr : qword

extern DLSS_CreateFeature : proto
extern vedf20B2890 : qword

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

ASMgetPlayerCountInHQ proc

	mov rdx, [vedf20B2890]
	mov rax, [rdx]
	mov rcx, [rax+14FF4h]
	call DLSS_CreateFeature
	;
	mov rax, r14
	lea r11, [rsp+230h]
	mov rbx, [r11+50h]
	movaps xmm6, [r11-10h]
	movaps xmm7, [r11-20h]
	mov rsp, r11
	pop r15
	pop r14
	pop r13
	pop r12
	pop rdi
	pop rsi
	pop rbp
	ret 
	int 3

ASMgetPlayerCountInHQ ENDP

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
