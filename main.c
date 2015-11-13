#include "ring_buffer.c"
#include <stdio.h>

void buffer_info(RingBuffer *buff)
{
	printf("Space Available: %i\n",RingBuffer_available_space(buff));
    printf("Start pointer: %i\n",buff->start);
    printf("End pointer: %i\n",buff->end);
    printf("Raw buffer contents: %s\n\n",buff->buffer);
}

int main()
{
	char *dat = "abc";
	char *dat2 = "xyz";

    RingBuffer *buff = RingBuffer_create(3);
    buffer_info(buff);
    
    RingBuffer_safe_write(buff,dat,1);
    buffer_info(buff);

    RingBuffer_safe_write(buff,dat,2);
    buffer_info(buff);

    RingBuffer_write(buff,dat2,2);
    buffer_info(buff);

    char *redat = RingBuffer_pop(buff,2);
    printf("Popped Data: %s\n",redat);
    buffer_info(buff);

    char *redat2 = RingBuffer_pop(buff,2);
    printf("Popped Data: %s\n",redat2);
    buffer_info(buff);
                
    return 0;
}
