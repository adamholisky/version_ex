#ifndef VGA_INCLUDED
#define VGA_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

enum vga_color
{
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

#define color_red 			0xFF0000
#define color_green 		0x00FF00
#define color_blue 			0x0000FF
#define color_white 		0xFFFFFF
#define color_black			0x000000
#define color_adams_blue	0x3753D1

#define cursor_up		1
#define cursor_down		2
#define	cursor_left		3
#define cursor_right	4	

typedef struct {
	uint32_t x;
	uint32_t y;
} point;

typedef struct {
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
} rect;

typedef struct {
	uint8_t * fbuffer;
	uint8_t * buffer;
	uint32_t pitch;
	uint32_t width;
	uint32_t height;
	uint32_t bpp;
	uint32_t pixel_width;

	uint32_t char_width;
	uint32_t char_height;

	uint32_t fg_color;
	uint32_t bg_color;
} graphics_information;

void graphics_initalize( void );
void fillrect( uint8_t * buffer, uint32_t color, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void put_pixel( unsigned int x, unsigned int y, int color );
void putpixel( uint8_t * buffer, unsigned int x, unsigned int y, int color);
void draw_string( char * string, unsigned int x, unsigned int y, unsigned int fg_color, unsigned int bg_color );
void draw_rect( rect r, unsigned int color );
void move_rect( rect dest, rect src );
void moverect( uint8_t *buff, rect dest, rect src );
void vga_move_line( unsigned int dest_y, unsigned int src_y );
void vga_put_char( unsigned char c, unsigned int x, unsigned int y );
void draw_char( uint8_t * buffer, unsigned int x, unsigned int y, uint32_t fg, uint32_t bg, unsigned int font_char );
void vga_draw_screen( void );
graphics_information * graphics_get_info( void );

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

#ifdef __cplusplus
}
#endif

#endif