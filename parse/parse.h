#ifndef PARSE_H
#define PARSE_H

#include "../base/types.h"

uint8_t read_byte(int fd);
uint16_t read_hword(int fd);
uint32_t read_word(int fd);
uint64_t read_dword(int fd);
uint16_t read_ushort(int fd);
int16_t read_short(int fd);
uint32_t read_ulong(int fd);
uint32_t read_fixed(int fd);
uint32_t read_tag(int fd);
int64_t read_longdatetime(int fd);

float fixed_to_float(uint32_t fixed);
uint32_t s_to_tag(const char *s);

uint32_t calc_table_check_sum(uint32_t *data, uint32_t length);
int validate_check_sums(TTF_Font *font);

int read_font_dir(TTF_Font *font);

int read_table_raw(TTF_Font *font, TTF_Table *table, uint32_t *buf);
int load_tables(TTF_Font *font);

void free_font(TTF_Font *font);

TTF_Font *parse_file(const char *filename);

void print_cmap_table(cmap_Table *cmap);
void print_head_table(head_Table *head);
void print_hhea_table(hhea_Table *hhea);
void print_table(TTF_Table *table);
void print_font_dir(TTF_Font *font);
void print_font(TTF_Font *font);

extern const char *mac_encoding[258];

#endif /* PARSE_H */
