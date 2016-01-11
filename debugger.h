
#ifdef HAS_DEBUGGER


#define debug_info_val (*(unsigned char*)0xFA)
#define break_points_enable_val (*(unsigned char*)0xFB)
#define break_point_val (*(unsigned char*)0xFF)


#define TIMER_ENABLE(x) break_points_enable_val |=(1<<x);
#define TIMER_DISABLE(x) break_points_enable_val &=~(1<<x);

#define TIMER_BEGIN(x) if(break_points_enable_val&(1<<x)) { debug_info_val |=(1<<x); break_point_val = 0; }
#define TIMER_TICK(x) if(break_points_enable_val&(1<<x)) break_point_val++;
#define TIMER_SET_VAL(x, y) if(break_points_enable_val&(1<<x)) break_point_val=y;
#define TIMER_END(x) if(break_points_enable_val&(1<<x)) { debug_info_val &=~(1<<x); break_point_val++; }

#define DEBUG_SET(x) break_point_val = x

#else

#define TIMER_ENABLE(x)
#define TIMER_DISABLE(x)

#define TIMER_BEGIN(x)
#define TIMER_TICK(x)
#define TIMER_SET_VAL(x, y)
#define TIMER_END(x)

#define DEBUG_SET(x)

#endif