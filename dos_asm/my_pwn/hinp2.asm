.model tiny
.code
org 100h
Uwu:
; FillerString: db "01234567890123456789012345678901"
PasswordBuffer: db 20h DUP(0)

BreakHandler proc
	push ax
	push dx

	mov ah, 02h
	mov dl, 06h
	add dl, 01h
	int 21h

SuperDuperCall: db 0e8h, 5fh, 0ffh

	pop dx
	pop ax

	iret
endp

Meow: db "aslkdfj"

end Uwu
