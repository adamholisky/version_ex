#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "bootstrap.h"
#include <serial.h>

char mem[1024 * 1024];
char *mem_ptr = mem;

void kfree( void *p ) {

}

void * kmalloc( size_t size ) {
	void * ret = (void *)mem_ptr;

	mem_ptr = mem_ptr + size;

	return ret;
}

void term_put_char( char c ) {
	serial_write( c );
}

void hang( void ) {
	for (;;) {
		__asm__("hlt");
	}
}