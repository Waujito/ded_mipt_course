.model tiny
.code
org 100h
Start:
	mov bp, sp
	mov word ptr ss:[bp], 0082h
	ret


end Start
