.model tiny

locals @@

W_HEIGHT	equ 25
W_WIDTH		equ 80

; FILLER_SYM	equ 0504h ; Pink heart, used for debugging.
FILLER_SYM	equ 3700h ; Blank symbol with default white font on black border

BORDER_COLOR	equ 3fh
TEXT_COLOR	equ 3eh

.code
org 100h

Start:
	call Main
	ret

;------------------------
; INT 9 handler
;------------------------
CaptureKeyboard		proc
	push ax

	in al, 60h
	mov ah, al
	and ah, not 80h

	cmp ah, 3ch	; F2 press/release
	je @@handle_key

	pop ax
	jmp DOS_INT9_ORIG_FJMP

@@handle_key:
	cmp al, 03ch or 80h	; F2 release
	jne @@exit_handler
	
	pop ax	

	call ProgCaptureHandler

	push ax

@@exit_handler:
	; blink to keyboard controller
	in al, 61h
	mov ah, al
	or al, 80h
	out 61h, al
	xchg ah, al
	out 61h, al

	; signal interrupt controller
	mov al, 20h
	out 20h, al

	pop ax

	iret
endp

; Far jump instruction encoded here
DOS_INT9_ORIG_FJMP:	db 0eah
DOS_INT9_HANDLER_PTR:	dw 0000h
DOS_INT9_HANDLER_SEG:	dw 0000h

;------------------------
; INT 8 handler
;------------------------
TimerHandler		proc
	push ax

	mov al, byte ptr cs:[offset DO_SHOW_POPUP]
	test al, al
	jz @@exit

	pop ax
	call ProgTimerHandler
	push ax

@@exit:
	pop ax
	jmp DOS_INT8_ORIG_FJMP

	iret
endp

; Far jump instruction encoded here
DOS_INT8_ORIG_FJMP:	db 0eah
DOS_INT8_HANDLER_PTR:	dw 0000h
DOS_INT8_HANDLER_SEG:	dw 0000h

DO_SHOW_POPUP:		db 0000h

;-----------------------
; Registers 9th interrupt in interrupt table
; Saves previous interrupt in memory
;-----------------------
Main	proc

	push 0000h 
	pop es

	; indexing in interrupt table
	mov di, 4 * 09h

	mov ax, word ptr es:[di]
	mov word ptr cs:[offset DOS_INT9_HANDLER_PTR], ax
	mov ax, word ptr es:[di + 2]
	mov word ptr cs:[offset DOS_INT9_HANDLER_SEG], ax

	cli
	mov es:[di], offset CaptureKeyboard
	mov ax, cs
	mov es:[di + 2], ax
	sti

	; indexing in interrupt table
	mov di, 4 * 08h

	mov ax, word ptr es:[di]
	mov word ptr cs:[offset DOS_INT8_HANDLER_PTR], ax
	mov ax, word ptr es:[di + 2]
	mov word ptr cs:[offset DOS_INT8_HANDLER_SEG], ax

	cli
	mov es:[di], offset TimerHandler
	mov ax, cs
	mov es:[di + 2], ax
	sti

@@exit:
	mov ax, 3100h
	mov dx, offset EOPPP
	shr dx, 4
	inc dx
	int 21h

endp

ProgCaptureHandler proc
	call SaveProgramState
	mov bp, sp

	push cs
	pop ds

	push 0b800h
	pop es

	mov al, byte ptr cs:[offset DO_SHOW_POPUP]
	xor al, 01h
	mov byte ptr cs:[offset DO_SHOW_POPUP], al

	test al, al
	jz @@load_buf

	mov si, FRAME_OFFSET
	mov di, offset RealContentBuffer
	mov bx, FRAME_NCOLS
	mov cx, FRAME_NROWS
	call CopyFrameToBuffer

	call DrawAllRegisters

	mov si, FRAME_OFFSET
	mov di, offset FrameContentBuffer
	mov bx, FRAME_NCOLS
	mov cx, FRAME_NROWS
	call CopyFrameToBuffer

	jmp @@exit_func

@@load_buf:
	mov si, offset RealContentBuffer
	mov di, FRAME_OFFSET
	mov bx, FRAME_NCOLS
	mov cx, FRAME_NROWS
	call CopyBufferToFrame	

@@exit_func:
	mov sp, bp
	call LoadProgramState	

	ret
endp

;---------------------------------
; Implements a custom logic for key handling.
; Works only for F2.
;
; Entry: AL - a keycode
; Destroys: AX
;---------------------------------
ProgTimerHandler proc
	; on stack is flags, cs, ip, handler_ret
	call SaveProgramState
	mov bp, sp

	push cs
	pop ds

	push 0b800h
	pop es

	call UpdateRealBuffer

	call DrawAllRegisters

	mov si, FRAME_OFFSET
	mov di, offset FrameContentBuffer
	mov bx, FRAME_NCOLS
	mov cx, FRAME_NROWS
	call CopyFrameToBuffer

	mov sp, bp
	call LoadProgramState	

	ret
endp

FRAME_NCOLS equ 20d
FRAME_NROWS equ 9d
FRAME_OFFSET equ 2 * 2 * W_WIDTH
FRAME_BUFLEN equ FRAME_NCOLS * FRAME_NROWS * 2

FrameContentBuffer: db FRAME_BUFLEN DUP(0)
RealContentBuffer: db FRAME_BUFLEN DUP(0)


;------------------------------------------------------
; Expects ES to be 0b800h
;------------------------------------------------------
UpdateRealBuffer proc
	mov si, FRAME_OFFSET
	mov di, offset FrameContentBuffer

	mov dx, FRAME_NROWS

@@copy_rowwise:	
	mov cx, FRAME_NCOLS
	push si

@@copy_colwise:
		mov ax, es:[si]
		cmp ax, ds:[di]

		je @@no_op
		mov ds:[di + FRAME_BUFLEN], ax

@@no_op:
		add di, 2
		add si, 2
	loop @@copy_colwise

	pop si
	add si, 2 * W_WIDTH

	dec dx
	test dx, dx
	jnz @@copy_rowwise

	ret

endp

;------------------------------------------------------
; Fill the area in rectangle with FILLER_SYM
; Starting from es:[si], bx is ncols, cx is nrows, buffer is ds:[di]
;------------------------------------------------------
CopyFrameToBuffer proc
@@copy_rowwise:	
	mov dx, cx
	mov cx, bx

	push si

	push ds
	push es
	pop ds
	pop es

	rep movsw

	push ds
	push es
	pop ds
	pop es

	pop si
	add si, 2 * W_WIDTH

	mov cx, dx
	loop @@copy_rowwise

	ret
endp

;------------------------------------------------------
; Fill the area in rectangle with FILLER_SYM
; Starting from es:[si], bx is ncols, cx is nrows, buffer is ds:[di]
;------------------------------------------------------
CopyBufferToFrame proc
@@copy_rowwise:	
	mov dx, cx
	mov cx, bx

	push di

	rep movsw

	pop di
	add di, 2 * W_WIDTH

	mov cx, dx
	loop @@copy_rowwise


	ret
endp

DrawAllRegisters proc
	push 0b800h
	pop es

	mov bx, FRAME_NCOLS
	mov cx, FRAME_NROWS
	mov si, FRAME_OFFSET

	mov ah, BORDER_COLOR

	push bx
	push cx
	push si

	call FillBorder

	pop di
	pop cx
	pop bx

	add di, 2 * W_WIDTH + 2
	sub cx, 2
	sub bx, 2

	push di

	call CleanRectangle

	pop di
	add di, 2
	push di

	mov ah, TEXT_COLOR


	mov si, offset REGISTER_AX_NAME
	mov bx, [bp + 16]
	call DisplayRegister

	mov al, " "
	mov es:[di], ax
	add di, 2

	mov si, offset REGISTER_BX_NAME
	mov bx, [bp + 12]
	call DisplayRegister

	pop di
	add di, 2 * W_WIDTH
	push di

	mov si, offset REGISTER_CX_NAME
	mov bx, [bp + 10]
	call DisplayRegister

	mov al, " "
	mov es:[di], ax
	add di, 2

	mov si, offset REGISTER_DX_NAME
	mov bx, [bp + 8]
	call DisplayRegister

	pop di
	add di, 2 * W_WIDTH
	push di

	mov si, offset REGISTER_SI_NAME
	mov bx, [bp + 6]
	call DisplayRegister

	mov al, " "
	mov es:[di], ax
	add di, 2

	mov si, offset REGISTER_DI_NAME
	mov bx, [bp + 4]
	call DisplayRegister

	pop di
	add di, 2 * W_WIDTH
	push di

	mov si, offset REGISTER_ES_NAME
	mov bx, [bp + 2]
	call DisplayRegister

	mov al, " "
	mov es:[di], ax
	add di, 2

	mov si, offset REGISTER_DS_NAME
	mov bx, [bp + 0]
	call DisplayRegister

	pop di
	add di, 2 * W_WIDTH
	push di


	mov si, offset REGISTER_BP_NAME
	mov bx, [bp + 18]
	call DisplayRegister	

	pop di
	add di, 2 * W_WIDTH
	push di

	mov si, offset REGISTER_CS_NAME
	mov bx, [bp + 24]
	call DisplayRegister

	mov al, " "
	mov es:[di], ax
	add di, 2

	mov si, offset REGISTER_IP_NAME
	mov bx, [bp + 22]
	call DisplayRegister

	pop di
	add di, 2 * W_WIDTH
	push di

	mov si, offset REGISTER_FLAGS_NAME
	mov bx, [bp + 26]
	call DisplayRegister

	pop di
	add di, 2 * W_WIDTH
	push di
	pop di

	ret
endp

REGISTER_AX_NAME: db "ax",0
REGISTER_BX_NAME: db "bx",0
REGISTER_CX_NAME: db "cx",0
REGISTER_DX_NAME: db "dx",0
REGISTER_SI_NAME: db "si",0
REGISTER_DI_NAME: db "di",0
REGISTER_ES_NAME: db "es",0
REGISTER_DS_NAME: db "ds",0
REGISTER_BP_NAME: db "bp",0
REGISTER_CS_NAME: db "cs",0
REGISTER_IP_NAME: db "ip",0
REGISTER_FLAGS_NAME: db "flags",0

UWU: db "FUCK YOU 1$"
UWU2: db "FUCK YOU 2$"

;------------------------------------------------------
; Displays register named in DS:[SI] (in ASCII, escaped by \0)
; with value in BX to ES:DI in VRAM ASCII
;
; Entry: AH - color, SI, BX, DI, ES, DS
; Destroys:	DI - shifted at the end of written register
;		SI - shifted to \0
;		BX, CX, AL
;------------------------------------------------------
DisplayRegister proc

@@print_reg_name:
	mov al, ds:[si]
	test al, al
	jz @@print_eq_sign

	mov es:[di], ax
	add di, 2
	inc si
	jmp @@print_reg_name

@@print_eq_sign:
	mov al, "="
	mov es:[di], ax
	add di, 2

	call DisplayHexWord

	ret
endp


;------------------------------------------------------
; Parses args
;
; Destroys: DX, SI
;------------------------------------------------------
ParseArgs proc
	mov dl, cs:[80h]
	mov si, 81h

	test dx, dx
	jz @@exit_func

	cmp byte ptr cs:[si], 2fh
	jne @@get_string

	push si
	push dx
@@get_string:

	cmp byte ptr cs:[si], 20h
	jne @@no_esc_space

	dec dx
	inc si

@@no_esc_space:
@@exit_func:
	mov byte ptr cs:[offset ARGS_STRING_LEN], dl
	mov word ptr cs:[offset ARGS_STRING_PTR], si

	ret
endp

;------------------------------------------------------
; Fills the Border. Uses SI as index of start,
; BX as number of cols, CX as number of rows.
; Automatically makes gaps and new lines.
;
; Border Color is set by AH
; 
; Entry: SI, BX, CX, AH
; Expects: ES = 0b800h, Window of W_HEIGHT x W_WIDTH dims
; Destroys: AL, BX, CX, SI, DX
;------------------------------------------------------
FillBorder	proc	
	push cx			; nrows
	push bx			; ncols


; fill top-left angle
	mov al, byte ptr cs:[offset DEFAULT_BORDER + 0]
	mov es:[si], ax

	add si, 2d

; fill top row
	mov al, byte ptr cs:[offset DEFAULT_BORDER + 1]

	sub bx, 2
	shl bx, 1
	add bx, si
@@fill_top_row_loop:
	mov es:[si], ax

	add si, 2
	cmp si, bx
	jl @@fill_top_row_loop

; fill top-right angle
	mov al, byte ptr cs:[offset DEFAULT_BORDER + 2]
	mov es:[si], ax

; fill right column
	mov al, byte ptr cs:[offset DEFAULT_BORDER + 3]

	sub cx, 2

	mov bx, ax
	mov ax, cx
	mov dx, 2 * W_WIDTH
	mul dx
	mov cx, ax
	mov ax, bx

	add cx, si

@@fill_right_col_loop:
	add si, 2 * W_WIDTH
	mov es:[si], ax

	cmp si, cx
	jl @@fill_right_col_loop

; fill bottom-right angle
	mov al, byte ptr cs:[offset DEFAULT_BORDER + 4]
	add si, 2 * W_WIDTH
	mov es:[si], ax

; fill bottom row
	mov al, byte ptr cs:[offset DEFAULT_BORDER + 5]
	sub si, 2

	pop bx

	sub bx, 2
	shl bx, 1

	; bx = si - bx = - (bx - si)
	sub bx, si
	neg bx

@@fill_bottom_row_loop:
	mov es:[si], ax

	sub si, 2
	cmp si, bx
	jg @@fill_bottom_row_loop
	
; fill bottom-left angle	
 	mov al, byte ptr cs:[offset DEFAULT_BORDER + 6]
	mov es:[si], ax

; fill left column
	mov al, byte ptr cs:[offset DEFAULT_BORDER + 7]

	pop cx

	sub cx, 2
	mov bx, ax
	mov ax, cx
	mov dx, 2 * W_WIDTH
	mul dx
	mov cx, ax
	mov ax, bx

	; cx = si - cx = - (cx - si)
	sub cx, si
	neg cx

@@fill_left_col_loop:
	sub si, 2 * W_WIDTH
	mov es:[si], ax

	cmp si, cx
	jg @@fill_left_col_loop

	ret
endp

;------------------------------------------------------
; Fill the area in rectangle with FILLER_SYM
; Starting from es:[di], bx is ncols, cx is nrows
;------------------------------------------------------
CleanRectangle	proc
@@fill_rowwise:	
	mov dx, cx
	mov cx, bx
	mov si, di

@@fill_colwise:
	mov es:[di], FILLER_SYM
	add di, 2
	loop @@fill_colwise

	mov di, si
	add di, 2 * W_WIDTH

	mov cx, dx
	loop @@fill_rowwise

	ret
endp

;------------------------------------------------------
; Write DX characters from ds:[SI]
; To the box with BX cols, rows are unlimited (may overflow)
; Starting from es:[DI]
; 
; Assumes window width is 80
;
; Text color is set by AH
; 
; Entry: AH, SI, BX, DI, DS
; Expects: ES = 0b800h
; Destroys: AL, SI, DX, DI, CX
;------------------------------------------------------
WriteCenteredText	proc
	push bp
	mov bp, sp

	test dx, dx
	jz @@exit

	push dx

	; if number of symbols is more than ncols,
	; write capped
@@write_capped:
	cmp dx, bx
	jl @@write_remainded
	
	push dx
	push di

	mov dx, bx
	add dx, si

@@wcap_loop:
	mov al, ds:[si]
	mov es:[di], ax

	inc si
	add di, 2
	cmp si, dx
	jl @@wcap_loop


	pop di
	add di, 2 * W_WIDTH

	pop dx
	sub dx, bx
	jmp @@write_capped


	add dx, si


@@write_remainded:
	test dx, dx
	jz @@exit

	mov cx, bx
	sub cx, dx
	
	shr cx, 1
	; multiply by 2 because of vram symbol size
	shl cx, 1

	add di, cx
	add dx, si

@@wremaind_loop:
	mov al, ds:[si]
	mov es:[di], ax

	inc si
	add di, 2
	cmp si, dx
	jl @@wremaind_loop

@@exit:

	mov sp, bp
	pop bp
	ret
endp

;-----------------------------------------------------------------------------
; Copies from es:[SI + i] to es:[DI + i], i = 0...DX
; Copies byte-by-byte, so memory areas may overlap (like memmove)
; Entry: DI, SI, DX, ES
; Destroys: AX, DI, SI, DX
;-----------------------------------------------------------------------------
MemMove proc
	add dx, si

@@move_mem_up:
	mov al, es:[si]
	mov es:[di], al

	inc di
	inc si
	cmp si, dx
	jl @@move_mem_up

	ret
endp

;-----------------------------------------------------------------------------
; Fill VRAM memory space starting from SI to SI + 2*DX with AX
; Entry: SI, DX
; Destroys: DX, SI
;-----------------------------------------------------------------------------
FillVRAMSpace proc
	shl dx, 1
	add dx, si

@@fill_mem_up:
	mov es:[si], ax

	inc si
	inc si
	cmp si, dx
	jl @@fill_mem_up

	ret
endp

;------------------------------------------------------
; Draws 2-byte number from BX to es:DI in hex-vram format
;
; Entry: AH - color, ES, BX - number, DI - destination ptr
; Destroys: AL, BX, CX
;		DI - shifted to the end of number
;------------------------------------------------------
DisplayHexWord	proc
	mov cx, 3d
@@phn_loop_ror:
	ror bx, 4d
	loop @@phn_loop_ror

	mov cx, 4d
@@phn_loop_pr_dg:
	mov al, bl
	call DrawHexDigit
	add di, 2d
	rol bx, 4d
	loop @@phn_loop_pr_dg

	ret

endp

;------------------------------------------------------
; Draws the lowest 4-bit __nibble__ of AL to 
; to es:DI in hex-vram format
;
; Entry: AH - color, AL
; Destroys: AL
;------------------------------------------------------
DrawHexDigit	proc
	and al, 0fh

	cmp al, 10d
	jl @@phd_write_hex_dg
	add al, 39d

@@phd_write_hex_dg:
	add al, '0'
	mov es:[di], ax

	ret
endp

DEFAULT_BORDER: db 0c9h, 0cdh, 0bbh, 0bah, 0bch, 0cdh, 0c8h, 0bah
ARGS_STRING_LEN: db 0h
ARGS_STRING_PTR: dw 81h

;--------------------------------
; Saves all the program registers except CS and SS + FLAGS
; to stack. The registers may be loaded via LoadProgramState
;
; Destroys: SP - it is shifted
; BP - not destroyed itself, but is unusable after operation
;--------------------------------
SaveProgramState proc
	push bp
	mov bp, sp
	push ax
	mov ax, [bp + 2] ; ret address here

	; stack is: ret, bp, ax

	push ax

	; stack is: ret, bp, ax, ret

	mov ax, [bp]
	mov [bp + 2], ax
	mov ax, [bp - 2]
	mov [bp], ax

	; stack is: bp, ax, ax, ret

	pop ax
	add sp, 2

	; stack is: bp, ax

	pushf
	push bx
	push cx
	push dx
	push si
	push di
	push es
	push ds

	; stack is filled

	push ax ; ret on top of stack

	mov ax, [bp]
	mov bp, [bp + 2]

	ret
endp

;--------------------------------
; Loads program state back saved by SaveProgramState
; to registers
;
; Entry: A valid SP, pointing to the SaveProgramState result
;
; Destroys: SP - it is shifted,
; Every register except CS and SS
;--------------------------------
LoadProgramState proc
	mov bp, sp

	; skip ret. Basically, this is add sp, 2
	pop ax	    

	; get registers
	pop ds
	pop es
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	popf

	mov ax, [bp]
	push ax
	; stack is : bp, ax, ret
	mov bp, sp

	mov ax, [bp + 2]
	push ax
	mov ax, [bp + 4]
	push ax
	mov ax, [bp]
	mov [bp + 4], ax

	pop bp
	pop ax
	; stack is: ret, ax, ret

	add sp, 4

	ret
endp

;------------------------------------------------------
; Prints 2-byte number from di in hex to console
; Puts \r\n at the end
;
; Entry: DI
; Destroys: AX, DX, CX, DI
;------------------------------------------------------
PrintHexNumberNL proc
	call PrintHexNumber

	mov ah, 02h
	mov dl, 0dh
	int 21h

	mov ah, 02h
	mov dl, 0ah
	int 21h

	ret
endp

;------------------------------------------------------
; Prints 2-byte number from di in hex to console
;
; Entry: DI
; Destroys: AX, DX, CX, DI
;------------------------------------------------------
PrintHexNumber	proc
	mov ax, di

	mov cx, 3d
@@phn_loop_ror:
	ror ax, 4d
	loop @@phn_loop_ror

	mov cx, 4d
@@phn_loop_pr_dg:
	mov di, ax
	push ax
	call PrintHexDigit
	pop ax
	rol ax, 4d
	loop @@phn_loop_pr_dg

	ret

endp

;------------------------------------------------------
; Prints the lowest 4-bit __nibble__ of DI to console
;
; Entry: DI
; Destroys: AX, DX
;------------------------------------------------------
PrintHexDigit	proc
	mov dx, di
	and dl, 0fh

	cmp dl, 10d
	jl @@phd_write_hex_dg
	add dl, 39d

@@phd_write_hex_dg:
	add dl, '0'
	mov ah, 02h
	int 21h

	ret
endp

EOPPP:

end Start

