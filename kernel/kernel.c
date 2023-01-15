#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "limine.h"
#include "bootstrap.h"
#include "serial.h"
#include "graphics.h"

extern void enable_sse( void );

static volatile struct limine_terminal_request terminal_request = {
	.id = LIMINE_TERMINAL_REQUEST,
	.revision = 0
};

static volatile struct limine_framebuffer_request fb_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0
};

static void done(void) {
	for (;;) {
		__asm__("hlt");
	}
}

void _start(void) {
	serial_initalize();

/* 	struct limine_terminal *terminal = terminal_request.response->terminals[0];
	terminal_request.response->write(terminal, "Hello World, completely automate!!", 34); */

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

	enable_sse();
	printf( "    SSE Enabled\n" );

	graphics_initalize();

	draw_string( "Hello, world!", 100, 100, color_white, 0x00262A30 );

	printf( "Exiting normally.\n" );

	done();

	do_immediate_shutdown();
}