/*

//Wii Network Config Editor by Bazmoc (2023)

If you have questions contact me on discord : "Bazmoc"

This program has been tested on both Dolphin and 2 real Wiis without any issues.
Please have a NAND backup before using my program. 

Thx to Supertazon & Aurelio for help troubleshooting my program.
Some textures and sound effects are from Usb loader GX
the ISFS part is partly inspired from Fix94's "simple Cert.sys extractor"
*/

#include <gccore.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <ogc/machine/processor.h>
#include <ogcsys.h>
#include <locale.h>
#include <wiiuse/wpad.h>
#include <stdlib.h>
#include <ogc/isfs.h>
#include <fat.h>
#include <network.h>
#include <time.h>
#include <ogc/lwp_watchdog.h>
#include <grrlib.h>
#include <wiiuse/wpad.h>

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


//network configuration file path and length :
#define ISFS_CONFIGDAT_PATH "/shared2/sys/net/02/config.dat"
#define CONFIGDAT_FILELENGTH 7004
//temporary file needed to know that the user chose to restart WNCE for the network test.
#define ISFS_TEMPFILE_PATH "/shared2/sys/net/02/WNCE.tmp" //this temporary file is there so that when the program reboots the connection test begins

int SSID1_pos=1996, PASSWORD1_pos=2040; //location of the first ssid and first password in the file config.dat 

//those characters determine which connection is selected. 
int connection1_selectpos = 8, connection2_selectpos = 8+2332, connection3_selectpos = 8+2332+2332;//like i said before, 2332 is the number of byte in 1 connection

int connection1_securitypos = 2033; //security type position (connection 1 only)

char connection_wireless_selectedchar = 0xA6, connection_wireless_unselectedchar = 0x26; //when a wifi connection is selected the character is 0xA6, if it's not selected but is wifi, it's 0x26
char connection_wired_selectedchar = 0xA7, connection_wired_unselectedchar = 0x27; //same thing but for a wired connection

int buttonWidth = 172, buttonHeight = 44; 
#include "security_menu_buttons.h"
#include "keyboard_texture_coords.h" //contains all the button's coordinates (x+y) and the #include's
#include "sounds.h"
#include "wiimote_vibration.h"
#include "isfs_readwrite.h"
#include "length2hex.h"

//include for textures,backgrounds,fonts..
#include "gfx/font/BMfont1.h"
#include "gfx/font/BMfont2.h"
#include "gfx/font/BMfont3.h"
#include "gfx/font/BMfont4.h"
#include "gfx/font/BMfont5.h"

#include "gfx/cursor/cursor.h"
#include "gfx/button/button1.h"
#include "gfx/button/selected.h"
#include "gfx/button/poweroff.h"

#include "gfx/button/wireless.h"
#include "gfx/button/wired.h"

#include "gfx/backgrounds/blank_kbd.h"
#include "gfx/backgrounds/settings_background.h"
#include "gfx/backgrounds/credits_bg.h"
#include "gfx/backgrounds/dialogue_box.h"

////////////Menu buttons ://///////////// (wired/wireless, credits,exit, button1/2/3.....)
int connectiontype_Width = 248;
int connectiontype_Height = 48;

int WirelessCoordX = 50, WirelessCoordY = 200;

int WiredCoordX = 350, WiredCoordY = 200;

int button1CoordX = 100-(buttonWidth/2), button1CoordY = 400;
int button2CoordX = 325-(buttonWidth/2), button2CoordY = 400;
int button3CoordX = (640-100)-(buttonWidth/2), button3CoordY = 400;

//
int mainmenubt1CoordX = button1CoordX, mainmenubt1CoordY = button1CoordY-60;
int mainmenubt2CoordX = button2CoordX, mainmenubt2CoordY = button2CoordY-60;
int mainmenubt3CoordX = button3CoordX, mainmenubt3CoordY = button3CoordY-60;

int creditbtnCoordX = button3CoordX, creditbtnCoordY = button3CoordY+40;

int exitCoordX = 550;
int exitCoordY = 25;
///////////////END MENU BUTTONS///////////////


//this small function checks if the cursor is 'on' a texture(this texture can be a button for example)
//, it returns true if the first texture is on the second one. The first texture is the cursor and the second may be a button
bool collisionButton(int button_CoordX, int button_coordY, int cursorPosX,int cursorPosY, int btnW, int btnH){
	if(GRRLIB_RectOnRect(button_CoordX,button_coordY,btnW,btnH,cursorPosX,cursorPosY,5,5)){ //64,64 too much
		return true;
	} else return false;
}

GRRLIB_texImg *tex_cursor_png; //initialises *tex_cursor_png to use on updatecursorpos()

u32 wpaddown, wpadheld, paddown, padheld;
s32 left = 0, top = 0, page = 0, frame = TILE_DOWN + 1;
u32 wait = TILE_DELAY, direction = TILE_DOWN, direction_new = TILE_DOWN;
ir_t ir1;

///////////CURSOR://///////////////

int cursor_positionX = 640/2, cursor_positionY = 480/2; //put cursor at center of screen on startup.

int cursor_rotation_angle = 0; //initial angle

int threshold = 10;
int sensitivity = 15;
	
void updatecursorpos(){
	WPAD_SetVRes(0, 640, 480);
        WPAD_ScanPads();
		WPADData *data = WPAD_Data(0);

		PAD_ScanPads();

        wpaddown = WPAD_ButtonsDown(0);
        wpadheld = WPAD_ButtonsHeld(0);
		
		paddown = PAD_ButtonsDown(0);
		padheld = PAD_ButtonsHeld(0);
		
		WPAD_IR(WPAD_CHAN_0, &ir1);

		if (data->ir.valid) {
			cursor_positionX = (int)ir1.x;
			cursor_positionY = (int)ir1.y;
			//CURSOR ROTATION FUNCTION WAS TEMPORARLY REMOVED BECAUSE IT CREATED ISSUES.
			//cursor_rotation_angle = (int)ir1.angle; //ir1.angle is a float so we convert it to an int
		} else {
			cursor_rotation_angle = 0; //resets the angle from the wiimote
			if ((int)PAD_StickY(0) > threshold || (int)PAD_StickY(0) < -threshold ||(int)PAD_StickX(0) > threshold ||(int)PAD_StickX(0) < -threshold){
				if (cursor_positionX+((int)PAD_StickX(0)/sensitivity) >=0 && cursor_positionX+((int)PAD_StickX(0)/sensitivity) <=640 && cursor_positionY-((int)PAD_StickY(0)/sensitivity) >= 0 && cursor_positionY-((int)PAD_StickY(0)/sensitivity) <= 480){ //makes sure the cursor never leaves the screen when using the GC joystick
							cursor_positionX = cursor_positionX+((int)PAD_StickX(0)/sensitivity);
							cursor_positionY = cursor_positionY-((int)PAD_StickY(0)/sensitivity); //minus needed or the controls are inversed.
				}
			}
			
			
			//wiimote nunchuck stick:
			expansion_t e;
			int nx;
			int ny;

			WPAD_Expansion(WPAD_CHAN_0, &e);
			if(e.type == WPAD_EXP_NUNCHUK) //if the thing that's plugged in the wiimote port is a nunchuck
			{
				nx = e.nunchuk.js.pos.x - e.nunchuk.js.center.x;
				ny = e.nunchuk.js.pos.y - e.nunchuk.js.center.y;
			} else {
				nx=0; //reset the x&y positions of the nunchuck stick. You may be thinking 'well if it's not plugged in, how could the value be different from 0??' well, if you plug the nunchuck in, push the stick a little, then unplug it at the same time, the last X&Y values will stay.
				ny=0;
			}
			
			
			if (ny > threshold || ny < -threshold ||nx > threshold ||nx < -threshold){
				if (cursor_positionX+(nx/sensitivity) >=0 && cursor_positionX+(nx/sensitivity) <=640 && cursor_positionY-(ny/sensitivity) >= 0 && cursor_positionY-(ny/sensitivity) <= 480){ //makes sure the cursor never leaves the screen when using the nunchuck stick
							cursor_positionX = cursor_positionX+(nx/sensitivity);
							cursor_positionY = cursor_positionY-(ny/sensitivity); //minus needed or the controls are inversed.
				}
			}
		}	
		
}

////////////////END CURSOR////////////////





char SSIDkeyboard_writtentext[33] = "", Passwordkeyboard_writtentext[64] = "";
int SSID_indexkeyboard = 0, Password_indexkeyboard = 0;
bool SSID_uppercase = false, Password_uppercase = false;
	

//////////////////////KEYBOARD://///////////////
//this small function clears either the SSID or the Password Keyboard when clicking "cancel" on the on-screen keyboard
void clearkeyboard(int kbd){ //1 = ssid keyboard, 2=password keyboard
	if (kbd==1){
		for(int i=0;i<=32;i++) {
			SSIDkeyboard_writtentext[i] = '\0'; //resets the keyboard
			SSID_indexkeyboard=0;
		}
	}
	
	if (kbd==2){
		for(int i=0;i<=63;i++) {
			Passwordkeyboard_writtentext[i] = '\0'; //resets the keyboard
			Password_indexkeyboard=0;
		}
	}
}



//this small function manages the backspace function. needed for the wiimote's B button, GC controller B button, and the 2 graphical keyboard's backspace buttons.

void kbd_backspace(int keyboard_type){//keyboard_type : 0= SSID keyboard | 1=Password keyboard

	if (keyboard_type == 0){
		if (SSID_indexkeyboard>0 ) { //you don't want that number to get negative lol 		
			SSIDkeyboard_writtentext[SSID_indexkeyboard-1] = '\0'; //erases the character
			SSID_indexkeyboard-=1;
			erasesound();
		} else maxsound();
		
	} else{ //must be 1 if it's not 0, right?
		if (Password_indexkeyboard>0 ) { 	
			Passwordkeyboard_writtentext[Password_indexkeyboard-1] = '\0'; //erases the character
			Password_indexkeyboard-=1;
			erasesound();
			}else maxsound();
	}
	
	wiirumble_set(0,vib_duration2);
}





bool CusorOnKey(int key_coordX,int key_coordY,int key_width,int key_height){
if(GRRLIB_RectOnRect(key_coordX,key_coordY,key_width,key_height,cursor_positionX, cursor_positionY,5,5)){return true;} else{return false;}
}
////////////////END KEYBOARD/////////////////////+


char shortenedssid[33], shortenedpassword[64];

char connection1_securitytype;
int connection1_newsecuritytype;

char password_firstline[46], password_secondline[63-45]; //63= max length of password

//those float values define the text's size on the on-screen keyboards.
float SSIDkbd_zoom = 1.5, passwordkbd_zoom = 1.5;

//this float variable determines the on-screen keyboard's key sizes.
float kbd_button_Xsize = 1.0, kbd_button_Ysize = 1.0;

//network testing:
int isNetworkWorking = 0; //0 = the user didn't even click on 'test connection'; 1=user clicked on 'test connection' but the wifi isnt working; and 2= the user clicked and the wifi works.
int NetworkError = 0; //this is the error that will be displayed if the network test returns an error
int networktype; //0=wired,1=wireless
s32 deletefile; //needed to delete file ISFS_TEMPFILE_PATH

int CurrentColor(int value, int maxValue){ //this function is here to make the character counter number Red when it is at its maximum. example : currentcolor(10,32) returns BLACK but 32,32 returns red
	if(value==maxValue) { return GRRLIB_RED; } else { return GRRLIB_BLACK; }
}

extern "C" {extern void exit(int status);}

u8 *certBuffer = NULL;
u32 certSize = 0;
void exitprogram(){
	//if (certBuffer != NULL || certSize > 0) free(certBuffer);
	ISFS_Deinitialize();
	ASND_End();
	GRRLIB_Exit();
	
	exit(0);
}


int chosenmenu = 0;

//main function:
int main(int argc,char** argv) {
	GRRLIB_Init();
	wiirumble_init();
	PAD_Init();
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	
	ASND_Init(); 
    ASND_Pause(0);
	track = ASND_GetFirstUnusedVoice();
	
	char SSID1[33];
	char PASSWORD1[64];
	
	GRRLIB_texImg *tex_wireless_png = GRRLIB_LoadTexture(wireless);
    GRRLIB_InitTileSet(tex_wireless_png, connectiontype_Width, connectiontype_Height, 0); 
	
	GRRLIB_texImg *tex_wired_png = GRRLIB_LoadTexture(wired);
    GRRLIB_InitTileSet(tex_wired_png, connectiontype_Width, connectiontype_Height, 0); 
	
	
	GRRLIB_texImg *tex_poweroff_png = GRRLIB_LoadTexture(poweroff);
    GRRLIB_InitTileSet(tex_poweroff_png, 640, 480, 0); 
	
	GRRLIB_texImg *tex_settings_background_png = GRRLIB_LoadTexture(settings_background);
    GRRLIB_InitTileSet(tex_settings_background_png, 640, 480, 0); 
	
	GRRLIB_texImg *tex_credits_bg_png = GRRLIB_LoadTexture(credits_bg);
    GRRLIB_InitTileSet(tex_credits_bg_png, 472, 320, 0); 

	GRRLIB_texImg *tex_dialogue_box_png = GRRLIB_LoadTexture(dialogue_box);
    GRRLIB_InitTileSet(tex_dialogue_box_png, 640, 480, 0); 

    GRRLIB_texImg *tex_BMfont1 = GRRLIB_LoadTexture(BMfont1);
    GRRLIB_InitTileSet(tex_BMfont1, 32, 32, 32);
	
	GRRLIB_texImg *tex_BMfont2 = GRRLIB_LoadTexture(BMfont2);
    GRRLIB_InitTileSet(tex_BMfont2, 16, 16, 32);

    GRRLIB_texImg *tex_BMfont3 = GRRLIB_LoadTexture(BMfont3);
    GRRLIB_InitTileSet(tex_BMfont3, 32, 32, 32);

    GRRLIB_texImg *tex_BMfont4 = GRRLIB_LoadTexture(BMfont4);
    GRRLIB_InitTileSet(tex_BMfont4, 16, 16, 32);

    GRRLIB_texImg *tex_BMfont5 = GRRLIB_LoadTexture(BMfont5);
    GRRLIB_InitTileSet(tex_BMfont5, 8, 16, 0);

	GRRLIB_texImg *tex_cursor_png = GRRLIB_LoadTexture(cursor);
    GRRLIB_InitTileSet(tex_cursor_png, 128, 128, 0);
	
	GRRLIB_texImg *tex_button1_png = GRRLIB_LoadTexture(button1);
    GRRLIB_InitTileSet(tex_button1_png, 128, 128, 0);

	//init lowercase letters:
	//dimensions of a wii keyboard button : x44 y36
	GRRLIB_texImg *tex_bta_png = GRRLIB_LoadTexture(bta);
    GRRLIB_InitTileSet(tex_bta_png, 44, 36, 0); 

	GRRLIB_texImg *tex_btb_png = GRRLIB_LoadTexture(btb);
    GRRLIB_InitTileSet(tex_btb_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btc_png = GRRLIB_LoadTexture(btc);
    GRRLIB_InitTileSet(tex_btc_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btd_png = GRRLIB_LoadTexture(btd);
    GRRLIB_InitTileSet(tex_btd_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bte_png = GRRLIB_LoadTexture(bte);
    GRRLIB_InitTileSet(tex_bte_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btf_png = GRRLIB_LoadTexture(btf);
    GRRLIB_InitTileSet(tex_btf_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btg_png = GRRLIB_LoadTexture(btg);
    GRRLIB_InitTileSet(tex_btg_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bth_png = GRRLIB_LoadTexture(bth);
    GRRLIB_InitTileSet(tex_bth_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bti_png = GRRLIB_LoadTexture(bti);
    GRRLIB_InitTileSet(tex_bti_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btj_png = GRRLIB_LoadTexture(btj);
    GRRLIB_InitTileSet(tex_btj_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btk_png = GRRLIB_LoadTexture(btk);
    GRRLIB_InitTileSet(tex_btk_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btl_png = GRRLIB_LoadTexture(btl);
    GRRLIB_InitTileSet(tex_btl_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btm_png = GRRLIB_LoadTexture(btm);
    GRRLIB_InitTileSet(tex_btm_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btn_png = GRRLIB_LoadTexture(btn);
    GRRLIB_InitTileSet(tex_btn_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bto_png = GRRLIB_LoadTexture(bto);
    GRRLIB_InitTileSet(tex_bto_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btp_png = GRRLIB_LoadTexture(btp);
    GRRLIB_InitTileSet(tex_btp_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btq_png = GRRLIB_LoadTexture(btq);
    GRRLIB_InitTileSet(tex_btq_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btr_png = GRRLIB_LoadTexture(btr);
    GRRLIB_InitTileSet(tex_btr_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bts_png = GRRLIB_LoadTexture(bts);
    GRRLIB_InitTileSet(tex_bts_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btt_png = GRRLIB_LoadTexture(btt);
    GRRLIB_InitTileSet(tex_btt_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btu_png = GRRLIB_LoadTexture(btu);
    GRRLIB_InitTileSet(tex_btu_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btv_png = GRRLIB_LoadTexture(btv);
    GRRLIB_InitTileSet(tex_btv_png, 44, 36, 0);
	
	GRRLIB_texImg *tex_btw_png = GRRLIB_LoadTexture(btw);
    GRRLIB_InitTileSet(tex_btw_png, 44, 36, 0);
	
	GRRLIB_texImg *tex_btx_png = GRRLIB_LoadTexture(btx);
    GRRLIB_InitTileSet(tex_btx_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bty_png = GRRLIB_LoadTexture(bty);
    GRRLIB_InitTileSet(tex_bty_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btz_png = GRRLIB_LoadTexture(btz);
    GRRLIB_InitTileSet(tex_btz_png, 44, 36, 0); 
	
	//init uppercase letters:
	GRRLIB_texImg *tex_bta_UP_png = GRRLIB_LoadTexture(BTA_UP);
    GRRLIB_InitTileSet(tex_bta_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btb_UP_png = GRRLIB_LoadTexture(BTB_UP);
    GRRLIB_InitTileSet(tex_btb_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btc_UP_png = GRRLIB_LoadTexture(BTC_UP);
    GRRLIB_InitTileSet(tex_btc_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btd_UP_png = GRRLIB_LoadTexture(BTD_UP);
    GRRLIB_InitTileSet(tex_btd_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bte_UP_png = GRRLIB_LoadTexture(BTE_UP);
    GRRLIB_InitTileSet(tex_bte_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btf_UP_png = GRRLIB_LoadTexture(BTF_UP);
    GRRLIB_InitTileSet(tex_btf_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btg_UP_png = GRRLIB_LoadTexture(BTG_UP);
    GRRLIB_InitTileSet(tex_btg_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bth_UP_png = GRRLIB_LoadTexture(BTH_UP);
    GRRLIB_InitTileSet(tex_bth_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bti_UP_png = GRRLIB_LoadTexture(BTI_UP);
    GRRLIB_InitTileSet(tex_bti_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btj_UP_png = GRRLIB_LoadTexture(BTJ_UP);
    GRRLIB_InitTileSet(tex_btj_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btk_UP_png = GRRLIB_LoadTexture(BTK_UP);
    GRRLIB_InitTileSet(tex_btk_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btl_UP_png = GRRLIB_LoadTexture(BTL_UP);
    GRRLIB_InitTileSet(tex_btl_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btm_UP_png = GRRLIB_LoadTexture(BTM_UP);
    GRRLIB_InitTileSet(tex_btm_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btn_UP_png = GRRLIB_LoadTexture(BTN_UP);
    GRRLIB_InitTileSet(tex_btn_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bto_UP_png = GRRLIB_LoadTexture(BTO_UP);
    GRRLIB_InitTileSet(tex_bto_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btp_UP_png = GRRLIB_LoadTexture(BTP_UP);
    GRRLIB_InitTileSet(tex_btp_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btq_UP_png = GRRLIB_LoadTexture(BTQ_UP);
    GRRLIB_InitTileSet(tex_btq_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btr_UP_png = GRRLIB_LoadTexture(BTR_UP);
    GRRLIB_InitTileSet(tex_btr_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bts_UP_png = GRRLIB_LoadTexture(BTS_UP);
    GRRLIB_InitTileSet(tex_bts_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btt_UP_png = GRRLIB_LoadTexture(BTT_UP);
    GRRLIB_InitTileSet(tex_btt_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btu_UP_png = GRRLIB_LoadTexture(BTU_UP);
    GRRLIB_InitTileSet(tex_btu_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btv_UP_png = GRRLIB_LoadTexture(BTV_UP);
    GRRLIB_InitTileSet(tex_btv_UP_png, 44, 36, 0);
	
	GRRLIB_texImg *tex_btw_UP_png = GRRLIB_LoadTexture(BTW_UP);
    GRRLIB_InitTileSet(tex_btw_UP_png, 44, 36, 0);
	
	GRRLIB_texImg *tex_btx_UP_png = GRRLIB_LoadTexture(BTX_UP);
    GRRLIB_InitTileSet(tex_btx_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bty_UP_png = GRRLIB_LoadTexture(BTY_UP);
    GRRLIB_InitTileSet(tex_bty_UP_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_btz_UP_png = GRRLIB_LoadTexture(BTZ_UP);
    GRRLIB_InitTileSet(tex_btz_UP_png, 44, 36, 0); 
	
	
	//init. numbers:

	GRRLIB_texImg *tex_one_png = GRRLIB_LoadTexture(one);
    GRRLIB_InitTileSet(tex_one_png, 48, 40, 0); 
	
	GRRLIB_texImg *tex_two_png = GRRLIB_LoadTexture(two);
    GRRLIB_InitTileSet(tex_two_png, 48, 40, 0); 
	
	GRRLIB_texImg *tex_three_png = GRRLIB_LoadTexture(three);
    GRRLIB_InitTileSet(tex_three_png, 48, 40, 0); 
	
	GRRLIB_texImg *tex_four_png = GRRLIB_LoadTexture(four);
    GRRLIB_InitTileSet(tex_four_png, 48, 40, 0); 
	
	GRRLIB_texImg *tex_five_png = GRRLIB_LoadTexture(five);
    GRRLIB_InitTileSet(tex_five_png, 48, 40, 0); 
	
	GRRLIB_texImg *tex_six_png = GRRLIB_LoadTexture(six);
    GRRLIB_InitTileSet(tex_six_png, 48, 40, 0); 
	
	GRRLIB_texImg *tex_seven_png = GRRLIB_LoadTexture(seven);
    GRRLIB_InitTileSet(tex_seven_png, 48, 40, 0); 
	
	GRRLIB_texImg *tex_eight_png = GRRLIB_LoadTexture(eight);
    GRRLIB_InitTileSet(tex_eight_png, 48, 40, 0); 
	
	GRRLIB_texImg *tex_nine_png = GRRLIB_LoadTexture(nine);
    GRRLIB_InitTileSet(tex_nine_png, 48, 40, 0); 
	
	GRRLIB_texImg *tex_zero_png = GRRLIB_LoadTexture(zero);
    GRRLIB_InitTileSet(tex_zero_png, 48, 40, 0); 
	
	
	
	
	////////other:///////
	
	GRRLIB_texImg *tex_erase_png = GRRLIB_LoadTexture(erase);
    GRRLIB_InitTileSet(tex_erase_png, erase_width, erase_height, 0); 
	
	
	GRRLIB_texImg *tex_caps_png = GRRLIB_LoadTexture(caps);
    GRRLIB_InitTileSet(tex_caps_png,caps_width, caps_height, 0); 
	
	GRRLIB_texImg *tex_caps_colored_png = GRRLIB_LoadTexture(caps_colored);
    GRRLIB_InitTileSet(tex_caps_colored_png, caps_width, caps_height, 0); 
	
	GRRLIB_texImg *tex_space_png = GRRLIB_LoadTexture(space);
    GRRLIB_InitTileSet(tex_space_png, space_width, space_height, 0); 
	
	
	GRRLIB_texImg *tex_dash_png = GRRLIB_LoadTexture(dash);
    GRRLIB_InitTileSet(tex_dash_png, 48, 40, 0); 
	
	GRRLIB_texImg *tex_underscore_png = GRRLIB_LoadTexture(underscore); //added v1.4
    GRRLIB_InitTileSet(tex_underscore_png, 44, 36, 0); 
	
	/*GRRLIB_texImg *tex_yen_png = GRRLIB_LoadTexture(yen); removed because png font doesn't support it.
    GRRLIB_InitTileSet(tex_yen_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_euro_png = GRRLIB_LoadTexture(euro);
    GRRLIB_InitTileSet(tex_euro_png, 44, 36, 0); */
	
	GRRLIB_texImg *tex_bracket_1_png = GRRLIB_LoadTexture(bracket_1);
    GRRLIB_InitTileSet(tex_bracket_1_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_bracket_2_png = GRRLIB_LoadTexture(bracket_2);
    GRRLIB_InitTileSet(tex_bracket_2_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_backslash_png = GRRLIB_LoadTexture(backslash);
    GRRLIB_InitTileSet(tex_backslash_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_slash_png = GRRLIB_LoadTexture(slash);
    GRRLIB_InitTileSet(tex_slash_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_apostrophe_png = GRRLIB_LoadTexture(apostrophe);
    GRRLIB_InitTileSet(tex_apostrophe_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_hash_png = GRRLIB_LoadTexture(hash);
    GRRLIB_InitTileSet(tex_hash_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_comma_png = GRRLIB_LoadTexture(comma);
    GRRLIB_InitTileSet(tex_comma_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_point_png = GRRLIB_LoadTexture(point);
    GRRLIB_InitTileSet(tex_point_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_equal_png = GRRLIB_LoadTexture(equal);
    GRRLIB_InitTileSet(tex_equal_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_semicolon_png = GRRLIB_LoadTexture(semicolon);
    GRRLIB_InitTileSet(tex_semicolon_png, 44, 36, 0); 
	
	
	
	GRRLIB_texImg *tex_blank_kbd_png = GRRLIB_LoadTexture(blank_kbd);
    GRRLIB_InitTileSet(tex_blank_kbd_png, 640, 480, 0); 	
	GRRLIB_texImg *tex_selected_png = GRRLIB_LoadTexture(selected);
    GRRLIB_InitTileSet(tex_selected_png, 392, 68, 0); 

	ISFS_Initialize();
	
	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32); 

	sprintf(filepath, ISFS_CONFIGDAT_PATH); 
	certSize=0;
	certBuffer = ISFS_GetFile((u8*)&filepath, &certSize, -1);
	if(certSize == 0 || certBuffer == NULL) exit(0); //error when reading ISFS ---> we exit immediately

while(1){//all the program runs in this loop

if(chosenmenu==0) {

	
	SSID_uppercase=false; //resets the keyboard's CAPS value.
	Password_uppercase=false;

			
			for (int i=0;i<=31;i++){  //this loops basically takes each letter and makes a string w/ it.
				SSID1[i] = certBuffer[SSID1_pos + i];
			}
			SSID1[32] = '\0'; //terminating character "\0" required or it shows weird crap at the end after the SSID
			
			for (int i=0;i<=62;i++){ //this loops basically takes each letter and makes a string w/ it. Doesn't change anything to certBuffer.
				PASSWORD1[i] = certBuffer[PASSWORD1_pos + i];
			}
			PASSWORD1[63] = '\0'; 

			//we read and from there we determine which connection is selected. 
			//ª= selected and &=unselected 
			/*char connection1_state = certBuffer[connection1_selectpos];
			char connection2_state = certBuffer[connection2_selectpos];
			char connection3_state = certBuffer[connection3_selectpos];
			*/
			//gets security type:
			connection1_securitytype = certBuffer[connection1_securitypos];
			
			
			//int NewSelectedNetwork = 1; //goes from 1 to 3. for now it's set to one but it will be eventually be possible to select your own network
			//writes the modified selected network in certBuffer
			//if (NewSelectedNetwork==1){
			
			if (certBuffer[8]==0x27)certBuffer[8]=0xA7; //==if network one is wired but not selected, make it wired AND selected
			if (certBuffer[8]==0x26)certBuffer[8]=0xA6; //if network one is wireless but not selected, make it wireless AND selected
				
				if (certBuffer[4]==0x02 && certBuffer[8] == 0xA7){ //this part makes sure only the first network is selected and it keeps the right network type(wired/wifi)
					certBuffer[connection1_selectpos] = connection_wired_selectedchar; 
					certBuffer[connection2_selectpos] = connection_wired_unselectedchar;
					certBuffer[connection3_selectpos] = connection_wired_unselectedchar;
					
				} else if (certBuffer[4]==0x01 && certBuffer[8] == 0xA6){
					certBuffer[connection1_selectpos] = connection_wireless_selectedchar; 
					certBuffer[connection2_selectpos] = connection_wireless_unselectedchar;
					certBuffer[connection3_selectpos] = connection_wireless_unselectedchar;
				}else { //if we end up here it means neither wifi or ethernet is selected, which means the file is somehow corrupted or is completly blank. We automatically switch to WiFi as it's the most popular choice.
					certBuffer[connection1_selectpos] = connection_wireless_selectedchar; 
					certBuffer[connection2_selectpos] = connection_wireless_unselectedchar;
					certBuffer[connection3_selectpos] = connection_wireless_unselectedchar;
					certBuffer[4]=0x01;
				}
				ISFS_WRITE_CONFIGDAT(certBuffer);

				
				
			/*}else if (NewSelectedNetwork==2){
				certBuffer[connection1_selectpos] = connection_wireless_unselectedchar;
				certBuffer[connection2_selectpos] = connection_wireless_selectedchar;
				certBuffer[connection3_selectpos] = connection_wireless_unselectedchar;
			}else if (NewSelectedNetwork==3){
				certBuffer[connection1_selectpos] = connection_wireless_unselectedchar;
				certBuffer[connection2_selectpos] = connection_wireless_unselectedchar;
				certBuffer[connection3_selectpos] = connection_wireless_selectedchar;
			}
			this part is disabled for now, will be added back when the user will be able to choose the 3 networks instead of only one.
			*/
	
	//network testing part: this part detects if the file located at ISFS_TEMPFILE_PATH exists and if it does we delete it and we jump to the network connection.
	//clever way to do two things at once : instead of first checking if the file exists then delete it, we try to delete it and if we get error -106 it means the file doesnt exists. 
	//but if we get a result which is >=0 then it means it's succesfull so we can jump to the network testing part.
	deletefile = ISFS_Delete(ISFS_TEMPFILE_PATH);
	//if (deletefile == -106) {printf("File not found\n");}
	if (deletefile>=0){
		chosenmenu=6;
	}//else{printf("delete error %d\n", deletefile);}
	

	////////////////////////////////////////main menu://///////////////////////

	while (chosenmenu==0){ //main menu
	
				

		        updatecursorpos();
				GRRLIB_DrawImg(0, 0, tex_settings_background_png, 0, 1, 1, GRRLIB_WHITE);  
				
				GRRLIB_DrawImg(WiredCoordX, WiredCoordY, tex_wired_png, 0, 1, 1, GRRLIB_WHITE); //button 1
				GRRLIB_DrawImg(WirelessCoordX, WirelessCoordY, tex_wireless_png, 0, 1, 1, GRRLIB_WHITE); //button 1
				
				//if wired selected :
				if (certBuffer[4]==0x02 && certBuffer[8] == 0xA7){
						GRRLIB_DrawImg(WiredCoordX-50, WiredCoordY-5, tex_selected_png, 0, 1.42, 1.1, GRRLIB_WHITE); //draws WIRED button
						networktype=0;
				}
				
				//if wireless selected : 
				if (certBuffer[4]==0x01 && certBuffer[8] == 0xA6){
						GRRLIB_DrawImg(WirelessCoordX-50, WirelessCoordY-5, tex_selected_png, 0, 1.42, 1.1, GRRLIB_WHITE); //WIRELESS button
						networktype=1;
						certBuffer[connection1_selectpos] = connection_wireless_selectedchar; 
				}

				if(networktype==1){
					GRRLIB_DrawImg(mainmenubt1CoordX, mainmenubt1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE); //button 1
					GRRLIB_DrawImg(mainmenubt2CoordX, mainmenubt2CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE); //button 2
					GRRLIB_DrawImg(mainmenubt3CoordX, mainmenubt3CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE); //button 3

					GRRLIB_Printf((mainmenubt1CoordX+buttonWidth/2)-35, (mainmenubt1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1 , "Edit SSID");
					GRRLIB_Printf((mainmenubt2CoordX+buttonWidth/2)-50, (mainmenubt2CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1 , "Edit Password");
					GRRLIB_Printf((mainmenubt3CoordX+buttonWidth/2)-70, (mainmenubt3CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1 , "Edit Security type");
				
				
					GRRLIB_Printf(mainmenubt1CoordX, 280, tex_BMfont5, GRRLIB_WHITE, 1, "SSID : ");
				
					//this part shortens the SSID if it's too long and adds "..." so that SSID and password don't overlap on the screen:
					int shorten_length = 20;
				
					strcpy(shortenedssid, SSID1);
				
					if((int)strlen(shortenedssid) >= shorten_length) { //using (int) to convert from size_t to int type (so that there's no warnings)
						shortenedssid[shorten_length] = '.';
						shortenedssid[shorten_length+1] = '.';
						shortenedssid[shorten_length+2] = '.';
						shortenedssid[shorten_length+3] = '\0';
					}
				
				
					strcpy(shortenedpassword, PASSWORD1);
				
					if((int)strlen(shortenedpassword) >= shorten_length) {
						shortenedpassword[shorten_length] = '.';
						shortenedpassword[shorten_length+1] = '.';
						shortenedpassword[shorten_length+2] = '.';
						shortenedpassword[shorten_length+3] = '\0';
					}
			
					GRRLIB_Printf(mainmenubt1CoordX, 300, tex_BMfont5, GRRLIB_WHITE, 1, "%s", shortenedssid);
					GRRLIB_Printf(mainmenubt2CoordX, 280, tex_BMfont5, GRRLIB_WHITE, 1, "Password : ");
					GRRLIB_Printf(mainmenubt2CoordX, 300, tex_BMfont5, GRRLIB_WHITE, 1, "%s", shortenedpassword);
				
					GRRLIB_Printf(mainmenubt3CoordX, 280, tex_BMfont5, GRRLIB_WHITE, 1, "Security type :");
					if (connection1_securitytype == 0x00){
						GRRLIB_Printf(mainmenubt3CoordX, 300, tex_BMfont5, GRRLIB_WHITE, 1, "Open");
					}else if (connection1_securitytype == 0x01){
						GRRLIB_Printf(mainmenubt3CoordX, 300, tex_BMfont5, GRRLIB_WHITE, 1, "WEP64");
					} else if (connection1_securitytype == 0x02){
						GRRLIB_Printf(mainmenubt3CoordX, 300, tex_BMfont5, GRRLIB_WHITE, 1, "WEP128");
					} else if (connection1_securitytype == 0x04){
						GRRLIB_Printf(mainmenubt3CoordX, 300, tex_BMfont5, GRRLIB_WHITE, 1, "WPA-PSK(TKIP)");
					} else if (connection1_securitytype == 0x05){
						GRRLIB_Printf(mainmenubt3CoordX, 300, tex_BMfont5, GRRLIB_WHITE, 1, "WPA2-PSK(AES)");
					} else if (connection1_securitytype == 0x06){
						GRRLIB_Printf(mainmenubt3CoordX, 300, tex_BMfont5, GRRLIB_WHITE, 1, "WPA-PSK(AES)");
					}
			
				}
				
				
				GRRLIB_DrawImg(creditbtnCoordX, creditbtnCoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE); //credit button
				GRRLIB_Printf((creditbtnCoordX+buttonWidth/2)-25, (creditbtnCoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1 , "Credits");
				
				GRRLIB_DrawImg(exitCoordX, exitCoordY, tex_poweroff_png, 0, 1, 1, GRRLIB_WHITE); //exit button (goes back to loader)
				GRRLIB_Printf(exitCoordX-15, exitCoordY+45, tex_BMfont5, GRRLIB_WHITE, 0.8, "Save and Exit"); //exit button

				GRRLIB_Printf(200, 45, tex_BMfont5, GRRLIB_BLACK,1 ,"Wii Network Config Editor by Bazmoc"); //title
				
				
				GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);  // Draw a jpeg

				if(wpaddown & WPAD_BUTTON_A || paddown & PAD_BUTTON_A ) { //if you clicked A
					
					if (collisionButton(WiredCoordX, WiredCoordY, cursor_positionX,cursor_positionY,connectiontype_Width,connectiontype_Height) == true){
						certBuffer[connection1_selectpos] = connection_wired_selectedchar; 
						certBuffer[connection2_selectpos] = connection_wired_unselectedchar;
						certBuffer[connection3_selectpos] = connection_wired_unselectedchar;
						certBuffer[4] = 0x02;
						ISFS_WRITE_CONFIGDAT(certBuffer);
						selectsound();
					}
					
					if (collisionButton(WirelessCoordX, WirelessCoordY, cursor_positionX,cursor_positionY,connectiontype_Width,connectiontype_Height) == true){
						certBuffer[connection1_selectpos] = connection_wireless_selectedchar; 
						certBuffer[connection2_selectpos] = connection_wireless_unselectedchar;
						certBuffer[connection3_selectpos] = connection_wireless_unselectedchar;
						certBuffer[4] = 0x01;
						ISFS_WRITE_CONFIGDAT(certBuffer);
						selectsound();
					}
				
				
				
				
					if (collisionButton(mainmenubt1CoordX, mainmenubt1CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						chosenmenu=1;
					}
					if (collisionButton(mainmenubt2CoordX, mainmenubt2CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						chosenmenu=2;
					}
					if (collisionButton(mainmenubt3CoordX, mainmenubt3CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						connection1_newsecuritytype = connection1_securitytype;
						chosenmenu=3;
					}
					if (collisionButton(exitCoordX, exitCoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){ //exit button
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						isNetworkWorking = 0;
						chosenmenu=4;
					}
					if (collisionButton(creditbtnCoordX, creditbtnCoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){ //exit button
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						chosenmenu=5;
					}
					
				}
				
				if(wpaddown & WPAD_BUTTON_HOME || paddown & PAD_BUTTON_START ) chosenmenu = 4;//if you clicked HOME or START : save changes.

        GRRLIB_Render();
	}
	
}
	
	///////////////////////////////////Keyboard for SSID://////////////////////////////////////////////////////////////////////////////////////////////////////////
	
if(chosenmenu==1){
	
    while(chosenmenu==1) { //keyboard for SSID. max 32 characters
	int collision_sizex = 5;
	int collision_sizey = 5;
        updatecursorpos();
	
				GRRLIB_DrawImg(0, 0, tex_blank_kbd_png, 0, 1, 1, GRRLIB_WHITE);  //shows the keyboard on screen

					//draw numbers:
				//GRRLIB_DrawImg(one_coordX-10, one_coordY-10, tex_one_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				GRRLIB_DrawImg(one_coordX, one_coordY, tex_one_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(two_coordX, two_coordY, tex_two_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(three_coordX, three_coordY, tex_three_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(four_coordX, four_coordY, tex_four_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(five_coordX, five_coordY, tex_five_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(six_coordX, six_coordY, tex_six_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(seven_coordX, seven_coordY, tex_seven_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(eight_coordX, eight_coordY, tex_eight_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(nine_coordX, nine_coordY, tex_nine_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(zero_coordX, zero_coordY, tex_zero_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(dash_coordX, dash_coordY, tex_dash_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(underscore_coordX, underscore_coordY, tex_underscore_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				
				//draw letters:
				if (SSID_uppercase==false){ //lowercase letters:
					
					GRRLIB_DrawImg(bta_coordX, bta_coordY, tex_bta_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btb_coordX, btb_coordY, tex_btb_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btc_coordX, btc_coordY, tex_btc_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btd_coordX, btd_coordY, tex_btd_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bte_coordX, bte_coordY, tex_bte_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btf_coordX, btf_coordY, tex_btf_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btg_coordX, btg_coordY, tex_btg_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bth_coordX, bth_coordY, tex_bth_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bti_coordX, bti_coordY, tex_bti_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btj_coordX, btj_coordY, tex_btj_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btk_coordX, btk_coordY, tex_btk_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btl_coordX, btl_coordY, tex_btl_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btm_coordX, btm_coordY, tex_btm_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btn_coordX, btn_coordY, tex_btn_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bto_coordX, bto_coordY, tex_bto_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btp_coordX, btp_coordY, tex_btp_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btq_coordX, btq_coordY, tex_btq_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btr_coordX, btr_coordY, tex_btr_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bts_coordX, bts_coordY, tex_bts_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btt_coordX, btt_coordY, tex_btt_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btu_coordX, btu_coordY, tex_btu_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btv_coordX, btv_coordY, tex_btv_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btw_coordX, btw_coordY, tex_btw_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btx_coordX, btx_coordY, tex_btx_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bty_coordX, bty_coordY, tex_bty_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btz_coordX, btz_coordY, tex_btz_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				}else{//uppercase letters:
					GRRLIB_DrawImg(bta_coordX, bta_coordY, tex_bta_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btb_coordX, btb_coordY, tex_btb_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btc_coordX, btc_coordY, tex_btc_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btd_coordX, btd_coordY, tex_btd_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bte_coordX, bte_coordY, tex_bte_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btf_coordX, btf_coordY, tex_btf_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btg_coordX, btg_coordY, tex_btg_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bth_coordX, bth_coordY, tex_bth_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bti_coordX, bti_coordY, tex_bti_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btj_coordX, btj_coordY, tex_btj_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btk_coordX, btk_coordY, tex_btk_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btl_coordX, btl_coordY, tex_btl_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btm_coordX, btm_coordY, tex_btm_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btn_coordX, btn_coordY, tex_btn_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bto_coordX, bto_coordY, tex_bto_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btp_coordX, btp_coordY, tex_btp_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btq_coordX, btq_coordY, tex_btq_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btr_coordX, btr_coordY, tex_btr_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bts_coordX, bts_coordY, tex_bts_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btt_coordX, btt_coordY, tex_btt_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btu_coordX, btu_coordY, tex_btu_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btv_coordX, btv_coordY, tex_btv_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btw_coordX, btw_coordY, tex_btw_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btx_coordX, btx_coordY, tex_btx_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bty_coordX, bty_coordY, tex_bty_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btz_coordX, btz_coordY, tex_btz_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				}



				//other characters and stuff:
				if (SSID_uppercase==true){
					GRRLIB_DrawImg(caps_coordX, caps_coordY, tex_caps_colored_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE); //show the yellow-colored CAPS button when uppercase==true
				}else{
					GRRLIB_DrawImg(caps_coordX, caps_coordY, tex_caps_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE); //else:show the normal one
				}

				GRRLIB_DrawImg(erase_coordX, erase_coordY, tex_erase_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(space_coordX, space_coordY, tex_space_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				//GRRLIB_DrawImg(yen_coordX, yen_coordY, tex_yen_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				//GRRLIB_DrawImg(euro_coordX, euro_coordY, tex_euro_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(bracket_1_coordX, bracket_1_coordY, tex_bracket_1_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(bracket_2_coordX, bracket_2_coordY, tex_bracket_2_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(backslash_coordX, backslash_coordY, tex_backslash_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(slash_coordX, slash_coordY, tex_slash_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(apostrophe_coordX, apostrophe_coordY, tex_apostrophe_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(hash_coordX, hash_coordY, tex_hash_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(comma_coordX, comma_coordY, tex_comma_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(point_coordX, point_coordY, tex_point_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(equal_coordX, equal_coordY, tex_equal_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(semicolon_coordX, semicolon_coordY, tex_semicolon_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);

                GRRLIB_Printf(35, 50, tex_BMfont5, GRRLIB_BLACK, SSIDkbd_zoom, SSIDkeyboard_writtentext); //write on the screen the text that is being written
				
				
				
				GRRLIB_Printf(35, 100, tex_BMfont5, CurrentColor(SSID_indexkeyboard,32), 1, "%d/32", SSID_indexkeyboard); //turns red when we are at 32/32


				GRRLIB_DrawImg(button1CoordX, button1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button1CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Cancel");
				
				GRRLIB_DrawImg(button3CoordX, button3CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button3CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Save");
				GRRLIB_Printf((button3CoordX+buttonWidth/2)-35, (button1CoordY+buttonHeight/2)+25, tex_BMfont5, GRRLIB_BLACK, 1, "HOME/Start");
				

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//this part determines which character was clicked:
				
	
	
	SSIDkeyboard_writtentext[32] = '\0'; 

	if (wpaddown & WPAD_BUTTON_HOME || paddown & PAD_BUTTON_START) goto savessid;
	
	if(wpaddown & WPAD_BUTTON_B || paddown & PAD_BUTTON_B) kbd_backspace(0); //erases last character when clicking B button on wiimote or gc controller
		
	if(wpaddown & WPAD_BUTTON_A || paddown & PAD_BUTTON_A) {

		if(GRRLIB_RectOnRect(bta_coordX,bta_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { //if we aren't at the maximum length ---> if SSID_uppercase==true we write 'A', else we write 'a'
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'A';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'a';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btb_coordX,btb_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'B';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'b';}
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btc_coordX,btc_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'C';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'c';}
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btd_coordX,btd_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'D';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'd';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bte_coordX,bte_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'E';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'e';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btf_coordX,btf_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'F';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'f';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btg_coordX,btg_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'G';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'g';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bth_coordX,bth_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'H';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'h';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bti_coordX,bti_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'I';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'i';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btj_coordX,btj_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'J';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'j';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btk_coordX,btk_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'K';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'k';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btl_coordX,btl_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'L';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'l';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btm_coordX,btm_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'M';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'm';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btn_coordX,btn_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'N';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'n';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bto_coordX,bto_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'O';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'o';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btp_coordX,btp_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'P';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'p';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btq_coordX,btq_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'Q';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'q';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btr_coordX,btr_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'R';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'r';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bts_coordX,bts_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'S';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 's';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btt_coordX,btt_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'T';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 't';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btu_coordX,btu_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'U';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'u';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btv_coordX,btv_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'V';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'v';}
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btw_coordX,btw_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'W';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'w';}
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btx_coordX,btx_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'X';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'x';}
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bty_coordX,bty_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'Y';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'y';}
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btz_coordX,btz_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				if (SSID_uppercase==true){SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'Z';} else {SSIDkeyboard_writtentext[SSID_indexkeyboard] = 'z';}
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(zero_coordX,zero_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '0';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(one_coordX,one_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '1';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(two_coordX,two_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '2';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(three_coordX,three_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '3';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(four_coordX,four_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '4';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(five_coordX,five_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '5';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(six_coordX,six_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '6';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(seven_coordX,seven_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '7';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(eight_coordX,eight_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '8';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(nine_coordX,nine_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '9';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(dash_coordX,dash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '-';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(underscore_coordX,underscore_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '_';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bracket_1_coordX,bracket_1_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '[';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bracket_2_coordX,bracket_2_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = ']';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(backslash_coordX,backslash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '\\';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(slash_coordX,slash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '/';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(apostrophe_coordX,apostrophe_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '\'';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(hash_coordX,hash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '#';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(comma_coordX,comma_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = ',';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(point_coordX,point_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '.';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(equal_coordX,equal_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '=';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(semicolon_coordX,semicolon_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = ';';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(caps_coordX,caps_coordY,caps_width,caps_height,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
				if (SSID_uppercase == true) {SSID_uppercase=false;} else {SSID_uppercase=true;}
				capssound();
				wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(erase_coordX,erase_coordY,erase_width,erase_height,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
				/*if (SSID_indexkeyboard>0 ) { //you don't want that number to get negative lol 		
					SSIDkeyboard_writtentext[SSID_indexkeyboard-1] = '\0'; //erases the character
					SSID_indexkeyboard-=1;
					erasesound();
				} else maxsound()
			wiirumble_set(0,vib_duration2);	*/
			kbd_backspace(0);			
		}else if(GRRLIB_RectOnRect(space_coordX,space_coordY,space_width,space_height,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
				if (SSID_indexkeyboard<=31) { 
					SSIDkeyboard_writtentext[SSID_indexkeyboard] = ' '; //space
					SSID_indexkeyboard+=1;
					presssound();
				} else maxsound();
				wiirumble_set(0,vib_duration2);
		}else if (collisionButton(button1CoordX, button1CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){				
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						clearkeyboard(1); 
						chosenmenu=0;//goes back to the menu without saving
		}else if (collisionButton(button3CoordX, button3CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true ){ //save button				
						savessid:
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						//saving new ssid:
						for (int i=0;i<=32;i++){  //replaces the ssid in certBuffer
							certBuffer[SSID1_pos + i] = SSIDkeyboard_writtentext[i];//NewSSID[i];
						}
						certBuffer[SSID1_pos+33] =  lengthtoHex(SSID_indexkeyboard);// this must be the length of the new SSID. example : 0x03 = three characters long | 0x20 = 32 characters.
						ISFS_WRITE_CONFIGDAT(certBuffer);
						
						clearkeyboard(1);  
						chosenmenu=0;//goes back to the menu after saving everything
						
						}
			
	} 	

				if (CusorOnKey(bta_coordX,bta_coordY,44,36)==true){
					if (SSID_uppercase==false){
					   GRRLIB_DrawImg(bta_coordX-10, bta_coordY-10, tex_bta_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(bta_coordX-10, bta_coordY-10, tex_bta_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(btb_coordX,btb_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btb_coordX-10, btb_coordY-10, tex_btb_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					   }else  GRRLIB_DrawImg(btb_coordX-10, btb_coordY-10, tex_btb_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btc_coordX,btc_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btc_coordX-10, btc_coordY-10, tex_btc_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
						}else GRRLIB_DrawImg(btc_coordX-10, btc_coordY-10, tex_btc_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
						
				}
				else if (CusorOnKey(btd_coordX,btd_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btd_coordX-10, btd_coordY-10, tex_btd_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else 					   GRRLIB_DrawImg(btd_coordX-10, btd_coordY-10, tex_btd_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(bte_coordX,bte_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(bte_coordX-10, bte_coordY-10, tex_bte_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else 					   GRRLIB_DrawImg(bte_coordX-10, bte_coordY-10, tex_bte_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btf_coordX,btf_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btf_coordX-10, btf_coordY-10, tex_btf_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else 					   GRRLIB_DrawImg(btf_coordX-10, btf_coordY-10, tex_btf_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btg_coordX,btg_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btg_coordX-10, btg_coordY-10, tex_btg_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else 					   GRRLIB_DrawImg(btg_coordX-10, btg_coordY-10, tex_btg_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(bth_coordX,bth_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(bth_coordX-10, bth_coordY-10, tex_bth_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else 					   GRRLIB_DrawImg(bth_coordX-10, bth_coordY-10, tex_bth_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(bti_coordX,bti_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(bti_coordX-10, bti_coordY-10, tex_bti_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(bti_coordX-10, bti_coordY-10, tex_bti_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btj_coordX,btj_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btj_coordX-10, btj_coordY-10, tex_btj_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btj_coordX-10, btj_coordY-10, tex_btj_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btk_coordX,btk_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btk_coordX-10, btk_coordY-10, tex_btk_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btk_coordX-10, btk_coordY-10, tex_btk_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btl_coordX,btl_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btl_coordX-10, btl_coordY-10, tex_btl_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btl_coordX-10, btl_coordY-10, tex_btl_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btm_coordX,btm_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btm_coordX-10, btm_coordY-10, tex_btm_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btm_coordX-10, btm_coordY-10, tex_btm_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btn_coordX,btn_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btn_coordX-10, btn_coordY-10, tex_btn_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btn_coordX-10, btn_coordY-10, tex_btn_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(bto_coordX,bto_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(bto_coordX-10, bto_coordY-10, tex_bto_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(bto_coordX-10, bto_coordY-10, tex_bto_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btp_coordX,btp_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btp_coordX-10, btp_coordY-10, tex_btp_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btp_coordX-10, btp_coordY-10, tex_btp_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btq_coordX,btq_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btq_coordX-10, btq_coordY-10, tex_btq_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btq_coordX-10, btq_coordY-10, tex_btq_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btr_coordX,btr_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btr_coordX-10, btr_coordY-10, tex_btr_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btr_coordX-10, btr_coordY-10, tex_btr_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(bts_coordX,bts_coordY,44,36)==true){
				if (SSID_uppercase==false){
					   GRRLIB_DrawImg(bts_coordX-10, bts_coordY-10, tex_bts_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(bts_coordX-10, bts_coordY-10, tex_bts_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btt_coordX,btt_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btt_coordX-10, btt_coordY-10, tex_btt_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btt_coordX-10, btt_coordY-10, tex_btt_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btu_coordX,btu_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btu_coordX-10, btu_coordY-10, tex_btu_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btu_coordX-10, btu_coordY-10, tex_btu_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btv_coordX,btv_coordY,44,36)==true){
					if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btv_coordX-10, btv_coordY-10, tex_btv_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(btv_coordX-10, btv_coordY-10, tex_btv_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				
				else if (CusorOnKey(btw_coordX,btw_coordY,44,36)==true){
					  if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btw_coordX-10, btw_coordY-10, tex_btw_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(btw_coordX-10, btw_coordY-10, tex_btw_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				
				else if (CusorOnKey(btx_coordX,btx_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btx_coordX-10, btx_coordY-10, tex_btx_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btx_coordX-10, btx_coordY-10, tex_btx_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(bty_coordX,bty_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(bty_coordX-10, bty_coordY-10, tex_bty_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(bty_coordX-10, bty_coordY-10, tex_bty_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btz_coordX,btz_coordY,44,36)==true){
					   if (SSID_uppercase==false){
					   GRRLIB_DrawImg(btz_coordX-10, btz_coordY-10, tex_btz_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btz_coordX-10, btz_coordY-10, tex_btz_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(zero_coordX,zero_coordY,44,36)==true){
					   GRRLIB_DrawImg(zero_coordX-10, zero_coordY-10, tex_zero_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(one_coordX,one_coordY,44,36)==true){
					   GRRLIB_DrawImg(one_coordX-10, one_coordY-10, tex_one_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(two_coordX,two_coordY,44,36)==true){
					   GRRLIB_DrawImg(two_coordX-10, two_coordY-10, tex_two_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(three_coordX,three_coordY,44,36)==true){
					   GRRLIB_DrawImg(three_coordX-10, three_coordY-10, tex_three_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(four_coordX,four_coordY,44,36)==true){
					   GRRLIB_DrawImg(four_coordX-10, four_coordY-10, tex_four_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(five_coordX,five_coordY,44,36)==true){
					   GRRLIB_DrawImg(five_coordX-10, five_coordY-10, tex_five_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(six_coordX,six_coordY,44,36)==true){
					   GRRLIB_DrawImg(six_coordX-10, six_coordY-10, tex_six_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(seven_coordX,seven_coordY,44,36)==true){
					   GRRLIB_DrawImg(seven_coordX-10, seven_coordY-10, tex_seven_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(eight_coordX,eight_coordY,44,36)==true){
					   GRRLIB_DrawImg(eight_coordX-10, eight_coordY-10, tex_eight_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(nine_coordX,nine_coordY,44,36)==true){
					   GRRLIB_DrawImg(nine_coordX-10, nine_coordY-10, tex_nine_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(bracket_1_coordX,bracket_1_coordY,44,36)==true){
					   GRRLIB_DrawImg(bracket_1_coordX-10, bracket_1_coordY-10, tex_bracket_1_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(bracket_2_coordX,bracket_2_coordY,44,36)==true){
					   GRRLIB_DrawImg(bracket_2_coordX-10, bracket_2_coordY-10, tex_bracket_2_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(backslash_coordX,backslash_coordY,44,36)==true){
					   GRRLIB_DrawImg(backslash_coordX-10, backslash_coordY-10, tex_backslash_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(slash_coordX,slash_coordY,44,36)==true){
					   GRRLIB_DrawImg(slash_coordX-10, slash_coordY-10, tex_slash_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(apostrophe_coordX,apostrophe_coordY,44,36)==true){
					   GRRLIB_DrawImg(apostrophe_coordX-10, apostrophe_coordY-10, tex_apostrophe_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(hash_coordX,hash_coordY,44,36)==true){
					   GRRLIB_DrawImg(hash_coordX-10, hash_coordY-10, tex_hash_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(comma_coordX,comma_coordY,44,36)==true){
					   GRRLIB_DrawImg(comma_coordX-10, comma_coordY-10, tex_comma_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(point_coordX,point_coordY,44,36)==true){
					   GRRLIB_DrawImg(point_coordX-10, point_coordY-10, tex_point_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(equal_coordX,equal_coordY,44,36)==true){
					   GRRLIB_DrawImg(equal_coordX-10, equal_coordY-10, tex_equal_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(equal_coordX,equal_coordY,44,36)==true){
					   GRRLIB_DrawImg(equal_coordX-10, equal_coordY-10, tex_equal_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(semicolon_coordX,semicolon_coordY,44,36)==true){
					   GRRLIB_DrawImg(semicolon_coordX-10, semicolon_coordY-10, tex_semicolon_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(dash_coordX,dash_coordY,44,36)==true){
					   GRRLIB_DrawImg(dash_coordX-10, dash_coordY-10, tex_dash_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(underscore_coordX,underscore_coordY,44,36)==true){
					   GRRLIB_DrawImg(underscore_coordX-10, underscore_coordY-10, tex_underscore_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
	
					



				GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);
		
			
        GRRLIB_Render();
}
	
}
	
	
	//////////////////////////////////////Keyboard for password:///////////////////////////////////////////////////////////////////

if(chosenmenu==2){
while(chosenmenu==2){//keyboard for password. max 63 characters
		

		updatecursorpos();
	
				GRRLIB_DrawImg(0, 0, tex_blank_kbd_png, 0, 1, 1, GRRLIB_WHITE);  //shows the keyboard on screen
				
				GRRLIB_DrawImg(button1CoordX, button1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button1CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Cancel");
				
				GRRLIB_DrawImg(button3CoordX, button3CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button3CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Save");
				GRRLIB_Printf((button3CoordX+buttonWidth/2)-35, (button1CoordY+buttonHeight/2)+25, tex_BMfont5, GRRLIB_BLACK, 1, "HOME/Start");
				
				
					//draw numbers:
				GRRLIB_DrawImg(one_coordX, one_coordY, tex_one_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(two_coordX, two_coordY, tex_two_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(three_coordX, three_coordY, tex_three_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(four_coordX, four_coordY, tex_four_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(five_coordX, five_coordY, tex_five_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(six_coordX, six_coordY, tex_six_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(seven_coordX, seven_coordY, tex_seven_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(eight_coordX, eight_coordY, tex_eight_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(nine_coordX, nine_coordY, tex_nine_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(zero_coordX, zero_coordY, tex_zero_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(dash_coordX, dash_coordY, tex_dash_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(underscore_coordX, underscore_coordY, tex_underscore_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				
				//draw letters:
             	if (Password_uppercase==false){
					GRRLIB_DrawImg(bta_coordX, bta_coordY, tex_bta_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btb_coordX, btb_coordY, tex_btb_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btc_coordX, btc_coordY, tex_btc_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btd_coordX, btd_coordY, tex_btd_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bte_coordX, bte_coordY, tex_bte_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btf_coordX, btf_coordY, tex_btf_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btg_coordX, btg_coordY, tex_btg_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bth_coordX, bth_coordY, tex_bth_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bti_coordX, bti_coordY, tex_bti_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btj_coordX, btj_coordY, tex_btj_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btk_coordX, btk_coordY, tex_btk_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btl_coordX, btl_coordY, tex_btl_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btm_coordX, btm_coordY, tex_btm_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btn_coordX, btn_coordY, tex_btn_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bto_coordX, bto_coordY, tex_bto_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btp_coordX, btp_coordY, tex_btp_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btq_coordX, btq_coordY, tex_btq_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btr_coordX, btr_coordY, tex_btr_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bts_coordX, bts_coordY, tex_bts_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btt_coordX, btt_coordY, tex_btt_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btu_coordX, btu_coordY, tex_btu_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btv_coordX, btv_coordY, tex_btv_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btw_coordX, btw_coordY, tex_btw_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btx_coordX, btx_coordY, tex_btx_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bty_coordX, bty_coordY, tex_bty_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btz_coordX, btz_coordY, tex_btz_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				}else{
					GRRLIB_DrawImg(bta_coordX, bta_coordY, tex_bta_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btb_coordX, btb_coordY, tex_btb_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btc_coordX, btc_coordY, tex_btc_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btd_coordX, btd_coordY, tex_btd_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bte_coordX, bte_coordY, tex_bte_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btf_coordX, btf_coordY, tex_btf_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btg_coordX, btg_coordY, tex_btg_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bth_coordX, bth_coordY, tex_bth_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bti_coordX, bti_coordY, tex_bti_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btj_coordX, btj_coordY, tex_btj_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btk_coordX, btk_coordY, tex_btk_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btl_coordX, btl_coordY, tex_btl_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btm_coordX, btm_coordY, tex_btm_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btn_coordX, btn_coordY, tex_btn_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bto_coordX, bto_coordY, tex_bto_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btp_coordX, btp_coordY, tex_btp_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btq_coordX, btq_coordY, tex_btq_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btr_coordX, btr_coordY, tex_btr_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bts_coordX, bts_coordY, tex_bts_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btt_coordX, btt_coordY, tex_btt_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btu_coordX, btu_coordY, tex_btu_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btv_coordX, btv_coordY, tex_btv_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btw_coordX, btw_coordY, tex_btw_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btx_coordX, btx_coordY, tex_btx_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(bty_coordX, bty_coordY, tex_bty_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
					GRRLIB_DrawImg(btz_coordX, btz_coordY, tex_btz_UP_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				}
				



				//other characters and stuff:
				if (Password_uppercase==true){
					GRRLIB_DrawImg(caps_coordX, caps_coordY, tex_caps_colored_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE); //show the yellow-colored CAPS button when uppercase==true
				}else{
					GRRLIB_DrawImg(caps_coordX, caps_coordY, tex_caps_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE); //else:show the normal one
				}

				GRRLIB_DrawImg(erase_coordX, erase_coordY, tex_erase_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(space_coordX, space_coordY, tex_space_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(bracket_1_coordX, bracket_1_coordY, tex_bracket_1_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(bracket_2_coordX, bracket_2_coordY, tex_bracket_2_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(backslash_coordX, backslash_coordY, tex_backslash_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(slash_coordX, slash_coordY, tex_slash_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(apostrophe_coordX, apostrophe_coordY, tex_apostrophe_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(hash_coordX, hash_coordY, tex_hash_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(comma_coordX, comma_coordY, tex_comma_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(point_coordX, point_coordY, tex_point_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(equal_coordX, equal_coordY, tex_equal_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				GRRLIB_DrawImg(semicolon_coordX, semicolon_coordY, tex_semicolon_png, 0, kbd_button_Xsize, kbd_button_Ysize, GRRLIB_WHITE);
				
				
				//this small part splits passwordkeyboard_wirttentext into two lines when it reached 45 caracters.
                GRRLIB_Printf(35, 50, tex_BMfont5, GRRLIB_BLACK, passwordkbd_zoom, password_firstline); //write on the screen the text that is being written
				GRRLIB_Printf(35, 75, tex_BMfont5, GRRLIB_BLACK, passwordkbd_zoom, password_secondline); //write on the screen the text that is being written
				
				GRRLIB_Printf(35, 100, tex_BMfont5, CurrentColor(Password_indexkeyboard,63), 1, "%d/63", Password_indexkeyboard); //turns red when we are at 63/63

				if (Password_indexkeyboard>45) {
					password_firstline[45] = '\0';
					password_secondline[Password_indexkeyboard-46] = Passwordkeyboard_writtentext[Password_indexkeyboard-1];
					password_secondline[1+Password_indexkeyboard-46] = '\0';
				} else {
					password_secondline[0] = '\0'; //basically resets it
					strncpy(password_firstline, Passwordkeyboard_writtentext, 45); 
					password_firstline[45] = '\0';
				}
				
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//this part determines which character was clicked:
				
	int collision_sizex = 5;
	int collision_sizey = 5;
	
	Passwordkeyboard_writtentext[63] = '\0'; //was set to 64

	
	if (wpaddown & WPAD_BUTTON_HOME || paddown & PAD_BUTTON_START) goto savepassword;
	if(wpaddown & WPAD_BUTTON_B || paddown & PAD_BUTTON_B) kbd_backspace(1); //erases last character when clicking B button on wiimote or gc controller

	if(wpaddown & WPAD_BUTTON_A || paddown & PAD_BUTTON_A) {			
		if(GRRLIB_RectOnRect(bta_coordX,bta_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { //if we aren't at the maximum length ---> if Password_uppercase==true we write 'A', else we write 'a'
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'A';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'a';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btb_coordX,btb_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'B';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'b';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btc_coordX,btc_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'C';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'c';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btd_coordX,btd_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'D';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'd';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bte_coordX,bte_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'E';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'e';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btf_coordX,btf_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'F';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'f';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btg_coordX,btg_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'G';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'g';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bth_coordX,bth_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'H';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'h';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bti_coordX,bti_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'I';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'i';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btj_coordX,btj_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'J';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'j';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btk_coordX,btk_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'K';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'k';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btl_coordX,btl_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'L';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'l';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btm_coordX,btm_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'M';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'm';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btn_coordX,btn_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'N';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'n';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bto_coordX,bto_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'O';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'o';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btp_coordX,btp_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'P';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'p';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btq_coordX,btq_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'Q';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'q';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btr_coordX,btr_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'R';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'r';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bts_coordX,bts_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'S';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 's';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btt_coordX,btt_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'T';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 't';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btu_coordX,btu_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'U';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'u';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btv_coordX,btv_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'V';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'v';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btw_coordX,btw_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'W';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'w';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btx_coordX,btx_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'X';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'x';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bty_coordX,bty_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'Y';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'y';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(btz_coordX,btz_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			if (Password_uppercase==true){Passwordkeyboard_writtentext[Password_indexkeyboard] = 'Z';} else {Passwordkeyboard_writtentext[Password_indexkeyboard] = 'z';}
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(zero_coordX,zero_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '0';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(one_coordX,one_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '1';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(two_coordX,two_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '2';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(three_coordX,three_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '3';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(four_coordX,four_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '4';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(five_coordX,five_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '5';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(six_coordX,six_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '6';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(seven_coordX,seven_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '7';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(eight_coordX,eight_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '8';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(nine_coordX,nine_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '9';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(dash_coordX,dash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '-';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(underscore_coordX,underscore_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
			Passwordkeyboard_writtentext[Password_indexkeyboard] = '_';
			Password_indexkeyboard+=1;
			presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bracket_1_coordX,bracket_1_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '[';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bracket_2_coordX,bracket_2_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = ']';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(backslash_coordX,backslash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '\\';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(slash_coordX,slash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '/';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(apostrophe_coordX,apostrophe_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '\'';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(hash_coordX,hash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '#';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(comma_coordX,comma_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = ',';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(point_coordX,point_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '.';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(equal_coordX,equal_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '=';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(semicolon_coordX,semicolon_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=62) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = ';';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(caps_coordX,caps_coordY,caps_width,caps_height,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
				if (Password_uppercase == true) {Password_uppercase=false;} else {Password_uppercase=true;}
				capssound();
				wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(erase_coordX,erase_coordY,erase_width,erase_height,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
				kbd_backspace(1);//1=password keyboard				
			} else if(GRRLIB_RectOnRect(space_coordX,space_coordY,space_width,space_height,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
				if (Password_indexkeyboard<=62) { 
					Passwordkeyboard_writtentext[Password_indexkeyboard] = ' '; //space
					Password_indexkeyboard+=1;
					presssound();
				} else maxsound();
			wiirumble_set(0,vib_duration2);
			} else if (collisionButton(button1CoordX, button1CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){ //cancel button					
							buttonsound1();
							wiirumble_set(0,vib_duration1);		
							clearkeyboard(2); 
							
							chosenmenu=0;//goes back to the menu without saving
					
			}else if (collisionButton(button3CoordX, button3CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true ){ //save button				
						savepassword:
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						//saving new password:
						for (int i=0;i<=62;i++){certBuffer[PASSWORD1_pos + i] = Passwordkeyboard_writtentext[i];}//replaces password in buf_fileread
						certBuffer[PASSWORD1_pos-3] =  lengthtoHex(Password_indexkeyboard);// this must be the length of the new password. example : 0x03 = three characters long | 0x20 = 32 characters.

						ISFS_WRITE_CONFIGDAT(certBuffer); 	
						
						clearkeyboard(2); 
						chosenmenu=0;//goes back to the menu after saving everything
						
						
			}	
		} 	
				
				

				if (CusorOnKey(bta_coordX,bta_coordY,44,36)==true){
					
					if (Password_uppercase==false){
					   GRRLIB_DrawImg(bta_coordX-10, bta_coordY-10, tex_bta_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(bta_coordX-10, bta_coordY-10, tex_bta_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(btb_coordX,btb_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btb_coordX-10, btb_coordY-10, tex_btb_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					   }else  GRRLIB_DrawImg(btb_coordX-10, btb_coordY-10, tex_btb_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(btc_coordX,btc_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btc_coordX-10, btc_coordY-10, tex_btc_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
						}else GRRLIB_DrawImg(btc_coordX-10, btc_coordY-10, tex_btc_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
						
				}
				else if (CusorOnKey(btd_coordX,btd_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btd_coordX-10, btd_coordY-10, tex_btd_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(btd_coordX-10, btd_coordY-10, tex_btd_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(bte_coordX,bte_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(bte_coordX-10, bte_coordY-10, tex_bte_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(bte_coordX-10, bte_coordY-10, tex_bte_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btf_coordX,btf_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btf_coordX-10, btf_coordY-10, tex_btf_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btf_coordX-10, btf_coordY-10, tex_btf_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btg_coordX,btg_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btg_coordX-10, btg_coordY-10, tex_btg_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btg_coordX-10, btg_coordY-10, tex_btg_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(bth_coordX,bth_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(bth_coordX-10, bth_coordY-10, tex_bth_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(bth_coordX-10, bth_coordY-10, tex_bth_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(bti_coordX,bti_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(bti_coordX-10, bti_coordY-10, tex_bti_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(bti_coordX-10, bti_coordY-10, tex_bti_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btj_coordX,btj_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btj_coordX-10, btj_coordY-10, tex_btj_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btj_coordX-10, btj_coordY-10, tex_btj_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btk_coordX,btk_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btk_coordX-10, btk_coordY-10, tex_btk_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btk_coordX-10, btk_coordY-10, tex_btk_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btl_coordX,btl_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btl_coordX-10, btl_coordY-10, tex_btl_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btl_coordX-10, btl_coordY-10, tex_btl_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btm_coordX,btm_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btm_coordX-10, btm_coordY-10, tex_btm_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btm_coordX-10, btm_coordY-10, tex_btm_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btn_coordX,btn_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btn_coordX-10, btn_coordY-10, tex_btn_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(btn_coordX-10, btn_coordY-10, tex_btn_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(bto_coordX,bto_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(bto_coordX-10, bto_coordY-10, tex_bto_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(bto_coordX-10, bto_coordY-10, tex_bto_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(btp_coordX,btp_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btp_coordX-10, btp_coordY-10, tex_btp_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btp_coordX-10, btp_coordY-10, tex_btp_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btq_coordX,btq_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btq_coordX-10, btq_coordY-10, tex_btq_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(btq_coordX-10, btq_coordY-10, tex_btq_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(btr_coordX,btr_coordY,44,36)==true){
					   
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btr_coordX-10, btr_coordY-10, tex_btr_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btr_coordX-10, btr_coordY-10, tex_btr_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(bts_coordX,bts_coordY,44,36)==true){
					
					if (Password_uppercase==false){
					   GRRLIB_DrawImg(bts_coordX-10, bts_coordY-10, tex_bts_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(bts_coordX-10, bts_coordY-10, tex_bts_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(btt_coordX,btt_coordY,44,36)==true){
					   
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btt_coordX-10, btt_coordY-10, tex_btt_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(btt_coordX-10, btt_coordY-10, tex_btt_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(btu_coordX,btu_coordY,44,36)==true){
					
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btu_coordX-10, btu_coordY-10, tex_btu_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(btu_coordX-10, btu_coordY-10, tex_btu_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(btv_coordX,btv_coordY,44,36)==true){
					
					if (Password_uppercase==false){
					   GRRLIB_DrawImg(btv_coordX-10, btv_coordY-10, tex_btv_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(btv_coordX-10, btv_coordY-10, tex_btv_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				
				else if (CusorOnKey(btw_coordX,btw_coordY,44,36)==true){
					  
					  if (Password_uppercase==false){
					   GRRLIB_DrawImg(btw_coordX-10, btw_coordY-10, tex_btw_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(btw_coordX-10, btw_coordY-10, tex_btw_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				
				else if (CusorOnKey(btx_coordX,btx_coordY,44,36)==true){
					   
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btx_coordX-10, btx_coordY-10, tex_btx_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(btx_coordX-10, btx_coordY-10, tex_btx_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(bty_coordX,bty_coordY,44,36)==true){
					   
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(bty_coordX-10, bty_coordY-10, tex_bty_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}else GRRLIB_DrawImg(bty_coordX-10, bty_coordY-10, tex_bty_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);

				}
				else if (CusorOnKey(btz_coordX,btz_coordY,44,36)==true){
					   
					   if (Password_uppercase==false){
					   GRRLIB_DrawImg(btz_coordX-10, btz_coordY-10, tex_btz_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
					}else GRRLIB_DrawImg(btz_coordX-10, btz_coordY-10, tex_btz_UP_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(zero_coordX,zero_coordY,44,36)==true){
					
					   GRRLIB_DrawImg(zero_coordX-10, zero_coordY-10, tex_zero_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(one_coordX,one_coordY,44,36)==true){
					
					   GRRLIB_DrawImg(one_coordX-10, one_coordY-10, tex_one_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(two_coordX,two_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(two_coordX-10, two_coordY-10, tex_two_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(three_coordX,three_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(three_coordX-10, three_coordY-10, tex_three_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(four_coordX,four_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(four_coordX-10, four_coordY-10, tex_four_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(five_coordX,five_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(five_coordX-10, five_coordY-10, tex_five_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(six_coordX,six_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(six_coordX-10, six_coordY-10, tex_six_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(seven_coordX,seven_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(seven_coordX-10, seven_coordY-10, tex_seven_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(eight_coordX,eight_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(eight_coordX-10, eight_coordY-10, tex_eight_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(nine_coordX,nine_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(nine_coordX-10, nine_coordY-10, tex_nine_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(bracket_1_coordX,bracket_1_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(bracket_1_coordX-10, bracket_1_coordY-10, tex_bracket_1_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(bracket_2_coordX,bracket_2_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(bracket_2_coordX-10, bracket_2_coordY-10, tex_bracket_2_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(backslash_coordX,backslash_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(backslash_coordX-10, backslash_coordY-10, tex_backslash_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(slash_coordX,slash_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(slash_coordX-10, slash_coordY-10, tex_slash_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(apostrophe_coordX,apostrophe_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(apostrophe_coordX-10, apostrophe_coordY-10, tex_apostrophe_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(hash_coordX,hash_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(hash_coordX-10, hash_coordY-10, tex_hash_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(comma_coordX,comma_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(comma_coordX-10, comma_coordY-10, tex_comma_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(point_coordX,point_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(point_coordX-10, point_coordY-10, tex_point_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(equal_coordX,equal_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(equal_coordX-10, equal_coordY-10, tex_equal_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(equal_coordX,equal_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(equal_coordX-10, equal_coordY-10, tex_equal_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(semicolon_coordX,semicolon_coordY,44,36)==true){
					   
					   GRRLIB_DrawImg(semicolon_coordX-10, semicolon_coordY-10, tex_semicolon_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(dash_coordX,dash_coordY,44,36)==true){
					   GRRLIB_DrawImg(dash_coordX-10, dash_coordY-10, tex_dash_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}
				else if (CusorOnKey(underscore_coordX,underscore_coordY,44,36)==true){
					   GRRLIB_DrawImg(underscore_coordX-10, underscore_coordY-10, tex_underscore_png, 0, kbd_button_Xsize+0.5, kbd_button_Ysize+0.5, GRRLIB_WHITE);
				}


				
						
		
			
    	GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE); 
        GRRLIB_Render();	
}
	
}


////////////////////////////////////////////////SECURITY MENU :////////////////////////////////////////////////////////////
if (chosenmenu==3){
	while(chosenmenu==3){

		
		
		updatecursorpos();
	
				GRRLIB_DrawImg(0, 0, tex_settings_background_png, 0, 1, 1, GRRLIB_WHITE);  //shows the keyboard on screen
				
				GRRLIB_DrawImg(button1CoordX, button1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button1CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Save");
				
				//GRRLIB_Printf(15, 25, tex_BMfont5, GRRLIB_BLACK, 1, "IR.x : %d Ir.y : %d", (int)ir1.x, (int)ir1.y);
					GRRLIB_Printf(200, 45, tex_BMfont5, GRRLIB_BLACK, 1, "Security Type"); //title

				
				GRRLIB_DrawImg(buttonsecurity1X, buttonsecurity1Y, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((buttonsecurity1X+buttonWidth/2)-20, (buttonsecurity1Y+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Open");
				
				GRRLIB_DrawImg(buttonsecurity2X, buttonsecurity2Y, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((buttonsecurity2X+buttonWidth/2)-20, (buttonsecurity2Y+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "WEP64");
				
				GRRLIB_DrawImg(buttonsecurity3X, buttonsecurity3Y, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((buttonsecurity3X+buttonWidth/2)-20, (buttonsecurity3Y+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "WEP128");

				GRRLIB_DrawImg(buttonsecurity4X, buttonsecurity4Y, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((buttonsecurity4X+buttonWidth/2)-50, (buttonsecurity4Y+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "WPA-PSK(TKIP)");

				GRRLIB_DrawImg(buttonsecurity5X, buttonsecurity5Y, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((buttonsecurity5X+buttonWidth/2)-50, (buttonsecurity5Y+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "WPA2-PSK(AES)");

				GRRLIB_DrawImg(buttonsecurity6X, buttonsecurity6Y, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((buttonsecurity6X+buttonWidth/2)-45, (buttonsecurity6Y+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "WPA-PSK(AES)");

				
				switch(connection1_newsecuritytype){ //shows the red "selected" thing on the selected network type.
					case 0x00:
						GRRLIB_DrawImg(selectedsecurity1X, selectedsecurity1Y, tex_selected_png, 0, 1, 1, GRRLIB_WHITE);
						break;				
					case 0x01:
						GRRLIB_DrawImg(selectedsecurity2X, selectedsecurity2Y, tex_selected_png, 0, 1, 1, GRRLIB_WHITE);  
						break;
					case 0x02:
						GRRLIB_DrawImg(selectedsecurity3X, selectedsecurity3Y, tex_selected_png, 0, 1, 1, GRRLIB_WHITE);  
						break;
					case 0x04:
						GRRLIB_DrawImg(selectedsecurity4X, selectedsecurity4Y, tex_selected_png, 0, 1, 1, GRRLIB_WHITE);  
						break;
					case 0x05:
						GRRLIB_DrawImg(selectedsecurity5X, selectedsecurity5Y, tex_selected_png, 0, 1, 1, GRRLIB_WHITE);  
						break;
					case 0x06:
						GRRLIB_DrawImg(selectedsecurity6X, selectedsecurity6Y, tex_selected_png, 0, 1, 1, GRRLIB_WHITE);  
						break;
				}
				
				
				GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);  // shows cursor
				

				if(wpaddown & WPAD_BUTTON_A || paddown & PAD_BUTTON_A ) {
					if (collisionButton(button1CoordX, button1CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){ //save changes:
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						if (connection1_newsecuritytype != connection1_securitytype){ //we check that the chosen security is different from the already used one. 
							certBuffer[connection1_securitypos] = connection1_newsecuritytype; 
							ISFS_WRITE_CONFIGDAT(certBuffer);
						}
						chosenmenu=0;
					}
					else if (collisionButton(buttonsecurity1X, buttonsecurity1Y, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
						connection1_newsecuritytype = 0x00;
						selectsound();
						wiirumble_set(0,vib_duration1);
					} //OPEN
					else if (collisionButton(buttonsecurity2X, buttonsecurity2Y, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
						connection1_newsecuritytype = 0x01;
						selectsound();
						wiirumble_set(0,vib_duration1);
					}//WEP64
					else if (collisionButton(buttonsecurity3X, buttonsecurity3Y, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
						connection1_newsecuritytype = 0x02;
						selectsound();
						wiirumble_set(0,vib_duration1);
					}//WEP128
	    			else if (collisionButton(buttonsecurity4X, buttonsecurity4Y, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
						connection1_newsecuritytype = 0x04;
						selectsound();
						wiirumble_set(0,vib_duration1);
					}//WPA-PSK(TKIP)
			        else if (collisionButton(buttonsecurity5X, buttonsecurity5Y, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
						connection1_newsecuritytype = 0x05;
						selectsound();
						wiirumble_set(0,vib_duration1);
					}//WPA2-PSK(AES)
					else if (collisionButton(buttonsecurity6X, buttonsecurity6Y, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
						connection1_newsecuritytype = 0x06;
						selectsound();
						wiirumble_set(0,vib_duration1);
					}//WPA-PSK(AES) 				 
				}
					
					
					
        GRRLIB_Render();
	}
	
	
	
}
	
	
	/////////////////////////CONFIRM CHANGES MENU:////(contains the "note" window for the network connection test function which is located at the label connectiontest:)
	
if(chosenmenu==4){
	while(chosenmenu==4){
		updatecursorpos();
	
		GRRLIB_DrawImg(0, 0, tex_settings_background_png, 0, 1, 1, GRRLIB_WHITE);  //shows the keyboard on screen
				
		GRRLIB_DrawImg(button1CoordX, button1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
		GRRLIB_Printf((button1CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Cancel");
		
				
		GRRLIB_DrawImg(button3CoordX, button3CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
		GRRLIB_Printf((button3CoordX+buttonWidth/2)-50, (button3CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Save and Exit");
				
		GRRLIB_Printf(250, 45, tex_BMfont5, GRRLIB_BLACK, 1, "Confirm changes?"); //title
		
		char networktype_text[9]; 
		if (networktype==0){
			GRRLIB_Printf(40, 75, tex_BMfont5, GRRLIB_WHITE, 1, "Connection Type : Wired", networktype_text); 
		}else{
			GRRLIB_Printf(40, 75, tex_BMfont5, GRRLIB_WHITE, 1, "Connection Type : Wireless", networktype_text); 
			GRRLIB_Printf(40, 100, tex_BMfont5, GRRLIB_WHITE, 1, "SSID : %s", SSID1); //shortenedssid
			GRRLIB_Printf(40, 125, tex_BMfont5, GRRLIB_WHITE, 1, "Password : %s", PASSWORD1);
			GRRLIB_Printf(40, 150, tex_BMfont5, GRRLIB_WHITE, 1, "Security type :");
			
			if (connection1_securitytype == 0x00){
				GRRLIB_Printf(165, 150, tex_BMfont5, GRRLIB_WHITE, 1, "Open");
			}else if (connection1_securitytype == 0x01){
				GRRLIB_Printf(165, 150, tex_BMfont5, GRRLIB_WHITE, 1, "WEP64");
			} else if (connection1_securitytype == 0x02){
				GRRLIB_Printf(165, 150, tex_BMfont5, GRRLIB_WHITE, 1, "WEP128");
			} else if (connection1_securitytype == 0x04){
				GRRLIB_Printf(165, 150, tex_BMfont5, GRRLIB_WHITE, 1, "WPA-PSK(TKIP)");
			} else if (connection1_securitytype == 0x05){
				GRRLIB_Printf(165, 150, tex_BMfont5, GRRLIB_WHITE, 1, "WPA2-PSK(AES)");
			} else if (connection1_securitytype == 0x06){
				GRRLIB_Printf(165, 150, tex_BMfont5, GRRLIB_WHITE, 1, "WPA-PSK(AES)");
			}
				
		}
		
        
		GRRLIB_DrawImg(button2CoordX, button2CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
		GRRLIB_Printf((button2CoordX+buttonWidth/2)-60, (button2CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Connection Test");
		
		
		GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);  // shows cursor
		
		if(wpaddown & WPAD_BUTTON_A || paddown & PAD_BUTTON_A ) {
			if (collisionButton(button1CoordX, button1CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true) chosenmenu=0;
			
			if (collisionButton(button3CoordX, button3CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
				exitprogram();
			}
			if (collisionButton(button2CoordX, button2CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
				
				isNetworkWorking = 0;
				while(1){ //we are now inside a new menu. this menu is there to warn the user that testing the network will require rebooting the program.
					
					updatecursorpos();
					GRRLIB_DrawImg(0, 0, tex_settings_background_png, 0, 1, 1, GRRLIB_WHITE);  
					GRRLIB_DrawImg(83, 67, tex_dialogue_box_png, 0, 1, 1, GRRLIB_WHITE);  
					GRRLIB_Printf(300-10,120, tex_BMfont5, GRRLIB_BLACK, 1.5, "Note :");
					GRRLIB_Printf(3+300-200,150, tex_BMfont5, GRRLIB_BLACK, 0.9, "The program needs to restart for the changes to take effect.");
					GRRLIB_Printf(3+300-200,165, tex_BMfont5, GRRLIB_BLACK, 0.9, "By clicking on proceed you will be sent back to your loader.");
					GRRLIB_Printf(3+300-200,180, tex_BMfont5, GRRLIB_BLACK, 0.9, "Launch the program again and the Network Test will begin.");
					
					GRRLIB_DrawImg(button1CoordX, button1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
					GRRLIB_Printf((button1CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Cancel");
		
				
					GRRLIB_DrawImg(button3CoordX, button3CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
					GRRLIB_Printf((button3CoordX+buttonWidth/2)-25, (button3CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Proceed");
					
					GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);  // shows cursor
					

										
					if(wpaddown & WPAD_BUTTON_A || paddown & PAD_BUTTON_A ) {
						if (collisionButton(button3CoordX, button3CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
							//we clicked the "proceed" button:
							
							//s32 deletefile = ISFS_Delete(ISFS_TEMPFILE_PATH);
							
							int tempfile_create = ISFS_CreateFile(ISFS_TEMPFILE_PATH,0,0,0,0); //0,3,3,3
							if(tempfile_create >= 0) {
								exitprogram();
							} //if we cannot create the file we leave the user where he is.
						}
						if (collisionButton(button1CoordX, button1CoordY, cursor_positionX,cursor_positionY,buttonWidth,buttonHeight) == true){
								
							break; //exiting the menu and going back to "confirm changes" menu.
						}
					}
				
					GRRLIB_Render();
				}
			}
					
		}
		GRRLIB_Render();
	}
	
	
	
	
}
	
	///////////////////////CREDITS MENU://////////////////////////////
	//you can jump into the network test menu by doing HOME/START + A. Will throw an error if the program wasnt restarted.

if (chosenmenu==5){
	while(chosenmenu==5){
		
		updatecursorpos();
	
		GRRLIB_DrawImg(0, 0, tex_credits_bg_png, 0, 1, 1, GRRLIB_WHITE);  
		
		GRRLIB_Printf(280, 25, tex_BMfont5, GRRLIB_WHITE, 1.5, "Credits"); //title
		
		GRRLIB_Printf(50, 100, tex_BMfont5, GRRLIB_WHITE, 1, "Wii Network Config Editor v1.4 made by Bazmoc"); //title
		GRRLIB_Printf(50, 125, tex_BMfont5, GRRLIB_WHITE, 1, "Download the latest versions at https://github.com/Bazmoc"); //title
		GRRLIB_Printf(50, 150, tex_BMfont5, GRRLIB_WHITE, 1, "If you have any questions or issues contact me on discord : @bazmoc"); //title
		GRRLIB_Printf(50, 200, tex_BMfont5, GRRLIB_WHITE, 1, "Press B to go back."); //title



		GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);  // shows cursor
		
		if(wpaddown & WPAD_BUTTON_B || paddown & PAD_BUTTON_B ) { //if B wiimote or B gamecube :
			chosenmenu=0;
		}
		if((wpaddown & WPAD_BUTTON_HOME || paddown & PAD_BUTTON_START) && (wpaddown & WPAD_BUTTON_A || paddown & PAD_BUTTON_A)) { 
				chosenmenu=6;
		}		
		
        GRRLIB_Render();
	}
	
}
	
	
	/////////////////NETWORK TEST MENU////////////
if (chosenmenu==6){
	int timeoutVal = 5; //number of times we are going to do net_init
	int timeout = 0;
	isNetworkWorking = 0;
	while(chosenmenu==6){
		updatecursorpos();
		GRRLIB_DrawImg(0, 0, tex_settings_background_png, 0, 1, 1, GRRLIB_WHITE); 

		
		GRRLIB_Printf(250, 45, tex_BMfont5, GRRLIB_BLACK, 1, "Connection Test"); //title
		GRRLIB_Printf(50, 275, tex_BMfont5, GRRLIB_WHITE, 1, "Press B to go back to the main menu."); //title

		//cursor
		GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);  // shows cursor
		
		
		if (timeout<timeoutVal) NetworkError = net_init();
		

		if(NetworkError<0){
		isNetworkWorking = 1;
		timeout +=1;
		} else {
		isNetworkWorking=2;
		timeout = 6; //stops the timeout loop
		}	
		
		
		if(isNetworkWorking == 1){
			if(timeout<timeoutVal){
				GRRLIB_Printf(250, 100, tex_BMfont5, GRRLIB_WHITE ,1.5, "Please Wait...");
				GRRLIB_Printf(250, 120, tex_BMfont5, GRRLIB_WHITE ,1.5, "%d out of %d Tries.",timeout,timeoutVal);
				
			}else{
				GRRLIB_Printf(50, 200, tex_BMfont5, GRRLIB_RED, 1.5, "Network test failed.");
				GRRLIB_Printf(50, 220, tex_BMfont5, GRRLIB_RED, 1.5, "Check SSID, Password and Security type. Error %d", NetworkError);
			}
		}
		if(isNetworkWorking == 2){
			GRRLIB_Printf(50, 200, tex_BMfont5, GRRLIB_GREEN, 1.5, "Network test Successful!");
			//GRRLIB_Printf(50, 215, tex_BMfont5, GRRLIB_WHITE, 1, "Measured Ping response time : %f ms.", time_taken);
		}

		if (wpaddown & WPAD_BUTTON_B || paddown & PAD_BUTTON_B ) {
			chosenmenu=0;//goes back to where we were before jumping into this menu
		}


		GRRLIB_Render();
	}
	
	
	
	
}//end network test

}//end loop

} //end main




