#include "ring_buffer.h"

/**
 * Create a RingBuffer of a specified length
 *
 * @param RingBuffer  *buffer  Pointer to the RingBuffer
 * @param int         length   Length of that byte-level data (in bytes)
 *
 * @return *RingBuffer         A pointer to the newly created RingBuffer
 */
RingBuffer *RingBuffer_create(int length)
{
	RingBuffer *buffer = calloc(1, sizeof(RingBuffer)); // The buffer consists of its pointers and the buffer contents
	buffer->length  = length; // Length of the buffer 
	buffer->start = length; // "Pointer" (index) to oldest (first written) data (at full length, points to no data)
	buffer->end = length; // "Pointer" (index) to newest (latest written) data
	buffer->buffer = calloc(length, 1); // Buffer itself is a pointer to the content space
	
	return buffer;
}

/**
 * Destroy a RingBuffer
 *
 * @param RingBuffer  *buffer  Pointer to the RingBuffer
 */
void RingBuffer_destroy(RingBuffer *buffer)
{
    if(buffer) {
        free(buffer->buffer);
        free(buffer);
    }
}

/**
 * Write a chunk of data to the buffer, with overwriting if necessary
 *
 * @param RingBuffer  *buffer  Pointer to the RingBuffer
 * @param char        *data    Pointer to some byte-level data 
 * @param int         length   Length of that byte-level data (in bytes)
 *
 * @return int                 Numbers of bytes written, -1 on error
 */
int RingBuffer_write(RingBuffer *buffer, char *data, int length)
{
	if(buffer->length < length) {
		// Impossible. Stop it.
		return -1;
	}

	int openSpace = RingBuffer_available_space(buffer);
	
	if(buffer->start == buffer->length)
	{
		// Empty ring, put things in from the front
		void *result1 = memcpy(buffer->buffer, data, length);
		if(result1 == NULL)
		{
			// Memcpy failed
			return -1;
		}
		buffer->start = 0;
		buffer->end = length - 1;
		return length;
	}

	int physRemains = RingBuffer_dist_to_end(buffer);
	if(physRemains < length)
	{
		// Not enough room at the end of the ring, but enough space total, so wrap around
		char *dataSplit = data + physRemains;
		void *result1 = memcpy(buffer->buffer + buffer->end + 1, data, physRemains);
		void *result2 = memcpy(buffer->buffer, dataSplit, length - physRemains);
		if(result1 == NULL || result2 == NULL)
		{
			// Memcpy failed
			return -1;
		}
		buffer->end = (buffer->end + length) % buffer->length;
		if(length > openSpace)
		{
			// Overwriting, update start pointer to compensate
			buffer->start = (buffer->start + (length - openSpace)) % buffer->length;
		}
		return length;
	}
	else
	{
		// Simple copy onto the end
		void *result1 = memcpy(buffer->buffer + buffer->end + 1, data, length);
		if(result1 == NULL)
		{
			// Memcpy failed
			return -1;
		}
		buffer->end += length;
		if(length > openSpace)
		{
			// Overwriting, update start pointer to compensate
			buffer->start = (buffer->start + (length - openSpace)) % buffer->length;
		}
		return length;
	}
}

/**
 * Write a chunk of data to the buffer, without overwriting
 *
 * @param RingBuffer  *buffer  Pointer to the RingBuffer
 * @param char        *data    Pointer to some byte-level data 
 * @param int         length   Length of that byte-level data (in bytes)
 *
 * @return int                 Numbers of bytes written, -1 on error
 */
int RingBuffer_safe_write(RingBuffer *buffer, char *data, int length)
{
	if(RingBuffer_available_space(buffer) < length) {
		// Unsafe insert (would overwrite), abort
		return -1;
	};
	
	return RingBuffer_write(buffer,data,length);
}

/**
 * Read a chunk of data from the buffer, removing it (effectively, by moving the read pointer)
 *
 * @param RingBuffer  *buffer  Pointer to the RingBuffer
 * @param int         length   Length of data to pop (in bytes)
 *
 * @return int                 Numbers of bytes popped, NULL on error
 */
char *RingBuffer_pop(RingBuffer *buffer, int length)
{
    if(RingBuffer_used_space(buffer) < length) {
		// Can't pop more than could possibly be popped, instead pop all we can
		length = RingBuffer_used_space(buffer);
	};

	if(buffer->start == buffer->length)
	{
		//Empty buffer, return NULL
		return NULL;
	}

	char *data = calloc(length, sizeof(char));
	int physRemains = RingBuffer_dist_to_end(buffer);
	
	if(physRemains < length)
	{
		// Data wraps at the end of the ring
		char *dataSplit = data + physRemains + 1;
		void *result1 = memcpy(data, buffer->buffer + buffer->start, physRemains + 1);
		void *result2 = memcpy(dataSplit, buffer->buffer, length - physRemains - 1);
		if(result1 == NULL || result2 == NULL)
		{
			// Memcpy failed
			return NULL;
		}
	}
	else
	{
		// Data doesn't wrap at the end of the ring, just copy over
		void *result1 = memcpy(data, buffer->buffer + buffer->start, length);
		if(result1 == NULL)
		{
			// Memcpy failed
			return NULL;
		}
	}
	buffer->start += length;
	buffer->start %= buffer->length;
	if(length != 0 && buffer->start == (buffer->end + 1))
	{
		// Popped every element, send indices to length
		buffer->start = buffer->length;
		buffer->end = buffer->length;
	}
	return data;
}

/**
 * Clears the ring buffer by resetting the index pointers
 */
void RingBuffer_clear(RingBuffer *buffer)
{
	buffer->start = buffer->length;
	buffer->end = buffer->length;
}

/**
 * Used space in the ring
 */
int RingBuffer_used_space(RingBuffer *buffer)
{
	// Empty buffer, each pointer points off of the ring
	if(buffer->start == buffer->length)
	{
		return 0;
	}
	else
	{
		return (buffer->length + buffer->end - buffer->start) % buffer->length + 1;
	}
}

/**
 * Available space remaining in the ring
 */
int RingBuffer_available_space(RingBuffer *buffer)
{
	// Empty buffer, each pointer points off of the ring
	if(buffer->start == buffer->length)
	{
		return buffer->length;
	}
	else
	{
		return buffer->length - ((buffer->length + buffer->end - buffer->start) % buffer->length + 1);
	}
}

/**
 * Physical distance to the end of the ring
 */
int RingBuffer_dist_to_end(RingBuffer *buffer)
{
	if(buffer->start == buffer->length)
	{
		// Empty ring, use full dist
		return buffer->length;
	}
	else if(buffer->start > buffer->end)
	{
		return buffer->start - buffer->end - 1;
	}
	else
	{
		return buffer->length - buffer->end - 1;
	}
}