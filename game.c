
#include <stdlib.h>
#include <string.h>

#include <stdlib.h>
#include <string.h>

// include NESLIB header
#include "neslib.h"

// include CC65 NES Header (PPU)
#include <nes.h>

// link the pattern table into CHR ROM
//#link "chr_generic.s"

// BCD arithmetic support
#include "bcd.h"
//#link "bcd.c"

// VRAM update buffer
#include "vrambuf.h"
//#link "vrambuf.c"

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x03,			// screen color

  0x11,0x30,0x27,0x0,	// background palette 0
  0x1c,0x20,0x2c,0x0,	// background palette 1
  0x00,0x10,0x20,0x0,	// background palette 2
  0x06,0x16,0x26,0x0,   // background palette 3

  0x16,0x35,0x24,0x0,	// sprite palette 0
  0x00,0x37,0x25,0x0,	// sprite palette 1
  0x0d,0x2d,0x3a,0x0,	// sprite palette 2
  0x0d,0x27,0x2a	// sprite palette 3
};

byte curx = 32;
byte cury = 16;

byte plrx = 16;
byte plry = 16;
sbyte plrxv;
sbyte plryv;


unsigned char tilemap[32][30];



void init_tilemap() {
  byte i;
  byte j;
  for (i=0; i<10; i++) {
    for (j=0; j<10; j++) {
      tilemap[j][i] = 0;
    }
  }
}

void set_tile(byte x, byte y, unsigned char t) {
  tilemap[y][x] = t;//put it in mem
  vrambuf_put(NTADR_A(x, y), &t, 1);//put the visual tile in vram buff
  vrambuf_flush();
}

unsigned char get_tile_px(byte x, byte y) {
  return tilemap[y/8][x/8];
}

const byte CURSPD = 2;
const byte PLRSPD = 2;
const byte GRAV = 25;

bool edit = true;

void control_cursor(unsigned char pad) {
  if (pad & PAD_LEFT)
    curx -= CURSPD;
  if (pad & PAD_RIGHT)
    curx += CURSPD;
  if (pad & PAD_DOWN)
    cury += CURSPD;
  if (pad & PAD_UP)
    cury -= CURSPD;
  if (pad & PAD_A)
    set_tile((curx/8), (cury/8), 0x06);
  else if (pad & PAD_B)
    set_tile((curx/8), (cury/8), 0x00);
  if (pad & PAD_START)
    edit = false;
}

void control_player(unsigned char pad) {
  if (pad & PAD_LEFT)
    plrxv = -PLRSPD;
  else if (pad & PAD_RIGHT)
    plrxv = PLRSPD;
  else plrxv = 0;
}

void update_player() {
  //collsiion
  plrx += plrxv;
  if (plrxv < 0 && (get_tile_px(plrx, plry) != 0) || (get_tile_px(plrx, plry + 7) != 0))//left
    plrx = (plrx & ~7) + 8;
  if (plrxv > 0 && (get_tile_px(plrx + 7, plry) != 0) || (get_tile_px(plrx + 7, plry + 7) != 0))//right
    plrx = plrx & ~7;
  
  plry += plryv/100;
  if (plryv < 0 && (get_tile_px(plrx, plry) != 0) || (get_tile_px(plrx + 7, plry) != 0)) {//top
    plry = (plry & ~7) + 8;
    plryv = 0;
  }
  if ((get_tile_px(plrx, plry + 7) != 0) || (get_tile_px(plrx + 7, plry + 7) != 0)) {//bottom
    plry = plry & ~7;
    plryv = 0;
  } else plryv += GRAV;
}

// setup PPU and tables
void setup_graphics() {
  // clear vram buffer
  vrambuf_clear();
  // set NMI handler
  set_vram_update(updbuf);
  // clear sprites
  oam_clear();
  // set palette colors
  pal_all(PALETTE);
}

void main(void)
{
  setup_graphics();
  
  // enable rendering
  ppu_on_all();
  // infinite loop
  while(1) {
    if (edit) {
      
      control_cursor(pad_poll(0));
      oam_spr(curx, cury, 1, 2, 0);
      
    } else {
      control_player(pad_poll(0));
      update_player();
    }
    
    oam_spr(plrx, plry, 0xc3, 4, 4);
    // wait for next frame
    ppu_wait_frame();
  }
}
