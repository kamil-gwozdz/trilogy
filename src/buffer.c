#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "trilogy/buffer.h"
#include "trilogy/error.h"

int trilogy_buffer_init(trilogy_buffer_t *buffer, size_t initial_capacity)
{
    buffer->len = 0;
    buffer->cap = initial_capacity;
    buffer->max = SIZE_MAX;
    buffer->buff = malloc(initial_capacity);

    if (buffer->buff == NULL) {
        return TRILOGY_SYSERR;
    }

    return TRILOGY_OK;
}

#define EXPAND_MULTIPLIER 2

int trilogy_buffer_expand(trilogy_buffer_t *buffer, size_t needed)
{
    // expand buffer if necessary
    if (buffer->len + needed > buffer->cap) {
        size_t new_cap = buffer->cap;

        size_t protocol_offset = buffer->continuation_headers_len; // server-imposed maximum capacity doesn't account for continuation headers

        while (buffer->len + needed > new_cap) {
            // would this next step cause an overflow?
            if (new_cap > SIZE_MAX / EXPAND_MULTIPLIER)
                return TRILOGY_TYPE_OVERFLOW;

            new_cap *= EXPAND_MULTIPLIER;
        }

printf("\n");
        if (new_cap - protocol_offset > buffer->max - 1) {
            printf("clamping new cap: %zu -> %zu\n", new_cap, buffer->max + protocol_offset -  1);
            new_cap = buffer->max + protocol_offset - 1;
        }

//        printf("needed: %zu\n", needed);
//        printf("new_cap: %zu\n", new_cap);
//        printf("buffer len: %zu\n", buffer->len);
//        printf("buffer cap: %zu\n", buffer->cap);
//        printf("buffer max: %zu\n", buffer->max);
//        printf("buffer continuation_headers_len: %zu\n", buffer->continuation_headers_len);
        // this print out buffer contents as a hex string
//        for (int i = 0; i < buffer->len; i++) {
//            printf("%02x", buffer->buff[i]);
//        }
//        printf("\n");

        if (new_cap < buffer->len + needed) {
            return TRILOGY_MAX_PACKET_EXCEEDED;
        }

        uint8_t *new_buff = realloc(buffer->buff, new_cap);
        if (new_buff == NULL)
            return TRILOGY_SYSERR;

        buffer->buff = new_buff;
        buffer->cap = new_cap;
    }

    return TRILOGY_OK;
}

int trilogy_buffer_putc(trilogy_buffer_t *buffer, uint8_t c)
{
    int rc = trilogy_buffer_expand(buffer, 1);

    if (rc) {
        return rc;
    }

    buffer->buff[buffer->len++] = c;

    return TRILOGY_OK;
}

void trilogy_buffer_free(trilogy_buffer_t *buffer) { free(buffer->buff); }
