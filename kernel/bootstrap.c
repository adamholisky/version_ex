#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "bootstrap.h"
#include <serial.h>

char mem[1024 * 1024 * 100];
char *mem_ptr = mem;

void kfree( void *p ) {
	printf( "kfree(0x%X)\n", p );
}

void * kmalloc( size_t size ) {
	printf( "kmalloc called.\n" );
	
	void * ret = (void *)mem_ptr;

	mem_ptr = mem_ptr + size;

	memset( ret, 0, size );

	printf( "kmalloc(%d) = 0x%X\n", size, ret );

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