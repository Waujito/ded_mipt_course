.model tiny

locals @@

W_HEIGHT	equ 25
W_WIDTH		equ 80

; FILLER_SYM	equ 0504h ; Pink heart, used for debugging.
FILLER_SYM	equ 1700h ; Blank symbol with default white font on black border

BORDER_COLOR	equ 1bh
TEXT_COLOR	equ 12h

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
	and ah, 7fh

	cmp ah, 3ch
	je @@handle_key

	pop ax
	jmp DOS_INT9_ORIG_FJMP

@@handle_key:
	cmp al, 0bch
	jne @@exit_handler

	mov al, byte ptr cs:[offset DO_SHOW_POPUP]
	xor al, 01h
	mov byte ptr cs:[offset DO_SHOW_POPUP], al
	
	pop ax
	call ProgTimerHandler
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

;---------------------------------
; Implements a custom logic for key handling.
; Works only for F2.
;
; Entry: AL - a keycode
; Destroys: AX
;---------------------------------
ProgTimerHandler proc
	call SaveProgramState
	mov bp, sp

	push cs
	pop ds

	call DrawAllRegisters

	mov sp, bp
	call LoadProgramState	

	ret
endp

DrawAllRegisters proc
	push 0b800h
	pop es

	mov bx, 20
	mov cx, 9
	mov ah, BORDER_COLOR
	mov si, 2 * W_WIDTH

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
	add di, 2 * W_WIDTH + 2
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

	mov si, offset REGISTER_FLAGS_NAME
	mov bx, [bp + 14]
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
REGISTER_BP_NAME: db "bp",0
REGISTER_ES_NAME: db "es",0
REGISTER_DS_NAME: db "ds",0
REGISTER_FLAGS_NAME: db "flags",0

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



;------------------------------------------------------
; Shifts previous text in video buffer to n rows up so the new data may be printed out
;
; Entry: DI = n
; Returns: AX = relative pointer to the first symbol of the filled area
;	The valid area for new information will be [AX; AX + n * W_WIDTH]
; Expects: ES = 0b800h
; Destroys: BX, DX, SI, CX
;------------------------------------------------------
ShiftText	proc

		; prologue
		push bp
		mov bp, sp

		push di				; nrows: bp - 2

		; Exchange cursor position
		mov ah, 03h
		xor bh, bh
		int 10h

		; dh is set to cursor row-pos (0-based)
		; Move it to bx
		mov bl, dh
		xor bh, bh
		push bx				; cursor row pos: bp - 4

		; dx = W_HEIGHT - n_rows - n_filled
		mov dx, W_HEIGHT
		sub dx, di
		sub dx, bx
		dec dx
		neg dx
		push dx				; Offset position: bp - 6

		cmp dx, 0
		jg @@shift_existing_text

		mov ax, 2 * W_WIDTH
		mul bx
		mov si, ax
		push si				; Write starting position: bp - 8

		jmp @@fill_free_space

@@shift_existing_text:
		mov ax, 2 * W_WIDTH
		mul di
		mov dx, 2 * W_WIDTH * (W_HEIGHT - 1)
		sub dx, ax
		push dx				; Write starting position: bp - 8

		xor di, di

		mov ax, 2 * W_WIDTH
		mov dx, [bp - 6]
		mul dx
		mov si, ax

		mov dx, 2 * W_WIDTH * W_HEIGHT
		sub dx, si
	
		; Copies from es:[SI + i] to es:[DI + i], i = 0...DX
		call MemMove

@@fill_free_space:
		mov di, [bp - 2]
		mov si, [bp - 8]

		mov ax, W_WIDTH
		mul di
		mov dx, ax

		; Fill VRAM memory space starting from SI to SI + 2*DX with AX
		mov ax, FILLER_SYM
		call FillVRAMSpace

@@shift_cursor:
		mov dx, [bp - 6]
 		cmp dx, 0
		jg @@shift_cursor_down
 
 		mov ah, 02h

 		xor bh, bh

		; old cursor pos + nrows to dh
 		mov dx, [bp - 4]
		add dx, [bp - 2]
		mov dh, dl
 		xor dl, dl
 		int 10h

		jmp @@exit

@@shift_cursor_down:
 		mov ah, 02h
		xor bh, bh
 		xor dl, dl
		mov dh, (W_HEIGHT - 1)
 		int 10h

@@exit:
		mov ax, [bp - 8]

		; epilogue
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
	call PrintHexDigit
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
PrintHexDigit	proc
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

EOPPP:

end Start

