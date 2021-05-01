/*
 * page_table.hpp
 *
 *  Created on: Apr 9, 2021
 *      Author: Jacob's Gaming PC
 */

#ifndef PAGE_TABLE_HPP_
#define PAGE_TABLE_HPP_

#include <array>
#include <cstdio>
#include <fstream>
#include <list>
#include <optional>
#include <stdexcept>
#include <string>

#include "alias.hpp"
#include "logical_address.hpp"

using std::array;
using std::list;
using std::ifstream;
using std::string;
using std::optional;

template<u16 FRAME_SIZE, u16 MAX_FRAME, u16 MAX_PAGE, u16 TLB_SIZE>
class PageTable {

	struct FrameMeta {
		u16 associated_page;
		optional<list<u16>::iterator> last_usage;
	};

	class TLB {

		struct Entry {
			u8 page;
			byte* p_addr_base = nullptr;
		};

		array<Entry, TLB_SIZE> table;
		list<u16> age_stack;

	public:

		TLB() {
			for (u16 i = 0; i < TLB_SIZE; ++i)
				age_stack.push_front(i);
		}

		byte* tryFind(u8 page) {

			for(Entry& entry : table)
				if(entry.page == page)
					return entry.p_addr_base;

			return nullptr;

		}

		void ensureResident(u8 page, byte* p_addr_base) {

			u16 replace = age_stack.back();
			age_stack.pop_back();
			age_stack.push_front(replace);
			table.at(replace).page = page;
			table.at(replace).p_addr_base = p_addr_base;

		}

	};

	array<byte[FRAME_SIZE], MAX_FRAME> main_memory;
	array<FrameMeta, MAX_FRAME> frame_metadata;
	array<optional<u16>, MAX_PAGE> page_to_frame;
	list<u16> age_stack;
	TLB tlb;
	u16 request_count = 0, page_fault_count = 0, tlb_hit_count = 0;

	ifstream backing;

	// Modifies the age stack such that the frame at frame_index is now the
	// most recently used frame. Should be called after every meaningful use
	// of this frame.
	u16 touchFrame(u16 frame_index) {

		FrameMeta& meta = frame_metadata.at(frame_index);

		// Move the this frame's iter to the front of the stack
		if (meta.last_usage.has_value())
			age_stack.erase(meta.last_usage.value());
		age_stack.push_front(frame_index);

		// Update the frame's iter to the new position
		meta.last_usage = age_stack.begin();

		return frame_index;
	}

	// Returns the index of an invalid frame. Will invalidate an existing frame
	// if no invalid frames exist.
	u16 openFrame() {

		// Pick the least recently used frame to invalidate
		// NOTE: Since the age stack is filled with every frame in the
		//		 class constructor, every frame will be used once before
		//		 any frames get reused
		++page_fault_count;
		u16 victim_index = age_stack.back();
		age_stack.pop_back();
		FrameMeta& meta = frame_metadata.at(victim_index);

		// Invalidate the victim frame and return it
		if (meta.last_usage.has_value())
			page_to_frame.at(meta.associated_page).reset(); // reset page as well, if a page is associated
		meta.last_usage.reset();
		return victim_index;

	}

	// Returns the frame_index corresponding to a page_index
	// If the referenced page isn't in memory, the page will be loaded
	// into a frame before returning that new frame's index.
	u16 pageToFrame(u16 page_index) {

		optional<u16>& frame = page_to_frame.at(page_index);

		// Check if the page is already in memory
		if (frame.has_value())
			return touchFrame(frame.value());

		// Grab an invalid frame and update it to be used by this page
		frame = openFrame();
		frame_metadata.at(frame.value()).associated_page = page_index;

		// Put the corresponding backing page's contents into the frame
		backing.seekg(FRAME_SIZE * page_index);
		backing.read(main_memory.at(frame.value()), FRAME_SIZE);

		// Touch and return
		return touchFrame(frame.value());
	}

public:

	PageTable(string filepath) {
		backing.open(filepath, std::ifstream::in | std::ifstream::binary);

		// Fill the age stack with every frame ID. Since every frame is
		// equally unused at launch, order doesn't really matter.
		for (u16 i = 0; i < MAX_FRAME; ++i)
			age_stack.push_front(i);
	}

	// Calculates the address (relative to the page table's physical memory)
	// corresponding to the given LogicalAddress. This only works if the given
	// LogicalAddress is currently loaded into memory
	u64 calcRelativeAddress(LogicalAddress addr) {
		if (!page_to_frame.at(addr.page()).has_value())
			throw std::invalid_argument("The data referenced by this "
					"logical address has not been loaded!");
		return pageToFrame(addr.page()) * FRAME_SIZE + addr.offset();
	}

	// Loads the data at a LogicalAddress and returns the physical address
	byte* load(LogicalAddress addr) {

		++request_count; // for statistics purposes only

		// Attempt to get the address from the TLB
		byte* p_addr_base = tlb.tryFind(addr.page());
		if (p_addr_base != nullptr) {
			++tlb_hit_count;
			return p_addr_base + addr.offset();
		}

		// TLB Missed. Calculate physical address, pulling the
		// frame into memory if necessary

		p_addr_base = main_memory.at(pageToFrame(addr.page()));
		tlb.ensureResident(addr.page(), p_addr_base);
		return p_addr_base + addr.offset();
	}

	constexpr u16 pageFaultCount() const {
		return page_fault_count;
	}

	constexpr u16 tlbHitCount() const {
		return tlb_hit_count;
	}

	void showStats() const {
		printf("\t    PAGE FAULTS: %-5d     \t    TLB HITS: %-5d\n",
					page_fault_count, tlb_hit_count);
		printf("\tPAGE FAULT RATE: %2.2f%%  \tTLB HIT RATE: %2.2f%%",
					page_fault_count/(float)request_count*100, tlb_hit_count/(float)request_count*100);
	}

};

#endif /* PAGE_TABLE_HPP_ */
