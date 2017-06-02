; ID_VL.ASM

	IDEAL
	MODEL	MEDIUM,C

	; *** SHAREWARE V1.0 APOGEE RESTORATION ***
	INCLUDE 'VERSION.EQU'

	INCLUDE	'ID_VL.EQU'

SCREENSEG	=	0a000h

; *** SHAREWARE V1.0 APOGEE RESTORATION ***
IF GAMEVER_WOLFREV LE 19920505
UPDATEWIDE	=	22
UPDATEHIGH	=	14
ELSE
UPDATEWIDE	=	20
UPDATEHIGH	=	13
ENDIF

	DATASEG


EXTRN	bufferofs			:WORD
EXTRN	displayofs			:WORD
EXTRN	ylookup				:WORD
EXTRN	linewidth			:WORD
EXTRN	blockstarts			:WORD	;offsets from drawofs for each update block

EXTRN	update				:BYTE

	CODESEG


;=================
;
; VH_UpdateScreen
;
;=================

PROC	VH_UpdateScreen
PUBLIC	VH_UpdateScreen
USES	si,di

; *** ALPHA RESTORATION ***
IF GAMEVER_WOLFREV LE 19920312
	mov	dx,SC_INDEX+1
	mov	al,15
	out	dx,al
ELSE
	mov	dx,SC_INDEX
	mov	ax,SC_MAPMASK+15*256
	out	dx,ax
ENDIF

	mov dx,GC_INDEX
	mov al,GC_MODE
	out dx,al

	inc dx
	in	al,dx
	and al,252
	or	al,1
	out dx,al

; *** SHAREWARE V1.0 APOGEE RESTORATION ***
IF GAMEVER_WOLFREV LE 19920505
	mov	bx,UPDATEWIDE*(UPDATEHIGH-1)-1		; bx is the tile number
ELSE
	mov	bx,UPDATEWIDE*UPDATEHIGH-1		; bx is the tile number
ENDIF
	mov	dx,[linewidth]

;
; see if the tile needs to be copied
;
@@checktile:
	test	[update+bx],1
	jnz	@@copytile
@@next:
	dec	bx
	jns	@@checktile

;
; done
;
	mov dx,GC_INDEX+1
	in	al,dx
	and al,NOT 3
	or	al,0
	out dx,al
	ret

;
; copy a tile
;
@@copytile:
	mov	[update+bx],0
	shl	bx,1
	mov	si,[blockstarts+bx]
	shr	bx,1
	mov	di,si
	add	si,[bufferofs]
	add	di,[displayofs]

	mov	ax,SCREENSEG
	mov	ds,ax

REPT	16
	mov	al,[si]
	mov	[di],al
	mov	al,[si+1]
	mov	[di+1],al
	mov	al,[si+2]
	mov	[di+2],al
	mov	al,[si+3]
	mov	[di+3],al
	add	si,dx
	add	di,dx
ENDM

	mov	ax,ss
	mov	ds,ax
	jmp	@@next

ENDP


; *** SHAREWARE V1.0 APOGEE + ALPHA RESTORATION ***
; A copy-paste of VH_UpdateScreen for most

IF GAMEVER_WOLFREV LE 19920505
IF GAMEVER_WOLFREV GT 19920312

;=================
;
; VH_UpdateSplitScreen
;
;=================

PROC	VH_UpdateSplitScreen
PUBLIC	VH_UpdateSplitScreen
USES	si,di

	mov	dx,SC_INDEX
	mov	ax,SC_MAPMASK+15*256
	out	dx,ax

	mov dx,GC_INDEX
	mov al,GC_MODE
	out dx,al

	inc dx
	in	al,dx
	and al,252
	or	al,1
	out dx,al

	mov	bx,218-1		; bx is the tile number
	mov	dx,[linewidth]

;
; see if the tile needs to be copied
;
@@checktile:
	test	[update+bx],1
	jnz	@@copytile
@@next:
	dec	bx
	jns	@@checktile

;
; done
;
	mov dx,GC_INDEX+1
	in	al,dx
	and al,NOT 3
	or	al,0
	out dx,al
	ret

;
; copy a tile
;
@@copytile:
	mov	[update+bx],0
	shl	bx,1
	mov	si,[blockstarts+bx]
	shr	bx,1
	mov	di,si
	add	si,[bufferofs]
	add	di,[displayofs]

	mov	ax,SCREENSEG
	mov	ds,ax

REPT	16
	mov	al,[si]
	mov	[di],al
	mov	al,[si+1]
	mov	[di+1],al
	mov	al,[si+2]
	mov	[di+2],al
	mov	al,[si+3]
	mov	[di+3],al
	add	si,dx
	add	di,dx
ENDM

	mov	ax,ss
	mov	ds,ax
	jmp	@@next

ENDP

ENDIF ; GAMEVER_WOLFREV GT 19920312
ENDIF ; GAMEVER_WOLFREV LE 19920505


	END

