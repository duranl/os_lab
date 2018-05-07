// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
    uint32_t pn = PGNUM((uint32_t)addr);
    if (!((err & FEC_WR) && (uvpt[pn] & PTE_P) && (uvpt[pn] & PTE_COW)))
        panic("pgfault: not copy-on-write page fault");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
    int perm = (uvpt[pn] & PTE_SYSCALL & (~PTE_COW)) | PTE_W;
    // Allocate a page at PFTEMP
    r = sys_page_alloc(0, PFTEMP, perm);
    if (r < 0)
        panic("pgfault: %e", r);

    // Copy the data from the old to the new page
    addr = ROUNDDOWN(addr, PGSIZE);
    memcpy((void *)PFTEMP, (void *)addr, PGSIZE);

    // Remap the page
    r = sys_page_map(0, (void *)PFTEMP, 
            0, (void *)addr, perm); 
    if (r < 0)
        panic("pgfault: %e", r);

    // Unmap the temp page
    r = sys_page_unmap(0, PFTEMP);
    if (r < 0)
        panic("pgfault: %e", r);
	//panic("pgfault not implemented");
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

	// LAB 4: Your code here.
    // Current page is writable or copy-on-write page
    if (!(uvpt[pn] & PTE_SHARE)
            && ((uvpt[pn] & PTE_W) == PTE_W 
            || (uvpt[pn] & PTE_COW) == PTE_COW)) {
        int perm = (uvpt[pn] & PTE_SYSCALL & (~PTE_W)) | PTE_COW;
        r = sys_page_map(0, (void *)(pn * PGSIZE), 
                envid, (void *)(pn * PGSIZE), perm);
        if (r < 0)
            panic("duppage: %e", r);
        r = sys_page_map(0, (void *)(pn * PGSIZE), 
                0, (void *)(pn * PGSIZE), perm);
        if (r < 0)
            panic("duppage: %e", r);

    } else { // Sharable or not writable or COW page
        r = sys_page_map(0, (void *)(pn * PGSIZE), 
                envid, (void *)(pn * PGSIZE), uvpt[pn] & PTE_SYSCALL);
        if (r < 0)
            panic("duppage: %e", r);

    }
	//panic("duppage not implemented");

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
	// LAB 4: Your code here.
    set_pgfault_handler(pgfault);

    // Fork a child env, here will return twice
    envid_t child_id = sys_exofork();
    if (child_id < 0)
        panic("fork: %e", child_id);

    // In the child env 
    if (child_id == 0) {
        // Adjust thisenv to the true one
        thisenv = &envs[ENVX(sys_getenvid())];
        return 0;
    }

    // Copy the memory map to the child env
    for (uintptr_t cur_addr = 0; cur_addr < USTACKTOP; cur_addr += PGSIZE) {
        if ((uvpd[PDX(cur_addr)] & PTE_P) && 
                ((uvpt[PGNUM(cur_addr)] & (PTE_P | PTE_U)) == (PTE_P | PTE_U)))
            duppage(child_id, PGNUM(cur_addr));
    }

    // Allocate a page for child env for its exception stack
    int rtn = sys_page_alloc(child_id, (void *)UXSTACKTOP - PGSIZE, PTE_U | PTE_W | PTE_P);
    if (rtn < 0)
        panic("fork: %e", rtn);

    // Set up the fault page handler for child env
    extern void _pgfault_upcall(void);
    sys_env_set_pgfault_upcall(child_id, _pgfault_upcall); 

    // Change the status of child to runnable
    rtn = sys_env_set_status(child_id, ENV_RUNNABLE);

    if (rtn < 0)
        panic("fork: %e", rtn);

    return child_id;

    //panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
