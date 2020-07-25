.setcpu		"65816"
.autoimport on
.importzp	sp

.export 	_gsram_getsize

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
	php				; Push status
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
	txa				; Transfer bank count to A
	plp				; Pull status again to pull I flag
	rts
.endproc
