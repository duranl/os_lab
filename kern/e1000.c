#include <kern/e1000.h>

// LAB 6: Your driver code here
//struct e1000_tx_desc tx_desc_buf[TXRING_LEN] __attribute__ ((aligned (PGSIZE)));
//struct e1000_data tx_data_buf[TXRING_LEN] __attribute__ ((aligned (PGSIZE)));

//struct e1000_rx_desc rx_desc_buf[RXRING_LEN] __attribute__ ((aligned (PGSIZE)));
//struct e1000_data rx_data_buf[RXRING_LEN] __attribute__ ((aligned (PGSIZE)));


static void e1000_init() {
    assert(e1000[E1000_STATUS] == 0x80080783);
    //e1000[E1000_TDBAL] = PADDR(tx_desc_buf);
    e1000[E1000_TDBAH] = 0x0;
    e1000[E1000_TDH] = 0x0;
    e1000[E1000_TDT] = 0x0;
    e1000[E1000_TDLEN] = TXRING_LEN * sizeof(struct e1000_tx_desc);
    e1000[E1000_TCTL] = VALUEATMASK(1, E1000_TCTL_EN) |
                        VALUEATMASK(1, E1000_TCTL_PSP) |
                        VALUEATMASK(0x10, E1000_TCTL_CT) |
                        VALUEATMASK(0x40, E1000_TCTL_COLD);
    e1000[E1000_TIPG] = VALUEATMASK(10, E1000_TIPG_IPGT)|
                        VALUEATMASK(8, E1000_TIPG_IPGR1) |
                        VALUEATMASK(6, E1000_TIPG_IPGR2);
}


int e1000_attach(struct pci_func *pcif) {
    // Enable PCI function
    pci_func_enable(pcif);
    // Init descriptor
    //init_desc();
    // Create virtual memory mapping
    e1000 = mmio_map_region(pcif->reg_base[0],
                pcif->reg_size[0]);
    // Check the status register
    assert(e1000[E1000_STATUS] == 0x80080783);
    // Init the hardware
    //e1000_init();
    return 0;
}
