// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

#include <inc/x86.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

extern void _pgfault_upcall(void);
//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	addr = ROUNDDOWN(addr, PGSIZE);
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	volatile pte_t *pte = &uvpt[PGNUM(addr)];
	if (!((err & FEC_WR) && (*pte & PTE_COW)))
		panic("pgfault %d %p %p", err, addr, utf->utf_eip);

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	if ((r = sys_page_alloc(0, PFTEMP, PTE_P|PTE_U|PTE_W)) < 0) panic ("pgfault alloc %d", r);

	memcpy(PFTEMP, addr, PGSIZE);

	if ((r = sys_page_map(0, PFTEMP, 0, addr, PTE_P|PTE_U|PTE_W)) < 0) panic ("pgfault map %d", r);
	if ((r = sys_page_unmap(0, PFTEMP)) < 0) panic ("pgfault unmap %d", r);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
	
	void *addr = (void*)(pn * PGSIZE);
	volatile pte_t *pte = &uvpt[PGNUM(addr)];
	// LAB 4: Your code here.
	int perm = *pte & (PTE_P|PTE_U|PTE_SHARE), write = (*pte & (PTE_W|PTE_COW)), shared = (*pte & PTE_SHARE);

	if (shared)
	{
//		cprintf("share %p\n", addr);
		if ((r = sys_page_map(0, addr, envid, addr, *pte & PTE_SYSCALL)) < 0) panic ("duppage map %d %x %x %d", r, addr, UTOP, perm);
	}
	else
	{
		if (write) perm |= PTE_COW;
		if ((r = sys_page_map(0, addr, envid, addr, perm)) < 0) panic ("duppage map %d %x %x %d", r, addr, UTOP, perm);
		if (write)
			if ((r = sys_page_map(0, addr, 0, addr, perm)) < 0) panic ("duppage map %d", r);
	}

	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	int status;
	uintptr_t p;
	envid_t envid;

	// LAB 4: Your code here.
	set_pgfault_handler(pgfault);

	if ((status = sys_exofork()) < 0) panic("fork %d", status);
	envid = (envid_t)status;

	if (!envid) //child
	{
		thisenv = envs + ENVX(sys_getenvid());
		return 0;
	}
	
	for (p = 0; p < USTACKTOP; p += PGSIZE)
	{
		
		if (	(uvpd[PDX(p)]   & PTE_P) &&
			(uvpt[PGNUM(p)] & PTE_P)	)
		{
			 duppage(envid, PGNUM(p));
		}
	}

	if ((status = sys_page_alloc(envid, (void*)(UXSTACKTOP-PGSIZE), PTE_P|PTE_W|PTE_U)) < 0) panic("fork %d", status);
	if ((status = sys_env_set_pgfault_upcall(envid, _pgfault_upcall)) < 0) panic("fork %d", status);
	if ((status = sys_env_set_status(envid, ENV_RUNNABLE)) < 0) panic("fork %d", status);

	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
