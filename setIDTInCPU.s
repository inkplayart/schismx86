.intel_syntax noprefix
//load and set up the Interrupt Descriptor Table
//Note: This is practically the same code as needed to load the GDT, except the LIDT instruction
//is used rather than the LGDT instruction

idtr: 
	.long 0 // For limit storage
    .word 0 // For base storage
	
.global setIDT
.type setIDT, @function 
setIDT:
   MOV   EAX, [esp + 4]
   MOV   [idtr + 2], EAX
   MOV   AX, [ESP + 8]
   DEC   AX
   MOV   [idtr], AX
   LIDT  [idtr]
   RET
   