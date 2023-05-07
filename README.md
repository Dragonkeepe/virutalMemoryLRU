Designing a Virtual Memory Manager
This project consists of writing a program that translates logical to physical
addresses for a virtual address space of size 216 = 65,536 bytes. Your program
will read from a file containing logical addresses and, using a TLB and a page
table, will translate each logical address to its corresponding physical address
and output the value of the byte stored at the translated physical address.
Your learning goal is to use simulation to understand the steps involved in
translating logical to physical addresses. This will include resolving page faults
using demand paging, managing a TLB, and implementing a page-replacement
algorithm

Other specifics include the following:
• 28 entries in the page table
• Page size of 28 bytes
• 16 entries in the TLB
• Frame size of 28 bytes
• 256 frames
• Physical memory of 65,536 bytes (256 frames × 256-byte frame size)


Your program is to output the following values:
1. The logical address being translated (the integer value being read from
addresses.txt).
2. The corresponding physical address (what your program translates the
logical address to).
3. The signed byte value stored in physical memory at the translated physical address.
We also provide the file correct.txt, which contains the correct output
values for the file addresses.txt. You should use this file to determine if your
program is correctly translating logical to physical addresses.
Statistics
After completion, your program is to report the following statistics:
1. Page-fault rate—The percentage of address references that resulted in
page faults.
2. TLB hit rate—The percentage of address references that were resolved in
the TLB.


To compile and run the program
First:
gcc -o virtualMemory virtualMemory.c 
Then:
./virtualMemory

