.intel_syntax noprefix
//Reload segment selectors to get the GDT to work

.global reloadSegments
.type reloadSegments, @function
reloadSegments:
   // Reload CS register containing code selector:
   JMP   0x08:.reload_CS // 0x08 points at the new code selector
.reload_CS:
   // Reload data segment registers:
   MOV   AX, 0x10 // 0x10 points at the new data selector
   MOV   DS, AX
   MOV   ES, AX
   MOV   FS, AX
   MOV   GS, AX
   MOV   SS, AX
   RET
   