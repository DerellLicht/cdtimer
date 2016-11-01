//****************************************************************************
//  Copyright (c) 2008-2016  Daniel D Miller
//  config.cpp - manage configuration data file
//
//  Produced and Directed by:  Dan Miller
//****************************************************************************
//  Filename will be same as executable, but will have .ini extensions.
//  Config file will be stored in same location as executable file.
//  Comments will begin with '#'
//  First line:
//  device_count=%u
//  Subsequent file will have a section for each device.
//****************************************************************************
#include <windows.h>
#include <stdio.h>   //  fopen, etc
#include <stdlib.h>  //  atoi()
#include <tchar.h>
#include <limits.h>     //  PATH_MAX

#include "common.h"
#include "cdtimer.h"
#include "system.h"

uint window_top = 100 ;
uint window_left = 200 ;
static char ini_name[PATH_MAX+1] = "" ;

//****************************************************************************
//  debug: message-reporting data
//****************************************************************************
uint dbg_flags =
               // DBG_WINMSGS ||
               0 ;

//****************************************************************************
//  dialog-position data
//****************************************************************************
#define  CDTIMER_LEFT     10
#define  CDTIMER_TOP      (MON_Y0 + 23)

typedef struct dialog_pos_s {
   bool pos_modified ;
   uint left ;
   uint top ;
} dialog_pos_t ;

//****************************************************************************
static void strip_comments(char *bfr)
{
   char *hd = strchr(bfr, '#') ;
   if (hd != 0)
      *hd = 0 ;
}

//****************************************************************************
LRESULT save_cfg_file(void)
{
   char *fname = ini_name ;
   FILE *fd = fopen(fname, "wt") ;
   if (fd == 0) {
      LRESULT result = (LRESULT) GetLastError() ;
      syslog("%s open: %s\n", fname, get_system_message(result)) ;
      return result;
   }
   //  save any global vars
   fprintf(fd, "dbg_flags=0x%X\n", dbg_flags) ;
   fprintf(fd, "window_top=%u\n", window_top) ;
   fprintf(fd, "window_left=%u\n", window_left) ;
   fprintf(fd, "max_timer_mins=%u\n", max_timer_mins) ;
   fprintf(fd, "ticks=%u\n", (unsigned) ticks) ;
   fprintf(fd, "wave_name=%s\n", wave_name) ;
   fclose(fd) ;
   return ERROR_SUCCESS;
}

//****************************************************************************
//  - derive ini filename from exe filename
//  - attempt to open file.
//  - if file does not exist, create it, with device_count=0
//    no other data.
//  - if file *does* exist, open/read it, create initial configuration
//****************************************************************************
LRESULT init_config(void)
{
   char inpstr[128] ;
   // int ivalue ;
   // uint uvalue ;
   LRESULT result ;
   
   result = derive_filename_from_exec(ini_name, ".ini") ;
   if (result != 0)
      return result;
   // if (dbg_flags & DBG_VERBOSE)
   //    syslog("INI fname=%s\n", ini_name) ;

   FILE *fd = fopen(ini_name, "rt") ;
   if (fd == 0) {
      return save_cfg_file() ;
   }

   // uint local_max_devs = 0 ;
   while (fgets(inpstr, sizeof(inpstr), fd) != 0) {
      strip_comments(inpstr) ;
      strip_newlines(inpstr) ;
      if (strlen(inpstr) == 0)
         continue;

      char *tl = strchr(inpstr, '=') ;
      //  if '=' not found, try ':' as separator
      if (tl == NULL) {
         tl = strchr(inpstr, ':') ;
         if (tl == NULL) 
            continue;
      }
      *tl++ = 0 ; //  split field name from value ;

   // fprintf(fd, "dbg_flags=0x%X\n", dbg_flags) ;
   // fprintf(fd, "max_timer_mins=%u\n", max_timer_mins) ;
   // fprintf(fd, "ticks=%u\n", (unsigned) ticks) ;
   // fprintf(fd, "wave_name=%s\n", wave_name) ;
      if (strcmp(inpstr, "dbg_flags") == 0) {
         dbg_flags = strtoul(tl, NULL, 0);    
      } else
      if (strcmp(inpstr, "window_top") == 0) {
         window_top = (unsigned) atoi(tl) ;
      } else
      if (strcmp(inpstr, "window_left") == 0) {
         window_left = (unsigned) atoi(tl) ;
      } else
      if (strcmp(inpstr, "max_timer_mins") == 0) {
         max_timer_mins = (unsigned) atoi(tl) ;
      } else
      if (strcmp(inpstr, "ticks") == 0) {
         ticks = (unsigned) atoi(tl) ;
      } else
      if (strcmp(inpstr, "wave_name") == 0) {
         sprintf(wave_name, "%s", tl);
      } else
      {
         // syslog("unknown: [%s]\n", inpstr) ;
      }

   }
   fclose(fd) ;
   return 0;
}

