include EDFSoldierClass.inc

.data

extern AssultSoldier_Initialization : proto
;extern eAssultSoldierRollingSpeedRetAddr : qword


.code

;align 16

ASMeAssultSoldierInitialization proc

	;
	mov rcx, rbx
	call AssultSoldier_Initialization
	mov rax, rbx
	mov rbx, [rsp+90h]
	add rsp, 60h
	pop rdi
	pop rsi
	pop rbp
	ret 
	int 3

ASMeAssultSoldierInitialization ENDP

END
