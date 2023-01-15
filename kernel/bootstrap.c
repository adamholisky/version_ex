#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "bootstrap.h"
#include <serial.h>

void kfree( void *p ) {

}

void * kmalloc( size_t size ) {

}

void term_put_char( char c ) {
	serial_write( c );
}