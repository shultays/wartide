#include "neslib.h"

#include "menu.h"



static unsigned char i;
static unsigned char pad, spr;
static unsigned char frame;


static unsigned char craft_x[2];
static unsigned char craft_y[2];

#define DIR_UP 1
#define DIR_RIGHT 2
#define DIR_DOWN 4
#define DIR_LEFT 8

#define DIR_UP_RIGHT (DIR_UP | DIR_RIGHT)
#define DIR_DOWN_RIGHT (DIR_DOWN | DIR_RIGHT)
#define DIR_DOWN_LEFT (DIR_DOWN | DIR_LEFT)
#define DIR_UP_LEFT (DIR_UP | DIR_LEFT)


static unsigned char sprite_dirs[2] = {DIR_UP, DIR_RIGHT};
static unsigned char sprite_look_dirs[2] = {0, 0};
static unsigned char craft_types[2] = {0, 1};
static unsigned char craft_lives[2] = {3, 3};

static unsigned char temp;
static unsigned char temp2;
static unsigned char temp3;
static unsigned char temp4;
static unsigned char temp5;
static unsigned char temp6;


const unsigned char palette[16]={ 
0x29,0x27,0x17,0x07, // mountains
0x29,0x27,0x19,0x18, // grass
0x29,0xF,0x2D,0x3D, // menu
0x29,0x21,0x1C,0xF, // water
}; 


#define CRAFT_BULLET_COUNT 16

static unsigned char craft_bullet_x[CRAFT_BULLET_COUNT];
static unsigned char craft_bullet_y[CRAFT_BULLET_COUNT] = {255};
static unsigned char craft_bullet_flag[CRAFT_BULLET_COUNT]; // dir
static unsigned char craft_bullet_timers[2] = {0, 0};

static int scr = 0;
static unsigned row_index = 0;
static unsigned int adr = 0;

static unsigned char update_list[3+32+3+8+1];//3 bytes address and length for 32 tiles, 3 bytes address and length for 8 attributes, end marker


#define GRASS 0
#define WALL 1
#define WALL_GREEN 3
#define WALL_BIG 5
#define WATER 2

static unsigned char next_line[18] = {GRASS};
static unsigned char next_wall;
static unsigned char current_wall;
static unsigned char prev_wall;
static unsigned char current_line[18] = {GRASS};
static unsigned char prev_line[18] = {GRASS};


static const unsigned char bg_colors[]={
	1,
	0,
	3,
	1,
	0,
    0,
};

unsigned char grand8(){
    return craft_x[0] ^ craft_x[1] ^ frame ^ rand8();
}

static unsigned blocked[15] = {0};
static unsigned bullet_blocked[15] = {0};

#define IS_BLOCKED(x, y) (blocked[y>>4] & (1<<(x>>4)))

unsigned char isFreeIn(unsigned char x, unsigned char y){
    if(scr&15)
    {
        y += (scr&15);
    }
    return !(blocked[y>>4] & (1<<(x>>4)));
}
unsigned char isFreeBulletIn(unsigned char x, unsigned char y){
    if(scr&15)
    {
        y += (scr&15);
    }
    return !(bullet_blocked[y>>4] & (1<<(x>>4)));
}

unsigned char isFree(unsigned char x, unsigned char y, unsigned char dir){
    if(dir == DIR_LEFT) x--;
    else if(dir == DIR_RIGHT) x++;
    
    if(dir == DIR_UP) y--;
    else if(dir == DIR_DOWN) y++;
    
    return isFreeIn(x, y);
    /*if(dir&3)
    {
        return isFreeIn(x, y+1) && isFreeIn(x, y-1) && isFreeIn(x, y);
    }
    else
    {
        return isFreeIn(x+1, y) && isFreeIn(x-1, y) && isFreeIn(x, y);
    }*/
}

void menu(){
    temp = 0;
    temp2 = 0;
	while(1){
		++frame;
		ppu_wait_frame();
        spr = 0;
        pad=pad_poll(0)|pad_poll(1);
        
        if(temp2 != pad){
            temp2 = pad;
            if(pad&PAD_UP){
                --temp;
                if(temp == 255) temp = 2;
            } else if(pad&PAD_DOWN){
                ++temp;
                if(temp == 3) temp = 0;
            } else if(pad&(PAD_A|PAD_B)){
                if(temp == 0){
                    craft_lives[0] = 3;
                    craft_lives[1] = 0;
                    break;
                }else if(temp == 1){
                    craft_lives[0] = 3;
                    craft_lives[1] = 3;
                    break;
                }
            }
        }
        spr=oam_spr(61,   139 + temp * 16, 0x24, 1, spr);
        spr=oam_spr(61+8, 139 + temp * 16, 0x34, 1, spr);
    }
}

void init(){
    oam_size(1);
    bank_spr(0);
    bank_bg(1);
	pal_bg(palette);
	
	vram_adr(NAMETABLE_A);
	vram_unrle(menu_data);
	
    //pal_bg_bright(2);
	
	update_list[0]=NT_UPD_EOF;
	update_list[1]=0x00;
	update_list[2]=32;
    
	update_list[35]=0x20|NT_UPD_HORZ;
	update_list[36]=0x00;
	update_list[37]=8;//length of attribute update sequence

	update_list[46]=NT_UPD_EOF;
    
	set_vram_update(update_list);

    //ppu_mask(172);
    
	ppu_on_all();
    

	frame=0;

    pal_col(17, 0x37);
    pal_col(18, 0x26);
    pal_col(19, 0x17);
                
    pal_col(21, 0x31);
    pal_col(22, 0x22);
    pal_col(23, 0x11);
}
void tick_bullets(){
    for(i=0; i<CRAFT_BULLET_COUNT; ++i){
        if(craft_bullet_y[i] == 255) continue;
        
        temp = craft_bullet_flag[i] & 0xF;
        
        switch(temp){
        case DIR_UP:
            craft_bullet_y[i] -= 3;
        break;
        case DIR_UP_RIGHT:
            craft_bullet_y[i] -= 2;
            craft_bullet_x[i] += 2;
        break;
        case DIR_RIGHT:
            craft_bullet_x[i] += 3;
        break;
        case DIR_DOWN_RIGHT:
            craft_bullet_y[i] += 2;
            craft_bullet_x[i] += 2;
        break;
        case DIR_DOWN:
            craft_bullet_y[i] += 3;
        break;
        case DIR_DOWN_LEFT:
            craft_bullet_y[i] += 2;
            craft_bullet_x[i] -= 2;
        break;
        case DIR_LEFT:
            craft_bullet_x[i] -= 3;
        break;
        case DIR_UP_LEFT:
            craft_bullet_y[i] -= 2;
            craft_bullet_x[i] -= 2;
        break;
        }
        
        if(isFreeBulletIn(craft_bullet_x[i], craft_bullet_y[i]) == FALSE || craft_bullet_x[i] < 5 || craft_bullet_y[i] < 5 || craft_bullet_x[i] >= 250 || craft_bullet_y[i] >= 250){
            craft_bullet_y[i] = 255;
            continue;
        }
        spr=oam_spr(craft_bullet_x[i]-2, craft_bullet_y[i]-2, 0x80, i&1, spr);
        
    }
}
void tick_crafts(){
    for(i=0;i<2;++i){
        if(!craft_lives[i]) continue;
        
        temp = craft_types[i]?0x00:0x40;
        
        switch(sprite_dirs[i]){
        case DIR_UP:
            temp += 0x04;
            temp2 = 0;
        break;
        case DIR_RIGHT:
            temp += 0x24;
            temp2 = 0;
        break;
        case DIR_DOWN:
            temp += 0x04;
            temp2 = OAM_FLIP_V;
        break;
        case DIR_LEFT:
            temp += 0x34;
            temp2 = OAM_FLIP_H;
        break;
        }
        if(sprite_look_dirs[i] == DIR_LEFT){
            temp += 0x08;
        }else if(sprite_look_dirs[i] == DIR_RIGHT){
            temp += 0x04;
        }
        
        pad=pad_poll(i);
        
        if((pad&(PAD_LEFT|PAD_RIGHT|PAD_UP|PAD_DOWN)) && (frame & 8)){
            temp += 2;
        }
        temp2 = i | temp2;
        
        // temp2 ^= (isFree(craft_x[i]+8, craft_y[i]+8) & (frame&1));
        
        spr=oam_spr(craft_x[i]-8, craft_y[i]-8, temp, temp2, spr);
        spr=oam_spr(craft_x[i],   craft_y[i]-8, temp^0x10, temp2, spr);
        
        
        temp2 = (pad&(PAD_UP|PAD_DOWN)) && sprite_dirs[i] != DIR_LEFT && sprite_dirs[i] != DIR_RIGHT;
        temp4 = (pad&(PAD_LEFT|PAD_RIGHT)) && sprite_dirs[i] != DIR_UP && sprite_dirs[i] != DIR_DOWN;
        
        temp3 = 0;
        sprite_look_dirs[i] = 0;
        temp5 = 0;
        if(pad&PAD_LEFT){
            temp5++;
            temp3 |= DIR_LEFT;
            if(temp2){
                sprite_look_dirs[i] = DIR_LEFT;
            }else{
                sprite_dirs[i] = DIR_LEFT;
            }
        } else if(pad&PAD_RIGHT){
            temp5++;
            temp3 |= DIR_RIGHT;
            if(temp2){
                sprite_look_dirs[i] = DIR_RIGHT;
            }else{
                sprite_dirs[i] = DIR_RIGHT;
            }
        } 
        
        if(pad&PAD_UP){
            temp5++;
            temp3 |= DIR_UP;
            if(temp4){
                sprite_look_dirs[i] = DIR_LEFT;
            }else{
                sprite_dirs[i] = DIR_UP;
            }
        } else if(pad&PAD_DOWN){
            temp5++;
            temp3 |= DIR_DOWN;
            if(temp4){
                sprite_look_dirs[i] = DIR_RIGHT;
            }else{
                sprite_dirs[i] = DIR_DOWN;
            }
        }
        if(temp5 == 1 || (frame&3) != 0)
        {
            if((temp3&DIR_LEFT) && isFree(craft_x[i], craft_y[i], DIR_LEFT)){
                craft_x[i]--;
            }
            if((temp3&DIR_RIGHT) && isFree(craft_x[i], craft_y[i], DIR_RIGHT)){
                craft_x[i]++;
            }
        }
        if(temp5 == 1 || (frame&3) != 1)
        {
            if((temp3&DIR_UP) && isFree(craft_x[i], craft_y[i], DIR_UP)){
                craft_y[i]--;
            }
            if((temp3&DIR_DOWN) && ((pad&PAD_A)|| isFree(craft_x[i], craft_y[i], DIR_DOWN))){
                craft_y[i]++;
            }
        }
        
        if(craft_y[i] >= 253-32) craft_y[i] = 253-32;
        
        if(craft_bullet_timers[i] == 0){
            if(pad&PAD_A){
                for(temp=i; temp < CRAFT_BULLET_COUNT; temp += 2){
                    if(craft_bullet_y[temp] != 255) continue;
                    craft_bullet_x[temp] = craft_x[i];
                    craft_bullet_y[temp] = craft_y[i];
                    craft_bullet_flag[temp] = temp3 | sprite_dirs[i];
                    craft_bullet_timers[i] = 8;
                    break;
                }                
            }
        }else{
            --craft_bullet_timers[i];
        }
    }
}

static const char water_tiles[] = {
0xBD,
0x8C,
0xAC,
0xDC,

0xBF,
0x8C,
0x9C,
0xDF,

0xBC,
0x7C,
0xAC,
0xDD,

0xBE,
0x7C,
0x9C,
0xDE,
};


static const char wall_tiles[] = {0x80,
0x84,
0x90,
0x99,
0x82,
0x84,
0xA0,
0x98,
0x70,
0x74,
0x90,
0x9B,
0x72,
0x74,
0xA0,
0x9A};


void scroll_screen(){
    if(temp < 150){
        temp2 = 150 - temp;
        
        for(i=0;i<2;++i){
            if(!craft_lives[i]) continue;
            craft_y[i] += temp2;
            if(craft_y[i] >= 253-32) craft_y[i] = 253-32;
        }
        
        for(i=0; i<CRAFT_BULLET_COUNT; ++i){
            if(craft_bullet_y[i] == 255) continue;
            
            if(craft_bullet_y[i] > 255 - temp2){
                craft_bullet_y[i] = 255;
            }else{
                craft_bullet_y[i] += temp2;
            }
        }
        scr -= temp2;
        if(scr<0) scr+=240*2;
        
        temp = scr>>3;
        if(temp>=60) temp-=60;
        if(row_index != temp){
            row_index = temp;
            if(temp<30){
                adr = NAMETABLE_A+(temp<<5);
                update_list[0]=MSB(adr)|NT_UPD_HORZ;
                update_list[1]=LSB(adr);
                                
                adr=NAMETABLE_A+960+((temp>>2)<<3);
                update_list[35]=MSB(adr)|NT_UPD_HORZ;//set attribute table update address
                update_list[36]=LSB(adr);
            }else{
                temp-=30;

                adr = NAMETABLE_C+(temp<<5);
                update_list[0]=MSB(adr)|NT_UPD_HORZ;
                update_list[1]=LSB(adr);
                                
                                
                adr=NAMETABLE_C+960+((temp>>2)<<3);
                update_list[35]=MSB(adr)|NT_UPD_HORZ;//set attribute table update address
                update_list[36]=LSB(adr);
            }
 
            if(temp&1){
                for(i=2; i<16; i++){
                    prev_line[i] = current_line[i];
                    current_line[i] = next_line[i];
                }
                prev_wall = current_wall;
                current_wall = next_wall;
                next_wall = 0;
                for(i=2; i<16; i++){
                    temp4 = grand8() & 15;
                    if(temp4 < 13) next_line[i] = GRASS;
                    else next_line[i] = WALL;
                }
                for(i=2; i<16; i++){
                    if(next_line[i] == GRASS){
                        if(current_line[i] == WALL_BIG){
                            next_line[i] = WALL;
                        }else{
                            temp4 = ((next_line[i+1]&WALL)<<1)+
                                    ((next_line[i-1]&WALL)<<1)+
                                    ((current_line[i]&WALL)<<1)+
                                    ((current_line[i-1]&WALL))+
                                    ((current_line[i+1]&WALL));
                            if(i==2 || i == 15) temp4 -= 2; 
                            if(temp4 >= 5){
                                next_line[i] = WALL;
                            }else if(temp4 >= 3){
                                if(rand8()&1){
                                    next_line[i] = WALL;
                                }
                            }
                            
                            
                            temp4 = ((next_line[i+1]==WATER)<<1)+
                                    ((next_line[i-1]==WATER)<<1)+
                                    ((current_line[i]==WATER)<<1)+
                                    ((current_line[i-1]==WATER))+
                                    ((current_line[i+1]==WATER));
                            if(temp4 >= 5){
                                next_line[i] = WATER;
                            }else if(temp4 >= 3){
                                if(rand8()&3){
                                    next_line[i] = WATER;
                                }
                            }
                        }
                    } else if(next_line[i] == WALL){
                        temp4 = ((next_line[i+1]&WALL))+
                                ((next_line[i-1]&WALL))+
                                ((current_line[i]&WALL));
                        
                        if(temp4 == 0){
                            if((rand8()&3) != 0){
                                next_line[i] = GRASS;
                            }
                        } 
                    }
                
                    if(next_line[i] & (WALL|WATER)){
                        next_wall++;
                    }
                }
                temp4 = 0;
                if(next_wall > 4) temp4 = next_wall - 4;
                if(next_wall + current_wall > 8){
                    temp4 += next_wall/2;
                }
                if(temp4 > next_wall) temp4 = next_wall;
                next_wall -= temp4;
                while(temp4--){
                    temp3 = 2+rand8()%14;
                    while(1){
                        if(next_line[temp3] & (WALL|WATER)){
                            if(next_line[temp3] != WALL_BIG && current_line[temp3] != WALL_BIG){
                                next_line[temp3] = GRASS; 
                            }
                            break;
                        }
                            
                        temp3++;
                        if(temp3 == 16) temp3 = 2;
                    }
                }
                
                if(next_wall + current_wall <= 2){
                    if(rand8()&1)
                    {
                        temp3 = 3 + (rand8()&7);
                        temp4 = (rand8()&1)+(rand8()&1)+2;
                        next_wall += temp4;
                        for(i=0; i<temp4; i++){
                            next_line[temp3+i] = WALL_BIG;
                        }
                        
                        for(i=temp3+temp4; i<16; i++){
                            if(next_line[i] == WALL &&  i!= 15 && (rand8()&3) == 0){
                                next_line[i] = WALL_BIG;
                            }else{
                                next_line[i] = GRASS;
                                break;
                            }
                        }
                        for(i=temp3-1; i>=2; i--){
                            if(next_line[i] == WALL &&  i!= 2 && (rand8()&3) == 0){
                                next_line[i] = WALL_BIG;
                            }else{
                                next_line[i] = GRASS;
                                break;
                            }
                        }
                    }
                    else
                    {
                        temp3 = 3 + (rand8()&7);
                        temp4 = (rand8()&1)+(rand8()&1)+2;
                        next_wall += temp4;
                        for(i=0; i<temp4; i++){
                            next_line[temp3+i] = WATER;
                        }

                        for(i=temp3+temp4; i<16; i++){
                            if(next_line[i] == WALL &&  i!= 15 && (rand8()&3) != 0){
                                next_line[i] = WATER;
                            }else{
                                next_line[i] = GRASS;
                                break;
                            }
                        }
                        for(i=temp3-1; i>=2; i--){
                            if(next_line[i] == WALL &&  i!= 2 && (rand8()&3) != 0){
                                next_line[i] = WATER;
                            }else{
                                next_line[i] = GRASS;
                                break;
                            }
                        }
                    }
                }
                
                for(i=2; i<16; i++){
                    if(next_line[i] == WALL && current_line[i] == WATER){
                        next_line[i] = WATER;
                    }
                }
                
                for(i=3; i<15; i++){
                    if(next_line[i] == WATER){
                        for(temp3=i+1; temp3<15; temp++){
                            if(next_line[i] == WALL && (rand8()&3) != 0){
                                next_line[i] = WATER;
                            }else{
                                break;
                            }
                        }
                        
                        for(temp3=i-1; temp3>2; temp--){
                            if(next_line[i] == WALL && (rand8()&3) != 0){
                                next_line[i] = WATER;
                            }else{
                                break;
                            }
                        }
                    }
                }
                
                for(i=2; i<16; i++){
                    if(current_line[i] == WALL && 
                    (
                    ((current_line[i-1]==WALL_GREEN) && (current_line[i+1]==WALL_GREEN) && 
                    (next_line[i]==WALL_GREEN) && (prev_line[i]==WALL_GREEN) 
                     && (next_line[i-1]==WALL_GREEN) && (next_line[i+1]==WALL_GREEN) 
                     && (prev_line[i-1]==WALL_GREEN) && (prev_line[i+1]==WALL_GREEN))
                     || 
                     
                    ((current_line[i-1]==WALL) && (current_line[i+1]==WALL) && 
                    (next_line[i]==WALL) && (prev_line[i]==WALL) 
                     && (next_line[i-1]==WALL) && (next_line[i+1]==WALL) 
                     && (prev_line[i-1]==WALL) && (prev_line[i+1]==WALL))
                     
                     )
                    ){
                        current_line[i] = WALL_GREEN;
                    }
                }
                
                for(i=14; i>0; i--)
                {
                    blocked[i] = blocked[i-1];
                    bullet_blocked[i] = bullet_blocked[i-1];
                }
                blocked[0] = 0;
                bullet_blocked[0] = 0;
                for(i=0; i<16; i++)
                {
                    temp4 = i+1;
                    if(current_line[i+1] != GRASS)
                    {
                        if(temp6 && i != 0 && i != 15){
                            current_line[i+1] = GRASS;
                        }else{
                            blocked[0] |= (1<<i);
                        }
                        
                        if(current_line[i+1] != WATER)
                        {
                            bullet_blocked[0] |= (1<<i);
                        }
                    }
                    
                }
            }
            
            for(i=0; i<32; i++){
                temp4 = 1+(i>>1);
                switch(current_line[temp4]){
                    case WALL:
                    case WATER:
                        temp2 = ((((temp&1)==0)<<1)+(i&1));
                        temp5 = current_line[temp4];
                        
                        if(temp2&1){
                            temp3 = (current_line[temp4+1]&temp5)!=0;
                            if(temp2<2){
                                temp3 += (((prev_line[temp4+1]&temp5)!=0)<<2);
                            }else{
                                temp3 += ((next_line[temp4+1]==temp5)<<2);
                            }
                        }else{
                            temp3 = (current_line[temp4-1]&temp5)!=0;
                            if(temp2<2){
                                temp3 += (((prev_line[temp4-1]&temp5)!=0)<<2);
                            }else{
                                temp3 += ((next_line[temp4-1]==temp5)<<2);
                            }
                        }
                        
                        if(temp2<2){
                            temp3 += (((prev_line[temp4]&temp5)!=0)<<1);
                        }else{
                            temp3 += ((next_line[temp4]==temp5)<<1);
                        }
                        
                        if(temp3 == 7){
                            if(rand8()&15){
                                update_list[3+i] = (temp5 == WALL?0:0xEC);
                            }else{
                                update_list[3+i] =  (temp5 == WALL?0x66:0xCC) + (rand8()&3);
                            }
                        }else{
                            if(temp3>=4) temp3 -= 4;
                            if(temp5 == WALL){
                                update_list[3+i] = wall_tiles[(temp2<<2)+temp3];
                            }else{
                                update_list[3+i] = water_tiles[(temp2<<2)+temp3];
                            }
                            if(temp3==1 || temp3 == 2) update_list[3+i] += (rand8()&3);
                            else if(temp3==0 && temp5 == WALL) update_list[3+i]  += (rand8()&1);
                        }
                    break;
                    case WALL_GREEN:
                    case GRASS:
                        temp2 = grand8()&0x3F;
                        if(temp2 > 9){
                            update_list[3+i] = 0;
                        }else{
                            update_list[3+i] = 0x60 + temp2;
                        }
                    break;
                    case WALL_BIG:
                        switch((((temp&1)==0)<<1)+(i&1)){
                            case 0:
                                if(current_line[temp4-1] != WALL_BIG){
                                    update_list[3+i] = 0xA4 + (rand8()&1);
                                }else{
                                    update_list[3+i] = 0x88 + (rand8()&3);
                                }
                            break;
                            case 1:
                                if(current_line[temp4+1] != WALL_BIG){
                                    update_list[3+i] = 0xA6 + (rand8()&1);
                                }else{
                                    update_list[3+i] = 0x88 + (rand8()&3);
                                }
                            break;
                            case 2:
                                if(current_line[temp4-1] != WALL_BIG){
                                    update_list[3+i] = 0x94 + (rand8()&1);
                                }else{
                                    update_list[3+i] = 0x78 + (rand8()&3);
                                }
                            break;
                            case 3:
                                if(current_line[temp4+1] != WALL_BIG){
                                    update_list[3+i] = 0x96 + (rand8()&1);
                                }else{
                                    update_list[3+i] = 0x78 + (rand8()&3);
                                }
                            break;
                        }
                    break;
                }
            }
            
            if( (temp&1) != 0){
                if( temp == 29){
                    for(i=0;i<8;++i){
                        update_list[38+i] = (bg_colors[current_line[1 + (i<<1)]] | (bg_colors[current_line[1 + (i<<1)+1]]<<2));
                    }
                }else if( (temp&3) == 3 ){
                    for(i=0;i<8;++i){
                        update_list[38+i] = (bg_colors[current_line[1 + (i<<1)]] | (bg_colors[current_line[1 + (i<<1)+1]]<<2))<<4;
                    }
                }else{
                    for(i=0;i<8;++i){
                        update_list[38+i] += (bg_colors[current_line[1 + (i<<1)]] | (bg_colors[current_line[1 + (i<<1)+1]]<<2));
                    }
                }
            }
        }
        
        scroll(0, scr);
    }
    
}

void reset(){
	craft_x[0]=78;
	craft_y[0]=180;
	craft_x[1]=178;
	craft_y[1]=180;
    
    sprite_dirs[0] = DIR_UP;
    sprite_dirs[1] = DIR_UP;
    
    next_line[0] = next_line[1] = WALL;
    next_line[16] = next_line[17] = WALL;
    
    current_line[0] = current_line[1] = WALL;
    current_line[16] = current_line[17] = WALL;
    
    prev_line[0] = prev_line[1] = WALL;
    prev_line[16] = prev_line[17] = WALL;
    
    next_wall = 0;
    current_wall = 0;
    prev_wall = 0;
}
void main(void)
{
	init();
    
    menu();
    
    reset();
    oam_clear();
    while(scr!=240){
		ppu_wait_frame();
        temp = 146;
        temp6 = scr > 400;
        scroll_screen();
		++frame;
	}
	craft_x[0]=78;
	craft_y[0]=200;
	craft_x[1]=178;
	craft_y[1]=200;
    
	while(1){
		ppu_wait_frame();
        oam_clear();
		spr=0;
        tick_crafts();
        tick_bullets();
            
        temp = 255;
        if(craft_lives[0] && temp > craft_y[0]) temp = craft_y[0];
        if(craft_lives[1] && temp > craft_y[1]) temp = craft_y[1];
        scroll_screen();
		++frame;
	}
}