#!/usr/bin/guile
!#

(define check (lambda (cond)
	(if (not cond) (begin
		(display "Nespravny vstup.\n")
		(exit 1)))))

(begin
	(display "Zadejte velikosti a a b:\n")
	(define a (read))
	(check (and (number? a) (> a 0)))
	(define b (read))
	(check (and (number? b) (> b 0)))

	(display "Zadejte velikosti a' a b':\n")
	(define aa (read))
	(check (and (number? aa) (> aa 0) (> a aa)))
	(define bb (read))
	(check (and (number? bb) (> bb 0) (> b bb)))

	(display "Zadejte velikost h:\n")
	(define h (read))
	(check (and (number? h) (> h 0)))

	(define volume (* h (+
		(/ (+ (* b aa) (* a bb)) 2)
		(/ (* (- a aa) (- b bb)) 3))))

	(display "Objem: ")
	(display volume)
	(display "\n"))

