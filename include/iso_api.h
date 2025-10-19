#ifndef ISO_API_H_
#define ISO_API_H_

#include "common.h"
#include "pages.h"
#include "sbank.h"
#include "thread.h"

struct iso_buffer_header {
    void *data;
    int data_size;
    unsigned int buffer_size;
    void *next;
};

struct file_record {
    char name[12];
    unsigned int location;
    unsigned int size;
};

struct load_stack_entry {
    struct file_record *fr;
    unsigned int location;
};

struct Buffer {
    u8 *decomp_buffer;
    int decompressed_size;
    struct Buffer *next;
    u8 *unk_12;
    int data_buffer_idx;
    int use_mode;
    struct PageList *plist;
    int num_pages;
    int unk_32;
    int free_pages;
    struct Page *page;
    int unk_44;
};

struct iso_message {
    struct MsgPacket pkt;
    int cmd_id;
    int status;
    int messagebox_to_reply;
    int thread_id;
    int ready_for_data;
    struct Buffer *cb_buf;
    int (*callback)(struct iso_message *, struct Buffer *);
    struct load_stack_entry *lse;
};

void UnLoadMusic(s32 *handle);
void LoadMusic(char *name, s32 *handle);
void LoadSoundBank(const char *name, struct BankRecord *rec);

#endif // ISO_API_H_
