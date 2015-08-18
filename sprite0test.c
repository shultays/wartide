#include "neslib.h"

#include "menu.h"


const unsigned char palette[]={ 
0x29,0x27,0x17,0x07, // mountains
0x29,0x27,0x19,0x18, // grass
0x29,0xF,0x2D,0x3D, // menu
0x29,0x21,0x1C,0xF, // water

0x29, 0x37, 0x26, 0x17,
0x29, 0x31, 0x22, 0x11,
0x29, 0x33, 0x23, 0x13,
0x29, 0xF, 0xF, 0xF,
}; 


void init(){
    oam_size(1);
    bank_spr(0);
    bank_bg(1);
	pal_all(palette);
	
	vram_unrle(menu_data);
	
	ppu_on_all();
}


static unsigned char tt = 0;

static unsigned char i = 0;

void main(void)
{
	init();
    
    oam_spr(10, 31, 61, 0, 0);
	while(1){
    
        scroll(0, 0);

		ppu_wait_nmi();//not ppu_wait_frame, because every 6th frame would not have the split
        split_vertical(tt);

		tt++;
        
	}
}