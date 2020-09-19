; Comment!

.main:
	lri 0x00, 0x00
	out 0x00, @hello_world
	hlt

.hello_world:
	.data "Hello!\n\x00"
