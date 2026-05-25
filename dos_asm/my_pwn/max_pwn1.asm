.model tiny

.code
org 100h

Start:
	cld

	mov ax, 2523h
	mov dx, offset BreakHandler
	int 21h

	mov ah, 09h
	mov dx, offset WelcomeString
	int 21h	

	call PasswordChecker

	test al, al

	jz @@pass
	
	mov ah, 09h
	mov dx, offset AccessDenied
	int 21h

	jmp @@exit_prog

@@pass:
	call SuperSecure

@@exit_prog:
	mov ax, 4c00h
	int 21h

PasswordChecker proc
	push bp
	mov bp, sp

	sub sp, 20h
	mov di, sp
	
@@continue_input:
	mov ah, 08h
	int 21h

	mov ss:[di], al	
	inc di

	cmp al, 0dh
	jne @@continue_input

	mov cx, di
	sub cx, sp

	push ss
	pop es
	mov di, sp
	mov si, offset Password

	repe cmpsb

	jne @@fail_password
	or cx, cx
	jnz @@fail_password
	xor bl, bl
	jmp @@stop_password

@@fail_password:
	mov bl, 01h

@@stop_password:
	mov ah, 02h
	mov dl, 0dh
	int 21h
	mov ah, 02h
	mov dl, 0ah
	int 21h


	add sp, 20h
	pop bp
	mov al, bl
	
	ret
endp

SuperSecure proc
	mov ah, 09h
	mov dx, offset AccessGranted
	int 21h

	ret
endp

BreakHandler proc
	stc
	retf
endp

Password: db "nigga", 0dh
WelcomeString: db "Welcome, traveller!", 0dh, 0ah, "Enter a password, please: $"
AccessGranted: db "Access Granted! POG", 0dh, 0ah, '$'
AccessDenied: db "Access Denied! LOOOOOOOOOSER", 0dh, 0ah, '$'

end Start
