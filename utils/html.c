#include "html.h"
#include "log.h"

html_t html_new() {
	html_t out;
	out._buf = vec_new(sizeof(char));
	html_body_t body;
	body.content = vec_new(sizeof(html_fc_t));
	out.body = body;
	return out;
}

html_fc_t html_p_new(char* content) {
	html_fc_t out;
	out.type = HTML_P;
	out.value.text = content;
	return out;
}

html_fc_t html_h1_new(char* content) {
	html_fc_t out;
	out.type = HTML_H1;
	out.value.text = content;
	return out;
}
 
void html_body_add(html_body_t* body, html_fc_t fc) {
	vec_push(&body->content, &fc);
}

void c_sv_pushs(vec_t* sv, char* str) {
	for (int i = 0; str[i] != '\0'; i++) {
		vec_push(sv, &str[i]);
	}
}

char* html_render(html_t* html) {
	c_sv_pushs(&html->_buf, "<!DOCTYPE html><html>");
	c_sv_pushs(&html->_buf, "<body>");
	for (int i = 0; i < html->body.content.vec_sz; i++) {
		html_fc_t* elem = vec_get(&html->body.content, i);
		switch (elem->type) {
		case HTML_TEXT:
			
		case HTML_P:
			c_sv_pushs(&html->_buf, "<p>");
			c_sv_pushs(&html->_buf, elem->value.text);
			c_sv_pushs(&html->_buf, "</p>");
			break;
		case HTML_H1:
			c_sv_pushs(&html->_buf, "<h1>");
			c_sv_pushs(&html->_buf, elem->value.text);
			c_sv_pushs(&html->_buf, "</h1>");
			break;
		default:
			log_error("Didn't handle rendering HTML element!");
		}		
	}
	c_sv_pushs(&html->_buf, "</body>");
	c_sv_pushs(&html->_buf, "</html>");
	vec_push(&html->_buf, "\0");

	return html->_buf.arr;
}
