.setcpu		"65816"
.autoimport on
.importzp	sp

.export 	_ram2gs_cmd
.export 	_ram2gs_getsize
.export 	_ram2gs_detect

.define	GetTWConfig			$BCFF3C
.define SetTWConfig			$BCFF40
.define DisableDataCache	$BCFF4C

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

.proc _thrash: near
.A8
.I8
	; Preamble
	php				; Push status
	AI16
	phb				; Push bank
	pha				; Push accumulator
	pha				; Push X
	phy				; Push Y

	; Read 0x100000-0x11FFFF
	AI8
	lda	#$20		; A = 0x10 (bank)
	_thrash_loop:
	pha				; Switch to bank stored in A
	plb
	AI16
	lda #$0000		; Get index in A
	clc				; Clear carry in preparation to add in loop

	; Read loop
	_thrash_loop0:
	tax				; Get index from A into X to do LDYs
	ldy $0100,X		; Read 64 bytes
	ldy $0102,X		; ...
	ldy $0104,X
	ldy $0106,X
	ldy $0108,X
	ldy $010A,X
	ldy $010C,X
	ldy $010E,X
	ldy $0110,X
	ldy $0112,X
	ldy $0114,X
	ldy $0116,X
	ldy $0118,X
	ldy $011A,X
	ldy $011C,X
	ldy $011E,X
	ldy $0120,X
	ldy $0122,X
	ldy $0124,X
	ldy $0126,X
	ldy $0128,X
	ldy $012A,X
	ldy $012C,X
	ldy $012E,X
	ldy $0130,X
	ldy $0132,X
	ldy $0134,X
	ldy $0136,X
	ldy $0138,X
	ldy $013A,X
	ldy $013C,X
	ldy $013E,X
	adc	#$0040		; Add 64 to index in A
	cmp #0
	bne _thrash_loop0 ; Repeat if we haven't passed 0xFFFF

	; Bank increment
	AI8
	phb				; Transfer bank to A
	pla
	inc				; Increment bank
	cmp #$21 		; Stop after bank 0x20
	bne _thrash_loop


	; Postamble
	AI16
	ply				; Restore Y
	plx				; Restore X
	pla				; Restore accumulator
	plb				; Restore bank
	AI8
	plp				; Restore status
	rts
.endproc

.proc _ram2gs_cmd: near
.A8
.I8
	; Preamble
	php				; Push status
	sei				; Disable interrupts
	clc				; Clear carry
	xce				; Clear emulation bit
	php				; Push status again, reflecting emulation bit
	phb				; Push bank
	pha				; Push command in accumulator
	AI8
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
	; Postamble
	plb				; Restore bank
	plp				; Restore status
	xce				; Restore emulation bit
	plp				; Pull status again to pull I flag
	rts
.endproc

.proc _unswap: near
.A8
.I8
	tya
	ora #$00
	jmp _ram2gs_cmd
.endproc

.proc _swap: near
.A8
.I8
	tya
	ora #$01
	jmp _ram2gs_cmd
.endproc

.proc _ram2gs_getsize: near
.A8
.I8
	; Preamble
	php				; Push status
	sei				; Disable interrupts
	clc				; Clear carry
	xce				; Clear emulation bit
	php				; Push status again, reflecting emulation bit
	phb				; Push bank
	AI8

	; Go to bank 3F
	ldy #$3F
	phy
	plb
	; Save 3F/3456
	lda $3456
	pha

	; Go to bank 7F
	ldy #$7F
	phy
	plb
	; Save and then invert 7F/3456
	lda $3456
	pha
	eor #$FF
	sta $3456

	; Go to bank 3F
	ldy #$3F
	phy
	plb
	; Has 3F/3456 changed?
	jsr _thrash
	plx ; X = saved 7F/3456
	pla ; A = saved 3F/3456
	cmp $3456
	php ; Push to save processor status

	; Go to bank 7F
	ldy #$7F
	phy
	plb
	; Restore 3F/3456
	sta $3456

	; Go to bank 7F
	ldy #$7F
	phy
	plb
	; Restore 7F/3456
	stx $3456

	; Check result
	lda #$80
	plp
	beq _ram2gs_getsize_return
	lda #$40

	; Postamble
	_ram2gs_getsize_return:
	plb				; Restore bank
	plp				; Restore status
	xce				; Restore emulation bit
	plp				; Pull status again to pull I flag
	rts
.endproc

.proc _ram2gs_detect_internal: near
.A8
.I8
	; Switch to bank 0x3F
	lda #$3F
	pha
	plb

	; Unswap
	jsr _unswap
	; Save unswapped 3F/8000
	jsr _thrash
	lda $8000
	pha
	; Swap
	jsr _swap
	; Save swapped 3F/8000
	jsr _thrash
	lda $8000
	pha

	; Store 0xFF in swapped
	lda #$FF
	sta $8000
	; Verify 0xFF stored
	jsr _thrash
	lda $8000
	cmp #$FF
	bne _ram2gs_detect_fail

	; Unswap
	jsr _unswap
	; Store 0x00 in unswapped
	lda #$00
	sta $8000
	; Verify 0x00 stored
	jsr _thrash
	lda $8000
	cmp #$00
	bne _ram2gs_detect_fail

	; Swap
	jsr _swap
	; Verify 0xFF stored
	jsr _thrash
	lda $8000
	cmp #$FF
	bne _ram2gs_detect_fail

	; Get success return value and jump to postamble
	ldx #$01		; Get success falue
	bne _ram2gs_detect_done ; Jump to postamble

	; Fail
	_ram2gs_detect_fail:
	ldx #$00		; Get fail value

	; Done, now put back clobbered bytes
	_ram2gs_detect_done:
	jsr _swap	; Swap
	pla				; Get value to restore to swapped bank 3F
	sta $8000		; Restore
	jsr _unswap	; Unswap
	pla				; Get value to restore to unswapped bank 3F
	sta $8000		; Restore

	; Return
	rts
.endproc

.proc _ram2gs_detect: near
.A8
.I8
	; Preamble
	phx				; Push X
	phy				; Push Y
	php				; Push status
	sei				; Disable interrupts
	clc				; Clear carry
	xce				; Clear emulation bit
	php				; Push status again, reflecting emulation bit
	phb				; Push bank
	AI8

	; Transfer typecode (shifted) to Y register
	and #$0E
	tay

	jsr _ram2gs_detect_internal

	; Postamble
	txa				; Get return value
	plb				; Restore bank
	plp				; Restore status
	xce				; Restore emulation bit
	plp				; Pull status again to pull I flag
	ply				; Pull X
	plx				; Pull Y
	rts
.endproc
