#ifndef HTML_H
#define HTML_H

#include "shitvec.h"

enum html_fc_type {
	HTML_TEXT,
	HTML_A,
	HTML_P,
	HTML_BR,
	HTML_H1,
};

typedef struct html_fc {
	enum html_fc_type type;
	union {
		char* text;
		struct { char* href; char* content; } a;
	} value;
} html_fc_t;

typedef struct title {
	char* text;
} html_title_t;

typedef struct base {
	char* href;
	char* target;
} html_base_t;

typedef struct head {
	shitvec_t meta;
} html_head_t;

typedef struct body {
	shitvec_t content;
} html_body_t;

typedef struct html {
	shitvec_t _buf;
	html_body_t body;	
} html_t;

html_t html_new();
html_fc_t html_p_new(char* content);
html_fc_t html_h1_new(char* content);
void html_body_add(html_body_t* body, html_fc_t fc);
char* html_render(html_t* html);

#endif
