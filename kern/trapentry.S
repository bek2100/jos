/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>

###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(div_handler,         0)
TRAPHANDLER_NOEC(debug_handler,       1)
TRAPHANDLER_NOEC(NULL1_handler,       2)
TRAPHANDLER_NOEC(bp_handler,          3)
TRAPHANDLER_NOEC(overflow_handler,    4)
TRAPHANDLER_NOEC(bounds_handler,      5)
TRAPHANDLER_NOEC(invalid_handler,     6)
TRAPHANDLER_NOEC(cocpunotavl_handler, 7)
TRAPHANDLER     (syserr_handler,      8)
TRAPHANDLER_NOEC(segover_handler,     9)
TRAPHANDLER     (TSS_handler,        10)
TRAPHANDLER     (segpres_handler,    11)
TRAPHANDLER     (stack_handler,      12)
TRAPHANDLER     (gp_handler,         13)
TRAPHANDLER     (pf_handler,         14)
TRAPHANDLER_NOEC(NULL2_handler,      15)
TRAPHANDLER_NOEC(cocpu_handler,      16)
TRAPHANDLER_NOEC(NULL3_handler,      17)
TRAPHANDLER_NOEC(NULL4_handler,      18)
TRAPHANDLER_NOEC(NULL5_handler,      19)
TRAPHANDLER_NOEC(NULL6_handler,      20)
TRAPHANDLER_NOEC(NULL7_handler,      21)
TRAPHANDLER_NOEC(NULL8_handler,      22)
TRAPHANDLER_NOEC(NULL9_handler,      23)
TRAPHANDLER_NOEC(NULLa_handler,      24)
TRAPHANDLER_NOEC(NULLb_handler,      25)
TRAPHANDLER_NOEC(NULLc_handler,      26)
TRAPHANDLER_NOEC(NULLd_handler,      27)
TRAPHANDLER_NOEC(NULLe_handler,      28)
TRAPHANDLER_NOEC(NULLf_handler,      29)
TRAPHANDLER_NOEC(NULL10_handler,     30)
TRAPHANDLER_NOEC(NULL11_handler,     31)

TRAPHANDLER_NOEC(syscall_handler, T_SYSCALL)


.globl sysenter_handler
.type sysenter_handler, @function
.align 2;
sysenter_handler:
	push %eax
	push %ecx
	push %edx
	call trap_lock_kernel
	pop  %edx
	pop  %ecx
	pop  %eax

	push $0
	push %edi
	push %ebx
	push %ecx
	push %edx
	push %eax
	call syscall
	add $24, %esp

	push %eax
	push %ecx
	push %edx
	call trap_unlock_kernel
	pop  %edx
	pop  %ecx
	pop  %eax

	mov %esi, %edx
	mov %ebp, %ecx
	sysexit

/*
 * Lab 3: Your code here for _alltraps
 */
.globl _alltraps
.type _alltraps, @function
.align 2;
_alltraps:
	push %ds
	push %es
	pusha
	mov $(GD_KD), %eax
	mov %ax, %es
	mov %ax, %ds
	pushl %esp
	call trap
