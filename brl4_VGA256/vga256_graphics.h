/**
 * Hunter Adams (vha3@cornell.edu)
 * modifed for 256 colors by BRL4
 * 
 *
 * RESOURCES USED
 *  - PIO state machines 0, 1, and 2 on PIO instance 0
 *  - DMA channels 0, 1, 2, and 3
 *  - 153.6 kBytes of RAM (for pixel color data)
 *
 * NOTE
 *  - This is a translation of the display primitives
 *    for the PIC32 written by Bruce Land and students
 *
 */

// Give the I/O pins that we're using some names that make sense - usable in main()
 enum vga_pins {HSYNC=16, VSYNC} ;

// 8-bit color
#define WHITE   0b11111111
#define BLACK   0b00000000
#define RED     0b11100000
#define GREEN   0b00011100
#define BLUE    0b00000011
#define CYAN    0b00011111
#define MAGENTA 0b11100011
#define YELLOW  0b11111100
#define GRAY1   0b00100100
#define GRAY2   0b01101101
#define GRAY3   0b10110110
#define GRAY4   0b11011010

// defining colors
#define rgb(r,g,b) (((r)<<5) & RED | ((g)<<2) & GREEN | ((b)<<0) & BLUE )

// VGA primitives - usable in main
void initVGA(void) ;
void drawPixel(short x, short y, char color) ;
void drawPixelDither(short x, short y, char color1, char color2) ;
void drawVLine(short x, short y, short h, char color) ;
void drawHLine(short x, short y, short w, char color) ;
void drawLine(short x0, short y0, short x1, short y1, char color) ;
void drawRect(short x, short y, short w, short h, char color);
void drawRectDither(short x, short y, short w, short h, char color1, char color2);
void drawCircle(short x0, short y0, short r, char color) ;
void drawCircleHelper( short x0, short y0, short r, unsigned char cornername, char color) ;
void fillCircle(short x0, short y0, short r, char color) ;
void fillCircleHelper(short x0, short y0, short r, unsigned char cornername, short delta, char color) ;
void drawRoundRect(short x, short y, short w, short h, short r, char color) ;
void fillRoundRect(short x, short y, short w, short h, short r, char color) ;
void fillRect(short x, short y, short w, short h, char color) ;
void fillRectDither(short x, short y, short w, short h, char color1, char color2) ;
void drawChar(short x, short y, unsigned char c, char color, char bg, unsigned char size) ;
void setCursor(short x, short y);
void setTextColor(char c);
void setTextColor2(char c, char bg);
void setTextSize(unsigned char s);
void setTextWrap(char w);
void tft_write(unsigned char c) ;
void writeString(char* str) ;