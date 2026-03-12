.model tiny

.code
org 100h

Start:
	sub sp, 100h

	mov dx, offset WelcomeString
	mov ah, 09h
	int 21h

	mov dx, offset PasswordBuffer
	mov ah, 0ah
	int 21h

	mov ah, 02h
	mov dl, 0dh
	int 21h
	mov dl, 0ah
	int 21h

	xor cx, cx
	mov cl, byte ptr [offset PasswordBuffer + 1]

	call FirstFN
	call SecondFN;done
	call ThirdFN;done
	call FourthFN; done
	call Fo


FourthFN:
	mov ax, 4c00h
	int 21h

ThirdFN proc
	mov ax, 0900h
	int 21h

	mov word ptr [offset AXBUF], ax
	mov word ptr [offset BPBUF], bp
	mov bp, sp
	mov ax, word ptr [bp]
	and ax, 0f800h
	; jmp D2PT
	db 0fh, 85h, 87h, 00h

	;jnz near ptr D2PT
	mov ax, word ptr [offset AXBUF]
	mov bp, word ptr [offset BPBUF]
	ret
endp

SecondFN proc
	test cx, cx
	jnz SFN_FPT
	nop
	nop
	mov dx, 01eah
	jmp SFN_SPT
SFN_FPT:
	mov dx, 01f9h
SFN_SPT:
	mov word ptr [offset AXBUF], ax
	mov word ptr [offset BPBUF], bp
	mov bp, sp
	mov ax, word ptr [bp]
	and ax, 0f800h
	jnz D2PT
	nop
	nop
	mov ax, word ptr [offset AXBUF]
	mov bp, word ptr [offset BPBUF]
	ret
endp

FirstFN proc
	sub sp, 000fh
	mov bp, sp
	mov si, offset PasswordBuffer + 2
	mov di, bp
	repe movsb
	call FifthFN
	add sp, 000fh
	mov word ptr [offset AXBUF], ax
	mov word ptr [offset BPBUF], bp
	mov bp, sp
	mov ax, word ptr [bp]
	and ax, 0f800h
	jnz D2PT
	nop
	nop
	mov ax, word ptr [offset AXBUF]
	mov bp, word ptr [offset BPBUF]
	ret
	
endp

FifthFN proc
	push ds
	pop es
	mov si, offset RealPassword
	mov di, bp
	mov cx, 000ch
	nop
	repe cmpsb
	mov word ptr [offset AXBUF], ax
	mov word ptr [offset BPBUF], bp
	mov bp, sp
	mov ax, word ptr [bp]
	and ax, 0f800h
	jnz D2PT
	nop
	nop
	mov ax, word ptr [offset AXBUF]
	mov bp, word ptr [offset BPBUF]
	ret
endp

D2PT:
	mov dx, offset SEGF
	call ThirdFN
	call FourthFN


WelcomeString: db "Enter string: $"
AccessGranted: db "Access granted$"
AccessDenied: db "Access denied$"
SEGF: db "SEGMENTATION FAULT IDI NAHUY$"
RealPassword: db "sigma67sigma"
PasswordBuffer: db 40h, 42h DUP(0)

AXBUF:; 0273h
org 275h
BPBUF:; 0275h

end Start

