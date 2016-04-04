#ifndef PARSE_H
#define PARSE_H

#include "../ttf.h"

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

void free_table(TTF_Table *table);
void free_font(TTF_Font *font);

TTF_Table *get_table(TTF_Font *font, uint32_t tag);

cmap_Table *get_cmap_table(TTF_Font *font);
cvt_Table *get_cvt_table(TTF_Font *font);
glyf_Table *get_glyf_table(TTF_Font *font);
head_Table *get_head_table(TTF_Font *font);
hhea_Table *get_hhea_table(TTF_Font *font);
loca_Table *get_loca_table(TTF_Font *font);
maxp_Table *get_maxp_table(TTF_Font *font);
post_Table *get_post_table(TTF_Font *font);

TTF_Table *get_table_by_name(TTF_Font *font, const char *name);

uint32_t get_glyph_index(TTF_Font *font, int16_t c);
TTF_Glyph *get_glyph(TTF_Font *font, int16_t c);

TTF_Font *parse_file(const char *filename);

void print_font_dir(TTF_Font *font);
void print_cmap_table(cmap_Table *cmap);
void print_head_table(head_Table *head);
void print_hhea_table(hhea_Table *hhea);
void print_table(TTF_Table *table);
void print_font(TTF_Font *font);

#endif /* PARSE_H */
