;THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
;SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
;END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
;ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
;IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
;SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
;FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
;CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
;AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.  
;COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
;
; $Source: f:/miner/source/texmap/rcs/tmap_mul.asm $
; $Revision: 1.4 $
; $Author: mike $
; $Date: 1994/11/30 00:56:56 $
; 
; Inner loop of 2 multiplies per 8 pixels texture mapper.
; 
; $Log: tmap_mul.asm $
; Revision 1.4  1994/11/30  00:56:56  mike
; optimization.
; 
; Revision 1.3  1994/11/12  16:41:00  mike
; jae -> ja.
; 
; Revision 1.2  1994/11/02  21:33:33  john
; Added Burger Bill's optimization, ie.. 2 muls per 8 pixels.
; 
; Revision 1.1  1994/11/02  20:57:36  john
; Initial revision
; 
; 

	.386

	public	asm_tmap_scanline_mul_

	extrn	_divide_table:dword

	include	tmap_inc.asm

	extrn	_fx_u:dword
	extrn	_fx_v:dword
	extrn	_fx_z:dword
	extrn	_fx_du_dx:dword
	extrn	_fx_dv_dx:dword
	extrn	_fx_dz_dx:dword
	extrn	_fx_y:dword
	extrn	_fx_xleft:dword
	extrn	_fx_xright:dword
	extrn	_pixptr:dword
	extrn	_x:dword
	extrn	_loop_count:dword
	extern	_pixel_data_selector:word, _gr_fade_table_selector:word
	extern	_Transparency_on:dword
	extern	_fx_l:dword
	extern 	_fx_dl_dx:dword
	extern	_dither_intensity_lighting:dword
	extern	_Lighting_on:dword


_DATA	SEGMENT DWORD PUBLIC USE32 'DATA'

;---------- local variables
	align	4
req_base	dd	?
req_size	dd	?
U0	dd	?
U1	dd	?
V0	dd	?
V1	dd	?
num_left_over	dd	?
DU1	dd	?
DV1	dd	?
DZ1	dd	?

_DATA	ENDS

DGROUP	GROUP	_DATA

_TEXT   SEGMENT PARA PUBLIC USE32 'CODE'
	ASSUME	DS:_DATA
	ASSUME	CS:_TEXT

; --------------------------------------------------------------------------------------------------
; Enter:
;	_xleft	fixed point left x coordinate
;	_xright	fixed point right x coordinate
;	_y	fixed point y coordinate
;	_pixptr	address of source pixel map
;	_u	fixed point initial u coordinate
;	_v	fixed point initial v coordinate
;	_z	fixed point initial z coordinate
;	_du_dx	fixed point du/dx
;	_dv_dx	fixed point dv/dx
;	_dz_dx	fixed point dz/dx

;   for (x = (int) xleft; x <= (int) xright; x++) {
;      _setcolor(read_pixel_from_tmap(srcb,((int) (u/z)) & 63,((int) (v/z)) & 63));
;      _setpixel(x,y);
;
;      u += du_dx;
;      v += dv_dx;
;      z += dz_dx;
;   }

asm_tmap_scanline_mul_:
	push	es
	pusha
	
;---------------------------- setup for loop ---------------------------------
; Setup for loop:	_loop_count  iterations = (int) xright - (int) xleft
;	esi	source pixel pointer = pixptr
;	edi	initial row pointer = y*320+x

; set esi = pointer to start of texture map data
	mov	es,_pixel_data_selector	; selector[0*2]

; set edi = address of first pixel to modify
	mov	edi,_fx_y
	cmp	edi,_window_bottom
	ja	_none_to_do

	imul	edi,_bytes_per_row
	mov	eax,_fx_xleft
	sar	eax,16
	jns	eax_ok
	sub	eax,eax
eax_ok:
	add	edi,eax
	add	edi,write_buffer

; set _loop_count = # of iterations
	mov	eax,_fx_xright
	sar	eax,16
	cmp	eax,_window_right
	jb	eax_ok1
	mov	eax,_window_right
eax_ok1:	cmp	eax,_window_left
	ja	eax_ok2
	mov	eax,_window_left
eax_ok2:

	mov	ebx,_fx_xleft
	sar	ebx,16
	sub	eax,ebx
	js	_none_to_do
	cmp	eax,_window_width
	jbe	_ok_to_do
	mov	eax,_window_width
_ok_to_do:
	mov	_loop_count,eax

;-------------------------- setup for dithering -----------------------------
; lighting values are passed in fixed point, but need to be in 8 bit integer, 8 bit fraction so we can easily
; get the integer by reading %bh
	sar	_fx_l, 8
	sar	_fx_dl_dx,8
	jns	dl_dx_ok
	inc	_fx_dl_dx	; round towards 0 for negative deltas
dl_dx_ok:

; set initial values
	mov	ebx,_fx_u
	mov	ebp,_fx_v
	mov	ecx,_fx_z

;	test	_per2_flag,-1
;	je	tmap_loop

	;cmp	_Lighting_on, 0
	;je	tmap_loop_mul_nolight
	jmp	tmap_loop_mul

;================ PERSPECTIVE TEXTURE MAP INNER LOOPS ========================
;
; Usage in loop:	eax	division, pixel value
;	ebx	u
;	ecx	z
;	edx	division
;	ebp	v
;	esi	source pixel pointer
;	edi	destination pixel pointer

;-------------------- NORMAL PERSPECTIVE TEXTURE MAP LOOP -----------------
	align	4
tmap_loop:
	xchg	ebx, esi

; compute v coordinate
	mov	eax,ebp	; get v
	cdq
	idiv	ecx	; eax = (v/z)

	and	eax,3fh	; mask with height-1
	mov	ebx,eax

; compute u coordinate
	mov	eax,esi	; get u
	cdq
	idiv	ecx	; eax = (u/z)

	shl 	eax,26
	shld 	ebx,eax,6	; esi = v*64+u

; read 1  pixel
	movzx	eax,byte ptr es:[ebx]	; get pixel from source bitmap

	cmp	_Lighting_on, 0
	je	NoLight1

; LIGHTING CODE
	mov	ebx, _fx_l	; get temp copy of lighting value
	mov	ah, bh	; get lighting level
	add	ebx, _fx_dl_dx	; update lighting value
	mov	al, _gr_fade_table[eax]	; xlat pixel thru lighting tables
	mov	_fx_l, ebx	; save temp copy of lighting value

; transparency check
NoLight1:	cmp	al,255
	je	skip1

	mov	[edi],al
skip1:	inc	edi
	
; update deltas
	add	ebp,_fx_dv_dx
	add	esi,_fx_du_dx
	add	ecx,_fx_dz_dx
	je	_div_0_abort	; would be dividing by 0, so abort

	xchg	esi, ebx

	dec	_loop_count
	jns	tmap_loop

_none_to_do:	
	popa
	pop	es
	ret

; We detected a z=0 condition, which seems pretty bogus, don't you think?
; So, we abort, but maybe we want to know about it.
_div_0_abort:
;**	int	3
	jmp	_none_to_do

;-------------------------- PER/4 TMAPPER ----------------
; 
;	x = x1
;	U0 = u/w; V0 = v/w;
;	while ( 1 )
;		u += du_dx*4; v+= dv_dx*4
;		U1 = u/w; V1 = v/w;
;		DUDX = (U1-U0)/4; DVDX = (V1-V0)/4;
;
;	; Pixel 0
;		pixels = texmap[V0*64+U0];
;		U0 += DUDX; V0 += DVDX
;	; Pixel 1
;		pixels = (pixels<<8)+texmap[V0*64+U0];
;		U0 += DUDX; V0 += DVDX
;	; Pixel 2
;		pixels = (pixels<<8)+texmap[V0*64+U0];
;		U0 += DUDX; V0 += DVDX
;	; Pixel 3
;		pixels = (pixels<<8)+texmap[V0*64+U0];
;
;		screen[x] = pixel
;		x += 4;
;		U0 = U1; V0 = V1 

NBITS = 3
ZSHIFT = 3

DIVIDE_SIG_BITS	  = 12
DIVIDE_SHIFT_BITS = (20-DIVIDE_SIG_BITS)	       
DIVIDE_TABLE_SIZE = (1 SHL DIVIDE_SIG_BITS)

PDIV_START MACRO
	push	ecx
	shr	ecx, DIVIDE_SHIFT_BITS
	and	ecx, DIVIDE_TABLE_SIZE-1
	mov	cx, word ptr _divide_table[ecx*2];
ENDM

PDIV_END MACRO
	pop	ecx
ENDM

PDIV MACRO
; Returns EAX/ECX in 16.16 format in EAX. Trashes EDX
;          sig bits   6.3
;--old--	cdq
;--old--	shld	edx,eax, ZSHIFT
;--old--	lea	eax, [eax*8]
;--old--	idiv	ecx	; eax = (v/z)
;--old--	shl	eax, 16-ZSHIFT

;	mov	edx,eax
;	shl	eax,ZSHIFT
;	sar	edx,32-ZSHIFT
;	idiv	ecx	; eax = (v/z)
;	shl	eax, 16-ZSHIFT

	mov	edx,eax
	shl	eax,ZSHIFT
	sar	edx,32-ZSHIFT
	imul	ecx	; eax = (v*z)
	shl	edx, 16-ZSHIFT
	mov	eax, edx
ENDM

wr_onepix	macro	num
	local	skip
	cmp	cl,255
	je	skip
	mov	num[edi],cl
	ror	ecx,8
skip:
	endm

public tmap_loop_mul

; -------------------------------------- Start of Getting Dword Aligned ----------------------------------------------
;	ebx	fx_u

tmap_loop_mul:
	mov	esi,ebx

	align	4
NotDwordAligned1:
	test	edi, 11b
	jz	DwordAligned1

; compute v coordinate
	mov	eax,ebp	; get v
	cdq
;	idiv	ecx	; eax = (v/z)

	push	ecx
	shr	ecx, DIVIDE_SHIFT_BITS
	and	ecx, DIVIDE_TABLE_SIZE-1
	mov	cx, word ptr _divide_table[ecx*2];
	imul	ecx
	mov	eax, edx

	and	eax,3fh	; mask with height-1
	mov	ebx,eax

; compute u coordinate
	mov	eax,esi	; get u
	cdq
;	idiv	ecx	; eax = (u/z)
	imul	ecx
	mov	eax, edx
	pop	ecx

	shl 	eax,26
	shld 	ebx,eax,6	; esi = v*64+u

; read 1  pixel
	movzx	eax,byte ptr es:[ebx]	; get pixel from source bitmap

; lighting code
	mov	ebx, _fx_l	; get temp copy of lighting value
	mov	ah, bh	; get lighting level
	add	ebx, _fx_dl_dx	; update lighting value
	mov	_fx_l, ebx	; save temp copy of lighting value

; transparency check
	cmp	al,255
	je	skip2	; this pixel is transparent, so don't write it (or light it)

	mov	al, _gr_fade_table[eax]	; xlat pixel thru lighting tables
					
; write 1 pixel
	mov	[edi],al
skip2:	inc	edi
	
; update deltas
	add	ebp,_fx_dv_dx
	add	esi,_fx_du_dx
	add	ecx,_fx_dz_dx
	je	_div_0_abort	; would be dividing by 0, so abort

	dec	_loop_count
	jns	NotDwordAligned1
	jmp	_none_to_do

; -------------------------------------- End of Getting Dword Aligned ----------------------------------------------

DwordAligned1:
	mov	ebx,esi	; get fx_u

	mov	eax, _loop_count
	inc	eax
	mov	num_left_over, eax
	shr	eax, NBITS

	cmp	eax, 0		
	je	tmap_loop
	
	mov 	_loop_count, eax	; _loop_count = pixels / NPIXS
	shl	eax, 3
	sub	num_left_over, eax	; num_left_over = obvious
		
; compute initial v coordinate
	mov	eax,ebp	; get v
;	mov	edx,ebp
;	shl	eax,ZSHIFT
;	sar	edx,32-ZSHIFT
;	idiv	ecx	; eax = (v/z)
;	shl	eax, 16-ZSHIFT
	PDIV_START
	PDIV
	mov	V0, eax

; compute initial u coordinate
	mov	eax,ebx	; get u
;	mov	edx,ebx
;	shl	eax,ZSHIFT
;	sar	edx,32-ZSHIFT
;	idiv	ecx	; eax = (v/z)
;	shl	eax, 16-ZSHIFT
	PDIV
	PDIV_END
	mov	U0, eax

; Set deltas to NPIXS pixel increments
	mov	eax, _fx_du_dx
	shl	eax, 3
	mov	DU1, eax
	mov	eax, _fx_dv_dx
	shl	eax, 3
	mov	DV1, eax
	mov	eax, _fx_dz_dx
	shl	eax, 3
	mov	DZ1, eax

	align	4
TopOfLoop4:
	add	ebx, DU1
	add	ebp, DV1
	add	ecx, DZ1
	je	_div_0_abort	; would be dividing by 0, so abort

; Done with ebx, ebp, ecx until next iteration
	push	ebx
	push	ecx
	push	ebp
	push	edi
	
; Find fixed U1		
	mov	eax, ebx
;	mov	edx,ebx
;	shl	eax,ZSHIFT
;	sar	edx,32-ZSHIFT
;	idiv	ecx	; eax = (v/z)
;	shl	eax, 16-ZSHIFT
	PDIV_START
	PDIV
	mov	ebx, eax	; ebx = U1 until pop's

; Find fixed V1		
	mov	eax, ebp
;	mov	edx, ebp
;	shl	eax,ZSHIFT
;	sar	edx,32-ZSHIFT
;	idiv	ecx	; eax = (v/z)
;	shl	eax, 16-ZSHIFT
	PDIV
	PDIV_END

	mov	ecx, U0	; ecx = U0 until pop's
	mov	edi, V0	; edi = V0 until pop's

	mov	ebp, eax	; ebp = V1 until pop's

; Make ESI =  V0:U0 in 6:10,6:10 format
	mov	eax, ecx
	shr	eax, 6
	mov	esi, edi
	shl	esi, 10
	mov	si, ax
		
; Make EDX = DV:DU in 6:10,6:10 format
	mov	eax, ebx
	sub	eax, ecx
	sar	eax, NBITS+6
	mov	edx, ebp
	sub	edx, edi
	shl	edx, 10-NBITS	; EDX = V1-V0/ 4 in 6:10 int:frac
	mov	dx, ax	; put delta u in low word

; Save the U1 and V1 so we don't have to divide on the next iteration
	mov	U0, ebx
	mov	V0, ebp

	pop	edi	; Restore EDI before using it
		
; LIGHTING CODE
	mov	ebx, _fx_l
	mov	ebp, _fx_dl_dx

	test	_Transparency_on,-1
	je	no_trans1

    REPT (1 SHL (NBITS-2))
	local	skip3,no_trans1
        REPT 2
	local	skipa1,skipa2

	mov	eax, esi	; get u,v
	shr	eax, 26	; shift out all but int(v)
	shld	ax,si,6	; shift in u, shifting up v
	add	esi, edx	; inc u,v
	mov 	al, es:[eax]	; get pixel from source bitmap
	cmp	al,255
	je	skipa1
	mov	ah, bh	; form lighting table lookup value
	add	ebx, ebp	; update lighting value
	mov	al, _gr_fade_table[eax]	; xlat thru lighting table into dest buffer
	mov	[edi],al
skipa1:
	inc	edi

; Do odd pixel
	mov	eax, esi	; get u,v
	shr	eax, 26	; shift out all but int(v)
	shld	ax,si,6	; shift in u, shifting up v
	add	esi, edx	; inc u,v
	mov 	al, es:[eax]	; get pixel from source bitmap
	cmp	al,255
	je	skipa2
	mov	ah, bh	; form lighting table lookup value
	add	ebx, ebp	; update lighting value
	mov	al, _gr_fade_table[eax]	; xlat thru lighting table into dest buffer
	mov	[edi],al
skipa2:
	inc	edi

        ENDM

    ENDM
	jmp	cont1

; -------------------------------------------------------
no_trans1:
    REPT (1 SHL (NBITS-2))
	local	skip3
        REPT 2

	mov	eax, esi	; get u,v
	shr	eax, 26	; shift out all but int(v)
	shld	ax,si,6	; shift in u, shifting up v
	add	esi, edx	; inc u,v
	mov 	al, es:[eax]	; get pixel from source bitmap
	mov	ah, bh	; form lighting table lookup value
	add	ebx, ebp	; update lighting value
	mov	cl, _gr_fade_table[eax]	; xlat thru lighting table into dest buffer

; Do odd pixel
	mov	eax, esi	; get u,v
	shr	eax, 26	; shift out all but int(v)
	shld	ax,si,6	; shift in u, shifting up v
	add	esi, edx	; inc u,v
	mov 	al, es:[eax]	; get pixel from source bitmap
	mov	ah, bh	; form lighting table lookup value
	add	ebx, ebp	; update lighting value
	mov	ch, _gr_fade_table[eax]	; xlat thru lighting table into dest buffer

	ror	ecx, 16	; move to next double dest pixel position

        ENDM
	mov 	[edi],ecx	; Draw 4 pixels to display
skip3:	add 	edi,4

    ENDM

cont1:

; -------------------------------------------------------

; LIGHTING CODE
	mov	_fx_l, ebx
	pop	ebp
	pop	ecx
	pop	ebx
	dec	_loop_count
	jnz	TopOfLoop4

EndOfLoop4:
	cmp	num_left_over, 0
	je	_none_to_do

; ----------------------------------------- Start of LeftOver Pixels ------------------------------------------
DoEndPixels:
	add	ebx, DU1
	add	ebp, DV1
	add	ecx, DZ1
	je	_div_0_abort

	push	edi	; use edi as a temporary variable

	cmp	ecx,10
	jg	ecx_ok
	mov	ecx,10
ecx_ok:

; Find fixed U1		
	mov	eax, ebx
 	PDIV_START
	PDIV
	mov	ebx, eax	; ebx = U1 until pop's

; Find fixed V1		
	mov	eax, ebp
	PDIV
	PDIV_END
	mov	ebp, eax	; ebp = V1 until pop's

	mov	ecx, U0	; ecx = U0 until pop's
	mov	edi, V0	; edi = V0 until pop's

; Make ESI =  V0:U0 in 6:10,6:10 format
	mov	eax, ecx
	shr	eax, 6
	mov	esi, edi
	shl	esi, 10
	mov	si, ax
		
; Make EDX = DV:DU in 6:10,6:10 format
	mov	eax, ebx
	sub	eax, ecx
	sar	eax, NBITS+6
	mov	edx, ebp
	sub	edx, edi
	shl	edx, 10-NBITS	; EDX = V1-V0/ 4 in 6:10 int:frac
	mov	dx, ax	; put delta u in low word

	pop	edi	; Restore EDI before using it
		
	mov	ecx, num_left_over

; LIGHTING CODE
	mov	ebx, _fx_l
	mov	ebp, _fx_dl_dx

	ITERATION = 0
	REPT (1 SHL (NBITS-1))
	local	skip4, skip5
; Do even pixel
	mov	eax, esi	; get u,v
	shr	eax, 26	; shift out all but int(v)
	shld	ax,si,6	; shift in u, shifting up v
	mov 	al, es:[eax]	; get pixel from source bitmap
	add	esi, edx	; inc u,v
	mov	ah, bh	; form lighting table lookup value
	;SAFETYand	ah,0fh
	add	ebx, ebp	; update lighting value
	cmp	al,255
	je	skip4
	mov	al, _gr_fade_table[eax]	; xlat thru lighting table into dest buffer
	mov	[edi+ITERATION], al	; write pixel
skip4:	dec	ecx
	jz	_none_to_do
	ITERATION = ITERATION + 1

; Do odd pixel
	mov	eax, esi	; get u,v
	shr	eax, 26	; shift out all but int(v)
	shld	ax,si,6	; shift in u, shifting up v
	mov 	al, es:[eax]	; get pixel from source bitmap
	add	esi, edx	; inc u,v
	mov	ah, bh	; form lighting table lookup value
	add	ebx, _fx_dl_dx	; update lighting value
	cmp	al,255
	je	skip5
	;SAFETYand	ah,0fh
	mov	al, _gr_fade_table[eax]	; xlat thru lighting table into dest buffer
	mov	[edi+ITERATION], al	; write pixel
skip5:	dec	ecx
	jz	_none_to_do
	ITERATION = ITERATION + 1
	ENDM

; ----------------------------------------- End of LeftOver Pixels ------------------------------------------

; Should never get here!!!!
	int	3
	jmp	_none_to_do

; ==================================================== No Lighting Code ======================================================
public tmap_loop_mul_nolight
tmap_loop_mul_nolight:
	mov	esi,ebx

	align	4
NotDwordAligned1_nolight:
	test	edi, 11b
	jz	DwordAligned1_nolight

; compute v coordinate
	mov	eax,ebp	; get v
	cdq
	idiv	ecx	; eax = (v/z)

	and	eax,3fh	; mask with height-1
	mov	ebx,eax

; compute u coordinate
	mov	eax,esi	; get u
	cdq
	idiv	ecx	; eax = (u/z)

	shl 	eax,26
	shld 	ebx,eax,6	; esi = v*64+u

; read 1  pixel
	mov	al,es:[ebx]	; get pixel from source bitmap

; write 1 pixel
	cmp	al,255
	je	skip6
	mov	[edi],al
skip6:	inc	edi
	
; update deltas
	add	ebp,_fx_dv_dx
	add	esi,_fx_du_dx
	add	ecx,_fx_dz_dx
	je	_div_0_abort	; would be dividing by 0, so abort

	dec	_loop_count
	jns	NotDwordAligned1_nolight
	jmp	_none_to_do

DwordAligned1_nolight:
	mov	ebx,esi

	mov	eax, _loop_count
	inc	eax
	mov	num_left_over, eax
	shr	eax, NBITS

	cmp	eax, 0		
	je	tmap_loop
	
	mov 	_loop_count, eax	; _loop_count = pixels / NPIXS
	shl	eax, NBITS
	sub	num_left_over, eax	; num_left_over = obvious
		
; compute initial v coordinate
	mov	eax,ebp	; get v
	PDIV_START
	PDIV
	mov	V0, eax

; compute initial u coordinate
	mov	eax,ebx	; get u
	PDIV	
	PDIV_END
	mov	U0, eax

; Set deltas to NPIXS pixel increments
	mov	eax, _fx_du_dx
	shl	eax, NBITS
	mov	DU1, eax
	mov	eax, _fx_dv_dx
	shl	eax, NBITS
	mov	DV1, eax
	mov	eax, _fx_dz_dx
	shl	eax, NBITS
	mov	DZ1, eax

	align	4
TopOfLoop4_nolight:
	add	ebx, DU1
	add	ebp, DV1
	add	ecx, DZ1
	je	_div_0_abort

; Done with ebx, ebp, ecx until next iteration
	push	ebx
	push	ecx
	push	ebp
	push	edi
	
; Find fixed U1		
	mov	eax, ebx
	PDIV_START
	PDIV
	mov	ebx, eax	; ebx = U1 until pop's

; Find fixed V1		
	mov	eax, ebp
	PDIV
	PDIV_END
	mov	ebp, eax	; ebp = V1 until pop's

	mov	ecx, U0	; ecx = U0 until pop's
	mov	edi, V0	; edi = V0 until pop's

; Make ESI =  V0:U0 in 6:10,6:10 format
	mov	eax, ecx
	shr	eax, 6
	mov	esi, edi
	shl	esi, 10
	mov	si, ax
		
; Make EDX = DV:DU in 6:10,6:10 format
	mov	eax, ebx
	sub	eax, ecx
	sar	eax, NBITS+6
	mov	edx, ebp
	sub	edx, edi
	shl	edx, 10-NBITS	; EDX = V1-V0/ 4 in 6:10 int:frac
	mov	dx, ax	; put delta u in low word

; Save the U1 and V1 so we don't have to divide on the next iteration
	mov	U0, ebx
	mov	V0, ebp

	pop	edi	; Restore EDI before using it
		
    REPT (1 SHL (NBITS-2))
	local	skip7, no_trans2
        REPT 4	
; Do 1 pixel 
	mov	eax, esi	; get u,v
	shr	eax, 26	; shift out all but int(v)
	shld	ax,si,6	; shift in u, shifting up v
	mov	cl, es:[eax]	; load into buffer register
	add	esi, edx	; inc u,v
	ror	ecx, 8	; move to next dest pixel

        ENDM
		
	test	_Transparency_on,-1
	je	no_trans2
	cmp	ecx,-1
	je	skip7

	wr_onepix	0
	wr_onepix	1
	wr_onepix	2
	wr_onepix	3
	jmp	skip7
no_trans2:
	mov 	[edi],ecx	; Draw 4 pixels to display
skip7:	add 	edi,4

    ENDM

	pop	ebp
	pop	ecx
	pop	ebx
	dec	_loop_count
	jnz	TopOfLoop4_nolight

EndOfLoop4_nolight:

	cmp	num_left_over, 0
	je	_none_to_do

DoEndPixels_nolight:
	add	ebx, DU1
	add	ebp, DV1
	add	ecx, DZ1
	je	_div_0_abort
	push	edi	; use edi as a temporary variable

; Find fixed U1		
	mov	eax, ebx
	PDIV_START
	PDIV
	mov	ebx, eax	; ebx = U1 until pop's

; Find fixed V1		
	mov	eax, ebp
	PDIV
	PDIV_END
	mov	ebp, eax	; ebp = V1 until pop's

	mov	ecx, U0	; ecx = U0 until pop's
	mov	edi, V0	; edi = V0 until pop's

; Make ESI =  V0:U0 in 6:10,6:10 format
	mov	eax, ecx
	shr	eax, 6
	mov	esi, edi
	shl	esi, 10
	mov	si, ax
		
; Make EDX = DV:DU in 6:10,6:10 format
	mov	eax, ebx
	sub	eax, ecx
	sar	eax, NBITS+6
	mov	edx, ebp
	sub	edx, edi
	shl	edx, 10-NBITS	; EDX = V1-V0/ 4 in 6:10 int:frac
	mov	dx, ax	; put delta u in low word

	pop	edi	; Restore EDI before using it
		
	mov	ecx, num_left_over

    ITERATION = 0
    REPT (1 SHL NBITS)
	local	skip8
; Do 1 pixel 
	mov	eax, esi	; get u,v
	shr	eax, 26	; shift out all but int(v)
	shld	ax,si,6	; shift in u, shifting up v
	mov	al, es:[eax]	; load into buffer register
	add	esi, edx	; inc u,v
	cmp	al,255
	je	skip8
	mov	[edi+ITERATION], al	; write pixel
skip8:	dec	ecx
	jz	_none_to_do
    ITERATION = ITERATION + 1
    ENDM

; Should never get here!!!!!
	int	3
	jmp	_none_to_do

	

_TEXT	ends

	end
