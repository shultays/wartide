#include "neslib.h"

#include "menu.h"


#define DIR_UP 1
#define DIR_RIGHT 2
#define DIR_DOWN 4
#define DIR_LEFT 8

#define DIR_UP_RIGHT (DIR_UP | DIR_RIGHT)
#define DIR_DOWN_RIGHT (DIR_DOWN | DIR_RIGHT)
#define DIR_DOWN_LEFT (DIR_DOWN | DIR_LEFT)
#define DIR_UP_LEFT (DIR_UP | DIR_LEFT)

static unsigned char i;
static unsigned char pad, spr;
static unsigned char frame;

static unsigned char craft_x[6];
static unsigned char craft_y[6];

static unsigned char sprite_dirs[6] = {DIR_UP, DIR_UP, DIR_DOWN, DIR_DOWN, DIR_DOWN, DIR_DOWN};
static unsigned char sprite_look_dirs[6] = {0, 0, 0, 0, 0, 0};
static unsigned char craft_types[6] = {0, 1, 1, 1, 1, 1};
static unsigned char craft_hps[6] = {0, 1, 1, 1, 1, 1};

static unsigned char craft_flags[6] = {0, 0, 0, 0, 0, 0};

static unsigned char enemy_spawn_scr;

static unsigned char wall_hit_x[2];
static unsigned char wall_hit_y[2];
static unsigned char wall_hit_hp[2];

static unsigned char craft_lives[2] = {3, 3};

static unsigned char temp;
static unsigned char temp2;
static unsigned char temp3;
static unsigned char temp4;
static unsigned char temp5;
static unsigned char temp6;

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

#define MAX_Y (253-16)
#define CRAFT_BULLET_COUNT 16
#define ENEMY_BULLET_COUNT 24

static unsigned char craft_bullet_x[ENEMY_BULLET_COUNT];
static unsigned char craft_bullet_y[ENEMY_BULLET_COUNT] = {255};
static unsigned char craft_bullet_flag[ENEMY_BULLET_COUNT]; // dir
static unsigned char craft_bullet_timers[6] = {0, 0};

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

static unsigned int blocked[15] = {0};
static unsigned int bullet_blocked[15] = {0};

#define IS_BLOCKED(x, y) (blocked[y>>4] & (1<<(x>>4)))

unsigned char isCellBulletFree(unsigned char i, unsigned char j){
    return !(bullet_blocked[j] & (1<<(i)));
}

unsigned char isFreeIn(unsigned char x, unsigned char y){
    y += (scr&15);
    return !(blocked[y>>4] & (1<<(x>>4)));
}

unsigned char isFree(unsigned char x, unsigned char y, unsigned char dir){
    if(dir == DIR_LEFT) x-=3;
    else if(dir == DIR_RIGHT) x+=3;
    
    if(dir == DIR_UP) y-=3;
    else if(dir == DIR_DOWN) y+=3;
    
    if(dir&3)
    {
        return isFreeIn(x, y+2) && isFreeIn(x, y-2)/* && isFreeIn(x, y)*/;
    }
    else
    {
        return isFreeIn(x+2, y) && isFreeIn(x-2, y)/* && isFreeIn(x, y)*/;
    }
}

unsigned char damage_craft(unsigned char index, unsigned char damage){
    if(craft_hps[index] > damage){
        craft_hps[index]  -= damage;
        return 0;
    }
    craft_hps[index] = 0;
    return 1;
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
            } else if(pad&(PAD_A|PAD_B|PAD_START|PAD_SELECT)){
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

void draw_tank(){
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
    
    if(i < 2){
        if((pad&(PAD_LEFT|PAD_RIGHT|PAD_UP|PAD_DOWN)) && (frame & 8)){
            temp += 2;
        }
        temp2 = i | temp2;
    }else{
        temp2 |= 2;
        if(frame & 8){
            temp += 2;
        }
    }
    
    spr=oam_spr(craft_x[i]-8, craft_y[i]-8, temp, temp2, spr);
    spr=oam_spr(craft_x[i],   craft_y[i]-8, temp^0x10, temp2, spr);
}

void init(){
    oam_size(1);
    bank_spr(0);
    bank_bg(1);
	pal_all(palette);
	
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

}
void tick_bullets(){
    for(i=0; i<ENEMY_BULLET_COUNT; ++i){
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
        if(craft_bullet_y[i] < 10 || craft_bullet_x[i] >= 245 || craft_bullet_y[i] >= 245 || craft_bullet_y[i] < 10){
            craft_bullet_y[i] = 255;
            continue;
        }
        temp2 = (craft_bullet_y[i] + (scr&15))>>4;
        temp3 = craft_bullet_x[i]>>4;
        temp = isCellBulletFree(temp3, temp2) == FALSE;
        if(i<CRAFT_BULLET_COUNT)
        {
            if(temp)
            {
                if(wall_hit_hp[i&1] == 0 ||(wall_hit_x[i&1]&15) != temp3 || (wall_hit_y[i&1]&15) != temp2)
                {
                    wall_hit_x[i&1] = (wall_hit_x[i&1]&0xF0) | temp3;
                    wall_hit_y[i&1] = (wall_hit_y[i&1]&0xF0) | temp2;
                    wall_hit_hp[i&1] = 10;
                }
                wall_hit_hp[i&1]--;
                if(wall_hit_hp[i&1] == 0 && temp3 != 0 && temp3 != 15)
                {
                
                    temp6 = 1;
                    bullet_blocked[temp2] ^= (1<<temp3);
                    blocked[temp2] ^= (1<<temp3);
                    temp = row_index;
                    if(temp&1) temp--;
                    temp +=(temp2<<1);
                    if(temp>=60) temp-=60;
                    if(temp<30){
                        adr = NAMETABLE_A+(temp<<5);
                    }else{
                        temp-=30;
                        adr = NAMETABLE_C+(temp<<5);
                    }
                    adr += temp3<<1;
                    update_list[0]=MSB(adr)|NT_UPD_HORZ;
                    update_list[1]=LSB(adr);
                    
                    update_list[2] = 2;
                    update_list[5]=NT_UPD_EOF;
                    
                    adr += 32;
                    update_list[5]=MSB(adr)|NT_UPD_HORZ;
                    update_list[6]=LSB(adr);
                    update_list[7] = 2;
                    
                    
                    update_list[10]=NT_UPD_EOF;
                    
                    temp5 = 0;
                    if(temp2)
                        temp5 |= (!isCellBulletFree(temp3, temp2-1) || (((wall_hit_x[i&1])>>4) == temp3 && ((wall_hit_y[i&1])>>4) == temp2-1));
                    if(temp3)
                        temp5 |= (!isCellBulletFree(temp3-1, temp2) || (((wall_hit_x[i&1])>>4) == temp3-1 && ((wall_hit_y[i&1])>>4) == temp2))<<1;
                        
                    if(temp2<14)
                        temp5 |= (!isCellBulletFree(temp3, temp2+1) || (((wall_hit_x[i&1])>>4) == temp3 && ((wall_hit_y[i&1])>>4) == temp2+1))<<2;
                    if(temp3<15)
                        temp5 |= (!isCellBulletFree(temp3+1, temp2) || (((wall_hit_x[i&1])>>4) == temp3+1 && ((wall_hit_y[i&1])>>4) == temp2))<<3;
                        
                    // 1 up
                    // 2 left
                    // 4 bottom
                    // 8 right
                    
                    // update_list[3] = 0xB0;
                    // update_list[4] = 0xB1;
                    // update_list[8] = 0xC0;
                    // update_list[9] = 0xC1;
                        
                    
                    if((temp5 & 3) == 3)
                        update_list[3] = 0xB0;
                    else if(temp5 & 1)
                        update_list[3] = 0xB4;
                    else if(temp5 & 2)
                        update_list[3] = 0xB2;
                    else 
                        update_list[3] = 0;
                        
                    if((temp5 & 9) == 9)
                        update_list[4] = 0xB1;
                    else if(temp5 & 1)
                        update_list[4] = 0xB5;
                    else if(temp5 & 8)
                        update_list[4] = 0xB3;
                    else 
                        update_list[4] = 0;
                    
                    
                    if((temp5 & 6) == 6)
                        update_list[8] = 0xC0;
                    else if(temp5 & 4)
                        update_list[8] = 0xC4;
                    else if(temp5 & 2)
                        update_list[8] = 0xC2;
                    else 
                        update_list[8] = 0;
                        
                    if((temp5 & 12) == 12)
                        update_list[9] = 0xC1;
                    else if(temp5 & 4)
                        update_list[9] = 0xC5;
                    else if(temp5 & 8)
                        update_list[9] = 0xC3;
                    else 
                        update_list[9] = 0;
                        
                        
                    wall_hit_x[i&1] <<= 4;
                    wall_hit_y[i&1] <<= 4;
                }
                
                craft_bullet_y[i] = 255;
                continue;
            }
            
            for(temp2=2; temp2<6; temp2++){
                if(craft_types[temp2] != 255){
                    if(craft_bullet_x[i] > craft_x[temp2]-6 && craft_bullet_x[i] < craft_x[temp2]+6 && craft_bullet_y[i] > craft_y[temp2]-6 && craft_bullet_y[i] < craft_y[temp2]+6){
                        damage_craft(temp2, 1);
                        craft_bullet_y[i] = 255;
                        break;
                    }
                }
            }
        
        }
        else
        {
            if(temp)
            {
                craft_bullet_y[i] = 255;
                continue;
            }
            for(temp2=0; temp2<2; temp2++){
                if(craft_lives[temp2] > 0){
                    if(craft_bullet_x[i] > craft_x[temp2]-6 && craft_bullet_x[i] < craft_x[temp2]+6 && craft_bullet_y[i] > craft_y[temp2]-6 && craft_bullet_y[i] < craft_y[temp2]+6){
                        damage_craft(temp2, 1);
                        craft_bullet_y[i] = 255;
                        continue;
                    }
                }
            }
        }
    
        if(craft_bullet_y[i] != 255)
            spr=oam_spr(craft_bullet_x[i]-2, craft_bullet_y[i]-2, 0x80, i<CRAFT_BULLET_COUNT?i&1:2, spr);
        
    }
}
void tick_crafts(){
    for(i=0;i<2;++i){

        if(!craft_lives[i]) continue;
      
        if(craft_hps[i] == 0)
        {
            craft_hps[i] = 8;
            craft_lives[i]--;
        }
        pad=pad_poll(i);
        
        draw_tank();
        
        temp2 = (craft_hps[i]&254);
        if((craft_hps[i]&1) && !(frame&16)){
            temp2 += 2;
        }
        spr=oam_spr(i?256-20-8:20, 220, 0xA0+temp2, i&1, spr);
        
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
            if((temp3&DIR_DOWN) && isFree(craft_x[i], craft_y[i], DIR_DOWN)){
                craft_y[i]++;
            }
        }
        
        if(craft_y[i] >= MAX_Y) craft_y[i] = MAX_Y;
        
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
        if(enemy_spawn_scr > temp2){
            enemy_spawn_scr -= temp2;
        }else{
            enemy_spawn_scr = 0;
        }
        for(i=0;i<6;++i){
            craft_y[i] += temp2;
            if(craft_y[i] >= MAX_Y+1) craft_y[i] = MAX_Y+1;
        }
        
        for(i=0; i<ENEMY_BULLET_COUNT; ++i){
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
            update_list[2]=32;
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
                if(wall_hit_hp[0]){
                    wall_hit_y[0] += 16;
                }
                if(wall_hit_hp[1]){
                    wall_hit_y[1] += 16;
                }
            
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
                            if(current_line[i+1] != WATER)
                            {
                                bullet_blocked[0] |= (1<<i);
                            }
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
    
    wall_hit_x[0] = 255;
    wall_hit_x[1] = 255;
    wall_hit_hp[0] = 0;
    wall_hit_hp[1] = 0;
    
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

void tick_enemies(){
    for(i=2; i<6; i++){
        if(craft_types[i] == 255){
            if(enemy_spawn_scr == 0){
                enemy_spawn_scr = 32 + (rand8()&31);
                temp = rand8()&15;
                while((blocked[0] & (1<<temp))){
                    temp++;
                    if(temp == 16) temp = 0;
                }                    
                
                craft_x[i] = temp*16 + (rand8()&7);
                craft_y[i] = 0;
                sprite_dirs[i] = DIR_DOWN;
                sprite_look_dirs[i] = 0;
                craft_types[i] = 1;
                craft_flags[i] = 15;
                craft_hps[i] = 2;
                craft_bullet_timers[i] = rand8()&127;
            }
        }
        else
        {
            temp = (sprite_dirs[i]>0)+(sprite_look_dirs[i]>0);
            temp2 = sprite_dirs[i];
            temp3 = craft_x[i];
            temp4 = craft_y[i];
            
            if(temp == 1 || (frame&3) != 0){
                if(temp2&DIR_LEFT){
                    craft_x[i]--;
                }
                if(temp2&DIR_RIGHT){
                    craft_x[i]++;
                }
            }
            if(temp == 1 || (frame&3) != 1){
                if(temp2&DIR_UP){
                    craft_y[i]--;
                }
                if(temp2&DIR_DOWN){
                    craft_y[i]++;
                }
            }
            temp2 = craft_flags[i]&15;
            if(temp2 && (frame & 7) == 0){
                craft_flags[i]--;
            }
            if(temp2 == 0 || (craft_y[i] < 20 && sprite_dirs[i] == DIR_UP) || (craft_y[i] > 220 && sprite_dirs[i] == DIR_DOWN)|| isFreeIn(craft_x[i], craft_y[i]) == FALSE){
                craft_x[i] = temp3;
                craft_y[i] = temp4;
                
                temp3 = 0;
                temp3 |= sprite_dirs[i];
                
                if(isFreeIn(craft_x[i], craft_y[i]-1) == FALSE){
                    temp3 |= DIR_UP;
                }
                if(isFreeIn(craft_x[i], craft_y[i]+1) == FALSE){
                    temp3 |= DIR_DOWN;
                }
                if(isFreeIn(craft_x[i]-1, craft_y[i]) == FALSE){
                    temp3 |= DIR_LEFT;
                }
                if(isFreeIn(craft_x[i]+1, craft_y[i]) == FALSE){
                    temp3 |= DIR_RIGHT;
                }
                
                if(craft_y[i] < 20){
                    temp3 |= DIR_UP;
                }
                if(craft_y[i] > 220){
                    temp3 |= DIR_DOWN;
                }
                craft_flags[i] = (craft_flags[i]&0xF0) + 4 + (rand8()&11);
                
                if(temp3 == 0xF){
                    craft_types[i] = 255;
                    continue;
                }else if(temp3 == 0){
                    sprite_dirs[i] = (1<<(rand8()&3));
                } else {
                    temp2 = rand8()&3;
                    
                    do{
                        sprite_dirs[i] = (1<<(rand8()&3));
                    }while(sprite_dirs[i]&temp3);
                }
                
            }
            if(craft_bullet_timers[i])
            {
                craft_bullet_timers[i]--;
            }
            
            if(craft_bullet_timers[i] == 0)
            {
                for(temp=CRAFT_BULLET_COUNT; temp < ENEMY_BULLET_COUNT; temp++){
                    if(craft_bullet_y[temp] != 255) continue;
                    craft_bullet_x[temp] = craft_x[i];
                    craft_bullet_y[temp] = craft_y[i];
                    craft_bullet_flag[temp] = sprite_dirs[i];
                    craft_bullet_timers[i] = 64 + (rand8()&127);
                    break;
                }   
            }
            
            if(craft_y[i] >= MAX_Y-1 || craft_hps[i] == 0){
                craft_types[i] = 255;
                continue;
            }
            for(temp2=0; temp2<2; temp2++){
                if(craft_lives[temp2] != 0){
                    if(craft_x[i] > craft_x[temp2]-12 && craft_x[i] < craft_x[temp2]+12 && craft_y[i] > craft_y[temp2]-12 && craft_y[i] < craft_y[temp2]+12){
                        damage_craft(temp2, 2);
                        craft_types[i] = 255;
                        break;
                    }
                }
            }
            if(craft_types[i] != 255)
                draw_tank();
        }
    }

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
    temp6 = 0;
	craft_x[0]=78;
	craft_y[0]=200;
	craft_x[1]=178;
	craft_y[1]=200;

    craft_types[2] = 255;
    craft_types[3] = 255;
    craft_types[4] = 255;
    craft_types[5] = 255;
    craft_hps[0] = 8;
    craft_hps[1] = 8;
    enemy_spawn_scr = 10;
    
	while(1){
		ppu_wait_frame();
        oam_clear();
		spr=0;
        tick_crafts();
        tick_enemies();
        tick_bullets();
            
        temp = 255;
        if(craft_lives[0] && temp > craft_y[0]) temp = craft_y[0];
        if(craft_lives[1] && temp > craft_y[1]) temp = craft_y[1];
        if(temp6){
            temp6 = 0;
        }
        else
        {
            scroll_screen();
        }
		++frame;
	}
}