;
; Simple position independent program
;

.main:
	lri 0x01, 0x00
	lri 0x02, 0x0c
	add 0x02, 0x0f
	sys 0x00
	ret

.hello:
	.data "Hello World from a program!\n\x00"
