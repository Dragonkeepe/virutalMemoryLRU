#include <stdio.h>
#include <stdlib.h>

#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256
#define FRAME_SIZE 256
#define PHYSICAL_MEMORY_SIZE (FRAME_SIZE * PAGE_TABLE_SIZE)
#define BACKING_STORE_FILE "BACKING_STORE.bin"
#define ADDRESS_FILE "addresses.txt"

typedef struct {
    int page_number;
    int frame_number;
} TLBEntry;

typedef struct {
    int frame_number;
    int valid;
} PageTableEntry;

void extractPageNumberAndOffset(int logical_address, unsigned char *page_number, unsigned char *offset) {
    *page_number = (logical_address >> 8) & 0xFF;
    *offset = logical_address & 0xFF;
}

int main() {
    FILE *address_file = fopen(ADDRESS_FILE, "r");
    FILE *backing_store = fopen(BACKING_STORE_FILE, "rb");

    if (address_file == NULL) {
        fprintf(stderr, "Error opening the address file.\n");
        return 1;
    }

    if (backing_store == NULL) {
        fprintf(stderr, "Error opening the backing store file.\n");
        fclose(address_file);
        return 1;
    }

    TLBEntry tlb[TLB_SIZE];
    PageTableEntry page_table[PAGE_TABLE_SIZE];
    signed char physical_memory[PHYSICAL_MEMORY_SIZE];

    // Initialize TLB and page table
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].page_number = -1;
        tlb[i].frame_number = -1;
    }

    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        page_table[i].frame_number = i;
        page_table[i].valid = 0;
    }

    int logical_address;
    int page_faults = 0;
    int tlb_hits = 0;
    int num_addresses = 0;

    // Read logical addresses from the address file
    while (fscanf(address_file, "%d", &logical_address) != EOF) {
        num_addresses++;

        // Extract page number and offset
        printf("Read logical address: %d\n", logical_address);

        unsigned char page_number, offset;
        extractPageNumberAndOffset(logical_address, &page_number, &offset);

        // Check if the page number is in the TLB
        int frame_number = -1;
        for (int i = 0; i < TLB_SIZE; i++) {
            if (tlb[i].page_number == page_number) {
                frame_number = tlb[i].frame_number;
                tlb_hits++;
                break;
            }
        }

        // If TLB miss, check the page table
        if (frame_number == -1) {
            if (page_table[page_number].valid == 1) {
                frame_number = page_table[page_number].frame_number;
            } else {
                // Page fault - read the page from the backing store
                fseek(backing_store, page_number * PAGE_SIZE, SEEK_SET);
                fread(physical_memory + (FRAME_SIZE * page_number), sizeof(signed char), PAGE_SIZE, backing_store);
                page_table[page_number].valid = 1;
                frame_number = page_table[page_number].frame_number;
                page_faults++;
            }
            
            // Update TLB
            int lru_index = 0;
            int lru_counter = tlb[0].frame_number;
            
            // Find the least recently used entry in the TLB
            for (int i = 1; i < TLB_SIZE; i++) {
                if (tlb[i].frame_number < lru_counter) {
                    lru_index = i;
                    lru_counter = tlb[i].frame_number;
                }
            }
            
            // Replace the least recently used entry in the TLB
            tlb[lru_index].page_number = page_number;
            tlb[lru_index].frame_number = frame_number;
        }

        // Calculate the physical address using the frame number and offset
        int physical_address = (frame_number * FRAME_SIZE) + offset;

        // Ensure the physical address is within the valid range
        if (physical_address >= PHYSICAL_MEMORY_SIZE || physical_address < 0) {
            printf("Invalid physical address. Physical address: %d\n", physical_address);
            continue;  // Move on to the next logical address
        }

        // Retrieve the value from physical memory at the physical address
        signed char value = physical_memory[physical_address];

        // Print the results
        printf("Logical address: %d - Physical address: %d - Value: %d\n", logical_address, physical_address, value);
    }

    // Print the statistics
    printf("\nNumber of Translated Addresses = %d\n", num_addresses);
    printf("Page Faults = %d\n", page_faults);
    printf("Page Fault Rate = %.3f\n", (float) page_faults / num_addresses);
    printf("TLB Hits = %d\n", tlb_hits);
    printf("TLB Hit Rate = %.3f\n", (float) tlb_hits / num_addresses);

    // Close the files
    fclose(address_file);
    fclose(backing_store);

    return 0;
}
