.model tiny

locals @@

.code
org 100h

Start:
	mov ax, 0a05h
	mov bx, 0b06h
	mov cx, 0c07h
	mov dx, 0d08h
	mov si, 0e09h

	xor di, di
@@loop:
	inc di

	push ax
	in al, 60h
	and al, 7fh
	cmp al, 01h
	je @@exit_prog
	pop ax

	jmp @@loop

@@exit_prog:
	mov ax, 4c00h
	int 21h

end Start

