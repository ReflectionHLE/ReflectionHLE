; WOLFHACK.ASM

.386C
IDEAL
MODEL	MEDIUM,C


;============================================================================

DATASEG

EXTRN	mr_rowofs:WORD
EXTRN	mr_count:WORD
EXTRN	mr_xstep:WORD
EXTRN	mr_ystep:WORD
EXTRN	mr_xfrac:WORD
EXTRN	mr_yfrac:WORD
EXTRN	mr_dest:WORD

FARDATA

planepics	db	8192 dup(?)	;	// 4k of ceiling, 4k of floor
PUBLIC	planepics
; *** S3DNA RESTORATION ***
IFDEF GAMEVER_RESTORATION_N3D_WIS10
lc_xstep	dw	?
lc_ystep	dw	?
lc_cxfrac	dw	?
lc_cyfrac	dw	?
ENDIF


;============================================================================

;============================
;
; MapRow
;
;
;============================

CODESEG

; *** S3DNA RESTORATION ***
; A modified implementation
IFDEF GAMEVER_RESTORATION_N3D_WIS10

PROC	MapRow
PUBLIC	MapRow
	push	si
	push	di
	push	bp
	push	ds

	mov		ax,0a000h
	mov		es,ax
	mov	bp,[mr_rowofs]
	mov		cx,[mr_count]
	mov		di,[mr_dest]
	push	cx
	mov		ax,[mr_ystep]
	mov		bx,[mr_xstep]
	mov		cx,[mr_yfrac]
	mov		dx,[mr_xfrac]
	push	seg planepics
	pop	ds
	mov		[ds:lc_ystep],ax
	mov		[ds:lc_xstep],bx
	mov		[ds:lc_cyfrac],cx
	mov		[ds:lc_cxfrac],dx
	pop	cx

pixelloop:
	mov		bx,[ds:lc_cxfrac]
	shr		bx,3
	and		bx,63*64*2
	mov		ax,[ds:lc_cyfrac]
	shr		ax,9
	or		bx,ax
	and		bx,63*65*2
	mov		ax,[ds:lc_xstep]
	add		[ds:lc_cxfrac],ax
	mov		ax,[ds:lc_ystep]
	adc		[ds:lc_cyfrac],ax
	mov		al,[bx]
	mov	es:[di],al
	mov		al,[bx+1]
	mov	es:[di+bp],al
	inc		di
	loop	pixelloop

	pop		ds
	pop		bp
	pop		di
	pop		si

	retf

ENDP

ELSE

PROC	MapRow
PUBLIC	MapRow
	push	esi
	push	edi
	push	ebp
	push 	ds

	mov     bp,[mr_rowofs]
	mov		cx,[mr_count]
	mov		dx,[mr_ystep]
	shl		edx,16
	mov		dx,[mr_xstep]
	mov		si,[mr_yfrac]
	shl		esi,16
	mov		si,[mr_xfrac]
	mov		di,[mr_dest]
	mov		ax,SEG planepics
	mov		ds,ax
	mov		ax,0a000h
	mov		es,ax
	mov		ax,1111111111110b

; eax		color lookup
; ebx		scratch offset and pixel values
; ecx     	loop counter
; edx		packed x / y step values
; esi		packed x / y fractional values
; edi		write pointer
; ebp		toprow to bottomrow delta
; es:		screenseg
; ds:		pictures

; mov	al,[esi]
; mov	al,[eax]
; mov	[edi],al

; mov   ax,[_variable+ebx+2]


pixelloop:
	shld	ebx,esi,22				; shift y units in
	shld	ebx,esi,7				; shift x units in and one extra bit
	and		bx,63*65*2				; mask off extra top bits and 0 low bit
	add		esi,edx					; position += step
	mov		al,[bx]
	mov		al,[eax]
	mov     [es:di],al     			; write ceiling pixel
	mov		al,[bx+1]
	mov		al,[eax]
	mov     [es:di+bp],al     		; write floor pixel
	inc		di
	loop	pixelloop

	pop		ds
	pop		ebp
	pop		edi
	pop		esi

	retf

ENDP

ENDIF ; S3DNA RESTORATION

END

