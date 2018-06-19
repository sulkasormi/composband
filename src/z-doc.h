#ifndef INCLUDED_Z_DOC_H
#define INCLUDED_Z_DOC_H

#include "h-basic.h"
#include "rect.h"

#include "str-map.h"
#include "int-map.h"
#include "c-vec.h"
#include "c-string.h"

/* Utilities for Formatted Text Processing
   We support the ability to render rich text to a "virtual terminal",
   and then rapidly transfer a region of rendered text to the actual
   terminal for display. This is useful any time you want to support
   scrolling, wordwrapping, or color coded text (e.g. messages or
   the help system). We also support linked navigation, named styles and
   named bookmarks (called topics).

   The virtual terminal is a fixed width beast, but can grow its height
   dynamically. This is the way a normal "document" behaves: It grows downward
   as you type in new content.

   We support tags to control output and provide help features:
     <color:x> where x is one of the color codes:dwsorgbuDWvyRGBU*
     <style:name>
     <topic:name>
     <link:file#topic>
     <$:var> where var is a valid variable reference (e.g. <$:version> for news.txt)
*/

/* The Datatypes
   Key concepts are a position (for the cursor), a region (for transfers),
   a character (for contents and color), a style (for named formatting) and,
   of course, a document.

   Sample Usage:
    void test(int width)
    {
        doc_ptr doc = doc_alloc(width);

        doc_read_file(doc, stdin);
        doc_write_file(doc, stdout);

        doc_free(doc);
    }
*/

struct doc_pos_s
{
    int x;
    int y;
};
typedef struct doc_pos_s doc_pos_t;

doc_pos_t doc_pos_create(int x, int y);
doc_pos_t doc_pos_invalid(void);
bool      doc_pos_is_valid(doc_pos_t pos);
int       doc_pos_compare(doc_pos_t left, doc_pos_t right);

struct doc_region_s
{/* [start, stop) */
    doc_pos_t start;
    doc_pos_t stop;
};
typedef struct doc_region_s doc_region_t, *doc_region_ptr;

doc_region_t doc_region_create(int x1, int y1, int x2, int y2);
doc_region_t doc_region_invalid(void);
bool doc_region_is_valid(doc_region_ptr region);
bool doc_region_contains(doc_region_ptr region, doc_pos_t pos);
int  doc_region_line_count(doc_region_ptr region);

struct doc_char_s
{
    char c;
    byte a; /* attribute */
};
typedef struct doc_char_s doc_char_t, *doc_char_ptr;

struct doc_style_s
{
    byte color;
    int  left;
    int  right;
    int  indent;
    int  options;
};
typedef struct doc_style_s doc_style_t, *doc_style_ptr;
enum doc_style_options_e
{
    DOC_STYLE_NO_WORDWRAP = 0x0001,
};

typedef void (*doc_style_f)(doc_style_ptr style);

struct doc_bookmark_s
{
    string_ptr name;
    doc_pos_t  pos;
};
typedef struct doc_bookmark_s doc_bookmark_t, *doc_bookmark_ptr;

struct doc_link_s
{
    string_ptr   file;
    string_ptr   topic;
    doc_region_t location;
};
typedef struct doc_link_s doc_link_t, *doc_link_ptr;

struct doc_s
{
    doc_pos_t      cursor;
    doc_region_t   selection;
    int            width;
    vec_ptr        pages;
    str_map_ptr    styles;
    vec_ptr        bookmarks;
    int_map_ptr    links;
    vec_ptr        style_stack;
    string_ptr     name;
    string_ptr     html_header;
};
typedef struct doc_s doc_t, *doc_ptr;


/* Document API
   All text is added at the current location. The intention is that you build your
   document up front (by reading a help file, for example), and then use the
   copy_to_term() method to render to the display one screen at a time, depending
   on the current scroll position.
*/

doc_ptr       doc_alloc(int width);
void          doc_free(doc_ptr doc);

doc_pos_t     doc_cursor(doc_ptr doc);

doc_region_t  doc_range_all(doc_ptr doc);
doc_region_t  doc_range_selection(doc_ptr doc);
doc_region_t  doc_range_top(doc_ptr doc, doc_pos_t stop);
doc_region_t  doc_range_top_lines(doc_ptr doc, int count);
doc_region_t  doc_range_bottom(doc_ptr doc, doc_pos_t start);
doc_region_t  doc_range_bottom_lines(doc_ptr doc, int count);
doc_region_t  doc_range_middle(doc_ptr doc, doc_pos_t start, doc_pos_t stop);
doc_region_t  doc_range_middle_lines(doc_ptr doc, int start_line, int stop_line);

int           doc_line_count(doc_ptr doc);
int           doc_width(doc_ptr doc);

doc_pos_t     doc_next_bookmark(doc_ptr doc, doc_pos_t pos);
doc_pos_t     doc_prev_bookmark(doc_ptr doc, doc_pos_t pos);
doc_pos_t     doc_find_bookmark(doc_ptr doc, cptr name);

doc_pos_t     doc_find_next(doc_ptr doc, cptr text, doc_pos_t start);
doc_pos_t     doc_find_prev(doc_ptr doc, cptr text, doc_pos_t start);

              /* <style:foo>Some Text<style:*> */
void          doc_push_style(doc_ptr doc, doc_style_ptr style);
void          doc_push_named_style(doc_ptr doc, cptr name);
void          doc_pop_style(doc_ptr doc);
doc_style_ptr doc_current_style(doc_ptr doc);

              /* Build a document from a text file */
doc_pos_t     doc_insert(doc_ptr doc, cptr text);
doc_pos_t     doc_read_file(doc_ptr doc, FILE *fp);
              enum { DOC_FORMAT_TEXT, DOC_FORMAT_HTML, DOC_FORMAT_DOC };
void          doc_write_file(doc_ptr doc, FILE *fp, int format);

              /* Build a document in code */
doc_pos_t     doc_insert_char(doc_ptr doc, byte a, char c);
doc_pos_t     doc_insert_text(doc_ptr doc, byte a, cptr text);
doc_pos_t     doc_insert_doc(doc_ptr dest_doc, doc_ptr src_doc, int indent);
doc_pos_t     doc_insert_cols(doc_ptr dest_doc, doc_ptr src_cols[], int col_count, int spacing);
doc_pos_t     doc_insert_space(doc_ptr dest_doc, int count);
doc_pos_t     doc_newline(doc_ptr doc);
void          doc_rollback(doc_ptr doc, doc_pos_t pos);
void          doc_clear(doc_ptr doc);

doc_pos_t     doc_printf(doc_ptr doc, const char *fmt, ...);
doc_pos_t     doc_cprintf(doc_ptr doc, byte a, const char *fmt, ...);

doc_char_ptr  doc_char(doc_ptr doc, doc_pos_t pos);
void          doc_sync_term(doc_ptr doc, doc_region_t range, doc_pos_t term_pos);
void          doc_sync_menu(doc_ptr doc);

void          doc_change_name(doc_ptr doc, cptr name);
void          doc_change_html_header(doc_ptr doc, cptr header);
vec_ptr       doc_get_links(doc_ptr doc);

/* Parsing */
enum doc_tag_e
{
    DOC_TAG_NONE,
    DOC_TAG_COLOR,
    DOC_TAG_CLOSE_COLOR,
    DOC_TAG_STYLE,
    DOC_TAG_CLOSE_STYLE,
    DOC_TAG_TOPIC,
    DOC_TAG_LINK,
    DOC_TAG_VAR,
    DOC_TAG_INDENT,
    DOC_TAG_CLOSE_INDENT,
    DOC_TAG_TAB,
};
struct doc_tag_s
{
    int  type;
    cptr arg;
    int  arg_size;
};
typedef struct doc_tag_s doc_tag_t, *doc_tag_ptr;

enum doc_token_e
{
    DOC_TOKEN_EOF,
    DOC_TOKEN_TAG,
    DOC_TOKEN_WHITESPACE,
    DOC_TOKEN_NEWLINE,
    DOC_TOKEN_WORD,
};
struct doc_token_s
{
    int       type;
    cptr      pos;
    int       size;
    doc_tag_t tag;
};
typedef struct doc_token_s doc_token_t, *doc_token_ptr;

cptr doc_parse_tag(cptr pos, doc_tag_ptr tag);
cptr doc_lex(cptr pos, doc_token_ptr token);


int doc_display_help(cptr file_name, cptr topic);
int doc_display_help_aux(cptr file_name, cptr topic, rect_t display);
int doc_display(doc_ptr doc, cptr caption, int top);
int doc_display_aux(doc_ptr doc, cptr caption, int top, rect_t display);
int weapon_exp_display(doc_ptr doc, cptr caption, int *top);

#endif
