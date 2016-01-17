//#define DEBUG

#define HAS_DEBUGGER
//#define DEBUG_COLLISONS


#define DIR_UP 1
#define DIR_DOWN 2
#define DIR_LEFT 4
#define DIR_RIGHT 8

#define DIR_UP_RIGHT (DIR_UP | DIR_RIGHT)
#define DIR_DOWN_RIGHT (DIR_DOWN | DIR_RIGHT)
#define DIR_DOWN_LEFT (DIR_DOWN | DIR_LEFT)
#define DIR_UP_LEFT (DIR_UP | DIR_LEFT)

#define MAX_Y (253-32)
#define CRAFT_BULLET_COUNT 8
#define ENEMY_BULLET_COUNT 12


#define GRASS 0x0
#define WALL 0x1
#define WALL_GREEN 0x3
#define WALL_BIG 0x5
#define WATER 0x2
#define FOREST 0x4
#define BUILDING 0x8
#define BUILDING_PASSABLE 0x18
#define GRASS_EMPTY 0x20

static const unsigned char palette[]={ 
0x29,0x27,0x17,0x07, // mountains
0x29,0x27,0x19,0x18, // grass, trees
0x29,0xF,0x2D,0x3D, // menu
0x29,0x21,0x1C,0xF, // water

0x29, 0x37, 0x26, 0x17,
0x29, 0x31, 0x22, 0x11,
0x29, 0x33, 0x23, 0x13,
0x29, 0x36, 0x26, 0x16,
}; 

static const unsigned char bg_colors[]={
	1,
	0,
	3,
	1,
	1,
    0,
    
    0,
    0,
    
    2
};

static const char water_tiles[] = {
0xBD,
0x8C,
0xAC,
0xE0,

0xBF,
0x8C,
0x9C,
0xE3,

0xBC,
0x7C,
0xAC,
0xE1,

0xBE,
0x7C,
0x9C,
0xE2,
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

static const char small_exp_0[] = {
    -8, -8, 0x8C, 3,
    -0, -8, 0x8E, 3,
    128
};
static const char small_exp_1[] = {
    -8, -8, 0x88, 3,
    0,  -8, 0x8A, 3,
    128
};
static const char small_exp_2[] = {
    -8, -8, 0x84, 3,
    0,  -8, 0x86, 3,
    128
};



static const char big_exp_0[] = {
    -17, -15, 0x90, 3,
    -9,  -15, 0x92, 3,
    -1,  -15, 0x94, 3,
    +7,  -15, 0x96, 3,
    
    -17, +1,  0xB0, 3,
    -9,  +1,  0xB2, 3,
    -1,  +1,  0xB4, 3,
    +7,  +1,  0xB6, 3,
    128
};

static const char big_exp_1[] = {
    -17, -15, 0x98, 3,
    -9,  -15, 0x9A, 3,
    -1,  -15, 0x9C, 3,
    +7,  -15, 0x9E, 3,
    
    -17, +1,  0xB8, 3,
    -9,  +1,  0xBA, 3,
    -1,  +1,  0xBC, 3,
    +7,  +1,  0xBE, 3,
    128
};
