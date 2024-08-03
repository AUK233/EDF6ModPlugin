.data

extern readMissionSavaDataRetAddr : qword

.code

ASMreadMissionSavaData proc

    checkOnline:
        test r8, r8
        je ofsE1D70 ; If is 0, allow access to game
        ; Otherwise crashes game
        mov dword ptr [0], 0
    ofsE1D70:
        mov [rsp+18h], rbx
        push rbp
        push rsi
        push rdi
        push r12
        push r13
        push r14
        push r15
        jmp readMissionSavaDataRetAddr
        int 3

ASMreadMissionSavaData ENDP

END