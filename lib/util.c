/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#include "util.h"
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

HybridStack*
hybrid_stack_create()
{
    HybridStack *stack;

    stack = g_new0(HybridStack, 1);

    return stack;
}

void
hybrid_stack_push(HybridStack *stack, gpointer data)
{
    HybridStackNode *node;

    g_return_if_fail(stack != NULL);

    node = g_new0(HybridStackNode, 1);
    node->data = data;
    node->next = stack->head;

    stack->head = node;
}

gpointer
hybrid_stack_pop(HybridStack *stack)
{
    HybridStackNode *node;
    gpointer         data;

    g_return_val_if_fail(stack != NULL, NULL);

    if (!stack->head) {

        hybrid_debug_info("stack", "stack empty");

        return NULL;
    }

    node = stack->head;
    data = node->data;

    stack->head = node->next;

    g_free(node);

    return data;
}

gboolean
hybrid_stack_empty(HybridStack *stack)
{
    return stack->head == NULL;
}

typedef struct {
    gchar *tag_start;
    gchar *text_start;
    gchar *text_end;
    gchar *tag_end;
    gchar *tag_name;
} HtmlTag;

gchar*
hybrid_strip_html(const gchar *html)
{
    gchar *pos;
    gchar *html_start_enter;
    gchar *html_text_leave;
    gchar *tag_name;
    gchar *temp_html;
    gchar *temp;
    gchar *html_tail;
    gint   html_length;
    gint   temp_length;

    HybridStack *stack;


    HtmlTag *tag;

    stack = hybrid_stack_create();

    temp_html   = g_strdup(html);
    html_length = strlen(temp_html);
    html_tail   = temp_html + strlen(temp_html);

    pos = temp_html;

new_tag:
    for (; *pos && *pos != '<'; pos ++);

    if (*pos == '\0') {
        /*
         * in this case, the input string must have no
         * html tags, so just return the original string.
         */
        if (hybrid_stack_empty(stack)) {

            g_free(stack);

            return temp_html;

        } else {
            goto bad_format;
        }
    }

    /* we encounter '<', tag start. */
    tag = g_new0(HtmlTag, 1);
    tag->tag_start = pos;

    pos ++;

    for (html_start_enter = pos; *pos && *pos != ' ' && *pos != '>'; pos ++);

    if (*pos == '\0') {
        goto bad_format;
    }

    tag->tag_name = g_strndup(html_start_enter, pos - html_start_enter);

    for (; *pos && *pos != '>'; pos ++);

    if (*pos == '\0') {
        goto bad_format;
    }

    pos ++;

    /* we encounter '>', text start. */
    tag->text_start = pos;

    for (; *pos && *pos != '<'; pos ++);

    if (*pos == '\0') {
        goto bad_format;
    }

    pos ++;
    hybrid_stack_push(stack, tag);

next_tag:

    /* if pos is '/', then the tag stops. */
    if (*pos == '/') {

        html_text_leave = pos - 1;

        html_start_enter = pos + 1;

        if (*html_start_enter == '\0' ||
            *html_start_enter == '/' ||
            *html_start_enter == '>') {
            goto bad_format;
        }

        for (; *pos && *pos != '>'; pos ++);

        if (*pos == '\0') {
            goto bad_format;
        }

        tag_name = g_strndup(html_start_enter, pos - html_start_enter);

        pos ++;

        if (!(tag = hybrid_stack_pop(stack))) {

            g_free(tag_name);

            goto bad_format;
        }

        tag->text_end = html_text_leave;
        tag->tag_end = pos;

        if (g_strcasecmp(tag_name, tag->tag_name) != 0) {

            g_free(tag_name);

            goto bad_format;
        }

        /* A tag closed, strip the html start tag. */
        temp_length = tag->text_start - tag->tag_start;
        memmove(tag->tag_start, tag->text_start, html_tail - tag->text_start);

        /* move the tag stop pointers. */
        html_tail -= temp_length;
        tag->text_end -= temp_length;
        tag->tag_end  -= temp_length;

        html_length -= temp_length;
        *html_tail = '\0';

        /* strip the html end tag. */
        temp_length = tag->tag_end - tag->text_end;
        if (html_tail == tag->tag_end) {

            *(tag->text_end) = '\0';
            html_tail = tag->text_end;

            goto strip_finish;

        } else {
            memmove(tag->text_end, tag->tag_end, html_tail - tag->tag_end);
        }

        html_length -= temp_length;
        html_tail -= temp_length;
        *html_tail = '\0';

        g_free(tag_name);
        g_free(tag->tag_name);

        for (pos = tag->text_end; *pos && *pos != '<'; pos ++);

        if (*pos == '\0') {
            if (hybrid_stack_empty(stack)) { /**< successful */

                goto strip_finish;

            } else {
                g_free(tag);

                goto bad_format;
            }
        }

        pos ++;

        goto next_tag;

    } else {
        pos --;

        goto new_tag;
    }


bad_format:

    while (!hybrid_stack_empty(stack)) {

        tag = hybrid_stack_pop(stack);

        g_free(tag->tag_name);

        g_free(tag);
    }

    g_free(stack);
    g_free(temp_html);

    hybrid_debug_error("html", "bad format");

    return g_strdup(html);

strip_finish:

    temp = g_strndup(temp_html, html_tail - temp_html);

    g_free(tag);
    g_free(temp_html);
    g_free(stack);

    return temp;
}

gchar*
hybrid_sha1(const gchar *in, gint size)
{
    SHA_CTX s;
    guchar hash[20];
    gchar *res;
    gint i;

    SHA1_Init(&s);
    SHA1_Update(&s, in, size);
    SHA1_Final(hash, &s);

    res = g_malloc0(41);

    for (i=0; i < 20; i++) {
        g_snprintf(res + i * 2, 41, "%.2x", (gint)hash[i]);
    }

    return res;
}

gchar*
hybrid_base64_encode(const guchar *input, gint size)
{
  BIO     *bmem;
  BIO     *b64;
  BUF_MEM *bptr;
  gchar   *buff;

  b64  = BIO_new(BIO_f_base64());
  bmem = BIO_new(BIO_s_mem());
  b64  = BIO_push(b64, bmem);

  BIO_write(b64, input, size);
  (void)BIO_flush(b64);
  BIO_get_mem_ptr(b64, &bptr);

  buff = (gchar *)g_malloc0(bptr->length);
  memcpy(buff, bptr->data, bptr->length - 1);

  BIO_free_all(b64);

  return buff;
}

guchar*
hybrid_base64_decode(const gchar *input, gint *size)
{
    guint  n , t = 0 , c = 0;
    guchar* res;
    guchar  out[3];
    guchar  inp[4];

    n = strlen(input);

    if(n % 4 != 0) {
        return NULL;
    }

    n = n / 4 * 3;
    if(size != NULL) {
        *size = n;
    }

    res = (guchar*)g_malloc0(n);

    while (1) {
        memset(inp, 0, 4);
        memset(out, 0, 3);
        memcpy(inp, input + c, 4);
        c += 4;
        n = EVP_DecodeBlock(out, inp, 4);
        memcpy(res + t, out, n);
        t += n;

        if(c >= strlen(input)) {
            break;
        }
    }

    return res;
}
