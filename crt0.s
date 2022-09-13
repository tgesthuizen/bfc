.section ".text", "ax"
.global _start
.type _start, %function
.func _start, _start
_start:
	ldr r0, =tape_mem
	ldr r3, =read_char
	ldr r4, =write_char
	blx main
	mov r0, #0
	mov r7, #1 @ sys_exit
	swi #0
.pool
.endfunc
.size _start, .-_start

.type read_char, %function
.func read_char, read_char
read_char:
	push {r0-r2, r7}
	mov  r7, #3  @ sys_read
	mov  r1, r0  @ Use current tape head as buffer
	mov  r0, #0  @ stdin
	mov  r2, #1
	swi  #0
	pop  {r0-r2, r7}
	bx   lr
.endfunc
.size read_char, .-read_char

.type write_char, %function
.func write_char, write_char
write_char:
	push {r0-r2, r7}
	mov  r7, #4  @ sys_write
	mov  r1, r0  @ Use current tape head as buffer
	mov  r0, #1  @ stdout
	mov  r2, #1
	swi  0
	pop  {r0-r2, r7}
	bx   lr
.endfunc
.size write_char, .-write_char

.section ".bss", "aw"
.type tape_mem, %object
tape_mem:
	.space 16384
.size tape_mem, .-tape_mem
