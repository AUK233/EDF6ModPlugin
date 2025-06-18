.data

extern readFile735C0RetAddr : qword

.code

ASMreadFile735C0 proc

	mov [rsp+18h], rbx
	mov [rsp+20h], rsi
	push rbp
	push rdi
	push r14
	mov rbp, rsp
	jmp readFile735C0RetAddr

ASMreadFile735C0 ENDP

END