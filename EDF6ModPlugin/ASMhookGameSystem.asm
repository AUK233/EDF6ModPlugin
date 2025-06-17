.data

extern readMissionSavaDataRetAddr : qword
extern setPlayerWeaponFriendlyFireRetAddr : qword

extern debug_MissionLoadingFail : dword
extern Debug_hookScriptRunRetAddr0 : qword
extern Debug_hookScriptRunRetAddr1 : qword

.code

ASMreadMissionSavaData proc

    checkOnline:
        cmp r8d, 1
        je toCrash
        cmp r8d, 3
        je toCrash
        cmp r8d, 5
        je toCrash ; If is offline mode, allow access to game
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
    toCrash:
        ; Otherwise crashes game
        mov dword ptr [0], 0
        int 3

ASMreadMissionSavaData ENDP

align 16

ASMsetPlayerWeaponFriendlyFire proc

        mov r9, [r14+1950h]
        mov r8, [r14+1960h]
        test r8, r8
        je ofs5A3655
        xor rcx, rcx
        align 16
    ofs5A3630:
        mov rdx, [r9+rcx*8] ; get weapon pointer
        ;mov byte ptr [rax+8B6h], 1 ; FriendlyFire
        ; damage
        movss xmm0, dword ptr [rdx+89Ch]
        addss xmm0, xmm0 ; *=2
        movss dword ptr [rdx+89Ch], xmm0
        ; reload time
        mov eax, [rdx+20Ch] ; get reload time
        cmp eax, 0
        jle nextWeapon ; if <= 0
        shr eax, 1 ; /=2
        mov [rdx+20Ch], eax
        mov [rdx+210h], eax
        mov [rdx+0E68h], eax
    nextWeapon:
        inc rcx
        cmp rcx, r8
        jl ofs5A3630
    ofs5A3655:
        jmp setPlayerWeaponFriendlyFireRetAddr
        int 3

ASMsetPlayerWeaponFriendlyFire ENDP

align 16

ASMdebug_hookScriptRun proc

        mov rcx, [rcx+40h]
        test rcx, rcx
        je ofs70E66F_ALis0
        mov rax, [rcx]
        call qword ptr [rax+30h]
        cmp eax, 3
        je ofs70E66F_ALis0
        cmp debug_MissionLoadingFail, 0
        jne ofsMissionFail
        ;
        mov rax, [rdi]
        mov rcx, rdi
        call qword ptr [rax+1A0h]
        xor eax, eax
        jmp Debug_hookScriptRunRetAddr1
    ofsMissionFail:
        mov debug_MissionLoadingFail, 0
    ofs70E66F_ALis0:
        jmp Debug_hookScriptRunRetAddr0
        int 3

ASMdebug_hookScriptRun ENDP

END