#include "log.h"
#include "shitvec.h"

enum html_fc_type {
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

html_t html_new() {
	html_t out;
	out._buf = shitvec_new(sizeof(char));
	html_body_t body;
	body.content = shitvec_new(sizeof(html_fc_t));
	out.body = body;
	return out;
}

html_fc_t html_p_new(char* content) {
	html_fc_t out;
	out.type = HTML_P;
	out.value.text = content;
	return out;
}

void html_body_add(html_body_t* body, html_fc_t* fc) {
	shitvec_push(&body->content, &fc);
}

void c_sv_pushs(shitvec_t* sv, char* str) {
	for (int i = 0; str[i] != '\0'; i++) {
		shitvec_push(sv, &str[i]);
	}
}

char* html_render(html_t* html) {
	c_sv_pushs(&html->_buf, "<!DOCTYPE html><html>");
	c_sv_pushs(&html->_buf, "<body>");
	for (int i = 0; i < html->body.content.vec_sz; i++) {
		html_fc_t* elem = shitvec_get(&html->body.content, i);
		switch (elem->type) {
		case HTML_P:
			c_sv_pushs(&html->_buf, "<p>");
			c_sv_pushs(&html->_buf, elem->value.text);
			c_sv_pushs(&html->_buf, "</p>");
			break;
		default:
			log_error("Didn't handle rendering HTML element!");
		}		
	}
	c_sv_pushs(&html->_buf, "</body>");
	c_sv_pushs(&html->_buf, "</html>");
}
