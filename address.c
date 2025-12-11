#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lab4.h"


Page_Entry* page_table;
TLB_Table tlb;
Main_Memory main_mem;
FILE* backing_store_fp;

// Implement get_page_from() and get_offset_from() functions below this line


unsigned char get_page_from(int logical_address){
    int Page_Number = (logical_address >> 8) & 0x00FF;
    return (unsigned char)(Page_Number);
}

unsigned char get_offset_from(int logical_address){
    int Offset = logical_address & 0x00FF;
    return (unsigned char)(Offset);
}


// Implement the page table functions below this line
void init_page_table(){
    page_table = (Page_Entry*)malloc(256 * sizeof(Page_Entry));
    for (int i = 0; i < 256; i++){
        page_table[i].is_valid = 0;
    }
}

void close_page_table(){
    free(page_table);
}


short page_table_lookup(unsigned char page){
    if (page_table[page].is_valid){
        return page_table[page].frame;
    }
    increment_page_fault_count();
    int free_frame = main_mem.next_available_frame;
    roll_in(page, free_frame);
    update_page_table(page, free_frame);
    update_tlb(page, free_frame);
    main_mem.next_available_frame++;
    return free_frame;
}

void update_page_table(unsigned char page, unsigned char frame){
    page_table[page].frame = frame;
    page_table[page].is_valid = 1;
}



// Implement the TLB functions below this line

void init_tlb(){
    tlb.entry = (TLB_Entry*)malloc(TLB_LEN * sizeof(TLB_Entry));
    tlb.head = 0;
    tlb.tail = 0;
    tlb.length = 0;
    tlb.is_full = 0;

    for(int i = 0; i < TLB_LEN; i++){
        tlb.entry[i].is_valid = 0;
    }
}


short tlb_lookup(unsigned char page){
    increment_total_page_request();

    for(int i = 0; i < TLB_LEN; i++){
        if(tlb.entry[i].is_valid && tlb.entry[i].page == page){
            increment_tlb_hit();
            return tlb.entry[i].frame;
        }
    }
    return -1;
}

void update_tlb(unsigned char page, unsigned char frame){
    int index = tlb.tail;

    tlb.entry[index].page = page;
    tlb.entry[index].frame = frame;
    tlb.entry[index].is_valid = 1;

    tlb.tail = (tlb.tail + 1) % TLB_LEN;

    if(tlb.is_full){
        tlb.head = (tlb.head + 1) % TLB_LEN;
    }else{
        tlb.length++;
        if(tlb.length == TLB_LEN){
            tlb.is_full = 1;
        }
    }
}

void close_tlb(){
    free(tlb.entry);
}


// Implement the Physical Memory functions below this line
void init_physical_memory(){
    main_mem.mem = (char*)malloc(256 * 256 * sizeof(char));
    main_mem.next_available_frame = 0;
}

void free_physical_memory(){
    free(main_mem.mem);
}

signed char get_value(int logical_address){
    increment_total_page_request();
    unsigned char page = get_page_from(logical_address);
    unsigned char offset = get_offset_from(logical_address);
    int frame = page_table_lookup(page);
    int physical_address = (frame << 8) | offset;
    return main_mem.mem[physical_address];
}

// Implement the Backing Store functions below this line
void init_backing_store(char* filename){
    backing_store_fp = fopen(filename, "rb");
    if(backing_store_fp == NULL){
        fprintf(stderr, "Error: Backing store file could not be opened %s\n", filename);
        exit(1);
    }
}

int roll_in(unsigned char page, unsigned char frame){
    if(fseek(backing_store_fp, page * 256, SEEK_SET) != 0){
        return -1;
    }

    char* dest_address = main_mem.mem + (frame * 256);

    if (fread(dest_address, sizeof(char), 256, backing_store_fp) < 256){
    }

    return 0;
}

void close_backing_store(){
    if (backing_store_fp != NULL){
        fclose(backing_store_fp);
    }
}
