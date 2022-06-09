.intel_syntax noprefix
//load and set up the GDT

gdtr: 
	.long 0 // For limit storage
    .word 0 // For base storage
	
.global setGDT
.type setGDT, @function 
setGDT:
   MOV   EAX, [esp + 4]
   MOV   [gdtr + 2], EAX
   MOV   AX, [ESP + 8]
   DEC   AX
   MOV   [gdtr], AX
   LGDT  [gdtr]
   RET
   