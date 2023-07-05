//audio:
#include <asndlib.h>

#include "audio/button_click2_sound.h"
#include "audio/erase_sound.h"
#include "audio/hoverkey_sound.h"
#include "audio/maxkey_sound.h"
#include "audio/presskey_sound.h"
#include "audio/caps_sound.h"
#include "audio/selectbutton_sound.h"

s32 track;

void buttonsound1(){ASND_SetVoice(track, VOICE_STEREO_16BIT, 44100, 0, button_click2_sound, button_click2_sound_size, 255, 255, NULL);}
void erasesound(){ASND_SetVoice(track, VOICE_STEREO_16BIT, 44100, 0, erase_sound, erase_sound_size, 255, 255, NULL);}
void hoversound(){ASND_SetVoice(track, VOICE_STEREO_16BIT, 44100, 0, hoverkey_sound, hoverkey_sound_size, 255, 255, NULL);}
void maxsound(){ASND_SetVoice(track, VOICE_STEREO_16BIT, 44100, 0, maxkey_sound, maxkey_sound_size, 255, 255, NULL);}
void presssound(){ASND_SetVoice(track, VOICE_STEREO_16BIT, 44100, 0, presskey_sound, presskey_sound_size, 255, 255, NULL);}
void capssound(){ASND_SetVoice(track, VOICE_STEREO_16BIT, 32000, 0, caps_sound, caps_sound_size, 255, 255, NULL);}
void selectsound(){ASND_SetVoice(track, VOICE_STEREO_16BIT, 32000, 0, selectbutton_sound, selectbutton_sound_size, 100, 100, NULL);} //100,100 is the volume, this sound effect is loud so i reduced the audio level