.global isr_generic
.align 4

isr_generic:
	pushal
	cld
	//call generic_event
	popal
	iret


/*isr:
	//save registers
	pushl %esp
	call keyboard_event
	add $4, %esp
	//restore registers
	add $8, %esp
	iret

.global isr_keyboard
isr_keyboard:
	pushl $0
	pushl $33 //IRQ1?
	jmp isr
*/
