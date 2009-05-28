;The MIT License
;
;Copyright (c) 2009 Dylan Smith
;
;Permission is hereby granted, free of charge, to any person obtaining a copy
;of this software and associated documentation files (the "Software"), to deal
;in the Software without restriction, including without limitation the rights
;to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;copies of the Software, and to permit persons to whom the Software is
;furnished to do so, subject to the following conditions:
;
;The above copyright notice and this permission notice shall be included in
;all copies or substantial portions of the Software.
;
;THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
;THE SOFTWARE.

; Initialization routines
F_init
        ld a, (v_pgb)           ; Who are we?
        call RESERVEPAGE        ; Reserve a page of static RAM.
        jr c, .failed
        ld b, a                 ; save the page number
        ld a, (v_pgb)           ; and save the page we got
        rlca                    ; in our 8 byte area in sysvars
        rlca                    ; which we find by multiplying our ROM number
        rlca                    ; by 8.
        ld h, 0x39              ; TODO - definition for this.
        ld l, a
        ld a, b
        ld (hl), a              ; put the page number in byte 0 of this area.
        ld hl, STR_init
        call PRINT42
        ret
.failed
        ld hl, STR_allocfailed
        call PRINT42
        ret
STR_allocfailed defb    "tnfs: No memory pages available\n",0
STR_init        defb    "TNFS 1.0 initialized\n",0
