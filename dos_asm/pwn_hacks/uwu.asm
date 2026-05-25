.model tiny
.code
org 100h

Start:
CDS:
	mov ah, 02h
	mov dl, 06h
	add dl, 01h
	int 21h

	push (offset AfterCall) - (offset CDS) + 82h
	push 014Eh
	ret
AfterCall:
	mov ax, 4c00h
	int 21h
end Start
