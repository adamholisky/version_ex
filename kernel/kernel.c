#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "limine.h"
#include "bootstrap.h"
#include "serial.h"
#include "graphics.h"

#define SSFN_IMPLEMENTATION   
#include <ssfn.h>

#define GRAPHICS_TEST true
#define LIMINE_MODULE_REQUEST { LIMINE_COMMON_MAGIC, 0x3e7e279702be32af, 0xca1c4f3bd1280cee }
#define LIMINE_MEMMAP_REQUEST { LIMINE_COMMON_MAGIC, 0x67cf3d9d378a806f, 0xe304acdfc50c3c62 }


static volatile struct limine_framebuffer_request fb_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0
};

static volatile struct limine_module_request mod_request = {
    .id = LIMINE_MODULE_REQUEST,
	.revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST,
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

	printf( "Screen Height: %d\n", fb->height );
	printf( "Screen Width: %d\n", fb->width );

	struct limine_memmap_entry **memmap = memmap_request.response->entries;
	int num_map_entries = memmap_request.response->entry_count;

	printf( "Memmap Entries: %d\n", num_map_entries );

	for( int x = 0; x < num_map_entries; x++ ) {
		printf( "Entry %d\n", x );
		printf( "    Base:   0x%X\n", memmap[x]->base );
		printf( "    Length: 0x%X\n", memmap[x]->length );
		printf( "    Type:   %d\n", memmap[x]->type );
	}

	uint64_t *pt = 0x00000000bff60000;
	for( int z = 0; z < 4; z++ ) {
		printf( "PT %d: %lX\n", z, *(pt + z) );
	}

	graphics_initalize();

	struct limine_file * mod = mod_request.response->modules[0];
	printf( "Module address: %X\n", mod->address );
	printf( "name: %s\n", mod->path );
	printf( "size: %d\n", mod->size );
	printf( "num mods: %d\n", mod_request.response->module_count);

	char * font = (char *)(mod->address);
	printf( "eof: %X\n", *(font + 51113 ) );

	if( GRAPHICS_TEST ) {
		ssfn_t ctx = { 0 };                                 /* the renderer context */

		ssfn_buf_t buf = {                                  /* the destination pixel buffer */
			.ptr = g->fbuffer,                      /* address of the buffer */
			.w = g->width,                             /* width */
			.h = g->height,                          /* height */
			.p = g->pitch,                         /* bytes per line */
			.x = 10,                                       /* pen position */
			.y = 10,
			.fg = 0x000000                                /* foreground color */
		};

		printf( "ctx = 0x%X\n", &ctx );
		printf( "buf = 0x%X\n", &buf );

		/* add one or more fonts to the context. Fonts must be already in memory */
		int i = ssfn_load(&ctx, font );          /* you can add different styles... */
		//printf( "i = %d\n", i );

		/* select the typeface to use */
		ssfn_select(&ctx,
			SSFN_FAMILY_ANY, NULL,                        /* family */
			SSFN_STYLE_REGULAR,      /* style */
			16                                              /* size */
		);

		/* rasterize the first glyph in an UTF-8 string into a 32 bit packed pixel buffer */
		/* returns how many bytes were consumed from the string */
		ssfn_render(&ctx, &buf, "Hello, world");

		//printf( "Exiting normally.\n" );

		hang();
	}	

	printf( "Exiting normally.\n" );

	do_immediate_shutdown();
}