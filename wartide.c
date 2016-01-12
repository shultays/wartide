#include "neslib.h"

#include "defines.h"
#include "debugger.h"
#include "temp_control.h"
#include "menu.h"

#pragma bssseg(push,"ZEROPAGE")
#pragma dataseg(push,"ZEROPAGE")
static unsigned char i, j;

static unsigned char craft_x[6];
static unsigned char craft_y[6];

static unsigned int blocked[15] = {0};
static unsigned int bullet_blocked[15] = {0};

#pragma bssseg(push,"BSS")
#pragma dataseg(push,"BSS")

static unsigned char spr;
static unsigned char frame;

static unsigned char update_list[3+32+3+8+1];//3 bytes address and length for 32 tiles, 3 bytes address and length for 8 attributes, end marker

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

static unsigned char wall_count;
static unsigned char has_big_wall;


static unsigned char craft_bullet_x[ENEMY_BULLET_COUNT];
static unsigned char craft_bullet_y[ENEMY_BULLET_COUNT] = {255};
static unsigned char craft_bullet_flag[ENEMY_BULLET_COUNT]; // dir
static unsigned char craft_bullet_timers[6] = {0, 0};

static int scr = 0;
static unsigned last_row_index = 0;
static unsigned int adr = 0;

static unsigned char next_line[18] = {GRASS};
static unsigned char current_line[18] = {GRASS};
static unsigned char prev_line[18] = {GRASS};

static unsigned char dont_change_bg_pallette;

#define IS_BLOCKED(x, y) (blocked[y>>4] & (1<<(x>>4)))

unsigned char __fastcall__ isCellBulletFree(unsigned char i, unsigned char j){
    return !(bullet_blocked[j] & (1<<(i)));
}

#define isFreeIn(x, y) (!(blocked[y + (scr&15)>>4] & (1<<(x>>4))))

void menu(void){
    CHECK_FREE(0);
    #define selected_item temp0
    
    CHECK_FREE(1);
    #define old_pad temp1
    
    CHECK_FREE(2);
    #define pad temp2
    
    selected_item = 0;
    old_pad = 0;
	while(1){
		++frame;
		ppu_wait_frame();
        spr = 0;
        pad=pad_poll(0)|pad_poll(1);
        
        if(old_pad != pad){
            old_pad = pad;
            if(pad&PAD_UP){
                --selected_item;
                if(selected_item == 255) selected_item = 2;
            } else if(pad&PAD_DOWN){
                ++selected_item;
                if(selected_item == 3) selected_item = 0;
            } else if(pad&(PAD_A|PAD_B|PAD_START|PAD_SELECT)){
                if(selected_item == 0){
                    craft_lives[0] = 3;
                    craft_lives[1] = 0;
                    craft_types[1] = 255;
                    break;
                }else if(selected_item == 1){
                    craft_lives[0] = 3;
                    craft_lives[1] = 3;
                    break;
                }
            }
        }
        spr=oam_spr(61,   139 + selected_item * 16, 0x24, 1, spr);
        spr=oam_spr(61+8, 139 + selected_item * 16, 0x34, 1, spr);
    }
    
    #undef pad
    SET_FREE(2);
    #undef old_pad
    SET_FREE(1);
    #undef selected_item
    SET_FREE(0);
}

void draw_tank(void){
    CHECK_FREE(0);
    #define craft_sprite temp0
    
    CHECK_FREE(1);
    #define craft_sprite_prop temp1
    
    craft_sprite = craft_types[i]?0x00:0x40;
    
    switch(sprite_dirs[i]){
    case DIR_UP:
        craft_sprite += 0x04;
        craft_sprite_prop = 0;
    break;
    case DIR_RIGHT:
        craft_sprite += 0x24;
        craft_sprite_prop = 0;
    break;
    case DIR_DOWN:
        craft_sprite += 0x04;
        craft_sprite_prop = OAM_FLIP_V;
    break;
    case DIR_LEFT:
        craft_sprite += 0x34;
        craft_sprite_prop = OAM_FLIP_H;
    break;
    }
    if(sprite_look_dirs[i] == DIR_LEFT){
        craft_sprite += 0x08;
    }else if(sprite_look_dirs[i] == DIR_RIGHT){
        craft_sprite += 0x04;
    }
    
    if(i < 2){
        if((frame & 8) == (i?8:0)&& (pad_poll(i)&(PAD_LEFT|PAD_RIGHT|PAD_UP|PAD_DOWN))){
            craft_sprite += 2;
        }
        craft_sprite_prop = i | craft_sprite_prop;
    }else{
        craft_sprite_prop |= 2;
        if(frame & 8){
            craft_sprite += 2;
        }
    }
    
    spr=oam_spr(craft_x[i]-8, craft_y[i]-8, craft_sprite, craft_sprite_prop, spr);
    spr=oam_spr(craft_x[i],   craft_y[i]-8, craft_sprite^0x10, craft_sprite_prop, spr);
    
    #undef craft_sprite_prop
    SET_FREE(1);
    #undef craft_sprite
    SET_FREE(0);
}

void draw_all(void){
    for(i=0;i<6; i++){
        if(craft_types[i] == 255) continue;
        draw_tank();
        
        if(i<2){
        
            CHECK_FREE(1);
            #define craft_hp_sprite temp1
            
            craft_hp_sprite = (craft_hps[i]&254);
            if((craft_hps[i]&1) && !(frame&16)){
                craft_hp_sprite += 2;
            }
            spr=oam_spr(i?256-20-8:20, 210, 0xA0+craft_hp_sprite, i, spr);
                    
                    
            #undef craft_hp_sprite
            SET_FREE(1);
            
        }
    }
    
    
    for(i=0; i<ENEMY_BULLET_COUNT; ++i){
        if(craft_bullet_y[i] == 255) continue;
         spr=oam_spr(craft_bullet_x[i]-2, craft_bullet_y[i]-2, 0x80, i<CRAFT_BULLET_COUNT?i&1:2, spr);
    }
}

void init(void){
    oam_size(1);
    bank_spr(0);
    bank_bg(1);
    DEBUG_SET(255);
    DEBUG_SET(0);
	pal_all(palette);
	
	vram_adr(NAMETABLE_A);
	vram_unrle(menu_data);
	
	
	update_list[0]=NT_UPD_EOF;
	update_list[1]=0x00;
	update_list[2]=32;
    
	update_list[35]=0x20|NT_UPD_HORZ;
	update_list[36]=0x00;
	update_list[37]=8;//length of attribute update sequence

	update_list[46]=NT_UPD_EOF;
    
	set_vram_update(update_list);

    
	ppu_on_all();
    
	frame=0;
}
void tick_bullets(void){
    if(pad_poll(0)&PAD_B) TIMER_ENABLE(0);
    
    TIMER_BEGIN(0);
    
    CHECK_FREE(1);
    #define bullet_grid_pos_y temp1

    CHECK_FREE(2);
    #define bullet_grid_pos_x temp2
    
    CHECK_FREE(4);
    #define has_collision temp4
    
    CHECK_FREE(6);
    #define bullet_x temp0
    
    CHECK_FREE(3);
    #define bullet_y temp3
    
    for(i=0; i<ENEMY_BULLET_COUNT; ++i){
        if(craft_bullet_y[i] == 255) continue;
            
        bullet_x = craft_bullet_x[i];
        bullet_y = craft_bullet_y[i];
        
        switch(craft_bullet_flag[i]){ // bullet_dir
        case DIR_UP:
            bullet_y -= 3;
        break;
        case DIR_UP_RIGHT:
            bullet_y -= 2;
            bullet_x += 2;
        break;
        case DIR_RIGHT:
            bullet_x += 3;
        break;
        case DIR_DOWN_RIGHT:
            bullet_y += 2;
            bullet_x += 2;
        break;
        case DIR_DOWN:
            bullet_y += 3;
        break;
        case DIR_DOWN_LEFT:
            bullet_y += 2;
            bullet_x -= 2;
        break;
        case DIR_LEFT:
            bullet_x -= 3;
        break;
        case DIR_UP_LEFT:
            bullet_y -= 2;
            bullet_x -= 2;
        break;
        }
        if(bullet_x < 10 || bullet_x >= 245 || bullet_y >= 245 || bullet_y < 10){
            craft_bullet_y[i] = 255;
            continue;
        }
        
        
        bullet_grid_pos_y = (bullet_y + (scr&15))>>4;
        bullet_grid_pos_x = bullet_x>>4;
        
        
        has_collision = (bullet_blocked[bullet_grid_pos_y] & (1<<(bullet_grid_pos_x)))>0;
        
        if(i<CRAFT_BULLET_COUNT)
        {
            if(has_collision){
                if(wall_hit_hp[i&1] == 0 ||(wall_hit_x[i&1]&15) != bullet_grid_pos_x || (wall_hit_y[i&1]&15) != bullet_grid_pos_y)
                {
                    wall_hit_x[i&1] = (wall_hit_x[i&1]&0xF0) | bullet_grid_pos_x;
                    wall_hit_y[i&1] = (wall_hit_y[i&1]&0xF0) | bullet_grid_pos_y;
                    wall_hit_hp[i&1] = 5;
                }
                wall_hit_hp[i&1]--;
                if(wall_hit_hp[i&1] == 0 && bullet_grid_pos_x != 0 && bullet_grid_pos_x != 15){
                    bullet_blocked[bullet_grid_pos_y] ^= (1<<bullet_grid_pos_x);
                    blocked[bullet_grid_pos_y] ^= (1<<bullet_grid_pos_x);
                    
                    
                                    
                    CHECK_FREE(0);
                    #define row_index_on_ns temp0
                    
                    CHECK_FREE(5);
                    #define collision_edge_data temp5
                    
                    row_index_on_ns = last_row_index;
                    if(row_index_on_ns&1) row_index_on_ns--;
                    row_index_on_ns +=(bullet_grid_pos_y<<1);
                    if(row_index_on_ns>=60) row_index_on_ns-=60;
                    if(row_index_on_ns<30){
                        adr = NAMETABLE_A+(row_index_on_ns<<5);
                    }else{
                        row_index_on_ns-=30;
                        adr = NAMETABLE_C+(row_index_on_ns<<5);
                    }
                    adr += bullet_grid_pos_x<<1;
                    update_list[0]=MSB(adr)|NT_UPD_HORZ;
                    update_list[1]=LSB(adr);
                    
                    update_list[2] = 2;
                    update_list[5]=NT_UPD_EOF;
                    
                    adr += 32;
                    update_list[5]=MSB(adr)|NT_UPD_HORZ;
                    update_list[6]=LSB(adr);
                    update_list[7] = 2;
                    
                    update_list[10]=NT_UPD_EOF;
                    
                    collision_edge_data = 0;
                    if(bullet_grid_pos_y)
                        collision_edge_data |= (!isCellBulletFree(bullet_grid_pos_x, bullet_grid_pos_y-1) || (((wall_hit_x[i&1])>>4) == bullet_grid_pos_x && ((wall_hit_y[i&1])>>4) == bullet_grid_pos_y-1));
                    if(bullet_grid_pos_x)
                        collision_edge_data |= (!isCellBulletFree(bullet_grid_pos_x-1, bullet_grid_pos_y) || (((wall_hit_x[i&1])>>4) == bullet_grid_pos_x-1 && ((wall_hit_y[i&1])>>4) == bullet_grid_pos_y))<<1;
                        
                    if(bullet_grid_pos_y<14)
                        collision_edge_data |= (!isCellBulletFree(bullet_grid_pos_x, bullet_grid_pos_y+1) || (((wall_hit_x[i&1])>>4) == bullet_grid_pos_x && ((wall_hit_y[i&1])>>4) == bullet_grid_pos_y+1))<<2;
                    if(bullet_grid_pos_x<15)
                        collision_edge_data |= (!isCellBulletFree(bullet_grid_pos_x+1, bullet_grid_pos_y) || (((wall_hit_x[i&1])>>4) == bullet_grid_pos_x+1 && ((wall_hit_y[i&1])>>4) == bullet_grid_pos_y))<<3;
                        
                    // 1 up
                    // 2 left
                    // 4 bottom
                    // 8 right
                    
                    // update_list[3] = 0xB0;
                    // update_list[4] = 0xB1;
                    // update_list[8] = 0xC0;
                    // update_list[9] = 0xC1;
                        
                    
                    if((collision_edge_data & 3) == 3)
                        update_list[3] = 0xB0;
                    else if(collision_edge_data & 1)
                        update_list[3] = 0xB4;
                    else if(collision_edge_data & 2)
                        update_list[3] = 0xB2;
                    else 
                        update_list[3] = 0;
                        
                    if((collision_edge_data & 9) == 9)
                        update_list[4] = 0xB1;
                    else if(collision_edge_data & 1)
                        update_list[4] = 0xB5;
                    else if(collision_edge_data & 8)
                        update_list[4] = 0xB3;
                    else 
                        update_list[4] = 0;
                    
                    
                    if((collision_edge_data & 6) == 6)
                        update_list[8] = 0xC0;
                    else if(collision_edge_data & 4)
                        update_list[8] = 0xC4;
                    else if(collision_edge_data & 2)
                        update_list[8] = 0xC2;
                    else 
                        update_list[8] = 0;
                        
                    if((collision_edge_data & 12) == 12)
                        update_list[9] = 0xC1;
                    else if(collision_edge_data & 4)
                        update_list[9] = 0xC5;
                    else if(collision_edge_data & 8)
                        update_list[9] = 0xC3;
                    else 
                        update_list[9] = 0;
                        
                        
                    wall_hit_x[i&1] <<= 4;
                    wall_hit_y[i&1] <<= 4;
                    
                                    
                    #undef collision_edge_data
                    SET_FREE(5);
                    
                    #undef row_index_on_ns
                    SET_FREE(0);
                    
                }
                
                craft_bullet_y[i] = 255;
                continue;
            }
            
            for(j=2; j<6; j++){
                if(craft_types[j] != 255){
                    if(bullet_x > craft_x[j]-6 && bullet_x < craft_x[j]+6 && bullet_y > craft_y[j]-6 && bullet_y < craft_y[j]+6){
                        if(craft_hps[j])craft_hps[j]--;
                        craft_bullet_y[i] = 255;
                        break;
                    }
                }
            }
        
        }
        else
        {
            if(has_collision){
                craft_bullet_y[i] = 255;
                continue;
            }
            for(j=0; j<2; j++){
                if(craft_lives[j] > 0){
                    if(craft_bullet_x[i] > craft_x[j]-6 && craft_bullet_x[i] < craft_x[j]+6 && craft_bullet_y[i] > craft_y[j]-6 && craft_bullet_y[i] < craft_y[j]+6){
                        if(craft_hps[i])craft_hps[i]--;
                        craft_bullet_y[i] = 255;
                        continue;
                    }
                }
            }
        }
        
        craft_bullet_x[i] = bullet_x;
        craft_bullet_y[i] = bullet_y;
        
        TIMER_TICK(0);
    }
    
    #undef bullet_x
    SET_FREE(6);
    
    #undef bullet_y
    SET_FREE(3);
    
    #undef has_collision
    SET_FREE(4);
    
    #undef bullet_grid_pos_x
    SET_FREE(2);
    
    #undef bullet_grid_pos_y
    SET_FREE(1);

    TIMER_END(0);
    if(pad_poll(0)&PAD_B)  TIMER_DISABLE(0);
}

void tick_crafts(void){
    //if(pad_poll(0)&PAD_B) TIMER_ENABLE(0);
    TIMER_BEGIN(0);
    
    
    CHECK_FREE(4);
    #define move_amount temp4
    CHECK_FREE(0);
    #define pad temp0
    CHECK_FREE(5);
    #define new_x temp5
    CHECK_FREE(6);
    #define new_y temp6
    
    CHECK_FREE(2);
    #define collision_temp_2 temp2
    
    for(i=0;i<2;++i){
        if(!craft_lives[i]) continue;
      
        if(craft_hps[i] == 0)
        {
            craft_hps[i] = 8;
            craft_lives[i]--;
        }
        pad=pad_poll(i);
        
        sprite_look_dirs[i] = 0;
        move_amount = 0;
        if(pad&PAD_LEFT){
            move_amount++;
            if((pad&(PAD_UP|PAD_DOWN)) && sprite_dirs[i] != DIR_LEFT && sprite_dirs[i] != DIR_RIGHT){
                sprite_look_dirs[i] = DIR_LEFT;
            }else{
                sprite_dirs[i] = DIR_LEFT;
            }
        } else if(pad&PAD_RIGHT){
            move_amount++;
            if((pad&(PAD_UP|PAD_DOWN)) && sprite_dirs[i] != DIR_LEFT && sprite_dirs[i] != DIR_RIGHT){
                sprite_look_dirs[i] = DIR_RIGHT;
            }else{
                sprite_dirs[i] = DIR_RIGHT;
            }
        } 
        
        if(pad&PAD_UP){
            move_amount++;
            if((pad&(PAD_LEFT|PAD_RIGHT)) && sprite_dirs[i] != DIR_UP && sprite_dirs[i] != DIR_DOWN){
                sprite_look_dirs[i] = DIR_LEFT;
            }else{
                sprite_dirs[i] = DIR_UP;
            }
        } else if(pad&PAD_DOWN){
            move_amount++;
            if((pad&(PAD_LEFT|PAD_RIGHT)) && sprite_dirs[i] != DIR_UP && sprite_dirs[i] != DIR_DOWN){
                sprite_look_dirs[i] = DIR_RIGHT;
            }else{
                sprite_dirs[i] = DIR_DOWN;
            }
        }
        if(move_amount){
            new_x = craft_x[i];
            new_y = craft_y[i];
            if(move_amount == 1 || (frame&3) != 1){
                if(pad&PAD_LEFT){
                    int_temp0 = (((unsigned int)1)<<(new_x-3>>4));
                    collision_temp_2 = new_y + (scr&15);
                    
                    if(!((blocked[(collision_temp_2+2)>>4] & int_temp0) || (blocked[(collision_temp_2-2)>>4]& int_temp0))){
                        new_x--;
                    }
                } else if(pad&PAD_RIGHT) {
                    int_temp0 = (((unsigned int)1)<<(new_x+3>>4));
                    collision_temp_2 = new_y + (scr&15);
                    
                    if(!((blocked[(collision_temp_2+2)>>4] & int_temp0) || (blocked[(collision_temp_2-2)>>4]& int_temp0))){
                        new_x++;
                    }
                }
            }
            if(move_amount == 1 || (frame&3)){
                if(pad&PAD_UP){
                    if(!((blocked[new_y-3+(scr&15)>>4] & ((1<<((new_x-2)>>4))|(1<<((new_x+2)>>4)))))){
                        new_y--;
                    }
                } else if(pad&PAD_DOWN) {
                    if(!((blocked[new_y+3+(scr&15)>>4] & ((1<<((new_x-2)>>4))|(1<<((new_x+2)>>4)))))){
                        new_y++;
                    }
                }
            }
            
            if(new_y >= MAX_Y) new_y = MAX_Y;
            
            craft_x[i] = new_x;
            craft_y[i] = new_y;
        }
        if(craft_bullet_timers[i]){
            --craft_bullet_timers[i];
        }else if(pad&PAD_A){
            for(j=i; j < CRAFT_BULLET_COUNT; j += 2){
                if(craft_bullet_y[j] != 255) continue;
                craft_bullet_x[j] = craft_x[i];
                craft_bullet_y[j] = craft_y[i];
                craft_bullet_flag[j] = ((pad&(PAD_UP|PAD_DOWN|PAD_LEFT|PAD_RIGHT))>>4) | sprite_dirs[i];
                craft_bullet_timers[i] = 16;
                break;
            }  
        }
        TIMER_TICK(0);
    }

    #undef collision_temp_2
    SET_FREE(2);
    #undef new_y
    SET_FREE(6);
    #undef new_x
    SET_FREE(5);
    #undef pad
    SET_FREE(0);
    #undef move_amount
    SET_FREE(4);
    
    TIMER_END(0);
    //if(pad_poll(0)&PAD_B)  TIMER_DISABLE(0);
}

//temp0 is scroll amount
void scroll_screen(void){
    if(temp0 < 150){
        CHECK_FREE(1);
        CHECK_FREE(2);
        CHECK_FREE(3);
        CHECK_FREE(4);
        CHECK_FREE(5);
    
        #define scroll_amount temp1
        
        CHECK_FREE(0);
        #define row_index temp0
    
        set_rand(rand16()^frame^craft_x[0]^craft_y[1]);
        
        if(enemy_spawn_scr > scroll_amount){
            enemy_spawn_scr -= scroll_amount;
        }else{
            enemy_spawn_scr = 0;
        }
        for(i=0;i<6;++i){
            craft_y[i] += scroll_amount;
            if(craft_y[i] >= MAX_Y+1) craft_y[i] = MAX_Y+1;
        }
        
        for(i=0; i<ENEMY_BULLET_COUNT; ++i){
            if(craft_bullet_y[i] == 255) continue;
            
            if(craft_bullet_y[i] > 255 - scroll_amount){
                craft_bullet_y[i] = 255;
            }else{
                craft_bullet_y[i] += scroll_amount;
            }
        }
        scr -= scroll_amount;
        if(scr<0) scr+=240*2;
        #undef scroll_amount
        
        row_index = scr>>3;
        if(row_index>=60) row_index-=60;
        if(last_row_index != row_index){
            last_row_index = row_index;
            update_list[2]=32;
            if(row_index<30){
                adr = NAMETABLE_A+(row_index<<5);
                update_list[0]=MSB(adr)|NT_UPD_HORZ;
                update_list[1]=LSB(adr);
                                
                adr=NAMETABLE_A+960+((row_index>>2)<<3);
                update_list[35]=MSB(adr)|NT_UPD_HORZ;//set attribute table update address
                update_list[36]=LSB(adr);
            }else{
                row_index-=30;

                adr = NAMETABLE_C+(row_index<<5);
                update_list[0]=MSB(adr)|NT_UPD_HORZ;
                update_list[1]=LSB(adr);
                                
                                
                adr=NAMETABLE_C+960+((row_index>>2)<<3);
                update_list[35]=MSB(adr)|NT_UPD_HORZ;//set attribute table update address
                update_list[36]=LSB(adr);
            }
 
            if(row_index&1){ // build new line
                if((wall_hit_y[0]&0xF) != 0xF){
                    wall_hit_y[0]++;
                }
                if((wall_hit_y[1]&0xF) != 0xF){
                    wall_hit_y[1]++;
                }
                
                if((wall_hit_y[0]&0xF0) != 0xF0){
                    wall_hit_y[0]+=0x10;
                }
                if((wall_hit_y[1]&0xF0) != 0xF0){
                    wall_hit_y[1]+=0x10;
                }
            
                for(i=2; i<16; i++){
                    prev_line[i] = current_line[i];
                    current_line[i] = next_line[i];
                    if(i==2 || i==15){
                        if(rand8() < 30){
                            if(next_line[i] == WALL) next_line[i] = GRASS;
                            else next_line[i] = WALL;
                        }
                    } else {
                        next_line[i] = GRASS;
                    }
                }
                
                wall_count -= (wall_count>>3);
                
                if(dont_change_bg_pallette) dont_change_bg_pallette--;
                if(wall_count) wall_count--;
                if(has_big_wall) has_big_wall--;
                
                if(wall_count < 2 && (rand8()<150)){
                    wall_count = 0;
                    #define selected_grid temp1
                    #define random temp2
                    random = rand8();
                    if(has_big_wall) selected_grid = WALL;
                    else if(random < 60) selected_grid = WATER;
                    else if(random < 120) selected_grid = FOREST;
                    else if(dont_change_bg_pallette == 0 /*&& random < 160*/){
                        selected_grid = BUILDING;
                        dont_change_bg_pallette = 16;
                                            
                        
                    }else if(random < 200){
                        selected_grid = WALL_BIG;
                        has_big_wall = 2;
                    } else {
                        selected_grid = WALL;
                    }
                    #undef random
                    
                    #define grid_start temp2
                    #define grid_end temp3
                    
                    grid_start = 4+(rand8()&1)+(rand8()&3)+(rand8()&5);
                    grid_end = grid_start;
                    if(selected_grid == BUILDING){
                        if(rand8()&1){
                            grid_start--;
                        }else{
                            grid_end++;
                        }
                    }else{
                    
                        if(rand8()&1) grid_start--;
                        if(rand8()&1) grid_end++;
                        
                        if(selected_grid != WALL_BIG){
                            if(rand8()&1) grid_start--;
                            if(rand8()&1) grid_end++;
                        }else{
                            if(grid_start == grid_end) (rand8()&1)?grid_end++:grid_start--;
                        }
                        
                    }
                
                    for(i=grid_start; i<=grid_end; i++){
                        next_line[i] = selected_grid;
                        wall_count++;
                    }
                    #undef grid_start
                    #undef grid_end
                    #undef selected_grid
                }
                
                for(i=2; i<16; i++){
                    if(next_line[i] == GRASS){
                        if(current_line[i] == WALL_BIG){
                            next_line[i] = WALL;
                        }else if(current_line[i] != BUILDING){
                            #define chance_to_grow temp3
                            #define grow_chance_reduction temp2
                        
                            
                            chance_to_grow = ((next_line[i+1]&WALL&(i<13))<<1)+
                                    ((next_line[i-1]&WALL&(i>3))<<1)+
                                    ((current_line[i]&WALL)<<1)+
                                    ((current_line[i-1]&WALL&(i>3)))+
                                    ((current_line[i+1]&WALL&(i<13)));
                                    
                            grow_chance_reduction = (wall_count>>2);
                            
                            if(chance_to_grow > grow_chance_reduction){
                                chance_to_grow -= grow_chance_reduction;
                            } else {
                                chance_to_grow = 0;
                            }
                            
                            if(chance_to_grow >= 4){
                                next_line[i] = WALL;
                                wall_count++;
                            }else if(chance_to_grow >= 2){
                                if(rand8()&1){
                                    next_line[i] = WALL;
                                    wall_count++;
                                }
                            }
                            
                            chance_to_grow = ((next_line[i+1]==WATER)<<1)+
                                    ((next_line[i-1]==WATER)<<1)+
                                    ((current_line[i]==WATER)<<1)+
                                    ((current_line[i-1]==WATER))+
                                    ((current_line[i+1]==WATER));
                                    
                                    
                            if(chance_to_grow > grow_chance_reduction){
                                chance_to_grow -= grow_chance_reduction;
                            } else {
                                chance_to_grow = 0;
                            }
                            if(chance_to_grow >= 5){
                                next_line[i] = WATER;
                                wall_count++;
                            }else if(chance_to_grow >= 3){
                                if(rand8()&3){
                                    next_line[i] = WATER;
                                    wall_count++;
                                }
                            }
                            
                            chance_to_grow = ((next_line[i+1]==FOREST)<<1)+
                                    ((next_line[i-1]==FOREST)<<1)+
                                    ((current_line[i]==FOREST)<<1)+
                                    ((current_line[i-1]==FOREST))+
                                    ((current_line[i+1]==FOREST));
                                    
                            if(chance_to_grow > grow_chance_reduction){
                                chance_to_grow -= grow_chance_reduction;
                            } else {
                                chance_to_grow = 0;
                            }
                            if(chance_to_grow >= 5){
                                next_line[i] = FOREST;
                                wall_count++;
                            }else if(chance_to_grow >= 3){
                                if(rand8()&3){
                                    next_line[i] = FOREST;
                                    wall_count++;
                                }
                            }
                            
                            #undef grow_chance_reduction
                            #undef chance_to_grow
                        }
                    }
                }
                
                for(i=2; i<16; i++){
                    if(next_line[i] == WALL && (next_line[i-1] == WALL_BIG || next_line[i+1] == WALL_BIG)){
                        if(i==2 || i == 15) next_line[i] = GRASS;
                        else next_line[i] = WALL_BIG;
                    }
                    if(current_line[i] == BUILDING && dont_change_bg_pallette == 15){
                        next_line[i] = BUILDING;
                    }
                }
                
                for(i=2; i<16; i++){
                    if(current_line[i] == WALL && 
                    ((current_line[i-1]&WALL) && (current_line[i+1]&WALL) && 
                    (next_line[i]&WALL) && (prev_line[i]&WALL) 
                     && (next_line[i-1]&WALL) && (next_line[i+1]&WALL) 
                     && (prev_line[i-1]&WALL) && (prev_line[i+1]&WALL))
                  
                    ){
                        current_line[i] = WALL_GREEN;
                    }
                }
                
                if((current_line[2]&WALL) && (prev_line[2]&WALL) && (next_line[2]&WALL)){
                    current_line[1] = WALL_GREEN;
                }
                
                if((current_line[15]&WALL) && (prev_line[15]&WALL) && (next_line[15]&WALL)){
                    current_line[16] = WALL_GREEN;
                }
                
                
                for(i=14; i>0; i--){
                    blocked[i] = blocked[i-1];
                    bullet_blocked[i] = bullet_blocked[i-1];
                }
                blocked[0] = 0;
                bullet_blocked[0] = 0;
                for(i=0; i<16; i++){
                    if(current_line[i+1] != GRASS){
                        blocked[0] |= (1<<i);
                        if(current_line[i+1] != WATER){
                            bullet_blocked[0] |= (1<<i);
                        }
                    }
                }
            }
            
            for(i=0; i<32; i++){
                #define column_index temp3
                column_index = 1+(i>>1);
                #define cell_index temp1
                cell_index = ((((row_index&1)==0)<<1)+(i&1));
                switch(current_line[column_index]){
                    case WALL:
                    case WATER:
                        #define cell_type temp4
                        cell_type = current_line[column_index];
                        #define same_neigbour_dirs temp2
                        if(cell_index&1){
                            same_neigbour_dirs = (current_line[column_index+1]&cell_type)!=0;
                            if(cell_index<2){
                                same_neigbour_dirs += (((prev_line[column_index+1]&cell_type)!=0)<<2);
                            }else{
                                same_neigbour_dirs += ((next_line[column_index+1]==cell_type)<<2);
                            }
                        }else{
                            same_neigbour_dirs = (current_line[column_index-1]&cell_type)!=0;
                            if(cell_index<2){
                                same_neigbour_dirs += (((prev_line[column_index-1]&cell_type)!=0)<<2);
                            }else{
                                same_neigbour_dirs += ((next_line[column_index-1]==cell_type)<<2);
                            }
                        }
                        
                        if(cell_index<2){
                            same_neigbour_dirs += (((prev_line[column_index]&cell_type)!=0)<<1);
                        }else{
                            same_neigbour_dirs += ((next_line[column_index]==cell_type)<<1);
                        }
                        
                        if(same_neigbour_dirs == 7){
                            if(rand8()&15){
                                update_list[3+i] = (cell_type == WALL?0:0xEC);
                            }else{
                                update_list[3+i] =  (cell_type == WALL?0x66:0xCC) + (rand8()&3);
                            }
                        }else{
                            if(same_neigbour_dirs>=4) same_neigbour_dirs -= 4;
                            if(cell_type == WALL){
                                update_list[3+i] = wall_tiles[(cell_index<<2)+same_neigbour_dirs];
                            }else{
                                update_list[3+i] = water_tiles[(cell_index<<2)+same_neigbour_dirs];
                            }
                            if(same_neigbour_dirs==1 || same_neigbour_dirs == 2) update_list[3+i] += (rand8()&3);
                            else if(same_neigbour_dirs==0 && cell_type == WALL) update_list[3+i]  += (rand8()&1);
                        }
                        #undef cell_type
                        #undef same_neigbour_dirs
                    break;
                    case WALL_GREEN:
                    case GRASS:
                        #define random temp4
                        random = rand8()&0x3F;
                        if(random > 9){
                            update_list[3+i] = 0;
                        }else{
                            update_list[3+i] = 0x60 + random;
                        }
                        #undef random
                    break;
                    case WALL_BIG:
                        switch(cell_index){
                            case 0:
                                if(current_line[column_index-1] != WALL_BIG){
                                    update_list[3+i] = 0xA4 + (rand8()&1);
                                }else{
                                    update_list[3+i] = 0x88 + (rand8()&3);
                                }
                            break;
                            case 1:
                                if(current_line[column_index+1] != WALL_BIG){
                                    update_list[3+i] = 0xA6 + (rand8()&1);
                                }else{
                                    update_list[3+i] = 0x88 + (rand8()&3);
                                }
                            break;
                            case 2:
                                if(current_line[column_index-1] != WALL_BIG){
                                    update_list[3+i] = 0x94 + (rand8()&1);
                                }else{
                                    update_list[3+i] = 0x78 + (rand8()&3);
                                }
                            break;
                            case 3:
                                if(current_line[column_index+1] != WALL_BIG){
                                    update_list[3+i] = 0x96 + (rand8()&1);
                                }else{
                                    update_list[3+i] = 0x78 + (rand8()&3);
                                }
                            break;
                        }
                    break;
                    
                    case FOREST:
                    {
                        #define neighbour_forest_count temp2
                        neighbour_forest_count = 0;
                        switch(cell_index){
                            case 0:
                                update_list[3+i] = 0x4;
                                neighbour_forest_count += current_line[column_index-1] == FOREST;
                                neighbour_forest_count += prev_line[column_index] == FOREST;
                            break;
                            case 1:
                                update_list[3+i] = 0x5;
                                neighbour_forest_count += current_line[column_index+1] == FOREST;
                                neighbour_forest_count += prev_line[column_index] == FOREST;
                            break;
                            case 2:
                                update_list[3+i] = 0x6;
                                neighbour_forest_count += current_line[column_index-1] == FOREST;
                                neighbour_forest_count += next_line[column_index] == FOREST;
                            break;
                            case 3:
                                update_list[3+i] = 0x7;
                                neighbour_forest_count += current_line[column_index+1] == FOREST;
                                neighbour_forest_count += next_line[column_index] == FOREST;
                            break;
                        
                        }
                        #define cell_can_be_cleared temp4
                        cell_can_be_cleared = 0;
                        if(cell_index == ((last_row_index + column_index)&3)){
                            if(neighbour_forest_count == 0){
                                cell_can_be_cleared = 1;
                            }else if(neighbour_forest_count==1){
                                cell_can_be_cleared = rand8() < 120;
                            }else{
                                cell_can_be_cleared = rand8() < 60;
                            }
                        }
                        if(cell_can_be_cleared){
                            #define random temp1
                            random = rand8()&0x3F;
                            if(random > 9){
                                update_list[3+i] = 0;
                            }else{
                                update_list[3+i] = 0x60 + random;
                            }
                            #undef random
                        }else{
                            if((cell_index < 2 && neighbour_forest_count <= 1 && (rand8()&3)) || (rand8()&3) == 0){
                                update_list[3+i] = 0xB8 + (rand8()&3);
                            } else {
                                update_list[3+i] = 0xA8 + (rand8()&3);
                            }
                        }
                        #undef cell_can_be_cleared
                        #undef neighbour_forest_count
                    }                    
                    break;
                    
                    case BUILDING:
                        #define sprite_id temp2
                        if(prev_line[column_index] != BUILDING){
                            sprite_id = 0xF8;
                        }else{
                            sprite_id = 0xD8;
                        }
                        if(current_line[column_index-1] == BUILDING){
                            sprite_id += 2;
                        }
                        sprite_id += (cell_index&1);
                        if(cell_index&2) sprite_id -= 0x10;
                        update_list[3+i] = sprite_id;
                        #undef sprite_id
                    break;
                }
                #undef column_index
                #undef cell_index
            }
            
            if( (row_index&1) != 0){
                if(row_index == 29){
                    for(i=0;i<8;++i){
                        update_list[38+i] = (bg_colors[current_line[1 + (i<<1)]] | (bg_colors[current_line[1 + (i<<1)+1]]<<2));
                    }
                }else if( (row_index&3) == 3 ){
                    for(i=0;i<8;++i){
                        update_list[38+i] = (bg_colors[current_line[1 + (i<<1)]] | (bg_colors[current_line[1 + (i<<1)+1]]<<2))<<4;
                    }
                }else{
                    for(i=0;i<8;++i){
                        update_list[38+i] += (bg_colors[current_line[1 + (i<<1)]] | (bg_colors[current_line[1 + (i<<1)+1]]<<2));
                    }
                }
            } else {
                current_line[1] = WALL;
                current_line[16] = WALL;
            }
            
        }
    
        SET_FREE(5);
        SET_FREE(4);
        SET_FREE(3);
        SET_FREE(2);
        SET_FREE(1);
        #undef row_index
        SET_FREE(0);
        scroll(0, scr);
    }
}

void reset(void){
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
    
    sprite_look_dirs[0] = 0;
    sprite_look_dirs[1] = 0;
    
    craft_types[0] = 0;
    craft_types[1] = 1;
    craft_bullet_timers[0] = 0;
    craft_bullet_timers[1] = 0;
    
    for(i=0; i<18; i++){
        next_line[i] = current_line[i] = prev_line[i] = GRASS;
    }
    
    next_line[0] = next_line[1] = WALL;
    next_line[16] = next_line[17] = WALL;
    
    current_line[0] = WALL_GREEN;
    current_line[1] = WALL;
    current_line[16] = WALL;
    current_line[17] = WALL_GREEN;
    
    prev_line[0] = prev_line[1] = WALL;
    prev_line[16] = prev_line[17] = WALL;
    
    dont_change_bg_pallette = 14;
    wall_count = 50;
    has_big_wall = 0;
    #ifdef DEBUG
        used_temps = 0;
    #endif
    
    #ifdef HAS_DEBUGGER
        debug_info_val = 0;
        break_points_enable_val = 0;
    #endif
    
    for(i=0; i<6; i++){
        craft_flags[i] = 0;
    }
    for(i=0; i<ENEMY_BULLET_COUNT; i++){
        craft_bullet_y[i] = 255;
    }
    
    for(i=0; i<15; i++){
        blocked[i] = 0;
        bullet_blocked[i] = 0;
    }
}

void tick_enemies(void){
    CHECK_FREE(5);
    #define spawn_x temp5
    CHECK_FREE(1);
    #define move_dir temp1
    CHECK_FREE(2);
    #define new_x temp2
    CHECK_FREE(3);
    #define new_y temp3
    CHECK_FREE(4);
    #define move_reset temp4
    
    for(i=2; i<6; i++){
        if(craft_types[i] == 255){
            if(enemy_spawn_scr == 0){
                enemy_spawn_scr = 32 + (rand8()&31);
                spawn_x = rand8()&15;
                while((blocked[0] & (1<<spawn_x))){
                    spawn_x++;
                    if(spawn_x == 16) spawn_x = 0;
                }                    
                
                new_x = spawn_x*16 + (rand8()&7);
                new_y = 0;
                move_dir = DIR_DOWN;
                sprite_look_dirs[i] = 0;
                craft_types[i] = 1;
                craft_flags[i] = 15;
                craft_hps[i] = 2;
                craft_bullet_timers[i] = rand8()&127;
            }
        }
        else
        {
            move_dir = sprite_dirs[i];
            new_x = craft_x[i];
            new_y = craft_y[i];
            
            if((frame+i)&3){
                if(frame&3){
                    if(move_dir&DIR_LEFT){
                        new_x--;
                    }
                    if(move_dir&DIR_RIGHT){
                        new_x++;
                    }
                }
                if((frame&3) != 1){
                    if(move_dir&DIR_UP){
                        new_y--;
                    }
                    if(move_dir&DIR_DOWN){
                        new_y++;
                    }
                }
                move_reset = craft_flags[i]&15;
                if(move_reset && (frame & 7) == 0){
                    craft_flags[i]--;
                }
                
                if(move_reset == 0 || (new_y < 20 && move_dir == DIR_UP) || (new_y > 220 && move_dir == DIR_DOWN)|| isFreeIn(new_x, new_y) == FALSE){
                    new_x = craft_x[i];
                    new_y = craft_y[i];
                    
                    if(isFreeIn(new_x, new_y-1) == FALSE){
                        move_dir |= DIR_UP;
                    }
                    if(isFreeIn(new_x, new_y+1) == FALSE){
                        move_dir |= DIR_DOWN;
                    }
                    if(isFreeIn(new_x-1, new_y) == FALSE){
                        move_dir |= DIR_LEFT;
                    }
                    if(isFreeIn(new_x+1, new_y) == FALSE){
                        move_dir |= DIR_RIGHT;
                    }
                    
                    if(new_y < 20){
                        move_dir |= DIR_UP;
                    }
                    if(new_y > 220){
                        move_dir |= DIR_DOWN;
                    }
                    craft_flags[i] = (craft_flags[i]&0xF0) + 4 + (rand8()&11);
                    
                    if(move_dir == 0xF){
                        craft_types[i] = 255;
                        continue;
                    }else if(move_dir == 0){
                        move_dir = (1<<(rand8()&3));
                    } else {
                        move_dir = rand8()&3;
                        
                        do{
                            sprite_dirs[i] = (1<<(rand8()&3));
                        }while(sprite_dirs[i]&move_dir);
                        move_dir = sprite_dirs[i];
                    }
                }
            }
            
            if(craft_bullet_timers[i] == 0)
            {
                for(j=CRAFT_BULLET_COUNT; j < ENEMY_BULLET_COUNT; j++){
                    if(craft_bullet_y[j] != 255) continue;
                    craft_bullet_x[j] = new_x;
                    craft_bullet_y[j] = new_y;
                    craft_bullet_flag[j] = move_dir;
                    craft_bullet_timers[i] = 64 + (rand8()&127);
                    break;
                }
            } else {
                craft_bullet_timers[i]--;
            }
            
            if(new_y >= MAX_Y-1 || craft_hps[i] == 0){
                craft_types[i] = 255;
                continue;
            }
            
            for(j=0; j<2; j++){
                if(craft_lives[j] != 0){
                    if(new_x > craft_x[j]-12 && new_x < craft_x[j]+12 && new_y > craft_y[j]-12 && new_y < craft_y[j]+12){
                        if(craft_hps[j]>2)craft_hps[j]-=2;
                        else craft_hps[j] = 0;
                        craft_types[i] = 255;
                        break;
                    }
                }
            }
        }
        
        craft_x[i] = new_x;
        craft_y[i] = new_y;
        sprite_dirs[i] = move_dir;
    }

    #undef move_reset
    SET_FREE(4);
    #undef new_y
    SET_FREE(3);
    #undef new_x
    SET_FREE(2);
    #undef move_dir
    SET_FREE(1);
    #undef spawn_x
    SET_FREE(5);
}

void check_pause(void){
    CHECK_FREE(0);
    #define alpha temp0
    
    
    CHECK_FREE(1);
    #define remaining_frame temp1
    
    if((pad_poll(0)|pad_poll(1)) & PAD_START){
        alpha = 4;
        
        while(alpha>0){
            alpha--;
            pal_bright(alpha);
            
            remaining_frame = 3;
            while(remaining_frame--){
                ppu_wait_frame();
            }
        }
        
        oam_clear();
		spr=0;
        for(i=0; i<7; i++){
            spr=oam_spr(100+(i<<3), 100, 0xF2+(i<<1), 3, spr);
        }
    
        alpha = 0;
        
        while(alpha<4){
            alpha++;
            pal_spr_bright(alpha);
            
            remaining_frame = 3;
            while(remaining_frame--){
                ppu_wait_frame();
            }
        }
        
        while(1){
            ppu_wait_frame();
            
            if((pad_poll(0)|pad_poll(1)) & PAD_START) break;
        }
        
        alpha = 4;
        while(alpha>0){
            alpha--;
            pal_spr_bright(alpha);
            
            remaining_frame = 3;
            while(remaining_frame--){
                ppu_wait_frame();
            }
        }
        
        oam_clear();
		spr=0;
        draw_all();
        alpha = 0;
        while(alpha<4){
            alpha++;
            pal_bright(alpha);
            
            remaining_frame = 3;
            while(remaining_frame--){
                ppu_wait_frame();
            }
        }
        
    }
    
    #undef remaining_frame
    SET_FREE(1);
    
    #undef alpha
    SET_FREE(0);
}


void main(void){
	init();
    
    reset();
    
    menu();
    
    oam_clear();
    while(scr!=240){
		ppu_wait_frame();
        temp1 = 4;
        scroll_screen();
		++frame;
	}

    pal_col(9, 0x3d);
    pal_col(10, 0x2d);
    pal_col(11, 0x1d);
    
    temp5 = 0;
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
        #ifdef DEBUG
            spr=oam_spr(20, wall_count, 0x79, 1, spr);
            spr=oam_spr(40, has_big_wall, 0x79, 1, spr);
            spr=oam_spr(60, wall_hit_y[0]&15, 0x79, 1, spr);
        #endif
        tick_crafts();
        tick_enemies();
        tick_bullets();
        draw_all();
        
        temp1 = 0;
        if(craft_lives[0] && craft_y[0] < 150) temp1 = 150-craft_y[0];
        if(craft_lives[1] && craft_y[1] < 150){
            temp2 = 150-craft_y[1];
            if(temp2 > temp1){
                temp1 = temp2;
            }
        }
    
    
        scroll_screen();
        
        
        check_pause();
        
        
		++frame;
	}
}
