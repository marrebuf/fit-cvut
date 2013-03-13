; FIXME: Most probably wrong but close enough.

        .global _delay_loop
        .text

_delay_loop:
        push w0
        mov #319, w0

du_1:   nop
        nop
        dec w0, w0
        bra nz, du_1

        nop
        pop w0
        return
