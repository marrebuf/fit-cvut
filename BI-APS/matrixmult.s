;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; BI-APS -- Nasobeni ctvercovych matic
; Vysledne skore pro N = 10: 6999 cyklu
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	.data
	.align 2

; Rozmer vstupnich matic a matice samotne

N:	.word 5

A: 	.float  1,  2,  3,  4,  5
	.float  6,  7,  8,  9, 10
	.float 11, 12, 13, 14, 15
	.float 16, 17, 18, 19, 20
	.float 21, 22, 23, 24, 25

B:	.float 21,  1, 20, 11, 10
	.float 22,  2, 19, 12,  9
	.float 23,  3, 18, 13,  8
	.float 24,  4, 17, 14,  7
	.float 25,  5, 16, 15,  6

; Prostor pro vystupni matici
; Nezapomente zmenit velikost pri zvetsovani N!

C:	.space 100

; Zasobnik

stack: .space 4096

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.text

start:
	; Nastaveni zasobniku
	lhi   r29,      (stack+4092) >> 16
	addui r29, r29, (stack+4092) & 0xffff

	; Ulozeni registru
	sw  -4(r29), r4
	sw  -8(r29), r5
	sw -12(r29), r6
	sf -16(r29), f2
	sf -20(r29), f3
	sf -24(r29), f4
	sf -28(r29), f5
	sf -32(r29), f6
	sf -36(r29), f7
	sf -40(r29), f8
	subui r29, r29, #40

	lhi r5, N >> 16
	lhi r2, A >> 16
	lhi r3, B >> 16
	lhi r4, C >> 16

	addui r5, r5, N & 0xffff
	addui r2, r2, A & 0xffff     ; r2 = A
	addui r3, r3, B & 0xffff     ; r3 = B
	addui r4, r4, C & 0xffff     ; r4 = C

	lw r5, (r5)                  ; r5 = *N
	addu r10, r0, r0             ; reorder v
	slli r6, r5, 2               ; r6 = *N * 4
Vloop:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Rozbalene ;;;;;
	srli r12, r5, 2
	addu r11, r0, r5
	beqz r12, HSloop
HMloop:
	subui r12, r12, 1
	subui r11, r11, 4

	movi2fp f0, r0
	movi2fp f1, r0
	movi2fp f2, r0
	movi2fp f3, r0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	slli r15, r11, 2
	addu r13, r0, r0
	addu r14, r3, r15
	addu r15, r15, r4
HMIloop:
	addu r1, r2, r13
	ld f4, 0(r14)
	lf f8, (r1)                  ; reorder ^
	ld f6, 8(r14)

	multf f4, f4, f8
	multf f5, f5, f8
	multf f6, f6, f8
	multf f7, f7, f8

	addui r13, r13, 4

	addf f0, f0, f4
	addf f1, f1, f5
	addf f2, f2, f6
	addf f3, f3, f7

	seq r1, r13, r6              ; reorder v
	addu r14, r14, r6

	beqz r1, HMIloop
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	sd 0(r15), f0
	sd 8(r15), f2
	bnez r12, HMloop
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Dojezd ;;;;;;;;
	beqz r11, Hend
HSloop:
	subui r11, r11, 1
	movi2fp f0, r0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	slli r15, r11, 2
	addu r13, r0, r5
	addu r14, r3, r15
	addu r15, r15, r4
	addu r1, r2, r0
HSIloop:
	lf f4, 0(r14)
	lf f8, 0(r1)

	subui r13, r13, 1            ; reorder vvv
	multf f4, f4, f8
	addu r14, r14, r6            ; reorder vv
	addui r1, r1, 4              ; reorder v
	addf f0, f0, f4

	bnez r13, HSIloop
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	sf (r15), f0
	bnez r11, HSloop
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Hend:
	addui r10, r10, 1
	addu r2, r2, r6
	seq r1, r10, r5              ; reorder v
	addu r4, r4, r6
	beqz r1, Vloop

	; Obnoveni registru
	addui r29, r29, #40
	lw r4,  -4(r29)
	lw r5,  -8(r29)
	lw r6, -12(r29)
	lf f2, -16(r29)
	lf f3, -20(r29)
	lf f4, -24(r29)
	lf f5, -28(r29)
	lf f6, -32(r29)
	lf f7, -36(r29)
	lf f8, -40(r29)

	trap #0

