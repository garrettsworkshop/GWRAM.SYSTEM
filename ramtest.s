.autoimport on
.importzp	sp

.export rtst_run
.import rtst_pat
.import rtst_scratch

.code

.proc ramtest_run: near
	; Preamble
	php					; Push status
	sei					; Disable interrupts
	phx					; Push X
	phy					; Push Y

	; Save entire ZP
	ldx #0				; X = 0
	savezp:
	lda $00,X
	sta rtst_scratch,X
	inx
	bne savezp

	; Set bank counter and address to 0
	lda #0
	sta $04
	sta $03
	sta $02

	; Set pattern address to 0xA000
	lda #$A0
	sta $01
	lda #$00
	sta $00

	bankloop:
	; Store 0x0000-01FF

	; Store 0x0200-BFFF

	; Switch to LC2

	; Store in 0xD000-DFFF

	; Switch to LC1

	; Store in 0xD000-FFFF

	; Increment bank and repeat if < 128
	inc $04
	lda $04
	cmp #$80
	blt bankloop

	; Restore entire ZP
	ldx #0				; X = 0
	savezp:
	lda rtst_scratch,X
	sta $00,X
	inx
	bne savezp

	; Postamble
	ply					; Pull Y
	plx					; Pull X
	plp					; Pull status
.endproc

.proc ramtest_incpat: near
	; Increment pattern pointer
	inc $00				; Increment low byte
	bne incpat1			; If low byte nonzero, skip incrementing high byte
	inc $01				; If low byte zero, increment high byte
	bne incpat2			; Unconditional branch to return
	beq incpat2

	; Check if pointer == 0xB001
	; if low byte didn't roll around 
	incpat1:	
	lda $01				; Load high byte of pointer
	cmp #$B0			; Check == 0xB0
	bne incpat2			; If not, goto return
	lda $00				; Load low byte of pointer
	cmp #$01			; Check == 0x01
	bne incpat2			; If not, goto return
	; Otherwise fall through

	; Reset pattern pointer
	lda #$A0
	sta $01
	lda #$00
	sta $00

	incpat2:
	rts
.endproc

.proc ramtest_wr256zp: near
	; Set up to copy
	ldy $02				; Y = address lo
	sty wr256zp_am1_1+1	; Set 1st address lo = 0
	sty wr256zp_am1_2+1	; Set 2nd address lo = 0
	ldy $03				; Y = address hi
	sty wr256zp_am1_1+2	; Set 1st address hi
	sty wr256zp_am1_2+2	; Set 2nd address hi
	ldy #0				; Y = 0

	wr256zp_loop:
	; Load two pattern bytes
	lda ($00)			; A = next pattern byte
	jsr ramtest_incpat	; Increment pattern pointer
	ldx ($00)			; Y = next pattern byte
	jsr ramtest_incpat	; Increment pattern pointer
	; Switch into ALTZP, store two pattern bytes, switch back
	sta $C009			; SETALTZP
	wr256zp_am1_1:
	sta $0000,Y			; Store in RAM
	iny					; Y++
	wr256zp_am1_2:
	stx $0000,Y			; Store in RAM
	iny					; Y++
	sta $C008			; SETSTDZP
	; Repeat
	bne wr256zp_loop	; Repeat until X rolls over (256 times)

	; Success exit
	rts
.endproc

.proc ramtest_wr256mn: near
	; Set up to copy
	ldy $02				; Y = address lo
	sty wr256zp_am1_1+1	; Set 1st address lo
	sty wr256zp_am1_2+1	; Set 2nd address lo
	ldy $03				; Y = address hi
	sty wr256zp_am1_1+2	; Set 1st address hi
	sty wr256zp_am1_2+2	; Set 2nd address hi
	ldy #0				; Y = 0

	wr256mn_loop:
	; Load two pattern bytes
	lda ($00)			; A = next pattern byte
	jsr ramtest_incpat	; Increment pattern pointer
	ldx ($00)			; Y = next pattern byte
	jsr ramtest_incpat	; Increment pattern pointer
	; RAMWRTON, store two pattern bytes, RAMWRTOFF
	sta $C009			; RAMWRTON
	wr256mn_am1_1:
	sta $0000,Y			; Store in RAM
	iny					; X++
	wr256mn_am1_2:
	stx $0000,Y			; Store in RAM
	iny					; X++
	sta $C008			; RAMWRTOFF
	; Repeat
	bne wr256mn_loop	; Repeat until X rolls over (256 times)

	; Success exit
	rts
.endproc

.proc ramtest_vfy256zp: near
	; Set up to verify
	ldx #0				; X = 0
	stx vfy256zp_am1+1	; Address lo = 0
	sty vfy256zp_am1+2	; Address hi = Y
	ldy #0				; Y = 0

	vfy256zp_loop:
	; Switch into ALTZP, load byte from RAM, switch back
	sta $C009			; SETALTZP
	vfy256zp_am1:
	lda $0000,X			; A = next RAM byte
	sta $C008			; SETSTDZP
	; Compare loaded byte from RAM with pattern
	cmp ($00),Y			; Compare with pattern byte
	bne vfy256zp_vfail
	jsr ramtest_incpat	; Increment pattern pointer
	inx
	; Repeat
	bne vfy256zp_loop	; Repeat until X rolls over (256 times)

	; Success exit
	rts
	; Fail exit
	vfy256zp_vfail:
	lda #$FF
	rts
.endproc
