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

	char buf[BUFLEN];
	unsigned frame = 0;
	unsigned logic_add;                  // read from file address.txt
	unsigned virt_add, phys_add, value;  // read from file correct.txt

	// ------------------------------------------------------------------------
	PageTable<FRAME_SIZE, FRAME_COUNT, PAGE_COUNT, TLB_COUNT>
			table("BACKING_STORE.bin");
	// ------------------------------------------------------------------------

	printf("Beginning tests...\n\n");

	while (frame < 1000) {

		fscanf(fcorr, "%s %s %d %s %s %d %s %d", buf, buf, &virt_add,
				buf, buf, &phys_add, buf, &value); // read from file correct.txt

		fscanf(fadd, "%d", &logic_add);  // read from file address.txt

		// Get the addresses, then load the data
		LogicalAddress addr(logic_add);
		byte* actual_addr = table.load(addr);
		u64 p_addr = table.calcRelativeAddress(addr);

		printf("\t  FOUND: logical: %5I32u (page: %3u, offset: %3u) "
				"---> physical: %5I64u ---> %4d (sys_addr: %p)\n",
				static_cast<u32>(addr), addr.page(), addr.offset(),
				(intptr_t) (p_addr), *actual_addr, actual_addr);
		printf("\tCORRECT: logical: %5u                          "
				"---> physical: %5u ---> %4d\n", logic_add, phys_add, value);

		assert(addr == virt_add);
		assert(*actual_addr == (byte )value);

		frame++;
		if (frame % 5 == 0) {
			printf("\n");
		}
	}
	fclose(fcorr);
	fclose(fadd);

	printf("ALL logical ---> physical assertions PASSED!\n");

	table.showStats();

	printf("\n\t\t...done.\n");
	return 0;
}
