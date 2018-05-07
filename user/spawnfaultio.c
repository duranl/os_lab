#include <inc/lib.h>

#define DBG

void
umain(int argc, char **argv)
{
	int r;
	cprintf("i am parent environment %08x\n", thisenv->env_id);
	if ((r = spawnl("faultio", "faultio", 0)) < 0)
		panic("spawn(faultio) failed: %e", r);
#ifdef DBG
    cprintf("faultio passed!\n");
#endif
}
