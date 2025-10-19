#include "pages.h"

#include "common.h"
#include "overlord.h"
#include "sysmem.h"
#include <stdio.h>
#include <string.h>

struct Page *AllocPages(struct PageList *page_list, u32 num_pages);

INCLUDE_RODATA("asm/nonmatchings/pages", D_00013110);

#ifdef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/pages", InitPagedMemory);
#else
void InitPagedMemory(struct PageList *pool, int pages, int page_size) {
    struct Page *page;
    u8 *mem;
    int i;

    ScratchPadMemory += pages * sizeof(struct Page);

    pool->pages = (struct Page *)&pool[1];
    pool->page_count = pages;
    pool->page_size = page_size;
    pool->sector_per_page = page_size / 2048;
    pool->free_pages = pages;
    pool->page_memory = 0;

    mem = (u8 *)((u32)(AllocSysMemory(0, pages * page_size + 0x100, 0) + 63) & ~63);

    i = 0;
    if (!mem) {
        Kprintf("======================================================================\n");
        Kprintf("IOP: pages InitPagedMemory: no memory for pages\n");
        Kprintf("======================================================================\n");
        while (1)
            ;
    }

    pool->page_memory = mem;
    page = pool->pages;
    while (pages) {
        page->state = 0;
        page->buffer = mem;
        page->page_idx = i;
        page->free_pages = 0;
        page->ptr = mem - 1;
        page->prev = NULL;
        page->next = NULL;
        page->end_page_first_only = 0;
        mem += page_size;
        page++;
        pages--;
        i++;
    }
}
#endif

struct Page *AllocPagesBytes(struct PageList *page_list, s32 size_bytes) {
    u32 num_pages = (size_bytes + page_list->page_size - 1) / page_list->page_size;
    return AllocPages(page_list, num_pages);
}

INCLUDE_ASM("asm/nonmatchings/pages", AllocPages);

struct Page *FreePagesList(struct PageList *page_list, struct Page *pages) {
    struct Page *i, *n;

    if (!pages) {
        return NULL;
    }

    if (pages->prev) {
        Kprintf("======================================================================\n");
        Kprintf("IOP: pages FreePages: First page %d is not top of list\n", pages->page_idx);
        Kprintf("======================================================================\n");
        while (1)
            ;
    }

    i = pages;
    while (1) {
        n = i->next;
        i->prev = NULL;
        i->next = NULL;
        i->end_page_first_only = NULL;
        i->state = PAGE_FREE;
        page_list->free_pages = page_list->free_pages + 1;
        i = n;

        if (!i) {
            break;
        }
    }

    return NULL;
}

struct Page *StepTopPage(struct PageList *list, struct Page *top_page) {
    struct Page *next;
    if (!top_page) {
        return NULL;
    }

    if (top_page->prev) {
        Kprintf("======================================================================\n");
        Kprintf("IOP: pages StepTopPage: Page %d is not top of list\n", top_page->page_idx);
        Kprintf("======================================================================\n");
        while (1)
            ;
    }

    next = top_page->next;
    if (next) {
        next->free_pages = top_page->free_pages - 1;
        next->end_page_first_only = top_page->end_page_first_only;
        next->prev = NULL;
    }
    top_page->next = NULL;
    top_page->state = PAGE_FREE;
    list->free_pages++;

    return next;
}

#ifdef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/pages", FromPagesCopy);
#else
void FromPagesCopy(struct Page *page, u8 *page_ptr, u8 *dest, int bytes_to_copy) {
    u32 in_page_size = (page->ptr - page_ptr) + 1;

    while (bytes_to_copy) {
        if (bytes_to_copy < in_page_size) {
            memcpy(dest, page_ptr, bytes_to_copy);
            bytes_to_copy = 0;
        } else {
            memcpy(dest, page_ptr, in_page_size);
            dest += in_page_size;
            bytes_to_copy -= in_page_size;
            if (page->next) {
                page = page->next;
                page_ptr = page->buffer;
                in_page_size = (page->ptr - page_ptr) + 1;
            }
        }
    }
}
#endif
