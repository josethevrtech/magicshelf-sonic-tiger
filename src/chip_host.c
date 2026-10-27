/* Sonic wiring adapter. CPU remains the pinned bzhxx/hap implementation. */
#include "gw_type_defs.h"
#include "gw_romloader.h"
#include "gw_system.h"
#include "sm510.h"
gwromheader_t gw_head;
unsigned char *gw_program,*gw_melody;
unsigned int keyboard_storage[10],*gw_keyboard=keyboard_storage;
unsigned int sonic_buttons;
short sonic_audio[2048];
unsigned sonic_audio_count;
unsigned int gw_get_buttons(){return sonic_buttons;}
unsigned char gw_readK(unsigned char s){
 unsigned k=(sonic_buttons&128)?8:0;
 if(s&2)if(sonic_buttons&2)k|=4;
 if(s&4){if(sonic_buttons&4)k|=2;if(sonic_buttons&1)k|=4;}
 if(s&8)if(sonic_buttons&8)k|=2;
 if(s&16)if(sonic_buttons&16)k|=1;
 if(s&32)if(sonic_buttons&32)k|=1;
 return k;
}
unsigned char gw_readBA(){return !(sonic_buttons&64);}
unsigned char gw_readB(){return !(sonic_buttons&256);}
void gw_writeR(unsigned char r){
 /* MAME tsonic two-bit DAC: R1 + 2*S1, sampled at 32768 Hz. */
 if(sonic_audio_count<2048)sonic_audio[sonic_audio_count++]=(((r&1)+2*(m_s_out&1))*6000)-9000;
}
void gw_gfx_init(){}
void gw_gfx_sm500_rendering(unsigned short*p){(void)p;}
void gw_gfx_sm510_rendering(unsigned short*p){(void)p;}
bool gw_romloader(){return false;}
