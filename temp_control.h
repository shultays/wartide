#pragma bssseg(push,"ZEROPAGE")
#pragma dataseg(push,"ZEROPAGE")


static unsigned char temp0;
static unsigned char temp1;
static unsigned char temp2;
static unsigned char temp3;
static unsigned char temp4;
static unsigned char temp5;
static unsigned char temp6;
static unsigned char temp7;

static unsigned int int_temp0;

#ifdef DEBUG

static unsigned char used_temps = 0;

#define CHECK_FREE(t) if(used_temps & (1<<t)) while(1) ppu_wait_nmi(); else used_temps |= (1<<t);
#define SET_FREE(t) used_temps &= ~(1<<t);

#else

#define CHECK_FREE(t)
#define SET_FREE(t) 

#endif


#pragma bssseg (push,"BSS")
#pragma dataseg(push,"BSS")