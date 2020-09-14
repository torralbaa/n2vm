; Comment!

.text:
	lri 0x00, 0x00
	out 0x00, 0x0c
	hlt

.data:
	.data "Hello!\n\x00"
