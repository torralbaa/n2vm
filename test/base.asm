;
; Simple OS-like program
;

; start
;  Setup syscalls
.start:
	lri 0x01, 0xff7f
	lrt 0x01, 0xff
	lri 0x02, @syscall
	lmr 0x01, 0x02

; main
;  Say hello!
.main:
	lri 0x01, 0x00
	lri 0x02, @hello
	sys 0x00
	lri 0x01, @parse
	jmp 0x01
	lri 0x01, @emp_init
	jmp 0x01
	hlt

; parse
;  Parse EMP
.parse:
	lri 0x01, 0x00
	lri 0x02, @parsing
	sys 0x00

	lri 0x01, @emp_ident_0
	lrc 0x01, 0x01
	lri 0x02, 0x45 ; 'E'
	cmp 0x01, 0x02
	lri 0x0e, 0x01 !ne
	ret !ne

	lri 0x01, @emp_ident_1
	lrc 0x01, 0x01
	lri 0x02, 0x4d ; 'M'
	cmp 0x01, 0x02
	lri 0x0e, 0x01 !ne
	ret !ne

	lri 0x01, @emp_ident_2
	lrc 0x01, 0x01
	lri 0x02, 0x50 ; 'P'
	cmp 0x01, 0x02
	lri 0x0e, 0x01 !ne
	ret !ne

	lri 0x01, 0x00
	lri 0x02, @done
	sys 0x00
	lri 0x0e, 0x00
	ret

;.err:
;	

; syscall
;  Handle syscalls
.syscall:
	; The only avaiable/implemented syscall is `print` (0x00)
	; So let's return if other syscall was requested
	lri 0x03, 0x00
	cmp 0x01, 0x03
	ret !ne
; print
	lri 0x00, 0x00
	out 0x00, 0x02
	lri 0x0e, 0x00
	ret

.hello:
	.data "Hello World!\n\x00"

.parsing:
	.data "Parsing...\n\x00"

.done:
	.data "Done!\n\x00"

; <[
; emp
;  chr emp_ident[3]
;  chr emp_arch
;  chr emp_mode
; ]>

.emp_ident_0:
	.data "E"

.emp_ident_1:
	.data "M"

.emp_ident_2:
	.data "P"

.emp_arch:
	.data 0x01
.emp_arch:
	.data 0x01
.emp_init:
