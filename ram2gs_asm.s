.setcpu		"65816"
.autoimport on
.importzp	sp

.export 	_ram2gs_getsize
.export 	_ram2gs_detect
.export 	_ram2gs_cmd

.macro A8
       sep #$20 ; put the 65C816 in 8-bit accumulator mode
      .A8
.endmacro

.macro I8
       sep #$10 ; put the 65C816 in 8-bit index register mode
      .I8
.endmacro

.macro AI8
       sep #$30 ; put the 65C816 in 8-bit accumulator and index register mode
      .A8
      .I8
.endmacro

.macro A16
       rep #$20 ; put the 65C816 in 8-bit accumulator mode
      .A16
.endmacro

.macro I16
       rep #$10 ; put the 65C816 in 8-bit index register mode
      .I16
.endmacro

.macro AI16
       rep #$30 ; put the 65C816 in 8-bit accumulator and index register mode
      .A16
      .I16
.endmacro

.segment	"CODE"

.proc _gsram_getsize: near
.A8
.I8
	; Preamble
	sei				; Disable interrupts
	clc				; Clear carry
	xce				; Clear emulation bit
	php				; Push status
	phb				; Push bank
	AI8

	; Store bank number at address 0xFFFF in each bank
	ldy #$7F		; Start at bank 0x7F
	BankSetLoop:
	phy				; Push future bank
	plb 			; Pull bank
	lda $8000		; Get address 0xFFFF in this bank
	pha				; Save old address 0xFFFF contents
	tya				; A = Y
	eor #$FF		; Flip all bits
	tay				; Y = A
	sty $8000		; Overwrite address 0xFFFF with bank number
	eor #$FF		; Flip all bits back
	tay				; Y = A
	dey				; Decrement bank number
	cpy #$FF		; Have we wrapped around?
	bne BankSetLoop ; If not, repeat

	; Count banks with matching bank number
	ldy #$00		; Y is bank
	ldx #$00		; X is count
	CountLoop:
	phy				; Push future bank
	plb				; Pull bank
	tya				; A = Y
	eor #$FF		; Flip all bits
	tay				; Y = A
	cpy $8000		; Is bank num stored at address 0xFFFF?
	bne AfterInc	; If not, skip increment
	inx				; If so, increment bank count
	AfterInc:
	eor #$FF		; Flip all bits back
	tay				; Y = A
	pla				; Get contents to restore
	sta $8000		; Restore address 0xFFFF in this bank
	iny				; Move to next bank
	cpy #$80		; Are we at bank 0x80 yet?
	bne CountLoop	; If not, repeat

	; Postamble
	plb				; Restore bank
	plp				; Restore status
	xce				; Restore emulation bit
	cli				; Enable interrupts
	txa				; Transfer bank count to A
	rts
.endproc

.proc _ram2gs_getsize: near
.A8
.I8
	; Preamble
	sei				; Disable interrupts
	clc				; Clear carry
	xce				; Clear emulation bit
	php				; Push status
	phb				; Push bank
	AI8

	; Go to bank 3F
	ldy #$3F
	phy
	plb
	; Save 3F/3456
	ldx $3456

	; Go to bank 7F
	ldy #$7F
	phy
	plb
	; Invert 7F/3456
	lda $3456
	eor #$FF
	sta $3456

	; Go to bank 3F
	ldy #$3F
	phy
	plb
	; Has 3F/3456 changed?
	cpx $3456
	php

	; Go to bank 7F
	ldy #$7F
	phy
	plb
	; Restore 7F/3456
	eor #$FF
	sta $3456

	; Check result
	ldx #$80
	plp
	beq _ram2gs_getsize_return
	ldx #$40

	; Postamble
	_ram2gs_getsize_return:
	plb				; Restore bank
	plp				; Restore status
	xce				; Restore emulation bit
	cli				; Enable interrupts
	txa				; Transfer bank count to A
	rts
.endproc


.proc _unswap: near
.A8
.I8
	; Save current bank and accumulator
	phb
	pha
	; Switch to bank 0xFB
	lda #$FB
	pha
	plb
	; Submit C1AD
	lda #$C1
	sta $FFFE
	lda #$AD
	sta $FFFF
	; Pull and submit command 
	lda #$00
	sta $FFFD
	; Restore accumulator and bank and return
	pla
	plb
	rts
.endproc

.proc _swap: near
.A8
.I8
	; Save current bank and accumulator
	phb
	pha
	; Switch to bank 0xFB
	lda #$FB
	pha
	plb
	; Submit C1AD
	lda #$C1
	sta $FFFE
	lda #$AD
	sta $FFFF
	; Pull and submit command 
	lda #$01
	sta $FFFD
	; Restore accumulator and bank and return
	pla
	plb
	rts
.endproc

.proc _ram2gs_detect: near
.A8
.I8
	; Preamble
	sei				; Disable interrupts
	clc				; Clear carry
	xce				; Clear emulation bit
	php				; Push status
	phb				; Push bank
	AI8

	; Switch to bank 0x3F
	lda #$3F
	pha
	plb

	; Unswap
	jsr _unswap
	; Save unswapped 3F/8000
	lda $8000
	pha
	; Swap
	jsr _swap
	; Save swapped 3F/8000
	lda $8000
	pha

	; Store 0xFF in swapped
	lda #$FF
	sta $8000
	; Verify 0xFF stored
	lda $8000
	cmp #$FF
	bne _ram2gs_detect_fail

	; Unswap
	jsr _unswap
	; Store 0x00 in unswapped
	lda #$00
	sta $8000
	; Verify 0x00 stored
	lda $8000
	cmp #$00
	bne _ram2gs_detect_fail

	; Swap
	jsr _swap
	; Verify 0xFF stored
	lda $8000
	cmp #$FF
	bne _ram2gs_detect_fail

	; Get success return value and jump to postamble
	ldx #$01		; Get success falue
	bne _ram2gs_detect_return ; Jump to postamble

	; Fail
	_ram2gs_detect_fail:
	ldx #$00		; Get fail value

	; Postamble
	_ram2gs_detect_return:
	jsr _swap		; Swap
	pla				; Get value to restore to swapped bank 3F
	sta $8000		; Restore
	jsr _unswap		; Unswap
	pla				; Get value to restore to unswapped bank 3F
	sta $8000		; Restore
	txa				; Put return value in accumulator
	plb				; Restore bank
	plp				; Restore status
	xce				; Restore emulation bit
	cli				; Enable interrupts
	rts
.endproc

.proc _ram2gs_cmd: near
.A8
.I8
	; Save current bank and command in accumulator
	phb
	pha
	; Switch to bank 0xFB
	lda #$FB
	pha
	plb
	; Submit C1AD
	lda #$C1
	sta $FFFE
	lda #$AD
	sta $FFFF
	; Pull and submit command 
	pla
	sta $FFFD
	; Restore bank and return
	plb
	rts
.endproc
