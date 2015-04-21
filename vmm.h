#define TLB_SIZE 16
#define PAGES 256
#define PAGE_SIZE 256
#define MEMORY_SIZE PAGES * PAGE_SIZE

struct tlbEntry {
    unsigned char logical, physical;
};

struct tlbEntry tlb[TLB_SIZE], *entry;

int i, pageTable[PAGES], backingFd, logicalAddress, physicalAddress, logicalPage, physicalPage;
int tlbIndex, totalAddresses, tlbHits, pageFaults = 0;
char buffer[10];
unsigned char freePage = 0;
signed char mainMemory[MEMORY_SIZE], *backing, value;
FILE * addressFile;
