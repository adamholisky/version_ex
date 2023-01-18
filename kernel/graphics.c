#include <bootstrap.h>
#include <stdint.h>
#include <stdbool.h>
#include <graphics.h>
#include <string.h>
#include <font.h>

uint32_t char_width;
uint32_t char_height;
uint32_t screen_border;
uint32_t fg_color;
uint32_t bg_color;
bool 	 render_background;
uint32_t *bg_mask;
bool	 font_smoothing_active;
uint8_t  *fb_buffer;
graphics_information graphics_info;

uint8_t buffer[1024*768*4];
uint32_t mask[4*14*7];

graphics_information * graphics_get_info( void ) {
	return &graphics_info;
}

void graphics_initalize( void ) {
	font_smoothing_active = true;

	graphics_info.char_height = 14;
	graphics_info.char_width = 7;
	graphics_info.fg_color = 0x00000000;
	graphics_info.bg_color = 0x00262A30;
	graphics_info.buffer = buffer;

	char_height = 14;
	char_width = 7;
	screen_border = 10;

	bg_color = 0x00FFFFFF;
	fg_color = 0x00000000;
	render_background = true;

	bg_mask = mask;

	fillrect( graphics_info.buffer, bg_color, 0, 0, 1024, 768 );
	vga_draw_screen();
}

inline void vga_draw_screen( void ) {
	memcpy( graphics_info.fbuffer, graphics_info.buffer, 1024*768*4 );
}

inline void fillrect( uint8_t * buffer, uint32_t color, unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
	uint8_t * where = (buffer + (x * graphics_info.pixel_width) + (y * graphics_info.pitch ));
	unsigned int i, j;
	
	//write_to_serial_port( ',' );

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			*(uint32_t *)(where + (j * graphics_info.pixel_width) ) = color;
			//debug_f( "%d ", j );
		}
		where += graphics_info.pitch;
		//debug_f( "%d\n", i );

		//klog("%d\n", i);
	}
} 

void put_pixel( unsigned int x, unsigned int y, int color ) {
	putpixel( graphics_info.buffer, x, y, color );
}

inline void putpixel( uint8_t * buffer, unsigned int x, unsigned int y, int color) {
	uint32_t where = (x * graphics_info.pixel_width) + (y * graphics_info.width * graphics_info.pixel_width);
	buffer[where] = color & 255;              // BLUE
	buffer[where + 1] = (color >> 8) & 255;   // GREEN
	buffer[where + 2] = (color >> 16) & 255;  // RED
}

void draw_string( char * string, unsigned int x, unsigned int y, unsigned int fg_color, unsigned int bg_color ) {
	while( *string ) {
		draw_char( graphics_info.buffer, x , y, fg_color, bg_color, *string );

		x = x + 7;

		string++;
	}

	vga_draw_screen();
}

void draw_rect( rect r, unsigned int color ) {
	fillrect( graphics_info.buffer, color, r.x, r.y, r.w, r.h );
}

inline void move_rect( rect dest, rect src ) {
	moverect( graphics_info.buffer, dest, src );
}

inline void moverect( uint8_t *buff, rect dest, rect src ) {
	unsigned int i = 0;
	uint8_t * mem_dest;
	uint8_t * mem_src;
	unsigned int mem_size;

	for( i = 0; i < src.h; i++ ) {
		mem_dest = buff + (dest.x * graphics_info.pixel_width) + ((dest.y + i) * graphics_info.pitch );
		mem_src = buff + (src.x * graphics_info.pixel_width) + ((src.y + i) * graphics_info.pitch );
		mem_size = (graphics_info.pixel_width * src.w);


		for(; mem_size != 0; mem_size--) *mem_dest++ = *mem_src++;
	}
}

void vga_move_line( unsigned int dest_y, unsigned int src_y ) {
	rect src;
	rect dest;

	src.x = 0;
	src.y = screen_border + (src_y * char_height);
	src.w = graphics_info.width;
	src.h = char_height;

	dest.x = 0;
	dest.y = screen_border + (dest_y * char_height );
	dest.w = graphics_info.width;
	dest.h = char_height;

	move_rect( dest, src );
}

void vga_put_char( unsigned char c, unsigned int x, unsigned int y ) {
	draw_char( graphics_info.buffer, screen_border + (x * char_width), screen_border + (y * char_height), graphics_info.fg_color, graphics_info.bg_color, c );

}


inline void draw_char( uint8_t * buffer, unsigned int x, unsigned int y, uint32_t fg, uint32_t bg, unsigned int font_char ) {
	int row, col;
	uint8_t * where = (buffer + (x * graphics_info.pixel_width) + (y * graphics_info.width * graphics_info.pixel_width));
	uint32_t new_r, new_g, new_b, new_fg;
	double alpha = 0.3;

	new_r = (((fg & 0xFF0000) >> 16) * alpha) + ((1 - alpha) * ((bg & 0xFF0000) >> 16));
	new_g = (((fg & 0x00FF00) >> 8) * alpha) + ((1 - alpha) * ((bg & 0x00FF00) >> 8));
	new_b = (((fg & 0x0000FF)) * alpha) + ((1 - alpha) * ((bg & 0x0000FF)));

	new_fg = ((new_r << 16) & 0xFF0000) | ((new_g << 8) & 0x00FF00) | (new_b & 0x0000FF);

	for (row = 0; row < 14; row++) {
		for (col = 0; col < 7; col++) {
			*(bg_mask + ((row) * 7) + (col) ) = 0;
		}
	}

	for (row = 0; row < 14; row++) {
		for (col = 0; col < 7; col++) {
			if( (_tamzen_bits[ _tamzen_offset[ font_char ] + row ] >> 8 & (1 << 7 - col)) ) {
				*(uint32_t *)(where + (col) * graphics_info.pixel_width) = fg;

				if( font_smoothing_active ) {
					//  X.
					//  .E
					//
					//IF pixel right 1 and down 1 exists, fill 1 right with alpha, and fill 1 down with alpha
					if( (col + 1 <= 7) && (row + 1 <= 14) ) { // if right 1 can happen and 1 down can happen
						if( (_tamzen_bits[ _tamzen_offset[ font_char ] + (row + 1) ] >> 8 & (1 << (7 - col - 1)) ) ) { // does right 1 and down 1 exist?
							if( !(_tamzen_bits[ _tamzen_offset[ font_char ] + (row) ] >> 8 & (1 << (7 - col - 1)) ) ) {	// does right 1 NOT exist? If so... fill it with alpha
								*(uint32_t *)(where + ((col + 1) * graphics_info.pixel_width)) = new_fg;
								*(bg_mask + ((row) * 7) + (col + 1) ) = 1;
							}

							if( !(_tamzen_bits[ _tamzen_offset[ font_char ] + (row + 1 ) ] >> 8 & (1 << (7 - col)) ) ) {// does down 1 NOT exist? If so... fill it with alpha
								*(uint32_t *)(where + ((col) * graphics_info.pixel_width ) + graphics_info.pitch ) = new_fg;
								*(bg_mask + ((row + 1) * 7 ) + (col) ) = 1;
							}
						}
					} 	   

					//  .X
					//  E.
					//IF pixel left 1 and down 1 exists, fill 1 left with alpha, and fill 1 down with alpga
					if( (col - 1 >= 0) && (row + 1 <= 14) ) { // if right 1 can happen
						if( (_tamzen_bits[ _tamzen_offset[ font_char ] + (row + 1) ] >> 8 & (1 << (7 - col + 1)) ) ) { // does right 1 and down 1 exist?
							if( !(_tamzen_bits[ _tamzen_offset[ font_char ] + (row) ] >> 8 & (1 << (7 - col + 1)) ) ) {	// does right 1 NOT exist? If so... fill it with alpha
								*(uint32_t *)(where + ((col - 1) * graphics_info.pixel_width)) = new_fg;
								*(bg_mask + ((row) * 7) + (col - 1) ) = 1;
							}

							if( !(_tamzen_bits[ _tamzen_offset[ font_char ] + (row + 1 ) ] >> 8 & (1 << (7 - col)) ) ) {// does down 1 NOT exist? If so... fill it with alpha
								*(uint32_t *)(where + ((col) * graphics_info.pixel_width ) + graphics_info.pitch ) = new_fg;
								*(bg_mask + ((row + 1) * 7 ) + (col) ) = 1;
							}
						}
					} 
				}
			} else {
				if( render_background ) {
					if( *(bg_mask + (row * 7) + (col)) == 0 ) { // Only render the bg if there isn't a filled pixel there
						*(uint32_t *)(where + (col) * graphics_info.pixel_width) = bg;
					}
				}
			}
		}
	   
		where += graphics_info.pitch;
	}
}