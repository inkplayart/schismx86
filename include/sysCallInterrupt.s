//.intel_syntax noprefix
//Allows us to do system calls by saving and restoring the state properly

.global sysCallHandler
.align 4

sysCallHandler:
	//save state
	pushal
	//clear direction flag. No I do not know why
	cld
	/*
		Now the system call C function passes two variables: a system call number and
		a pointer to a system-call specific structure. These are sent in via eax (sys call no) and ecx (pointer). The System-V ABI that we are using requires us to push these onto the stack in reverse order - the first input argument to the function is the lowest value on the stack. The C system call handler takes in call no and ptr in that order, so we push them in rersed then call the C function.
	*/
	push %ecx
	push %eax
	call sysCallC
	/*
		OK, now we need to pop those things off before we restore the state, or else we will be popping some weird stuff
	*/
	pop %eax
	pop %ecx
	
	//restore state
	popal
	
	//return from interrupt
	iret
