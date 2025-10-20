#ifndef PAGES_H_
#define PAGES_H_

#include "common.h"

enum PageState {
    PAGE_FREE = 0,
    PAGE_ALLOCATED_EMPTY = 3,
    PAGE_ALLOCATED_FILLED = 4,
    PAGE_SIX = 6,
};

/*!
 * A linked list of pages associated with a single read.
 * Each "page" points to a buffer of memory.
 */
struct Page {
    // FREE = belongs to the pool, ALLOCATED_EMPTY = in a chain, but no data
    int state;

    int page_idx;

    // how many pages in the chain after this one
    int pages_after_this;

    // how many pages are ALLOCATED_EMPTY in the chain?
    int free_pages;

    // the memory for the page
    u8 *buffer;

    u8 *ptr;

    struct Page *prev;
    struct Page *next;
    struct Page *end_page_first_only;
};

struct PageList {
    u32 page_count;
    s32 page_size;
    u32 sector_per_page; // round down
    u32 free_pages;
    u8 *page_memory;
    struct Page *pages;
};

#endif // PAGES_H_
