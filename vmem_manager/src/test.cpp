//
//  test.cpp
//
//  Built upon memmgr.c
//  Created by William McCarthy on 17/11/20.
//  Copyright © 2020 William McCarthy. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "page_table.hpp"

#define ARGC_ERROR 1
#define FILE_ERROR 2
#define BUFLEN 256

#define FRAME_SIZE 256
#define PAGE_COUNT 256
#define FRAME_COUNT 128
#define TLB_COUNT 16

template<u16 _FRAME_SIZE, u16 MAX_FRAME, u16 MAX_PAGE, u16 TLB_SIZE>
void run_tests(FILE* fadd, FILE* fcorr, PageTable<_FRAME_SIZE, MAX_FRAME, MAX_PAGE, TLB_SIZE>& table) {

	char buf[BUFLEN];
	unsigned frame = 0;
	unsigned logic_add;                  // read from file address.txt
	unsigned virt_add, phys_add, value;  // read from file correct.txt
	u16 last_page_fault_count = 0, last_tlb_hit_count = 0;

	while (fscanf(fadd, "%d", &logic_add) != EOF) {  // read from file address.txt

		fscanf(fcorr, "%s %s %d %s %s %d %s %d", buf, buf, &virt_add,
				buf, buf, &phys_add, buf, &value); // read from file correct.txt

		// Get the addresses, then load the data
		LogicalAddress addr(logic_add);
		byte* actual_addr = table.load(addr);
		u64 p_addr = table.calcRelativeAddress(addr);

		printf("\t  FOUND: logical: %5u (page: %3u, offset: %3u) "
				"---> physical: %5lu ---> %4d (sys_addr: %p)",
				static_cast<unsigned int>(addr), addr.page(), addr.offset(),
				(unsigned long int) (p_addr), *actual_addr, actual_addr);
		if(last_page_fault_count < table.pageFaultCount())
			printf("\tPAGE FAULT");
		if(last_tlb_hit_count < table.tlbHitCount())
			printf("\tTLB HIT");
		printf("\n\tCORRECT: logical: %5u ---> %4d\n", logic_add, value);

		last_page_fault_count = table.pageFaultCount();
		last_tlb_hit_count = table.tlbHitCount();

		assert(addr == virt_add);
		assert(*actual_addr == (byte)value);

		frame++;
		if (frame % 5 == 0) {
			printf("\n");
		}
		if(frame % 50 == 0) {
			printf("\t=============================== %5d READS =================================\n\n", frame);
		}
	}

	printf("\tTest Completed.\n");
	fflush(stdout);

	table.showStats();

}

int main(int argc, const char* argv[]) {

	FILE* fadd = fopen("addresses.txt", "r"); // open file addresses.txt  (contains the logical addresses)
	if (fadd == NULL) {
		fprintf(stderr, "Could not open file: 'addresses.txt'\n");
		exit(FILE_ERROR);
	}

	FILE* fcorr = fopen("correct.txt", "r"); // contains the logical and physical address, and its value
	if (fcorr == NULL) {
		fprintf(stderr, "Could not open file: 'correct.txt'\n");
		exit(FILE_ERROR);
	}

	printf("Beginning tests...\n\n");

	// ------------------------------------------------------------------------
	{
		// Run the test where FRAME_COUNT = PAGE_COUNT (page fault once per frame at most! yay!)
		printf("\tPerforming test with matching page and frame count (256)!\n\n");
		PageTable<FRAME_SIZE, PAGE_COUNT, PAGE_COUNT, TLB_COUNT> table_A("BACKING_STORE.bin");
		run_tests(fadd, fcorr, table_A);
	}

	fseek(fadd, 0, SEEK_SET);
	fseek(fcorr, 0, SEEK_SET);

	{
		// Run the test where FRAME_COUNT != PAGE_COUNT (oh no! so many page faults!)
		printf("\n\n\n\tPerforming test with twice as many pages (256) as frames (128)!\n\n");
		PageTable<FRAME_SIZE, FRAME_COUNT, PAGE_COUNT, TLB_COUNT> table_B("BACKING_STORE.bin");
		run_tests(fadd, fcorr, table_B);
	}
	// ------------------------------------------------------------------------

	fclose(fcorr);
	fclose(fadd);

	printf("\n\t\t...done.\n");
	return 0;
}
