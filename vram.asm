.model tiny

locals @@

W_HEIGHT	equ 25
W_WIDTH		equ 80
; FILLER_SYM	equ 0504h ; Pink heart, used for debugging. Replace with 0000 to clear the area
FILLER_SYM	equ 0700h ; Blank symbol with default white font on black border

BORDER_COLOR equ 0bh
TEXT_COLOR equ 02h

.code
org 100h

Start		proc

		mov bp, sp

		; ES to vram
		mov ax, 0b800h
		mov es, ax

		mov di, 8
		call ShiftText


		mov bx, 32
		mov cx, 8

		mov si, ax
		mov ax, W_WIDTH
		sub ax, bx
		shr ax, 1
		shl ax, 1
		add si, ax

		push si			; starting of border [bp - 2]
		push bx			; ncols	 [bp - 4]
		push cx			; nrows	 [bp - 6]

		mov ah, BORDER_COLOR

		call FillBorder

		mov bx, [bp - 4]
		mov cx, [bp - 6]

		sub bx, 4
		sub cx, 4

 		mov dl, cs:[80h]
		push cs
		pop ds
		mov si, 81h

		mov di, [bp - 2]
		add di, 2 * 2 * W_WIDTH + 2 * 2

		test dx, dx
		jz @@no_write_text

		cmp byte ptr ds:[81h], 20h
		jne @@no_esc_space

		dec dx
		inc si
@@no_esc_space:

		mov ah, TEXT_COLOR
		call WriteCenteredText
@@no_write_text:

		; mov di, [bp - 2]
		; mov bx, [bp - 4]
		; mov cx, [bp - 6]

		; sub bx, 2
		; sub cx, 2
		; add di, 2 * W_WIDTH + 2

		; call CleanRectangle

		mov ax, 4c00h
		int 21h
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

DEFAULT_BORDER: db 0c9h, 0cdh, 0bbh, 0bah, 0bch, 0cdh, 0c8h, 0bah

end Start
