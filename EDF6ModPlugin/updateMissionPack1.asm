.data

extern GetOnlineRoomType : proto
extern onlineBlockPublicRoomRetAddr : qword

.code

ASMonlineBlockPublicRoom proc

        mov edx, [rbx+99Ch]
        mov ecx, [rbx+998h]
        call GetOnlineRoomType
        cmp eax, 2
        jb toCrash
        mov r8d, edi
        jmp onlineBlockPublicRoomRetAddr
    toCrash:
        ; Otherwise crashes game
        mov [0], eax
        int 3

ASMonlineBlockPublicRoom ENDP

END