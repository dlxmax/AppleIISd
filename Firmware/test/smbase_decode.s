; -----------------------------------------------------------------------------
; smbase_decode.s
;
; Verbatim copy of the PRODOS dispatcher's SMBASE-decode prefix from
; Firmware/src/ProDOS.s for use under sim65. If you change ProDOS.s, mirror
; the change here. The fixed ZP addresses match AppleIISd.inc.
; -----------------------------------------------------------------------------

.export _decode_smbase
.exportzp _zp_slot16, _zp_dsnumber, _zp_smbase

_zp_slot16   := $3E
_zp_dsnumber := $43
_zp_smbase   := $4D

.proc _decode_smbase
            lda   _zp_slot16
            eor   _zp_dsnumber
            and   #$70        ; slot bits
            beq   @myslot
            lda   #2          ; phantom slot (ProDOS 2.4 D3/D4)
            bra   @addbits
@myslot:    lda   #0
@addbits:   sta   _zp_smbase
            lda   _zp_dsnumber  ; D S S S 0 0 X Y
            and   #$03        ; isolate X Y
            asl   a           ; X*4 + Y*2
            clc
            adc   _zp_smbase
            sta   _zp_smbase
            lda   _zp_dsnumber
            bpl   @done       ; D = 0
            inc   _zp_smbase  ; D = 1
@done:      rts
.endproc
