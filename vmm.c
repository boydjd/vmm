#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "vmm.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Missing input file name!\n");
        return 1;
    }

    /* Open backing store and map into memory */
    backingFd = open("BACKING_STORE.bin", O_RDONLY);
    backing = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, backingFd, 0);

    /* Open address file */
    addressFile = fopen(argv[1], "r");

    /* Initialize page table, -1 means empty */ 
    for (i = 0; i < PAGES; i++)
        pageTable[i] = -1;

    /* Fetch addresses from file and process them individually */ 
    while (fgets(buffer, sizeof(buffer), addressFile) != NULL) {
        ++totalAddresses;
        logicalAddress = atoi(buffer);
        logicalPage = (logicalAddress >> 8) & 0x00FF;
        physicalPage = -1;

        /* Find the physical page in the TLB for the logical page */
        for (i = 0; i < tlbIndex; i++) {
            entry = &tlb[i % TLB_SIZE];

            if (entry->logical == logicalPage)
                physicalPage = entry->physical;
        }

        if (physicalPage != -1) {
            ++tlbHits;
        } else {
            /* Not present in the TLB */
            
            physicalPage = pageTable[logicalPage];

            /* Page fault. Whomp whomp. */
            if (physicalPage == -1) {
                ++pageFaults;
                physicalPage = freePage;
                ++freePage;
                pageTable[logicalPage] = physicalPage;

                /* Copy to main memory */
                memcpy(mainMemory + physicalPage * PAGE_SIZE, backing + logicalPage * PAGE_SIZE, PAGE_SIZE);

            }

            /* Add mapping to the TLB */
            entry = &tlb[tlbIndex % TLB_SIZE];
            entry->logical = (unsigned char) logicalPage;
            entry->physical = (unsigned char) physicalPage;
            ++tlbIndex;
        }

        physicalAddress = (physicalPage << 8) | (logicalAddress & 0x00FF);
        value = mainMemory[physicalPage * PAGE_SIZE + (logicalAddress & 0x00FF)];

        printf("Virtual address: %d Physical address: %d Value: %d\n", logicalAddress, physicalAddress, value);
    }

    printf("\nPage-fault rate\t%.1f%%\n", (pageFaults / (1. * totalAddresses))*100.);
    printf("TLB hit rate\t%.1f%%\n", (tlbHits / (1. * totalAddresses))*100.);

    return 0;
}
