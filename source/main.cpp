/*

//Wii Network Config Editor by Bazmoc (2023)

This program has been tested on both Dolphin and 2 real Wiis (hundreds of times) without any issues.
	
	
Thanks to Fix94 for sharing w/ me the source code of "simple Cert.sys extractor" which inspired me for the ISFS reading part.

Also thanks to Aurelio for helping me out for the **numerous** issues i had with my program.

Also thanks to Supertazon for helping me out for the sound effects and GRRLIB.
*/

#include <gccore.h>
#include <sdcard/wiisd_io.h>
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

//file paths :
#define ISFS_CONFIGDAT_PATH "/shared2/sys/net/02/config.dat"

#define CONFIGDAT_FILELENGTH 7004

//Which characters in the config.dat:
int SSID1_pos=1996; //beginning of the first SSID's character
int PASSWORD1_pos=2040; //beginning of the first Password's character

//those characters determine which connection is selected. 
int connection1_selectpos = 8; 
int connection2_selectpos = 8+2332; //like i said before, 2332 is the number of byte in 1 connection
int connection3_selectpos = 8+2332+2332;

int connection1_securitypos = 2033; //security type position (connection 1 only)

char connection_selectedchar = 0xA6; //0xa6 = ª, i had to write it in hex cuz it would create issues otherwise..
char connection_unselectedchar = '&';


#include <grrlib.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <fat.h>

#include "gfx/font/BMfont1.h"
#include "gfx/font/BMfont2.h"
#include "gfx/font/BMfont3.h"
#include "gfx/font/BMfont4.h"
#include "gfx/font/BMfont5.h"


#include "gfx/cursor/cursor.h"
#include "gfx/button/button1.h"
#include "gfx/button/selected.h"

#include "gfx/button/poweroff.h"

		
	
int cursor_positionX = 640/2; //put cursor at center of screen on startup.
int cursor_positionY = 480/2;

int cursor_rotation_angle = 0; //initial angle

//const int
int buttonWidth = 172; //pixels
int buttonHeight = 44; //pixels

int button1CoordX = 100-(buttonWidth/2);
int button1CoordY = 400;

int button2CoordX = 325-(buttonWidth/2); 
int button2CoordY = 400;

int button3CoordX = (640-100)-(buttonWidth/2);
int button3CoordY = 400;

//
int mainmenubt1CoordX = button1CoordX;
int mainmenubt1CoordY = button1CoordY-60;

int mainmenubt2CoordX = button2CoordX;
int mainmenubt2CoordY = button2CoordY-60;

int mainmenubt3CoordX = button3CoordX;
int mainmenubt3CoordY = button3CoordY-60;
//
#include "security_menu_buttons.h"



int exitCoordX = 550;
int exitCoordY = 25;

#include "gfx/backgrounds/blank_kbd.h"
#include "gfx/backgrounds/settings_background.h"


#include "keyboard_texture_coords.h" //contains all the button's coordinates (x+y) and the #include's


#include "sounds.h"

bool collisionButton(int button_CoordX, int button_coordY, int cursorPosX,int cursorPosY){
	if(GRRLIB_RectOnRect(button_CoordX,button_coordY,buttonWidth,buttonHeight,cursorPosX,cursorPosY,5,5)){ //64,64 too much
		return true;
	} else return false;
}

GRRLIB_texImg *tex_cursor_png; //initialises *tex_cursor_png to use on updatecursorpos()


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
			cursor_rotation_angle = (int)ir1.angle; //ir1.angle is a float so we convert it to an int
		} else {
			cursor_rotation_angle = 0; //resets the angle from the wiimote
			if ((int)PAD_StickY(0) > threshold || (int)PAD_StickY(0) < -threshold ||(int)PAD_StickX(0) > threshold ||(int)PAD_StickX(0) < -threshold){
				if (cursor_positionX+((int)PAD_StickX(0)/sensitivity) >=0 && cursor_positionX+((int)PAD_StickX(0)/sensitivity) <=640 && cursor_positionY-((int)PAD_StickY(0)/sensitivity) >= 0 && cursor_positionY-((int)PAD_StickY(0)/sensitivity) <= 480){ //makes sure the cursor never leaves the screen when using the GC joystick
							cursor_positionX = cursor_positionX+((int)PAD_StickX(0)/sensitivity);
							cursor_positionY = cursor_positionY-((int)PAD_StickY(0)/sensitivity); //minus needed or the controls are inversed.
				}
			}
			
		}	
		
}


extern "C" {extern void exit(int status);}



static fstats stats ATTRIBUTE_ALIGN(32);

u8 *ISFS_GetFile(u8 *path, u32 *size, s32 length){
	
	*size = 0;
	
	s32 fd = ISFS_Open((const char *)path, ISFS_OPEN_READ);
		u8 *buf = NULL;

		if(fd >= 0)
		{
			memset(&stats, 0, sizeof(fstats));
			if(ISFS_GetFileStats(fd, &stats) >= 0)
			{
				if(length <= 0)
					length = stats.file_length;
				if(length > 0)
					buf = (u8 *)memalign(32, length);
				if(buf)
				{
					*size = stats.file_length;
					if(ISFS_Read(fd, (char*)buf, length) != length){
						*size = 0;
						free(buf);
					}
				}
			}
		ISFS_Close(fd);
		}

		if(*size > 0)
		{	
			DCFlushRange(buf, *size);
			ICInvalidateRange(buf, *size);
		}
		return buf;
}

u8 *ISFS_WRITE_CONFIGDAT(u8 *buf2){ 
	s32 fd2 = ISFS_Open(ISFS_CONFIGDAT_PATH, ISFS_OPEN_WRITE); //opens config.dat on the nand. Write mode
	if(fd2<0) //negative = error so we show what error it is
	{
		//printf("Failed to open, error %d\n", fd2);
		//sleep(5);
		exit(0);
	}
	else //everything's fine:
	{
		//printf("opened isfs \n");
		//sleep(5);
		s32 ret = ISFS_Write(fd2, buf2, 7004);
		if(ret<0){ //like before, under 0 is an error code so we show it.
			//printf("Error %d \n", ret);
					//sleep(5);
					exit(0);
		}
		//printf("isfs write ok \n");
		//sleep(5);
		/*free(buf2);
		printf("buf2 freed \n");
		sleep(5);*/
		ISFS_Close(fd2);
		//printf("ISFS closed \n");
		//sleep(5);
	}
	return 0;
}

char lengthtoHex(int text_length){ //this function takes the length of the new SSID and returns a hex value.
	
	switch(text_length){
		case 0:
			return 0x00;
			break;
		case 1:
			return 0x01;
			break;
		case 2:
			return 0x02;
			break;
		case 3:
			return 0x03;
			break;
		case 4:
			return 0x04;
			break;
		case 5:
			return 0x05;
			break;
		case 6:
			return 0x06;
			break;
		case 7:
			return 0x07;
			break;
		case 8:
			return 0x08;
			break;
		case 9:
			return 0x09;
			break;
		case 10:
			return 0x0A;
			break;
		case 11:
			return 0x0B;
			break;
		case 12:
			return 0x0C;
			break;
		case 13:
			return 0x0D;
			break;
		case 14:
			return 0x0E;
			break;
		case 15:
			return 0x0F;
			break;
		case 16:
			return 0x10;
			break;
		case 17:
			return 0x11;
			break;
		case 18:
			return 0x12;
			break;
		case 19:
			return 0x13;
			break;
		case 20:
			return 0x14;
			break;
		case 21:
			return 0x15;
			break;
		case 22:
			return 0x16;
			break;
		case 23:
			return 0x17;
			break;
		case 24:
			return 0x18;
			break;
		case 25:
			return 0x19;
			break;
		case 26:
			return 0x1A;
			break;
		case 27:
			return 0x1B;
			break;
		case 28:
			return 0x1C;
			break;
		case 29:
			return 0x1D;
			break;
		case 30:
			return 0x1E;
			break;
		case 31:
			return 0x1F;
			break;
		case 32:
			return 0x20;
			break;
	}
}




////////////////////for vibration of the wiimote://///////////////////////

static syswd_t wiirumble = SYS_WD_NULL;

static void wiirumble_callback(syswd_t wiirumble, void *cb_arg){
    WPAD_Rumble(WPAD_CHAN_0,0); //stops the vibration
}

void wiirumble_init(void){SYS_CreateAlarm(&wiirumble);}

void wiirumble_set(int sec,int millisec){	
	WPAD_Rumble(WPAD_CHAN_0,1); //starts vibrating

    struct timespec tv;

    tv.tv_sec  = sec;
    tv.tv_nsec = millisec*1000000; //convert ms to ns (ms*1000000) = ns
	
    SYS_SetAlarm(wiirumble, &tv, wiirumble_callback, NULL);
}

int vib_duration1 = 100;
int vib_duration2 = 50;

/////////////////////////////////////////////////////////

char SSIDkeyboard_writtentext[33] = "";
	int SSID_indexkeyboard = 0;
	bool SSID_uppercase = false;
	
	char Passwordkeyboard_writtentext[33] = "";
	int Password_indexkeyboard = 0;
	bool Password_uppercase = false;
	
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




int main() {
		wiirumble_init();

			char SSID1[32];
			char PASSWORD1[63];//from index 0 to 62 = password(password max 63chars long) | character 63 is the terminating character \0
			//char WIFI_type; 
			
    GRRLIB_Init();
	
	PAD_Init();
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	
	ASND_Init(); 
    ASND_Pause(0);
	track = ASND_GetFirstUnusedVoice();
	

	GRRLIB_texImg *tex_poweroff_png = GRRLIB_LoadTexture(poweroff);
    GRRLIB_InitTileSet(tex_poweroff_png, 640, 480, 0); 
	
	GRRLIB_texImg *tex_settings_background_png = GRRLIB_LoadTexture(settings_background);
    GRRLIB_InitTileSet(tex_settings_background_png, 640, 480, 0); 
	
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
	GRRLIB_texImg *tex_bta_png = GRRLIB_LoadTexture(bta);
    GRRLIB_InitTileSet(tex_bta_png, 44, 36, 0); //dimensions of a wii keyboard button : x44 y36

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
	
	GRRLIB_texImg *tex_yen_png = GRRLIB_LoadTexture(yen);
    GRRLIB_InitTileSet(tex_yen_png, 44, 36, 0); 
	
	GRRLIB_texImg *tex_euro_png = GRRLIB_LoadTexture(euro);
    GRRLIB_InitTileSet(tex_euro_png, 44, 36, 0); 
	
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
	
	
	
	

	
	//test crash:
		
		FILE *file_r=NULL;
    char buf_fileread[CONFIGDAT_FILELENGTH]="";
	
	FILE *file_w=NULL;
		
	ISFS_Initialize();
	
	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32); //not the cause of crash

	sprintf(filepath, ISFS_CONFIGDAT_PATH); 
	u32 certSize = 0;
	u8 *certBuffer = ISFS_GetFile((u8*)&filepath, &certSize, -1);
	if(certSize == 0 || certBuffer == NULL)
	{
		exit(0);
	}
//end test crash

//final debug edit: crash was located in this code. moving it before mainmenu: fixed the issue. possible cause = initializing something multilpe times when it's already used.

	
	
	
	
	mainmenu: //used to go back to the main menu
	
	
	SSID_uppercase=false; //resets the keyboard's CAPS value.
	Password_uppercase=false;

			
			for (int i=0;i<=31;i++){  //this loops basically takes each letter and makes a string w/ it.
				SSID1[i] = certBuffer[SSID1_pos + i];
			}
			SSID1[32] = '\0'; //terminating character "\0" required or it shows weird crap at the end after the SSID
			
			for (int i=0;i<=62;i++){  //this loops basically takes each letter and makes a string w/ it. Doesn't change anything to certBuffer.
				PASSWORD1[i] = certBuffer[PASSWORD1_pos + i];
			}
			PASSWORD1[63] = '\0'; 

			//we read and from there we determine which connection is selected. 
			//like i said before ª= selected and &=unselected 
			char connection1_state = certBuffer[connection1_selectpos];
			char connection2_state = certBuffer[connection2_selectpos];
			char connection3_state = certBuffer[connection3_selectpos];
			
			//gets security type:
			char connection1_securitytype = certBuffer[connection1_securitypos];
			
			
			int NewSelectedNetwork = 1; //goes from 1 to 3.
			//writes the modified selected network in certBuffer
			if (NewSelectedNetwork==1){
				certBuffer[connection1_selectpos] = connection_selectedchar; 
				certBuffer[connection2_selectpos] = connection_unselectedchar;
				certBuffer[connection3_selectpos] = connection_unselectedchar;
			}else if (NewSelectedNetwork==2){
				certBuffer[connection1_selectpos] = connection_unselectedchar;
				certBuffer[connection2_selectpos] = connection_selectedchar;
				certBuffer[connection3_selectpos] = connection_unselectedchar;
			}else if (NewSelectedNetwork==3){
				certBuffer[connection1_selectpos] = connection_unselectedchar;
				certBuffer[connection2_selectpos] = connection_unselectedchar;
				certBuffer[connection3_selectpos] = connection_selectedchar;
			}
	
	while (1){ //main menu
	
				
		        updatecursorpos();
				GRRLIB_DrawImg(0, 0, tex_settings_background_png, 0, 1, 1, GRRLIB_WHITE);  
				
				GRRLIB_DrawImg(mainmenubt1CoordX, mainmenubt1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE); //button 1
				GRRLIB_DrawImg(mainmenubt2CoordX, mainmenubt2CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE); //button 2
				GRRLIB_DrawImg(mainmenubt3CoordX, mainmenubt3CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE); //button 3
				
				GRRLIB_DrawImg(exitCoordX, exitCoordY, tex_poweroff_png, 0, 1, 1, GRRLIB_WHITE); //exit button (goes back to HBC)
				

				GRRLIB_Printf((mainmenubt1CoordX+buttonWidth/2)-35, (mainmenubt1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1 , "Edit SSID");
				GRRLIB_Printf((mainmenubt2CoordX+buttonWidth/2)-50, (mainmenubt2CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1 , "Edit Password");
				GRRLIB_Printf((mainmenubt3CoordX+buttonWidth/2)-70, (mainmenubt3CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1 , "Edit Security type");
				
				
				GRRLIB_Printf(exitCoordX-15, exitCoordY+45, tex_BMfont5, GRRLIB_WHITE, 0.8, "Save and Exit"); //exit button

				GRRLIB_Printf(200, 40, tex_BMfont5, GRRLIB_BLACK,1 ,"Network Settings"); //title
				
				
				//GRRLIB_Printf(15, 25, tex_BMfont5, GRRLIB_WHITE, 1, "IR.x : %d Ir.y : %d", (int)ir1.x, (int)ir1.y);
				
				GRRLIB_Printf(mainmenubt1CoordX, 280, tex_BMfont5, GRRLIB_WHITE, 1, "SSID : ");
				
				//this part shortens the SSID if it's too long and adds "..." so that SSID and password don't overlap on the screen:
				int shorten_length = 20;
				char shortenedssid[33];
				strcpy(shortenedssid, SSID1);
				
				if(strlen(shortenedssid) >= shorten_length) {
					shortenedssid[shorten_length] = '.';
					shortenedssid[shorten_length+1] = '.';
					shortenedssid[shorten_length+2] = '.';
					shortenedssid[shorten_length+3] = '\0';
				}
				
				char shortenedpassword[64];
				strcpy(shortenedpassword, PASSWORD1);
				
				if(strlen(shortenedpassword) >= shorten_length) {
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
			
				GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);  // Draw a jpeg

				if(wpaddown & WPAD_BUTTON_A || paddown & PAD_BUTTON_A ) { //if you clicked A
					if (collisionButton(mainmenubt1CoordX, mainmenubt1CoordY, cursor_positionX,cursor_positionY) == true){
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						goto SSIDkeyboard;
					}
					if (collisionButton(mainmenubt2CoordX, mainmenubt2CoordY, cursor_positionX,cursor_positionY) == true){
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						goto PasswordKeyboard;
					}
					if (collisionButton(mainmenubt3CoordX, mainmenubt3CoordY, cursor_positionX,cursor_positionY) == true){
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						goto securitymenu;
					}
					if (collisionButton(exitCoordX, exitCoordY, cursor_positionX,cursor_positionY) == true){ //exit button
						ISFS_Deinitialize();
						ASND_End();
						//free(certBuffer); causes crash. need to fix
						exit(0);
					}
				}
        GRRLIB_Render();
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	SSIDkeyboard:
    while(1) { //keyboard for SSID. max 32 characters
        updatecursorpos();
	
				GRRLIB_DrawImg(0, 0, tex_blank_kbd_png, 0, 1, 1, GRRLIB_WHITE);  //shows the keyboard on screen

					//draw numbers:
				GRRLIB_DrawImg(one_coordX, one_coordY, tex_one_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(two_coordX, two_coordY, tex_two_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(three_coordX, three_coordY, tex_three_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(four_coordX, four_coordY, tex_four_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(five_coordX, five_coordY, tex_five_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(six_coordX, six_coordY, tex_six_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(seven_coordX, seven_coordY, tex_seven_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(eight_coordX, eight_coordY, tex_eight_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(nine_coordX, nine_coordY, tex_nine_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(zero_coordX, zero_coordY, tex_zero_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(dash_coordX, dash_coordY, tex_dash_png, 0, 1, 1, GRRLIB_WHITE);
				
				//draw letters:
				if (SSID_uppercase==false){ //lowercase letters:
					GRRLIB_DrawImg(bta_coordX, bta_coordY, tex_bta_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btb_coordX, btb_coordY, tex_btb_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btc_coordX, btc_coordY, tex_btc_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btd_coordX, btd_coordY, tex_btd_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bte_coordX, bte_coordY, tex_bte_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btf_coordX, btf_coordY, tex_btf_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btg_coordX, btg_coordY, tex_btg_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bth_coordX, bth_coordY, tex_bth_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bti_coordX, bti_coordY, tex_bti_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btj_coordX, btj_coordY, tex_btj_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btk_coordX, btk_coordY, tex_btk_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btl_coordX, btl_coordY, tex_btl_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btm_coordX, btm_coordY, tex_btm_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btn_coordX, btn_coordY, tex_btn_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bto_coordX, bto_coordY, tex_bto_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btp_coordX, btp_coordY, tex_btp_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btq_coordX, btq_coordY, tex_btq_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btr_coordX, btr_coordY, tex_btr_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bts_coordX, bts_coordY, tex_bts_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btt_coordX, btt_coordY, tex_btt_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btu_coordX, btu_coordY, tex_btu_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btv_coordX, btv_coordY, tex_btv_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btw_coordX, btw_coordY, tex_btw_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btx_coordX, btx_coordY, tex_btx_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bty_coordX, bty_coordY, tex_bty_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btz_coordX, btz_coordY, tex_btz_png, 0, 1, 1, GRRLIB_WHITE);
				}else{//uppercase letters:
					GRRLIB_DrawImg(bta_coordX, bta_coordY, tex_bta_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btb_coordX, btb_coordY, tex_btb_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btc_coordX, btc_coordY, tex_btc_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btd_coordX, btd_coordY, tex_btd_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bte_coordX, bte_coordY, tex_bte_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btf_coordX, btf_coordY, tex_btf_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btg_coordX, btg_coordY, tex_btg_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bth_coordX, bth_coordY, tex_bth_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bti_coordX, bti_coordY, tex_bti_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btj_coordX, btj_coordY, tex_btj_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btk_coordX, btk_coordY, tex_btk_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btl_coordX, btl_coordY, tex_btl_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btm_coordX, btm_coordY, tex_btm_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btn_coordX, btn_coordY, tex_btn_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bto_coordX, bto_coordY, tex_bto_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btp_coordX, btp_coordY, tex_btp_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btq_coordX, btq_coordY, tex_btq_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btr_coordX, btr_coordY, tex_btr_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bts_coordX, bts_coordY, tex_bts_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btt_coordX, btt_coordY, tex_btt_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btu_coordX, btu_coordY, tex_btu_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btv_coordX, btv_coordY, tex_btv_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btw_coordX, btw_coordY, tex_btw_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btx_coordX, btx_coordY, tex_btx_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bty_coordX, bty_coordY, tex_bty_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btz_coordX, btz_coordY, tex_btz_UP_png, 0, 1, 1, GRRLIB_WHITE);
				}



				//other characters and stuff:
				if (SSID_uppercase==true){
					GRRLIB_DrawImg(caps_coordX, caps_coordY, tex_caps_colored_png, 0, 1, 1, GRRLIB_WHITE); //show the yellow-colored CAPS button when uppercase==true
				}else{
					GRRLIB_DrawImg(caps_coordX, caps_coordY, tex_caps_png, 0, 1, 1, GRRLIB_WHITE); //else:show the normal one
				}

				GRRLIB_DrawImg(erase_coordX, erase_coordY, tex_erase_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(space_coordX, space_coordY, tex_space_png, 0, 1, 1, GRRLIB_WHITE);
				//GRRLIB_DrawImg(yen_coordX, yen_coordY, tex_yen_png, 0, 1, 1, GRRLIB_WHITE);
				//GRRLIB_DrawImg(euro_coordX, euro_coordY, tex_euro_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(bracket_1_coordX, bracket_1_coordY, tex_bracket_1_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(bracket_2_coordX, bracket_2_coordY, tex_bracket_2_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(backslash_coordX, backslash_coordY, tex_backslash_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(slash_coordX, slash_coordY, tex_slash_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(apostrophe_coordX, apostrophe_coordY, tex_apostrophe_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(hash_coordX, hash_coordY, tex_hash_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(comma_coordX, comma_coordY, tex_comma_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(point_coordX, point_coordY, tex_point_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(equal_coordX, equal_coordY, tex_equal_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(semicolon_coordX, semicolon_coordY, tex_semicolon_png, 0, 1, 1, GRRLIB_WHITE);
	
				
				
                GRRLIB_Printf(35, 50, tex_BMfont5, GRRLIB_BLACK, 1.5, SSIDkeyboard_writtentext); //write on the screen the text that is being written
				GRRLIB_DrawImg(button1CoordX, button1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button1CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Save");
				GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);  // Draw a jpeg
				
				//GRRLIB_Printf(15, 25, tex_BMfont5, GRRLIB_BLACK, 1, "IR.x : %d Ir.y : %d", (int)ir1.x, (int)ir1.y);
				//GRRLIB_Printf(15, 100, tex_BMfont5, GRRLIB_BLACK, 1, "SSID_indexkeyboard = %d", SSID_indexkeyboard);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//this part determines which character was clicked:
				
	int collision_sizex = 5;
	int collision_sizey = 5;
	
	SSIDkeyboard_writtentext[33] = '\0';

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
		} else if(GRRLIB_RectOnRect(zero_coordX,zero_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '0';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(one_coordX,one_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '1';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(two_coordX,two_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '2';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(three_coordX,three_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '3';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(four_coordX,four_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '4';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(five_coordX,five_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '5';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(six_coordX,six_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '6';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(seven_coordX,seven_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '7';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(eight_coordX,eight_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '8';
				SSID_indexkeyboard+=1;
				presssound();
			} else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(nine_coordX,nine_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '9';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(dash_coordX,dash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (SSID_indexkeyboard<=31) { 
				SSIDkeyboard_writtentext[SSID_indexkeyboard] = '-';
				SSID_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		} else if(GRRLIB_RectOnRect(bracket_1_coordX,bracket_1_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
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
		} else if(GRRLIB_RectOnRect(erase_coordX,erase_coordY,erase_width,erase_height,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
				if (SSID_indexkeyboard>0 ) { //you don't want that number to get negative lol 		
					SSIDkeyboard_writtentext[SSID_indexkeyboard-1] = '\0'; //erases the character
					SSID_indexkeyboard-=1;
					erasesound();
				} else maxsound();
			wiirumble_set(0,vib_duration2);				
			} else if(GRRLIB_RectOnRect(space_coordX,space_coordY,space_width,space_height,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
				if (SSID_indexkeyboard<=31) { 
					SSIDkeyboard_writtentext[SSID_indexkeyboard] = ' '; //space
					SSID_indexkeyboard+=1;
					presssound();
				} else maxsound();
				wiirumble_set(0,vib_duration2);
			}else if (collisionButton(button1CoordX, button1CoordY, cursor_positionX,cursor_positionY) == true){				
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						clearkeyboard(1); //not the cause of crash
						//free(certBuffer);
						//ISFS_Deinitialize();
						goto mainmenu; //goes back to the menu without saving
			}else if (collisionButton(button3CoordX, button3CoordY, cursor_positionX,cursor_positionY) == true){ //save button				
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						//saving new ssid:
						for (int i=0;i<=32;i++){  //replaces the ssid in certBuffer
							certBuffer[SSID1_pos + i] = SSIDkeyboard_writtentext[i];//NewSSID[i];
						}
						certBuffer[SSID1_pos+33] =  lengthtoHex(SSID_indexkeyboard);// this must be the length of the new SSID. example : 0x03 = three characters long | 0x20 = 32 characters.
						ISFS_WRITE_CONFIGDAT(certBuffer);
						}
			
		} 	
		
				
				
				GRRLIB_DrawImg(button1CoordX, button1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button1CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Cancel");
				
				GRRLIB_DrawImg(button3CoordX, button3CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button3CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Save");

				
				GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);
		
			
        GRRLIB_Render();
    }
	
	//////////////////////////////////////Keyboard for password:///////////////////////////////////////////////////////////////////
	
	PasswordKeyboard:
	while(1){//keyboard for password. max 63 characters
		
		
		
		
		
		updatecursorpos();
	
				GRRLIB_DrawImg(0, 0, tex_blank_kbd_png, 0, 1, 1, GRRLIB_WHITE);  //shows the keyboard on screen

					//draw numbers:
				GRRLIB_DrawImg(one_coordX, one_coordY, tex_one_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(two_coordX, two_coordY, tex_two_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(three_coordX, three_coordY, tex_three_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(four_coordX, four_coordY, tex_four_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(five_coordX, five_coordY, tex_five_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(six_coordX, six_coordY, tex_six_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(seven_coordX, seven_coordY, tex_seven_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(eight_coordX, eight_coordY, tex_eight_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(nine_coordX, nine_coordY, tex_nine_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(zero_coordX, zero_coordY, tex_zero_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(dash_coordX, dash_coordY, tex_dash_png, 0, 1, 1, GRRLIB_WHITE);
				
				//draw letters:
             	if (Password_uppercase==false){
					GRRLIB_DrawImg(bta_coordX, bta_coordY, tex_bta_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btb_coordX, btb_coordY, tex_btb_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btc_coordX, btc_coordY, tex_btc_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btd_coordX, btd_coordY, tex_btd_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bte_coordX, bte_coordY, tex_bte_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btf_coordX, btf_coordY, tex_btf_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btg_coordX, btg_coordY, tex_btg_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bth_coordX, bth_coordY, tex_bth_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bti_coordX, bti_coordY, tex_bti_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btj_coordX, btj_coordY, tex_btj_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btk_coordX, btk_coordY, tex_btk_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btl_coordX, btl_coordY, tex_btl_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btm_coordX, btm_coordY, tex_btm_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btn_coordX, btn_coordY, tex_btn_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bto_coordX, bto_coordY, tex_bto_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btp_coordX, btp_coordY, tex_btp_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btq_coordX, btq_coordY, tex_btq_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btr_coordX, btr_coordY, tex_btr_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bts_coordX, bts_coordY, tex_bts_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btt_coordX, btt_coordY, tex_btt_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btu_coordX, btu_coordY, tex_btu_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btv_coordX, btv_coordY, tex_btv_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btw_coordX, btw_coordY, tex_btw_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btx_coordX, btx_coordY, tex_btx_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bty_coordX, bty_coordY, tex_bty_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btz_coordX, btz_coordY, tex_btz_png, 0, 1, 1, GRRLIB_WHITE);
				}else{
					GRRLIB_DrawImg(bta_coordX, bta_coordY, tex_bta_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btb_coordX, btb_coordY, tex_btb_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btc_coordX, btc_coordY, tex_btc_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btd_coordX, btd_coordY, tex_btd_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bte_coordX, bte_coordY, tex_bte_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btf_coordX, btf_coordY, tex_btf_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btg_coordX, btg_coordY, tex_btg_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bth_coordX, bth_coordY, tex_bth_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bti_coordX, bti_coordY, tex_bti_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btj_coordX, btj_coordY, tex_btj_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btk_coordX, btk_coordY, tex_btk_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btl_coordX, btl_coordY, tex_btl_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btm_coordX, btm_coordY, tex_btm_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btn_coordX, btn_coordY, tex_btn_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bto_coordX, bto_coordY, tex_bto_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btp_coordX, btp_coordY, tex_btp_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btq_coordX, btq_coordY, tex_btq_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btr_coordX, btr_coordY, tex_btr_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bts_coordX, bts_coordY, tex_bts_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btt_coordX, btt_coordY, tex_btt_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btu_coordX, btu_coordY, tex_btu_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btv_coordX, btv_coordY, tex_btv_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btw_coordX, btw_coordY, tex_btw_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btx_coordX, btx_coordY, tex_btx_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(bty_coordX, bty_coordY, tex_bty_UP_png, 0, 1, 1, GRRLIB_WHITE);
					GRRLIB_DrawImg(btz_coordX, btz_coordY, tex_btz_UP_png, 0, 1, 1, GRRLIB_WHITE);
				}
				



				//other characters and stuff:
				if (Password_uppercase==true){
					GRRLIB_DrawImg(caps_coordX, caps_coordY, tex_caps_colored_png, 0, 1, 1, GRRLIB_WHITE); //show the yellow-colored CAPS button when uppercase==true
				}else{
					GRRLIB_DrawImg(caps_coordX, caps_coordY, tex_caps_png, 0, 1, 1, GRRLIB_WHITE); //else:show the normal one
				}

				GRRLIB_DrawImg(erase_coordX, erase_coordY, tex_erase_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(space_coordX, space_coordY, tex_space_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(bracket_1_coordX, bracket_1_coordY, tex_bracket_1_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(bracket_2_coordX, bracket_2_coordY, tex_bracket_2_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(backslash_coordX, backslash_coordY, tex_backslash_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(slash_coordX, slash_coordY, tex_slash_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(apostrophe_coordX, apostrophe_coordY, tex_apostrophe_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(hash_coordX, hash_coordY, tex_hash_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(comma_coordX, comma_coordY, tex_comma_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(point_coordX, point_coordY, tex_point_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(equal_coordX, equal_coordY, tex_equal_png, 0, 1, 1, GRRLIB_WHITE);
				GRRLIB_DrawImg(semicolon_coordX, semicolon_coordY, tex_semicolon_png, 0, 1, 1, GRRLIB_WHITE);
				
				
				
                GRRLIB_Printf(35, 50, tex_BMfont5, GRRLIB_BLACK, 1, Passwordkeyboard_writtentext); //write on the screen the text that is being written
				GRRLIB_DrawImg(button1CoordX, button1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button1CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Save");
				GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);  // Draw a jpeg
				
				//GRRLIB_Printf(15, 25, tex_BMfont5, GRRLIB_BLACK, 1, "IR.x : %d Ir.y : %d", (int)ir1.x, (int)ir1.y);
				//GRRLIB_Printf(15, 100, tex_BMfont5, GRRLIB_BLACK, 1, "Password_indexkeyboard = %d", Password_indexkeyboard);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//this part determines which character was clicked:
				
	int collision_sizex = 5;
	int collision_sizey = 5;
	
	Passwordkeyboard_writtentext[64] = '\0';

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
		}else if(GRRLIB_RectOnRect(bracket_1_coordX,bracket_1_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=31) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '[';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(bracket_2_coordX,bracket_2_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=31) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = ']';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(backslash_coordX,backslash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=31) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '\\';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(slash_coordX,slash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=31) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '/';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(apostrophe_coordX,apostrophe_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=31) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '\'';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(hash_coordX,hash_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=31) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '#';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(comma_coordX,comma_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=31) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = ',';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(point_coordX,point_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=31) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '.';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(equal_coordX,equal_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=31) { 
				Passwordkeyboard_writtentext[Password_indexkeyboard] = '=';
				Password_indexkeyboard+=1;
				presssound();
			}else maxsound();
			wiirumble_set(0,vib_duration2);
		}else if(GRRLIB_RectOnRect(semicolon_coordX,semicolon_coordY,44,36,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
			if (Password_indexkeyboard<=31) { 
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
				if (Password_indexkeyboard>0 ) { //you don't want that number to get negative lol 		
				Passwordkeyboard_writtentext[Password_indexkeyboard-1] = '\0'; //erases the character
				Password_indexkeyboard-=1;
				erasesound();
				}else maxsound();
				wiirumble_set(0,vib_duration2);				
			} else if(GRRLIB_RectOnRect(space_coordX,space_coordY,space_width,space_height,cursor_positionX, cursor_positionY,collision_sizex,collision_sizey)){
				if (Password_indexkeyboard<=62) { 
					Passwordkeyboard_writtentext[Password_indexkeyboard] = ' '; //space
					Password_indexkeyboard+=1;
					presssound();
				} else maxsound();
			wiirumble_set(0,vib_duration2);
			} else if (collisionButton(button1CoordX, button1CoordY, cursor_positionX,cursor_positionY) == true){ //cancel button					
							buttonsound1();
							wiirumble_set(0,vib_duration1);		
							clearkeyboard(2); //not the cause of crash					
							//free(certBuffer);			
						   //ISFS_Deinitialize();
							
							goto mainmenu; //goes back to the menu without saving
					
			}else if (collisionButton(button3CoordX, button3CoordY, cursor_positionX,cursor_positionY) == true){ //save button				
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						//saving new password:
						for (int i=0;i<=62;i++){certBuffer[PASSWORD1_pos + i] = Passwordkeyboard_writtentext[i];}//replaces password in buf_fileread
			
						ISFS_WRITE_CONFIGDAT(certBuffer); 	
			}	
		} 	

				
				
				GRRLIB_DrawImg(button1CoordX, button1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button1CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Cancel");
				
				GRRLIB_DrawImg(button3CoordX, button3CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button3CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Save");

				
				GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE); 
						
			
    
        GRRLIB_Render();
		
		
		
		
		
	}
	
////////////////////////////////////////////////SECURITY MENU :////////////////////////////////////////////////////////////
	securitymenu:
	int connection1_newsecuritytype = connection1_securitytype;
	while(1){
	
		
		
		updatecursorpos();
	
				GRRLIB_DrawImg(0, 0, tex_settings_background_png, 0, 1, 1, GRRLIB_WHITE);  //shows the keyboard on screen
				
                GRRLIB_Printf(35, 50, tex_BMfont5, GRRLIB_BLACK, 1, Passwordkeyboard_writtentext); //write on the screen the text that is being written
				GRRLIB_DrawImg(button1CoordX, button1CoordY, tex_button1_png, 0, 1, 1, GRRLIB_WHITE);  
				GRRLIB_Printf((button1CoordX+buttonWidth/2)-20, (button1CoordY+buttonHeight/2)-5, tex_BMfont5, GRRLIB_BLACK, 1, "Save");
				GRRLIB_DrawImg(cursor_positionX, cursor_positionY, tex_cursor_png, cursor_rotation_angle, 0.5, 0.5, GRRLIB_WHITE);  
				
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
					if (collisionButton(button1CoordX, button1CoordY, cursor_positionX,cursor_positionY) == true){ //save changes:
						buttonsound1();
						wiirumble_set(0,vib_duration1);
						if (connection1_newsecuritytype != connection1_securitytype){ //we check that the chosen security is different from the already used one. 
							certBuffer[connection1_securitypos] = connection1_newsecuritytype; 
							ISFS_WRITE_CONFIGDAT(certBuffer);
						}
						goto mainmenu;
					}
					else if (collisionButton(buttonsecurity1X, buttonsecurity1Y, cursor_positionX,cursor_positionY) == true){
						connection1_newsecuritytype = 0x00;
						selectsound();
						wiirumble_set(0,vib_duration1);
					} //OPEN
					else if (collisionButton(buttonsecurity2X, buttonsecurity2Y, cursor_positionX,cursor_positionY) == true){
						connection1_newsecuritytype = 0x01;
						selectsound();
						wiirumble_set(0,vib_duration1);
					}//WEP64
					else if (collisionButton(buttonsecurity3X, buttonsecurity3Y, cursor_positionX,cursor_positionY) == true){
						connection1_newsecuritytype = 0x02;
						selectsound();
						wiirumble_set(0,vib_duration1);
					}//WEP128
	    			else if (collisionButton(buttonsecurity4X, buttonsecurity4Y, cursor_positionX,cursor_positionY) == true){
						connection1_newsecuritytype = 0x04;
						selectsound();
						wiirumble_set(0,vib_duration1);
					}//WPA-PSK(TKIP)
			        else if (collisionButton(buttonsecurity5X, buttonsecurity5Y, cursor_positionX,cursor_positionY) == true){
						connection1_newsecuritytype = 0x05;
						selectsound();
						wiirumble_set(0,vib_duration1);
					}//WPA2-PSK(AES)
					else if (collisionButton(buttonsecurity6X, buttonsecurity6Y, cursor_positionX,cursor_positionY) == true){
						connection1_newsecuritytype = 0x06;
						selectsound();
						wiirumble_set(0,vib_duration1);
					}//WPA-PSK(AES) 				 
				}
					
					
					
        GRRLIB_Render();
	}
}