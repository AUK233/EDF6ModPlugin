.data

extern ammo_SolidBullet01HitFXRetAddr : qword

.code

ASMammo_SolidBullet01HitFX proc

		mov r9, [rdx+38h]
		cmp dword ptr [r9+12], 4 ; has problem
		ja ofs28A820
		or dword ptr [rcx+0AF4h+140h], 4 ; set hit bit
	ofs28A820:
		mov [rsp+10h], rbx
		mov [rsp+18h], rbp
		mov [rsp+20h], rsi
		jmp ammo_SolidBullet01HitFXRetAddr

ASMammo_SolidBullet01HitFX ENDP

END