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
