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

	mov si, offset PasswordBuffer
	mov di, si

@@continue_input:
	mov ah, 01h
	int 21h

	mov cs:[di], al	
	inc di

	cmp al, 0dh
	jne @@continue_input

	mov cx, di
	sub cx, si

	push cs
	pop es
	mov di, si
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

	mov al, bl

	mov sp, bp
	pop bp
	ret
endp

SuperSecure proc
	mov ah, 09h
	mov dx, offset AccessGranted
	int 21h

	ret
endp

Password: db "nigga", 0dh
WelcomeString: db "Welcome, traveller!", 0dh, 0ah, "Enter a password, please: $"
AccessGranted: db "Access Granted! POG", 0dh, 0ah, '$'
AccessDenied: db "Access Denied! LOOOOOOOOOSER", 0dh, 0ah, '$'

PasswordBuffer: db 20h DUP(0)

BreakHandler proc
	stc
	retf
endp

DUMMY: db 20h DUP(90h)

end Start
