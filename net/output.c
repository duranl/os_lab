#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
    envid_t from_envid;
    uint32_t req;
    while (1) {
        req = ipc_recv(&from_envid, &nsipcbuf, NULL);
        if (from_envid != ns_envid || req != NSREQ_OUTPUT)
            continue;

        while (sys_net_tx(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len) < 0) {
            sys_yield();
        }
    }
}
