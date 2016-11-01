//****************************************************************************
//  debug: message-reporting data
//****************************************************************************
#define  DBG_GENERAL       0x001
#define  DBG_WINMSGS       0x002

//  cdtimer.cpp
extern unsigned max_timer_mins ;
extern unsigned ticks ;
extern char wave_name[PATH_MAX+1] ; 

//  config.cpp
extern uint dbg_flags ;
extern uint window_top ;
extern uint window_left ;
LRESULT init_config(void);
LRESULT save_cfg_file(void);

