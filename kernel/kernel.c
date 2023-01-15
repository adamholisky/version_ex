#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "limine.h"
#include "bootstrap.h"
#include "serial.h"
#include "graphics.h"

#define GRAPHICS_TEST false

static volatile struct limine_framebuffer_request fb_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0
};

void kernel_main(void) {
	sse_initalize();
	serial_initalize();

	printf( "VVOS Example/Clean Kernel\n" );

	struct limine_framebuffer *fb = fb_request.response->framebuffers[0];
	graphics_information *g = graphics_get_info();

	g->pitch = fb->pitch;
	g->width = fb->width;
	g->height = fb->height;
	g->bpp = fb->bpp;
	g->pixel_width = g->pitch / g->width;
	g->fbuffer = fb->address;

	printf( "    Screen Height: %d\n", fb->height );
	printf( "    Screen Width: %d\n", fb->width );

	graphics_initalize();

	if( GRAPHICS_TEST ) {
		draw_string( "Hello, world!", 5, 5, color_white, 0x00262A30 );
		printf( "Exiting normally.\n" );

		hang();
	}	

	printf( "Exiting normally.\n" );

	do_immediate_shutdown();
}