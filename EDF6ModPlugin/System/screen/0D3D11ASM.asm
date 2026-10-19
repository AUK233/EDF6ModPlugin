.data

extern module_InitializeD3D11 : proto
extern dx11CreateDeviceRetAddr : qword

extern streamline_SetFeature : proto
extern vedf20B2890 : qword

extern streamline_ExecuteSR : proto
extern RenderBufferToScreenBufferRetAddr : qword
extern RenderBufferToScreenBufferRetAddr2 : qword

extern streamline_SwapChainPresent : proto
extern Call_IDXGISwapChain_PresentRetAddr : qword

extern streamline_SwapChainGetBuffer : proto
extern Call_IDXGISwapChain_GetBufferRetAddr : qword

extern streamline_SwapChainResizeBuffer : proto
extern Call_IDXGISwapChain_ResizeBufferRetAddr : qword

extern streamline_Release : proto
extern Call_IDXGISwapChain_ExitGameRetAddr : qword

extern streamline_CreateSwapChain : proto
extern GetDXGISwapChainRetAddr : qword

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
	xor edx, edx
	;call DLSS_CreateFeature
	call streamline_SetFeature
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

ASMCall_IDXGISwapChain_GetBuffer proc

		lea r8, [rsp+60h]
		mov rdx, rbp
		mov rcx, rbx
		call streamline_SwapChainGetBuffer
		test eax, eax
		js ofs11203BA
		jmp Call_IDXGISwapChain_GetBufferRetAddr
	ofs11203BA:
		xor al, al
		mov rbx, [rsp+70h]
		add rsp, 40h
		pop rdi
		pop rsi
		pop rbp
		ret 
		int 3

ASMCall_IDXGISwapChain_GetBuffer ENDP

align 16

ASMCall_IDXGISwapChain_Present proc

		;
		mov rsi, [rsp+20h]
		mov edx, [rsp+28h]
		mov rcx, rsi
		call streamline_SwapChainPresent
		test al, al
		jnz JustReturn
		;
		xor r8d, r8d
		mov edx, [rsp+28h]
		mov rcx, [rsi+90h]
		mov rax, [rcx]
		call qword ptr [rax+40h]
		lea ecx, [rax+7785FFFBh]
		cmp ecx, 2
		ja JustReturn
		cdqe 
		mov [rsi+2A78h], rax
	JustReturn:
		jmp Call_IDXGISwapChain_PresentRetAddr
		int 3

ASMCall_IDXGISwapChain_Present ENDP

align 16

ASMGetDXGISwapChain proc

		mov rsi, [rsp+0C0h]
		mov rbp, [rsp+0B8h]
		;
		mov r8, [rdi+90h]
		lea rdx, [rsp+28h]
		lea rcx, [rsp+30h]
		call streamline_CreateSwapChain
		mov qword ptr [rdi+90h], 0
		test al, al
		jnz ReturnOriginal
	ofs111A71C:
		mov rcx, [rdi+0F8h] ; IDXGIFactory*
		lea r9, [rsp+28h] ; IDXGISwapChain**
		mov rdx, [rdi+80h] ; ID3D11Device*
		lea r8, [rsp+30h] ; DXGI_SWAP_CHAIN_DESC*
		mov qword ptr [rsp+28h], 0
		mov rax, [rcx]
		call qword ptr [rax+50h]
		test eax, eax
		js ofs111A787
	ReturnOriginal:
		mov rbx, [rsp+28h]
		mov eax, [rsp+30h]
		jmp GetDXGISwapChainRetAddr
		int 3
	ofs111A787:
		add rsp, 90h
		pop r15
		pop rdi
		pop rbx
		ret 
		int 3

ASMGetDXGISwapChain ENDP

align 16

ASMCall_IDXGISwapChain_ResizeBuffer proc

		mov r8d, ebp
		mov edx, r14d
		mov rcx, rbx
		call streamline_SwapChainResizeBuffer
	ofs111B23C:
		jmp Call_IDXGISwapChain_ResizeBufferRetAddr
		int 3

ASMCall_IDXGISwapChain_ResizeBuffer ENDP

align 16

ASMCall_IDXGISwapChain_ExitGame proc

		; rcx is input
		call streamline_Release
		mov [rdi+90h], rsi
	ofs111B4DE:
		jmp Call_IDXGISwapChain_ExitGameRetAddr
		int 3

ASMCall_IDXGISwapChain_ExitGame ENDP

align 16

ASMRenderBufferToScreenBuffer proc

		mov r8, rax
		mov edx, [rax+60h]
		mov rcx, rbx
		call streamline_ExecuteSR
		test rax, rax
		jnz ofs115134B
		jmp RenderBufferToScreenBufferRetAddr2
	ofs115134B:
		lea rcx, [rsp+40h]
		mov dword ptr [rsp+30h], 4
		mov rdx, rbx
		jmp RenderBufferToScreenBufferRetAddr
		int 3

ASMRenderBufferToScreenBuffer ENDP

END
