.data

extern ammo_SentryGunBulletOpen01_FireAmmoRetAddr : qword
extern Game_CreateObjectFromObjectFactory : proto
extern ammo_EfsExposureBulletVFT : qword
extern vft_EfsExposureBullet_AutoAlive : qword


.code

ASMammo_SentryGunBulletOpen01_FireAmmo proc

		mov [rdi+0B80h+70h], rbx
	ofs2A367B:
		mov rdx, rbx
		mov qword ptr [r9+50h], r12
		mov dword ptr [r9+58h], r12d
		mov dword ptr [r9+5Ch], 3F800000h
		; rcx is operated by the function
		call Game_CreateObjectFromObjectFactory
		; check vft is EfsExposureBullet
		test rax, rax
		jz ofs2A3696
		mov rdx, [rax]
		cmp rdx, ammo_EfsExposureBulletVFT
		jne ofs2A3696
		lea rcx, vft_EfsExposureBullet_AutoAlive+8
		mov [rax], rcx
		; end
	ofs2A3696:
		jmp ammo_SentryGunBulletOpen01_FireAmmoRetAddr

ASMammo_SentryGunBulletOpen01_FireAmmo ENDP

END