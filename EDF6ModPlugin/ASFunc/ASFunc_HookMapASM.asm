.data

extern ASFunc_ToNew_Map : proto

.code

ASMasFunc_Hook_Map proc

	mov rbx, [rsp+70h]
	add rsp, 60h
	pop rdi
	jmp ASFunc_ToNew_Map
	int 3

ASMasFunc_Hook_Map ENDP

END