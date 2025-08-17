.data

extern AngelScript_RegisterGlobalFunction : proto

.code

ASMaddAngelScriptFunction proc

	mov rax, [rsi]
	mov rdx, [rax+48h]
	mov rcx, rsi
	call AngelScript_RegisterGlobalFunction
	mov rbx, [rsp+58A8h]
	add rsp, 5870h
	pop rdi
	pop rsi
	pop rbp
	ret 
	int 3

ASMaddAngelScriptFunction ENDP

END