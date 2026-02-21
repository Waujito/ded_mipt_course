.model tiny

locals @@

W_HEIGHT	equ 25
W_WIDTH		equ 80
FILLER_SYM	equ 0504h ; Pink heart, used for debugging. Replace with 0000 to clear the area

BORDER_COLOR equ 0bh

.code
org 100h

Start		proc
		; ES to vram
		mov ax, 0b800h
		mov es, ax

		mov di, 8
		call ShiftText

		mov si, ax
		add si, 10
		mov bx, 50
		mov cx, 8
		call FillBorder

; 		; Starting of row-wised width
; 		add di, 2 * 2 * W_WIDTH + 2 * 2
; 		mov bx, si
; 		sub bx, 4
; 		shl bx, 1
; 		add bx, di
; 		mov si, bx
; 
; 		mov dl, ds:[80h]
; 
; 		test dl, dl
; 		jz .pw_loop_end
; 
; 		dec dl
; 		
; 		push di
; 		push di
; 		xor cx, cx
; .pw_loop_start:
; 		cmp cx, dx
; 		jge .pw_loop_end
; 
; 		mov bx, cx
; 		mov al, ds:[bx + 82h]
; 		mov ah, 0ceh
; 
; 		mov es:[di], ax
; 		add di, 2
; 
; 		cmp di, si
; 		jle .pw_loop_no_act
; 
; 		pop di
; 		add di, 2 * W_WIDTH
; 		push di
; 		add si, 2 * W_WIDTH
; 	
; .pw_loop_no_act:
; 
; 
; 		inc cx
; 		jmp .pw_loop_start
; .pw_loop_end:
; 
; 		pop di
; 		pop di



		mov ax, 4c00h
		int 21h
endp

;------------------------------------------------------
; Fills the Border. Uses SI as index of start,
; BX as number of rows, CX as number of cols.
; Automatically makes gaps and new lines.
; 
; Entry: SI, BX, CX
; Expects: ES = 0b800h, Window of W_HEIGHT x W_WIDTH dims
; Destroys: AX, BX, CX, SI, DX
;------------------------------------------------------
FillBorder	proc	
	push cx			; ncols		[bp - 2]
	push bx			; nrows		[bp - 4]

	mov ah, BORDER_COLOR

; fill top-left angle
	mov al, 0c9h
	mov es:[si], ax

	add si, 2d

; fill top row
	mov al, 0cdh

	sub bx, 2
	shl bx, 1
	add bx, si
@@fill_top_row_loop:
	mov es:[si], ax

	add si, 2
	cmp si, bx
	jl @@fill_top_row_loop

; fill top-right angle
	mov al, 0bbh
	mov es:[si], ax

; fill right column
	mov al, 0bah

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
	mov al, 0bch
	add si, 2 * W_WIDTH
	mov es:[si], ax

; fill bottom row
	mov al, 0cdh
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
 	mov al, 0c8h	
	mov es:[si], ax

; fill left column
	mov al, 0bah

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

		jmp @@exit_func

@@shift_cursor_down:
 		mov ah, 02h
		xor bh, bh
 		xor dl, dl
		mov dh, (W_HEIGHT - 1)
 		int 10h

@@exit_func:	
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

end Start
