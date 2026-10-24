
.data

extern Vehicle403_Tank_Initialization : proto
;extern hudShowSupportSlotRetAddr : qword

.code

ASMeVehicle403_Tank_Initialization proc

	mov rcx, r15
	call Vehicle403_Tank_Initialization
	mov rax, r15
	mov rbx, [rsp+0E0h]
	add rsp, 90h
	pop r15
	pop r14
	pop r13
	pop r12
	pop rdi
	pop rsi
	pop rbp
	ret 
	int 3

ASMeVehicle403_Tank_Initialization ENDP

;align 16

END
