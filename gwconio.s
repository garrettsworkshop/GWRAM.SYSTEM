;
; Ullrich von Bassewitz, 06.08.1998
; Modified Sep. 5, 2020 by Zane Kaminski, Engineer @ Garrett's Workshop
;
;
        .ifdef  __APPLE2ENH__
        .constructor    initconio
        .endif
        .export         _gwcputcxy, _gwcputc
        .export         _gwcputsxy, _gwcputs
        .import         gotoxy, VTABZ
        .importzp       ptr1, tmp1

        .include        "apple2.inc"

        .segment        "ONCE"

        .ifdef  __APPLE2ENH__
initconio:
        sta     SETALTCHAR      ; Switch in alternate charset
        bit     LORES           ; Limit SET80COL-HISCR to text
        rts
        .endif

        .code


; void gwcputsxy (unsigned char x, unsigned char y, const char* s);             
_gwcputsxy:
        sta     ptr1            ; Save s for later
        stx     ptr1+1
        jsr     gotoxy          ; Set cursor, pop x and y
        jmp     L0              ; Same as cputs...
; void gwcputs (const char* s);
_gwcputs: sta     ptr1            ; Save s
        stx     ptr1+1
L0:     ldy     #0
L1:     lda     (ptr1),y
        beq     L9              ; Jump if done
        iny
        sty     tmp1            ; Save offset
        jsr     _gwcputc          ; Output char, advance cursor
        ldy     tmp1            ; Get offset
        bne     L1              ; Next char
        inc     ptr1+1          ; Bump high byte
        bne     L1
; Done
L9:     rts


; void __fastcall__ gwcputcxy (unsigned char x, unsigned char y, char c);
_gwcputcxy:
        pha                     ; Save C
        jsr     gotoxy          ; Call this one, will pop params
        pla                     ; Restore C and run into _gwcputc
; void __fastcall__ gwcputc (char c);
_gwcputc:
        cmp     #$0D            ; Test for \r = carrage return
        beq     left
        cmp     #$0A            ; Test for \n = line feed
        beq     newline
        eor     #$80            ; Invert high bit

cputdirect:
        jsr     putchar
        inc     CH              ; Bump to next column
        lda     CH
        cmp     WNDWDTH
        bcc     :+
        jsr     newline
left:   lda     #$00            ; Goto left edge of screen
        sta     CH
:       rts

newline:
        inc     CV              ; Bump to next line
        lda     CV
        cmp     WNDBTM
        bcc     :+
        lda     WNDTOP          ; Goto top of screen
        sta     CV
:       jmp     VTABZ

putchar:
        .ifdef  __APPLE2ENH__
        ldy     INVFLG
        cpy     #$FF            ; Normal character display mode?
        beq     putchardirect
        cmp     #$E0            ; Lowercase?
        bcc     mask
        and     #$7F            ; Inverse lowercase
        bra     putchardirect
        .endif
mask:   and     INVFLG          ; Apply normal, inverse, flash

putchardirect:
        pha
        ldy     CH
        .ifdef  __APPLE2ENH__
        bit     RD80VID         ; In 80 column mode?
        bpl     put             ; No, just go ahead
        tya
        lsr                     ; Div by 2
        tay
        bcs     put             ; Odd cols go in main memory
        bit     HISCR           ; Assume SET80COL
        .endif
put:    lda     (BASL),Y        ; Get current character
        tax                     ; Return old character for _cgetc
        pla
        sta     (BASL),Y
        .ifdef  __APPLE2ENH__
        bit     LOWSCR          ; Doesn't hurt in 40 column mode
        .endif
        rts