#ifndef _RingBuffer_h
#define _RingBuffer_h

#include <stdlib.h>

typedef struct {
	char *buffer;
	int length;
	int start;
	int end;
} RingBuffer;

RingBuffer *RingBuffer_create(int length);

void RingBuffer_destroy(RingBuffer *buffer);

int RingBuffer_unsafe_write(RingBuffer *buffer, char *data, int length);

int RingBuffer_safe_write(RingBuffer *buffer, char *data, int length);

char *RingBuffer_pop(RingBuffer *buffer, int num_bytes);

void RingBuffer_clear(RingBuffer *buffer);

int RingBuffer_used_space(RingBuffer *buffer);

int RingBuffer_available_space(RingBuffer *buffer);

int RingBuffer_dist_to_end(RingBuffer *buffer);

#endif