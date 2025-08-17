.data

extern setPlayerWeaponFriendlyFireRetAddr : qword

extern debug_MissionLoadingFail : dword
extern Debug_hookScriptRunRetAddr0 : qword
extern Debug_hookScriptRunRetAddr1 : qword

extern wstr_range : word
extern LoadDSGONode : qword
extern LoadDSGOValue : qword
extern Weapon_Drone_LaserMarker_InitRetAddr : qword

.code

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

align 16

ASMWeapon_Drone_LaserMarker_Init proc

        movss dword ptr [rbp+84h], xmm0 ; laser speed
        ;
        lea r8, wstr_range ; range
        lea rdx, [rbp-58h]
        mov rcx, r13
        call LoadDSGONode
        cmp dword ptr [rax+8], -1
        je FixedLaserAlive
        mov rcx, rax
        call LoadDSGOValue
        cvttsd2si rax, xmm0
        shr rax, 1 ; /2, because speed is 2
        mov [rbp+88h], rax
        jmp ofs6828A0
        ;
    FixedLaserAlive:
        mov qword ptr [rbp+88h], 180 ; laser alive
    ofs6828A0:
        mov [rbp-58h], r14
        mov [rbp-58h+8], r14
        jmp Weapon_Drone_LaserMarker_InitRetAddr
        int 3

ASMWeapon_Drone_LaserMarker_Init ENDP

END