/**
 * Hunter Adams (vha3@cornell.edu)
 * converted to 319x240 with 256 colors by Bruce; brue.land@cornell.edu
 * 
 * HARDWARE CONNECTIONS
 *  - GPIO 16 ---> VGA Hsync
 *  - GPIO 17 ---> VGA Vsync
 * 
 *  - GPIO 08 ---> 330 ohm resistor ---> VGA Blue lo-bit |__ both wired to 150 ohm to ground 
 *  - GPIO 09 ---> 220 ohm resistor ---> VGA Blue hi_bit |   and to VGA blue
 * 
 *  - GPIO 10 ---> 1000 ohm resistor ---> VGA Green lo-bit |__ three wired to 100 ohm to ground
 *  - GPIO 11 ---> 680 ohm resistor ---> VGA Green mid_bit |   and to VGA Green
 *  - GPIO 12 ---> 330 ohm resistor ---> VGA Green hi_bit  |   
 * 
 *  - GPIO 13 ---> 1000 ohm resistor ---> VGA Red lo-bit |__ three wired to 100 ohm to ground
 *  - GPIO 14 ---> 680 ohm resistor ---> VGA Red mid_bit |   and to VGA red
 *  - GPIO 15 ---> 330 ohm resistor ---> VGA Red hi_bit  |   
 * 
 *  - RP2040 GND ---> VGA GND
 *
 * RESOURCES USED
 *  - PIO state machines 0 to 3 on PIO instance 0
 *  - DMA channels 0, 1, 2, 3 data send to two PIO
 *  - 76.8 kBytes of RAM (for pixel color data)
 * color encoding: bits 7:5 red; 4:2 green; 1:0 blue
 *
 * Protothreads v1.3 dual core, priority
 * graphics demo thread
 * serial thread to set the color of two boxes using r,g,b and h,s,v
 * the usual blinky thread for a hearbeat
 */
// ==========================================
// === VGA graphics library
// ==========================================
#include "vga256_graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

// ==========================================
// === protothreads globals
// ==========================================
#include "hardware/sync.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "string.h"
// protothreads header
#include "pt_cornell_rp2040_v1_3.h"
// interactive color
char rgb_box ;

// ==================================================
// === convert HSV to rgb value
// ==================================================
char hsv2rgb(float h, float s, float v){
    float C, X, m, rp, gp, bp ;
    unsigned char r, g, b ;
    // hsv to rgb conversion from
    // http://www.rapidtables.com/convert/color/hsv-to-rgb.htm
    C = v * s;
    //X = C * (1 - abs((int)(h/60)%2 - 1));
    // (h/60) mod 2  = (h/60 - (int)(h/60))
    X = C * (1.0 - fabsf(fmodf(h/60.0, 2.0) - 1.));
    m = v - C;
    if      ((0<=h) && (h<60))   { rp = C; gp = X; bp = 0;}
    else if ((60<=h) && (h<120)) { rp = X; gp = C; bp = 0;}
    else if ((120<=h) && (h<180)){ rp = 0; gp = C; bp = X;}
    else if ((180<=h) && (h<240)){ rp = 0; gp = X; bp = C;}
    else if ((240<=h) && (h<300)){ rp = X; gp = 0; bp = C;}
    else if ((300<=h) && (h<360)){ rp = C; gp = 0; bp = X;}
    else                         { rp = 0; gp = 0; bp = 0;}
    // scale to 8-bit rgb
    r = (unsigned char)((rp+m)*7) ;
    g = (unsigned char)((gp+m)*7) ;
    b = (unsigned char)((bp+m)*3) ;
     //       
    return rgb(r,g,b) ;
}

// ==================================================
// === graphics demo -- RUNNING on core 0
// ==================================================
static PT_THREAD (protothread_graphics(struct pt *pt)) {
    PT_BEGIN(pt);
    // the protothreads interval timer
    PT_INTERVAL_INIT() ;

    // position of the disc primitive
    static short disc_x = 0 ;
    // position of the box primitive
    static short box_x = 0 ;
    // position of vertical line primitive
    static short Vline_x = 350;
    // position of horizontal line primitive
    static short Hline_y = 250;

    // background
    fillRect(0, 0, 319, 239, BLACK); // 

    // Draw some filled rectangles
    fillRect(0, 0, 76, 10, BLUE); // blue box
    fillRect(100, 0, 150, 10, WHITE); // red box
    //fillRect(200, 0, 76, 10, GREEN); // green box

    // Write some text
    setTextColor(WHITE) ;
    setCursor(10, 1) ;
    setTextSize(1) ;
    writeString("ECE 4760") ;

    setTextColor(BLACK) ;
    setCursor(102, 1) ;
    setTextSize(1) ;
    writeString("VGA 319x240 8-bit color ") ;

     char video_buffer[32];
    setTextColor2(WHITE, BLACK) ;
    setTextSize(1) ;
    int x=0 ;
    // blues
    for (int i=0; i<4; i++) {
        fillRect(i*10+10, 20, 9, 9, rgb(0,0,i));
    }
    // reds
    for (int i=0; i<8; i++) {
        fillRect(i*10+80, 20, 9, 9, rgb(i,0,0));
    }

    // greens
    for (int i=0; i<8; i++) {
        fillRect(i*10+190, 20, 9, 9, rgb(0,i,0));
    }

    // four levels of blue with 8x8 levels red/green
    for (int i=0; i<8; i++) {
      for (int j=0; j<8; j++){
        fillRect(i*10+10, 40+j*10, 9, 9, rgb(i,j,0));  
      }
    }

    for (int i=0; i<8; i++) {
      for (int j=0; j<8; j++){
        fillRect(i*10+100, 40+j*10, 9, 9, rgb(i,j,1));  
      }
    }

    for (int i=0; i<8; i++) {
      for (int j=0; j<8; j++){
        fillRect(i*10+10, 150+j*10, 9, 9, rgb(i,j,2));  
      }
    }

    for (int i=0; i<8; i++) {
      for (int j=0; j<8; j++){
        fillRect(i*10+100, 150+j*10, 9, 9, rgb(i,j,3));  
      }
    }

    // HSV four intensity levels each with array of H and S
    #define hsv_hres 120
    #define hsv_sres 80
    for (int i=0; i<=hsv_hres; i++) {
      for (int j=0; j<=hsv_sres; j++){
        fillRect(i+190, 40+j, 1, 1, hsv2rgb((float)i*360/hsv_hres, (float)j/hsv_sres, 0.90)); 
      }
    }

      
    static int r, g, b;
    static float hue; 
    while(true) {
        hue += 5 ;
        if (hue>=360) hue = 0 ;
        rgb_box = hsv2rgb(hue, 1.0, 1.0) ;
        fillRect(230, 200, 30, 30, rgb_box);  //230
        setCursor(200, 200) ; 
        sprintf(video_buffer, "HSV ");
        setTextColor2(WHITE, BLACK) ;
        writeString(video_buffer) ;
        setCursor(200, 210) ; 
        // pace the animation
        PT_YIELD_usec(50000) ;
   }
   PT_END(pt);
} // graphics thread

// ==================================================
// === toggle25 thread on core 0
// ==================================================
// the on-board LED blinks
static PT_THREAD (protothread_toggle25(struct pt *pt))
{
    PT_BEGIN(pt);
    static bool LED_state = false ;
    
     // set up LED p25 to blink
     gpio_init(25) ;	
     gpio_set_dir(25, GPIO_OUT) ;
     gpio_put(25, true);
     // data structure for interval timer
     PT_INTERVAL_INIT() ;

      while(1) {
        // yield time 0.1 second
        //PT_YIELD_usec(100000) ;
        PT_YIELD_INTERVAL(100000) ;

        // toggle the LED on PICO
        LED_state = LED_state? false : true ;
        gpio_put(25, LED_state);
        //
        // NEVER exit while
      } // END WHILE(1)
  PT_END(pt);
} // blink thread


// ==================================================
// === user's serial input thread on core 0
// ==================================================
// serial_read an serial_write do not block any thread
// except this one
static int r=7, g=7, b=3 ;
float h,s,v;
static PT_THREAD (protothread_serial(struct pt *pt))
{
    PT_BEGIN(pt);
        char video_buffer[20];
      //
      while(1) {
        // print prompt
        sprintf(pt_serial_out_buffer, "input r, g, b: ");
        // spawn a thread to do the non-blocking write
        serial_write ;

        // spawn a thread to do the non-blocking serial read
         serial_read ;
        sscanf(pt_serial_in_buffer, "%d %d %d ", &r, &g, &b) ;
        //printf("%d\n\r" bit_bucket) ;
        // 
        rgb_box = rgb(r,g,b) ;
        printf("%02x\n\r", rgb_box) ;
        fillRect(230, 170, 30, 30, rgb_box); 
        setCursor(200, 150) ; 
        sprintf(video_buffer, "rgb  r,g,b");
        setTextColor2(WHITE, BLACK) ;
        writeString(video_buffer) ;
        setCursor(200, 160) ; 
        sprintf(video_buffer, "0x%02x %1d,%1d,%1d", rgb_box, r&7, g&7, b&3);
        setTextColor2(rgb_box, BLACK) ;
        writeString(video_buffer) ;

        // print prompt
        sprintf(pt_serial_out_buffer, "input h 0-360,s 0-1,v 0-1: ");
        // spawn a thread to do the non-blocking write
        serial_write ;

        // spawn a thread to do the non-blocking serial read
         serial_read ;
        sscanf(pt_serial_in_buffer, "%f %f %f ", &h, &s, &v) ;
        //printf("%d\n\r" bit_bucket) ;
        // 
        rgb_box = hsv2rgb(h,s,v) ;
        printf("%02x\n\r", rgb_box) ;
        fillRect(270, 170, 30, 30, rgb_box); 
        setCursor(270, 150) ; 
        sprintf(video_buffer, "H,S,V  ");
        setTextColor2(WHITE, BLACK) ;
        writeString(video_buffer) ;
        setCursor(270, 160) ; 
        sprintf(video_buffer, "H %3.0f ", h);
        setTextColor2(rgb_box, BLACK) ;
        writeString(video_buffer) ;
        setCursor(270, 201) ; 
        sprintf(video_buffer, "S %2.2f",s);
        setTextColor2(rgb_box, BLACK) ;
        writeString(video_buffer) ;
        setCursor(270, 211) ; 
        sprintf(video_buffer, "V %2.2f", v);
        setTextColor2(rgb_box, BLACK) ;
        writeString(video_buffer) ;


        // NEVER exit while
      } // END WHILE(1)
  PT_END(pt);
} // serial thread

// ========================================
// === core 1 main -- started in main below
// ========================================
void core1_main(){ 
  //
  //  === add threads  ====================
  // for core 1
  //pt_add_thread(protothread_toggle_gpio4) ;
  //pt_add_thread(protothread_serial) ;
  //
  // === initalize the scheduler ==========
  pt_schedule_start ;
  // NEVER exits
  // ======================================
}

// ========================================
// === core 0 main
// ========================================
int main(){
  // set the clock
  //set_sys_clock_khz(250000, true); // 171us
  // start the serial i/o
  stdio_init_all() ;
  // announce the threader version on system reset
  printf("\n\rProtothreads RP2040 v1.11 two-core\n\r");

  // Initialize the VGA screen
  initVGA() ;
     
  // start core 1 threads
  //multicore_reset_core1();
  //multicore_launch_core1(&core1_main);

  // === config threads ========================
  // for core 0
  pt_add_thread(protothread_graphics);
  pt_add_thread(protothread_toggle25);
  pt_add_thread(protothread_serial) ;
  //
  // === initalize the scheduler ===============
  pt_schedule_start ;
  // NEVER exits
  // ===========================================
} // end main