/*
This file contains all the required stuff for the on-screen keyboard. #includes, GRRLIB init stuff etc..

*/

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
#include "gfx/keyboard/other/underscore.h" //added at v1.4
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
//line 1 = q w e r t y u i o p _
//line 2 = CAPS - a s d f g h j k l ;
//line 3 = z x c v b n m , . =
//line 4 = ¥ € [ ] SPACE \ / ' #

int line0_y = 167;
int line1_y = 205; 
int line2_y = 242;
int line3_y = 279;
int line4_y = 318;

//line 1 first letter (q):
int btq_coordX = 53,btq_coordY = line1_y;
//line 2 first letter(a):
int bta_coordX = 123,bta_coordY = line2_y;
//line 3 first letter(z):
int btz_coordX = 133,btz_coordY = line3_y;

int btb_coordX = btz_coordX + (46*4),btb_coordY = line3_y;
int btc_coordX = btz_coordX + (46*2),btc_coordY = line3_y;
int btd_coordX = bta_coordX + (46*2),btd_coordY = line2_y;
int bte_coordX = btq_coordX + (46*2),bte_coordY = line1_y;
int btf_coordX = bta_coordX + (46*3),btf_coordY = line2_y;
int btg_coordX = bta_coordX + (46*4),btg_coordY = line2_y;
int bth_coordX = bta_coordX + (46*5),bth_coordY = line2_y;
int bti_coordX = btq_coordX + (46*7),bti_coordY = line1_y;
int btj_coordX = bta_coordX + (46*6),btj_coordY = line2_y;
int btk_coordX = bta_coordX + (46*7),btk_coordY = line2_y;
int btl_coordX = bta_coordX + (46*8),btl_coordY = line2_y;
int btm_coordX = btz_coordX + (46*6),btm_coordY = line3_y;
int btn_coordX = btz_coordX + (46*5),btn_coordY = line3_y;
int bto_coordX = btq_coordX + (46*8),bto_coordY = line1_y;
int btp_coordX = btq_coordX + (46*9),btp_coordY = line1_y;
int btr_coordX = btq_coordX + (46*3),btr_coordY = line1_y;
int bts_coordX = bta_coordX + (46*1),bts_coordY = line2_y;
int btt_coordX = btq_coordX + (46*4),btt_coordY = line1_y;
int btu_coordX = btq_coordX + (46*6),btu_coordY = line1_y;
int btv_coordX = btz_coordX + (46*3),btv_coordY = line3_y;
int btw_coordX = btq_coordX + 46,btw_coordY = line1_y;
int btx_coordX = btz_coordX + (46*1),btx_coordY = line3_y;
int bty_coordX = btq_coordX + (46*5),bty_coordY = line1_y;

//coordinates for other buttons:

int caps_coordX = 30 ,caps_coordY = line2_y;//this is used by caps and caps_colored 
int space_coordX = 259, space_coordY = line4_y;
int yen_coordX = 76, yen_coordY = line4_y;
int euro_coordX = 76+(46*1), euro_coordY = line4_y;
int bracket_1_coordX = 76+(46*2), bracket_1_coordY = line4_y;
int bracket_2_coordX = 76+(46*3), bracket_2_coordY = line4_y;
int backslash_coordX = 427, backslash_coordY = line4_y;
int slash_coordX = backslash_coordX+(46*1), slash_coordY = line4_y;
int apostrophe_coordX = slash_coordX+(46*1), apostrophe_coordY = line4_y;
int hash_coordX = apostrophe_coordX+46, hash_coordY = line4_y;
int comma_coordX = btz_coordX + (46*7), comma_coordY = line3_y;
int point_coordX = btz_coordX + (46*8), point_coordY = line3_y;
int equal_coordX = btz_coordX + (46*9),equal_coordY = line3_y;
int semicolon_coordX = bta_coordX + (46*9),semicolon_coordY = line2_y;
 
//coords for numbers :

int one_coordX = 30, one_coordY = line0_y;
int two_coordX = 30+(48*1), two_coordY = line0_y;
int three_coordX = 30+(48*2), three_coordY = line0_y;
int four_coordX = 30+(48*3), four_coordY = line0_y;
int five_coordX = 30+(48*4), five_coordY = line0_y;
int six_coordX = 30+(48*5), six_coordY = line0_y;
int seven_coordX = 30+(48*6), seven_coordY = line0_y;
int eight_coordX = 30+(48*7), eight_coordY = line0_y;
int nine_coordX = 30+(48*8), nine_coordY = line0_y;
int zero_coordX = 30+(48*9), zero_coordY = line0_y;
int dash_coordX = 30+(48*10), dash_coordY = line0_y;
int underscore_coordX = btq_coordX + (46*10), underscore_coordY = line1_y; //added at v1.4
int erase_coordX = 30+(48*11), erase_coordY = line0_y;
int erase_width = 76, erase_height = 36;
int caps_width = 92, caps_height = 36;
int space_width = 164, space_height = 36;