/*
This file contains all the required stuff for the on-screen keyboard. #includes, GRRLIB init stuff etc..

*/

#define TILE_DELAY  10
#define TILE_UP     12*0
#define TILE_RIGHT  12*1
#define TILE_DOWN   12*2
#define TILE_LEFT   12*3
#define TILE_UP2    12*4+9
#define TILE_RIGHT2 12*5+9
#define TILE_DOWN2  12*6+9
#define TILE_LEFT2  12*7+9
// RGBA Colors
#define GRRLIB_BLACK   0x000000FF
#define GRRLIB_MAROON  0x800000FF
#define GRRLIB_GREEN   0x008000FF
#define GRRLIB_OLIVE   0x808000FF
#define GRRLIB_NAVY    0x000080FF
#define GRRLIB_PURPLE  0x800080FF
#define GRRLIB_TEAL    0x008080FF
#define GRRLIB_GRAY    0x808080FF
#define GRRLIB_SILVER  0xC0C0C0FF
#define GRRLIB_RED     0xFF0000FF
#define GRRLIB_LIME    0x00FF00FF
#define GRRLIB_YELLOW  0xFFFF00FF
#define GRRLIB_BLUE    0x0000FFFF
#define GRRLIB_FUCHSIA 0xFF00FFFF
#define GRRLIB_AQUA    0x00FFFFFF
#define GRRLIB_WHITE   0xFFFFFFFF

//lowercase letter button textures:
#include "gfx/keyboard/letters/lowercase/bta.h"
#include "gfx/keyboard/letters/lowercase/btb.h"
#include "gfx/keyboard/letters/lowercase/btc.h"
#include "gfx/keyboard/letters/lowercase/btd.h"
#include "gfx/keyboard/letters/lowercase/bte.h"
#include "gfx/keyboard/letters/lowercase/btf.h"
#include "gfx/keyboard/letters/lowercase/btg.h"
#include "gfx/keyboard/letters/lowercase/bth.h"
#include "gfx/keyboard/letters/lowercase/bti.h"
#include "gfx/keyboard/letters/lowercase/btj.h"
#include "gfx/keyboard/letters/lowercase/btk.h"
#include "gfx/keyboard/letters/lowercase/btl.h"
#include "gfx/keyboard/letters/lowercase/btm.h"
#include "gfx/keyboard/letters/lowercase/btn.h"
#include "gfx/keyboard/letters/lowercase/bto.h"
#include "gfx/keyboard/letters/lowercase/btp.h"
#include "gfx/keyboard/letters/lowercase/btq.h"
#include "gfx/keyboard/letters/lowercase/btr.h"
#include "gfx/keyboard/letters/lowercase/bts.h"
#include "gfx/keyboard/letters/lowercase/btt.h"
#include "gfx/keyboard/letters/lowercase/btu.h"
#include "gfx/keyboard/letters/lowercase/btv.h"
#include "gfx/keyboard/letters/lowercase/btw.h"
#include "gfx/keyboard/letters/lowercase/btx.h"
#include "gfx/keyboard/letters/lowercase/bty.h"
#include "gfx/keyboard/letters/lowercase/btz.h"
//uppercase letter button textures:
#include "gfx/keyboard/letters/uppercase/bta_UP.h"
#include "gfx/keyboard/letters/uppercase/btb_UP.h"
#include "gfx/keyboard/letters/uppercase/btc_UP.h"
#include "gfx/keyboard/letters/uppercase/btd_UP.h"
#include "gfx/keyboard/letters/uppercase/bte_UP.h"
#include "gfx/keyboard/letters/uppercase/btf_UP.h"
#include "gfx/keyboard/letters/uppercase/btg_UP.h"
#include "gfx/keyboard/letters/uppercase/bth_UP.h"
#include "gfx/keyboard/letters/uppercase/bti_UP.h"
#include "gfx/keyboard/letters/uppercase/btj_UP.h"
#include "gfx/keyboard/letters/uppercase/btk_UP.h"
#include "gfx/keyboard/letters/uppercase/btl_UP.h"
#include "gfx/keyboard/letters/uppercase/btm_UP.h"
#include "gfx/keyboard/letters/uppercase/btn_UP.h"
#include "gfx/keyboard/letters/uppercase/bto_UP.h"
#include "gfx/keyboard/letters/uppercase/btp_UP.h"
#include "gfx/keyboard/letters/uppercase/btq_UP.h"
#include "gfx/keyboard/letters/uppercase/btr_UP.h"
#include "gfx/keyboard/letters/uppercase/bts_UP.h"
#include "gfx/keyboard/letters/uppercase/btt_UP.h"
#include "gfx/keyboard/letters/uppercase/btu_UP.h"
#include "gfx/keyboard/letters/uppercase/btv_UP.h"
#include "gfx/keyboard/letters/uppercase/btw_UP.h"
#include "gfx/keyboard/letters/uppercase/btx_UP.h"
#include "gfx/keyboard/letters/uppercase/bty_UP.h"
#include "gfx/keyboard/letters/uppercase/btz_UP.h"
//number:
#include "gfx/keyboard/numbers/zero.h"
#include "gfx/keyboard/numbers/one.h"
#include "gfx/keyboard/numbers/two.h"
#include "gfx/keyboard/numbers/three.h"
#include "gfx/keyboard/numbers/four.h"
#include "gfx/keyboard/numbers/five.h"
#include "gfx/keyboard/numbers/six.h"
#include "gfx/keyboard/numbers/seven.h"
#include "gfx/keyboard/numbers/eight.h"
#include "gfx/keyboard/numbers/nine.h"
//symbols:
#include "gfx/keyboard/other/dash.h"
#include "gfx/keyboard/other/erase.h"
#include "gfx/keyboard/other/caps.h"
#include "gfx/keyboard/other/caps_colored.h"
#include "gfx/keyboard/other/space.h"
#include "gfx/keyboard/other/yen.h"
#include "gfx/keyboard/other/euro.h"
#include "gfx/keyboard/other/bracket_1.h"
#include "gfx/keyboard/other/bracket_2.h"
#include "gfx/keyboard/other/slash.h"
#include "gfx/keyboard/other/backslash.h"
#include "gfx/keyboard/other/apostrophe.h"
#include "gfx/keyboard/other/hash.h"
#include "gfx/keyboard/other/comma.h"
#include "gfx/keyboard/other/point.h"
#include "gfx/keyboard/other/semicolon.h"
#include "gfx/keyboard/other/equal.h"


//line 0 = 1 2 3 4 5 6 7 8 9 0 - ERASE
//line 1 = q w e r t y u i o p
//line 2 = CAPS - a s d f g h j k l ;
//line 3 = z x c v b n m , . =
//line 4 = ¥ € [ ] SPACE \ / ' #

int line0_y = 167;
int line1_y = 205; 
int line2_y = 242;
int line3_y = 279;
int line4_y = 318;

//line 1 first letter (q):
int btq_coordX = 53;
int btq_coordY = line1_y;
//line 2 first letter(a):
int bta_coordX = 123;
int bta_coordY = line2_y;
//line 3 first letter(z):
int btz_coordX = 133;
int btz_coordY = line3_y;


int btb_coordX = btz_coordX + (46*4);
int btb_coordY = line3_y;

int btc_coordX = btz_coordX + (46*2);
int btc_coordY = line3_y;

int btd_coordX = bta_coordX + (46*2);
int btd_coordY = line2_y;

int bte_coordX = btq_coordX + (46*2);
int bte_coordY = line1_y;

int btf_coordX = bta_coordX + (46*3);
int btf_coordY = line2_y;

int btg_coordX = bta_coordX + (46*4);
int btg_coordY = line2_y;

int bth_coordX = bta_coordX + (46*5);
int bth_coordY = line2_y;

int bti_coordX = btq_coordX + (46*7);
int bti_coordY = line1_y;

int btj_coordX = bta_coordX + (46*6);
int btj_coordY = line2_y;

int btk_coordX = bta_coordX + (46*7);
int btk_coordY = line2_y;

int btl_coordX = bta_coordX + (46*8);
int btl_coordY = line2_y;

int btm_coordX = btz_coordX + (46*6);
int btm_coordY = line3_y;

int btn_coordX = btz_coordX + (46*5);
int btn_coordY = line3_y;

int bto_coordX = btq_coordX + (46*8);
int bto_coordY = line1_y;

int btp_coordX = btq_coordX + (46*9);
int btp_coordY = line1_y;

int btr_coordX = btq_coordX + (46*3);
int btr_coordY = line1_y;

int bts_coordX = bta_coordX + (46*1);
int bts_coordY = line2_y;

int btt_coordX = btq_coordX + (46*4);
int btt_coordY = line1_y;

int btu_coordX = btq_coordX + (46*6);
int btu_coordY = line1_y;

int btv_coordX = btz_coordX + (46*3);
int btv_coordY = line3_y;

int btw_coordX = btq_coordX + 46;
int btw_coordY = line1_y;

int btx_coordX = btz_coordX + (46*1);
int btx_coordY = line3_y;

int bty_coordX = btq_coordX + (46*5);
int bty_coordY = line1_y;

//coordinates for other buttons:

int caps_coordX = 30; //this is used by caps and caps_colored 
int caps_coordY = line2_y;

int space_coordX = 259;
int space_coordY = line4_y;

int yen_coordX = 76;
int yen_coordY = line4_y;

int euro_coordX = 76+(46*1);
int euro_coordY = line4_y;

int bracket_1_coordX = 76+(46*2);
int bracket_1_coordY = line4_y;

int bracket_2_coordX = 76+(46*3);
int bracket_2_coordY = line4_y;

int backslash_coordX = 427;
int backslash_coordY = line4_y;

int slash_coordX = backslash_coordX+(46*1);
int slash_coordY = line4_y;

int apostrophe_coordX = slash_coordX+(46*1);
int apostrophe_coordY = line4_y;

int hash_coordX = apostrophe_coordX+46;
int hash_coordY = line4_y;
 
int comma_coordX = btz_coordX + (46*7);
int comma_coordY = line3_y;
 
int point_coordX = btz_coordX + (46*8);
int point_coordY = line3_y;
 
int equal_coordX = btz_coordX + (46*9);
int equal_coordY = line3_y;
 
int semicolon_coordX = bta_coordX + (46*9);
int semicolon_coordY = line2_y;
 


//coords for numbers :


int one_coordX = 30;
int one_coordY = line0_y;

int two_coordX = 30+(48*1);
int two_coordY = line0_y;

int three_coordX = 30+(48*2);
int three_coordY = line0_y;

int four_coordX = 30+(48*3);
int four_coordY = line0_y;

int five_coordX = 30+(48*4);
int five_coordY = line0_y;

int six_coordX = 30+(48*5);
int six_coordY = line0_y;

int seven_coordX = 30+(48*6);
int seven_coordY = line0_y;

int eight_coordX = 30+(48*7);
int eight_coordY = line0_y;

int nine_coordX = 30+(48*8);
int nine_coordY = line0_y;

int zero_coordX = 30+(48*9);
int zero_coordY = line0_y;

int dash_coordX = 30+(48*10);
int dash_coordY = line0_y;

int erase_coordX = 30+(48*11);
int erase_coordY = line0_y;

int erase_width = 76;
int erase_height = 36;
	
int caps_width = 92;
int caps_height = 36;
	
int space_width = 164;
int space_height = 36;

u32 wpaddown, wpadheld, paddown, padheld;
s32 left = 0, top = 0, page = 0, frame = TILE_DOWN + 1;
u32 wait = TILE_DELAY, direction = TILE_DOWN, direction_new = TILE_DOWN;
ir_t ir1;

//init textures:
GRRLIB_texImg *tex_bta_png; 
GRRLIB_texImg *tex_btb_png; 
GRRLIB_texImg *tex_btc_png; 
GRRLIB_texImg *tex_btd_png; 
GRRLIB_texImg *tex_bte_png; 
GRRLIB_texImg *tex_btf_png; 
GRRLIB_texImg *tex_btg_png; 
GRRLIB_texImg *tex_bth_png; 
GRRLIB_texImg *tex_bti_png; 
GRRLIB_texImg *tex_btj_png; 
GRRLIB_texImg *tex_btk_png; 
GRRLIB_texImg *tex_btl_png; 
GRRLIB_texImg *tex_btm_png; 
GRRLIB_texImg *tex_btn_png; 
GRRLIB_texImg *tex_bto_png; 
GRRLIB_texImg *tex_btp_png; 
GRRLIB_texImg *tex_btq_png; 
GRRLIB_texImg *tex_btr_png; 
GRRLIB_texImg *tex_bts_png; 
GRRLIB_texImg *tex_btt_png; 
GRRLIB_texImg *tex_btu_png; 
GRRLIB_texImg *tex_btv_png; 
GRRLIB_texImg *tex_btw_png; 
GRRLIB_texImg *tex_btx_png; 
GRRLIB_texImg *tex_bty_png; 
GRRLIB_texImg *tex_btz_png; 

GRRLIB_texImg *tex_bta_UP_png; 
GRRLIB_texImg *tex_btb_UP_png; 
GRRLIB_texImg *tex_btc_UP_png; 
GRRLIB_texImg *tex_btd_UP_png; 
GRRLIB_texImg *tex_bte_UP_png; 
GRRLIB_texImg *tex_btf_UP_png; 
GRRLIB_texImg *tex_btg_UP_png; 
GRRLIB_texImg *tex_bth_UP_png; 
GRRLIB_texImg *tex_bti_UP_png; 
GRRLIB_texImg *tex_btj_UP_png; 
GRRLIB_texImg *tex_btk_UP_png; 
GRRLIB_texImg *tex_btl_UP_png; 
GRRLIB_texImg *tex_btm_UP_png; 
GRRLIB_texImg *tex_btn_UP_png; 
GRRLIB_texImg *tex_bto_UP_png; 
GRRLIB_texImg *tex_btp_UP_png; 
GRRLIB_texImg *tex_btq_UP_png; 
GRRLIB_texImg *tex_btr_UP_png; 
GRRLIB_texImg *tex_bts_UP_png; 
GRRLIB_texImg *tex_btt_UP_png; 
GRRLIB_texImg *tex_btu_UP_png; 
GRRLIB_texImg *tex_btv_UP_png; 
GRRLIB_texImg *tex_btw_UP_png; 
GRRLIB_texImg *tex_btx_UP_png; 
GRRLIB_texImg *tex_bty_UP_png; 
GRRLIB_texImg *tex_btz_UP_png; 


GRRLIB_texImg *tex_one_png; 
GRRLIB_texImg *tex_two_png; 
GRRLIB_texImg *tex_three_png; 
GRRLIB_texImg *tex_four_png; 
GRRLIB_texImg *tex_five_png; 
GRRLIB_texImg *tex_six_png; 
GRRLIB_texImg *tex_seven_png; 
GRRLIB_texImg *tex_eight_png; 
GRRLIB_texImg *tex_nine_png; 
GRRLIB_texImg *tex_zero_png; 

GRRLIB_texImg *tex_dash_png; 
GRRLIB_texImg *tex_erase_png; 
GRRLIB_texImg *tex_caps_png;
GRRLIB_texImg *tex_caps_colored_png; 
GRRLIB_texImg *tex_space_png;
GRRLIB_texImg *tex_yen_png; 
GRRLIB_texImg *tex_euro_png; 
GRRLIB_texImg *tex_bracket_1_png;
GRRLIB_texImg *tex_bracket_2_png; 
GRRLIB_texImg *tex_backslash_png;
GRRLIB_texImg *tex_slash_png;
GRRLIB_texImg *tex_apostrophe_png;
GRRLIB_texImg *tex_hash_png;
GRRLIB_texImg *tex_comma_png;
GRRLIB_texImg *tex_point_png;
GRRLIB_texImg *tex_equal_png;
GRRLIB_texImg *tex_semicolon_png;


GRRLIB_texImg *tex_blank_kbd_png;
GRRLIB_texImg *tex_BMfont5;
GRRLIB_texImg *tex_settings_background_png;

GRRLIB_texImg *tex_button1_png;
