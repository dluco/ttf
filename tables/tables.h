#ifndef TABLES_H
#define TABLES_H

#include "../base/types.h"

TTF_Table *get_table(TTF_Font *font, uint32_t tag);
TTF_Table *get_table_by_name(TTF_Font *font, const char *name);

cmap_Table *get_cmap_table(TTF_Font *font);
cvt_Table *get_cvt_table(TTF_Font *font);
glyf_Table *get_glyf_table(TTF_Font *font);
head_Table *get_head_table(TTF_Font *font);
hhea_Table *get_hhea_table(TTF_Font *font);
loca_Table *get_loca_table(TTF_Font *font);
maxp_Table *get_maxp_table(TTF_Font *font);
post_Table *get_post_table(TTF_Font *font);

void free_table(TTF_Table *table);

void free_cmap_table(cmap_Table *cmap);
void free_cvt_table(cvt_Table *cvt);
void free_glyf_table(glyf_Table *glyf);
void free_head_table(head_Table *head);
void free_hhea_table(hhea_Table *hhea);
void free_loca_table(loca_Table *loca);
void free_maxp_table(maxp_Table *maxp);
void free_post_table(post_Table *post);

#endif /* TABLES_H */
