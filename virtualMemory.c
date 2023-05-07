//Created by Jeffrey Bengis
//HW #4 5/5/23
#include <stdio.h>
#include <stdlib.h>

#define tlbSize 16
#define pageTableSize 256
#define pageSize 256
#define frameSize 256
#define physicalMemorySize (frameSize * pageTableSize)
#define backFile "BACKING_STORE.bin"
#define addressFile "addresses.txt"

typedef struct {
    int tlbPageNum;
    int tlbFrameNum;
} TLBEntry;

typedef struct {
    int pageFrameNum;
    int pageValid;
} PageTableEntry;

void extractPageNumberAndOffset(int logicalAddress, unsigned char *pageNumber, unsigned char *offset) {
    *pageNumber = (logicalAddress >> 8) & 0xFF;
    *offset = logicalAddress & 0xFF;
}

int main() {
    FILE *mainAddressFile = fopen(addressFile, "r");
    FILE *mainBackingStore = fopen(backFile, "rb");

    if (mainAddressFile == NULL) {
        fprintf(stderr, "Error opening the address file.\n");
        return 1;
    }

    if (mainBackingStore == NULL) {
        fprintf(stderr, "Error opening the backing store file.\n");
        fclose(mainAddressFile);
        return 1;
    }

    TLBEntry tlb[tlbSize];
    PageTableEntry pageTable[pageTableSize];
    signed char signedPhysicalMemory[physicalMemorySize];

    // Initialize TLB and page table
    for (int i = 0; i < tlbSize; i++) {
        tlb[i].tlbPageNum = -1;
        tlb[i].tlbFrameNum = -1;
    }

    for (int i = 0; i < pageTableSize; i++) {
        pageTable[i].pageFrameNum = -1;
        pageTable[i].pageValid = 0;
    }

    int intLogicalAddress;
    int intPageFaults = 0;
    int intTblHits = 0;
    int intNumOfAddresses = 0;
    int intNextFrame = 0;

    // Read logical addresses
    while (fscanf(mainAddressFile, "%d", &intLogicalAddress) != EOF) {
        intNumOfAddresses++;

        // Extract page number and unOffset
        printf("Read logical address: %d\n", intLogicalAddress);

        unsigned char unPageNum, unOffset;
        extractPageNumberAndOffset(intLogicalAddress, &unPageNum, &unOffset);

        // Check if the page number is in the TLB
        int intFrameNum = -1;

        for (int i = 0; i < tlbSize; i++) {

            if (tlb[i].tlbPageNum == unPageNum)
            {
                intFrameNum = tlb[i].tlbFrameNum;
                intTblHits++;
                break;
            }
        }

        // If TLB miss, check the page table
        if (intFrameNum == -1)
        {
            if (pageTable[unPageNum].pageValid == 1)
            {
                intFrameNum = pageTable[unPageNum].pageFrameNum;
            } else
            {
                // Page fault
                fseek(mainBackingStore, unPageNum * pageSize, SEEK_SET);
                fread(signedPhysicalMemory + (frameSize * intNextFrame), sizeof(signed char), pageSize, mainBackingStore);
                pageTable[unPageNum].pageFrameNum = intNextFrame;
                pageTable[unPageNum].pageValid = 1;
                intFrameNum = intNextFrame;
                intPageFaults++;
                intNextFrame = (intNextFrame + 1) % pageTableSize;
            }

            // Update TLB
            int intLRUIndex = 0;

            // Find the LRU
            for (int i = 1; i < tlbSize; i++)
            {
                if (tlb[i].tlbFrameNum < tlb[intLRUIndex].tlbFrameNum)
                {
                    intLRUIndex = i;
                }
            }

            // Replace the least recently used entry in the TLB
            tlb[intLRUIndex].tlbPageNum = unPageNum;
            tlb[intLRUIndex].tlbFrameNum = intFrameNum;
        }

        // Calculate the physical address
        int intPhysicalMemory = (intFrameNum * frameSize) + unOffset;

        //Error checking (this was annoying)
        if (intPhysicalMemory >= physicalMemorySize || intPhysicalMemory < 0)
        {
            printf("Invalid physical address. Physical address: %d\n", intPhysicalMemory);
            continue;  // Move on to the next logical address
        }

        // Retrieve value stored
        signed char value = signedPhysicalMemory[intPhysicalMemory];

        // Print the results
        printf("Virtual address: %d - Physical address: %d - Value: %d\n", intLogicalAddress, intPhysicalMemory, value);
    }

    // Print the statistics
    printf("\nNumber of Translated Addresses = %d\n", intNumOfAddresses);
    printf("Page Faults = %d\n", intPageFaults);
    printf("Page Fault Rate = %.3f\n", (float) intPageFaults / intNumOfAddresses);
    printf("TLB Hits = %d\n", intTblHits);
    printf("TLB Hit Rate = %.3f\n", (float) intTblHits / intNumOfAddresses);

    // Close the files
    fclose(mainAddressFile);
    fclose(mainBackingStore);

    return 0;
}
