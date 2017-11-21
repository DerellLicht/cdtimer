//*********************************************************************
//  cdtimer.cpp - Countdown timer using slider for setting time 
//  
//  Written by:   Daniel D. Miller
//  
//  Last Update:  11/20/17 
//*********************************************************************

static const char Version[] = "Countdown Timer V1.03" ;

#include <windows.h>
#include <commctrl.h>			  //  link to comctl32.lib
// #include <string.h>
#include <limits.h>     //  PATH_MAX
#include <math.h>
#include <tchar.h>
#include <sys/stat.h>

#include "resource.h"
#include "common.h"
#include "commonw.h"
#include "cdtimer.h"
/* #include "system.h" */
#include "statbar.h"
#include "winmsgs.h"

//lint -esym(1055, atoi)

static UINT  timerID = 0 ;

//*********************************************************************
// variables

static HINSTANCE g_hinst;
static HWND hwndTitle = NULL ;
static HWND hwndMessage = NULL ;
static HWND hwndMaxMins = NULL ;
static HWND hwndSecsPerTick = NULL ;
static HWND hwndWaveFile = NULL ;
static HWND hwndUpdate = NULL ;
static HWND hwndCountDir = NULL;

//  tracker data
static HWND hwndTrack;
static HMENU const ID_TRACKBAR = NULL;
static UINT  giSelMin, giSelMax ;

#define  MIN_TIME    (0)
static unsigned MAX_TIME = (10 * 60) ;
static unsigned TICK_SPREAD = (30) ;

unsigned max_timer_mins = (MAX_TIME / 60) ;
unsigned ticks = TICK_SPREAD ;
char wave_name[PATH_MAX+1] = "c:\\waves\\MAGIC.WAV" ; 

static unsigned tcount = 0 ;
static unsigned tmins  = 0 ;
static unsigned tsecs  = 0 ;

static CStatusBar *MainStatusBar = NULL;
//*************************************
// #define  HEADER_Y    (20)
// 
// #define  DIALOG_X    (70)
// #define  DIALOG_Y    (85)
// 
// // #define  SLIDER_X    ( 30)
#define  SLIDER_Y    ( 40)
// #define  DIALOG_DX   (260)

#define  SLIDER_DX   (300)
#define  SLIDER_DY   ( 30)

#define  LEFT_EDGE   (75)

// #define  STATUS_Y    (100)

static char tempstr[260] ;

static int cxClient = 0 ;
static int cyClient = 0 ;

typedef enum ct_mode_e {
   COUNT_DOWN = 0,
   COUNT_UP
} ct_mode_t ;

typedef enum ct_state_e {
   STATE_IDLE = 0,
   STATE_COUNTING
} ct_state_t ;

static ct_mode_t count_mode = COUNT_DOWN ;
static ct_state_t count_state = STATE_IDLE ;

//*********************************************************************
static void update_slidebar(HWND hwnd)
{
   // min. & max. positions 
   SendMessage (hwnd, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG (0, MAX_TIME)); 
   // new page size ( distance that bar is moved by PgUp/PgDn )
   SendMessage (hwnd, TBM_SETPAGESIZE, 0, (LPARAM) TICK_SPREAD) ;
   SendMessage (hwnd, TBM_SETSEL, (WPARAM) FALSE, (LPARAM) MAKELONG (0, MAX_TIME));
   SendMessage (hwnd, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) 0);
   SendMessage (hwnd, TBM_SETTICFREQ, (WPARAM) TICK_SPREAD, (LPARAM) 0);

   SetFocus (hwnd);
}

//*********************************************************************
//  tracker/slider sample from MSDN
//*********************************************************************
// CreateTrackbar - creates and initializes a trackbar. 
// 
// Global variable 
//     g_hinst - instance handle 
static HWND WINAPI CreateTrackbar (HWND hwndDlg,	// handle of dialog box (parent window) 
	UINT iMin,						  // minimum value in trackbar range 
	UINT iMax,						  // maximum value in trackbar range 
	UINT iSelMin,					  // minimum value in trackbar selection 
	UINT iSelMax)					  // maximum value in trackbar selection 
{
   giSelMin = iSelMin ;
   giSelMax = iSelMax ;

	hwndTrack = CreateWindowEx (0,	// no extended styles 
		TRACKBAR_CLASS,			  // class name 
		"Trackbar Control",		  // title (caption) 
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,	// style 
      LEFT_EDGE, SLIDER_Y,      // position 
      SLIDER_DX, SLIDER_DY,     // size 
		hwndDlg,						  // parent window 
		ID_TRACKBAR,				  // control identifier 
      g_hinst,                  // instance 
		NULL							  // no WM_CREATE parameter 
		);

   update_slidebar(hwndTrack) ;

	return hwndTrack;
}  //lint !e715

//*********************************************************************
static void update_timer_count(HWND hwnd)
{
   // dprints_centered_x(hwnd, HEADER_Y, RGB(0, 104, 139), "derelict's timer");
   // dprints_centered_x(hwnd, HEADER_Y, 0x71, " derelict's timer ");
   if (count_mode == COUNT_UP) {
      tmins = tcount / 60 ;
      tsecs = tcount % 60 ;
      wsprintf(tempstr, " elapsed: %2u mins, %2u secs ", tmins, tsecs) ;
      SetWindowText(hwndMessage, tempstr);

      wsprintf(tempstr, "%2u mins, %2u secs ", tmins, tsecs) ;
      SetWindowText(hwnd, tempstr) ;
   } else
   if (tcount == 0) {
      SetWindowText(hwndMessage, "timer expired");
      SetWindowText(hwnd, Version) ;
   } else {
      tmins = tcount / 60 ;
      tsecs = tcount % 60 ;
      wsprintf(tempstr, " setting: %2u mins, %2u secs ", tmins, tsecs) ;
      SetWindowText(hwndMessage, tempstr);
      SendMessage (hwndTrack, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) tcount);
      wsprintf(tempstr, "%2u mins, %2u secs ", tmins, tsecs) ;
      SetWindowText(hwnd, tempstr) ;
   }
}

//****************************************************************************
//****************************************************************************
#define  FONT_HEIGHT_TITLE    26
#define  FONT_HEIGHT_MESSAGE  22
// static char ascii_font_name[LF_FULLFACESIZE] = "Wingdings" ;
// static char ascii_font_name[LF_FULLFACESIZE] = "Times New Roman" ;
static TCHAR font_name_title[LF_FULLFACESIZE] = _T("CaligulaA") ;
static TCHAR font_name_message[LF_FULLFACESIZE] = _T("Times New Roman") ;

static void set_control_font(HWND hwnd, TCHAR *fname, uint fheight, uint flags)
{
   // HFONT hfont = build_font(ascii_font_name, STAT_FONT_HEIGHT, EZ_ATTR_BOLD) ;
   HFONT hfont = build_font(fname, fheight, flags) ;
   if (hfont == 0) {
      syslog("build_font: %s\n", get_system_message()) ;
   } else {
      PostMessage(hwnd, WM_SETFONT, (WPARAM) hfont, (LPARAM) true) ;
   }
}

//******************************************************************
static BOOL CALLBACK DoneProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch(msg) {
   case WM_INITDIALOG:
      if (PlaySound(wave_name, NULL, SND_FILENAME) == FALSE) {
         syslog("PlaySound: %s\n", get_system_message());
      } 
      // else {
      //    syslog("PlaySound: %s completed successfully\n", wave_name);
      // }
		return TRUE;	

   case WM_COMMAND:
      if (HIWORD (wParam) == BN_CLICKED) {
         switch(LOWORD(wParam)) {
         case IDOK: //  take the new settings
      		DestroyWindow(hwnd);
				break;
         }  //lint !e744  switch(target)
		}	//  if btn_clicked
      return FALSE;

   case WM_CLOSE:
      DestroyWindow(hwnd);
      return TRUE ;

   case WM_DESTROY:
      return TRUE ;

   default:
      return FALSE;
	}
}  //lint !e715

//***********************************************************************
static LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
   RECT myRect ;
   DWORD dwPos ;
   char msgstr[81];
   // bool bResult ;

   //***************************************************
   //  debug: log all windows messages
   //***************************************************
   if (dbg_flags & DBG_WINMSGS) 
   {
      switch (iMsg) {
      //  list messages to be ignored
      case WM_CTLCOLORBTN:
      case WM_CTLCOLORSTATIC:
      case WM_CTLCOLOREDIT:
      case WM_CTLCOLORLISTBOX:
      // case WM_MOUSEMOVE:
      // case WM_NCMOUSEMOVE:
      // case WM_NCHITTEST:
      // case WM_SETCURSOR:
      case WM_TIMER:
      case WM_NCACTIVATE:
      case WM_NOTIFY:
      case WM_COMMAND:  //  prints its own msgs below
         break;
      default:
         syslog("TOP [%s]\n", lookup_winmsg_name(iMsg)) ;
         break;
      }
   }

   switch(iMsg) {
   case WM_INITDIALOG:
      SetClassLongA(hwnd, GCL_HICON,   (LONG) LoadIcon(g_hinst, (LPCTSTR)IDI_CDTIMER));
      SetClassLongA(hwnd, GCL_HICONSM, (LONG) LoadIcon(g_hinst, (LPCTSTR)IDI_CDTIMER));
      SetWindowText(hwnd, Version) ;
      
      hwndTitle = GetDlgItem(hwnd, IDC_TITLE) ;
      hwndMessage = GetDlgItem(hwnd, IDC_MESSAGE) ;
      hwndMaxMins = GetDlgItem(hwnd, IDC_EDIT2) ;
      hwndSecsPerTick = GetDlgItem(hwnd, IDC_EDIT3) ;
      hwndWaveFile = GetDlgItem(hwnd, IDC_EDIT4) ;
      hwndUpdate = GetDlgItem(hwnd, IDC_UPDATE) ;
      hwndCountDir = GetDlgItem(hwnd, IDC_COUNTDIR) ;

      //  set trackbar variables
      wsprintf(msgstr, "%u", max_timer_mins);
      SetWindowText(hwndMaxMins, msgstr);
      MAX_TIME = max_timer_mins * 60 ;
      wsprintf(msgstr, "%u", ticks);
      SetWindowText(hwndSecsPerTick, msgstr);
      SetWindowText(hwndWaveFile, wave_name);

      //  create the trackbar
      hwndTrack = CreateTrackbar (hwnd, MIN_TIME, MAX_TIME, MIN_TIME, MAX_TIME) ;
      
      GetWindowRect(hwnd, &myRect) ;
      // GetClientRect(hwnd, &myRect) ;
      cxClient = (myRect.right - myRect.left) ;
      cyClient = (myRect.bottom - myRect.top) ;
      //***************************************************************
      //  create/configure status bar
      //***************************************************************
      // hwndStatusBar = InitStatusBar(hwnd);
      MainStatusBar = new CStatusBar(hwnd) ;
      MainStatusBar->MoveToBottom(cxClient, cyClient) ;
      
      //  re-position status-bar parts
      int sbparts[3];
      sbparts[0] = (int) (60 * cxClient / 100) ;
      sbparts[1] = -1;
      // sbparts[1] = (int) (8 * cxClient / 10) ;
      // sbparts[2] = -1;
      MainStatusBar->SetParts(2, &sbparts[0]);
      
      //  set window position
      SetWindowPos(hwnd, HWND_TOP, window_left, window_top, 0, 0, SWP_NOSIZE);
   
      set_control_font(hwndTitle, font_name_title, FONT_HEIGHT_TITLE, 0);
      set_control_font(hwndMessage, font_name_message, FONT_HEIGHT_MESSAGE, EZ_ATTR_BOLD);
      timerID = SetTimer(hwnd, IDT_TIMER, 987, (TIMERPROC) NULL) ;

      return true;

   //********************************************************************
   case WM_TIMER:
      switch (wParam) {
      case IDT_TIMER:
         //  if we're inactive, do nothing
         if (count_mode == COUNT_UP) {
            if (count_state == STATE_COUNTING) 
               tcount++ ;
         } else {
            if (tcount == 0)
               break;

            //  if alarm has triggered, display message
            if (--tcount == 0) {
               CreateDialog(g_hinst, MAKEINTRESOURCE(IDD_DONE_DIALOG), hwnd, DoneProc) ;
               // syslog("countdown complete\n");
            }
         }
         //  either way, update display
         update_timer_count(hwnd) ;
         break;
      }  //lint !e744

      break;

   // case WM_WINDOWPOSCHANGING:
   case WM_EXITSIZEMOVE:
      {
      RECT rect ;
      GetWindowRect(hwnd, &rect);
      window_top = rect.top ;
      window_left = rect.left ;
      save_cfg_file();
      }
      break ;
      
   case WM_HSCROLL:
      // wsprintf(tempstr, "wParam=0x%08X  ", wParam) ;
      // hdc = GetDC (hwnd) ;
      // TextOut(hdc, 20, 200, tempstr, strlen(tempstr)) ;
      // ReleaseDC (hwnd, hdc) ;
// #define TB_LINEUP               0
// #define TB_LINEDOWN             1
// #define TB_PAGEUP               2
// #define TB_PAGEDOWN             3
// #define TB_THUMBPOSITION        4
// #define TB_THUMBTRACK           5
// #define TB_TOP                  6
// #define TB_BOTTOM               7
// #define TB_ENDTRACK             8
      switch (LOWORD(wParam)) {
      //  this message is sent when the tracker "thumb" is released
      case TB_ENDTRACK:
         dwPos = SendMessage (hwndTrack, TBM_GETPOS, 0, 0);
         if (dwPos > giSelMax)
            SendMessage (hwndTrack, TBM_SETPOS, (WPARAM) TRUE, // redraw flag 
               (LPARAM) giSelMax);
         else if (dwPos < giSelMin)
            SendMessage (hwndTrack, TBM_SETPOS, (WPARAM) TRUE, // redraw flag 
               (LPARAM) giSelMin);
         
         tcount = dwPos ;
         update_timer_count(hwnd) ;
         break;

      case TB_THUMBTRACK:
         dwPos = SendMessage (hwndTrack, TBM_GETPOS, 0, 0);
         if (dwPos > giSelMax)
            SendMessage (hwndTrack, TBM_SETPOS, (WPARAM) TRUE, // redraw flag 
               (LPARAM) giSelMax);
         else if (dwPos < giSelMin)
            SendMessage (hwndTrack, TBM_SETPOS, (WPARAM) TRUE, // redraw flag 
               (LPARAM) giSelMin);
         
         tcount = dwPos ;
         update_timer_count(hwnd) ;
         break;

      default:
         // wsprintf(tempstr, "wParam=0x%08X  ", wParam) ;
         // hdc = GetDC (hwnd) ;
         // TextOut(hdc, 20, 200, tempstr, strlen(tempstr)) ;
         // ReleaseDC (hwnd, hdc) ;
         break;
      }
      break;

   //********************************************************************
   //  menu/misc handlers
   //********************************************************************
   // case WM_COMMAND:
   //    bResult = do_command(hwnd, wParam, lParam) ;
   //    if (!bResult) {
   //       bResult = do_menu(hwnd, wParam, lParam) ;
   //    }
   //    return bResult;

   case WM_COMMAND:
      {
      DWORD cmd = HIWORD (wParam) ;
      DWORD target = LOWORD(wParam) ;
      int  tempEditLength ;
      char tempEditText[2048];
      unsigned utemp ;
      int changed ;

      switch (cmd) {
      case BN_CLICKED:
         switch (target) {
         case IDC_WAVEFILE:
            {
            bool bresult = select_file(hwnd, wave_name, "wav");
            if (bresult) {
               // wsprintf(msgstr, "lpstrFile: %s\n", ofn.lpstrFile) ;
               // OutputDebugString(msgstr) ;
               // strncpy(wave_name, ofn.lpstrFile, sizeof(wave_name)) ;
               // wsprintf(msgstr, " %s", wave_name) ;
               SetWindowText(hwndWaveFile, wave_name) ;
               save_cfg_file();
               // inireg.set_param("wavefile", wave_name) ;
               // hwndWaveFile = CreateEdit (tempstr, DATA_COL, WAVE_ROW, WAV_FIELD_LEN, FIELD_HEIGHT, IDC_EDIT3, hwnd, g_hInst);
            } else {
               wsprintf(msgstr, "GetOpenFileName: %s\n", get_system_message()) ;
               OutputDebugString(msgstr) ;
            }
            }
            break;

         //  switch to count-up mode and start counting
         case IDC_COUNTDIR:
            if (count_mode == COUNT_DOWN) {
               count_mode = COUNT_UP ;
               count_state = STATE_COUNTING ;
               SetWindowText(hwndUpdate, "Cancel CountUp") ;
            } else {
               if (count_state == STATE_COUNTING) {
                  count_state = STATE_IDLE ;
                  SetWindowText(hwndCountDir, "CountUp paused") ;
               } else {
                  count_state = STATE_COUNTING ;
                  SetWindowText(hwndCountDir, "Count Up") ;
               }
            }
            break;
            
         //  The button is the one with the identifier IDC_BUTTON
         case IDC_UPDATE:
            if (count_mode == COUNT_DOWN) {
               changed = 0 ;

               //*******************************************
               //  get first counter
               //*******************************************
               tempEditLength = GetWindowTextLength (hwndMaxMins);
               GetWindowText (hwndMaxMins, tempEditText, tempEditLength + 1);
               tempEditText[tempEditLength] = '\0';

               utemp = (unsigned) atoi(tempEditText) ;
               if (utemp == 0) {
                  wsprintf(tempEditText, "%u", max_timer_mins) ;
                  SetWindowText(hwndMaxMins, tempEditText) ;
               }
               else if (utemp != max_timer_mins) {
                  max_timer_mins = utemp ;
                  wsprintf(tempEditText, "%u", max_timer_mins) ;
                  SetWindowText(hwndMaxMins, tempEditText) ;
                  // ini_write_tag(my_ini_group, "max_time", max_timer_mins) ;
                  // inireg.set_param("max_time", max_timer_mins) ;
                  MAX_TIME = max_timer_mins * 60 ;
                  changed = 1 ;
               }

               //*******************************************
               //  get second counter
               //*******************************************
               tempEditLength = GetWindowTextLength (hwndSecsPerTick);
               GetWindowText (hwndSecsPerTick, tempEditText, tempEditLength + 1);
               tempEditText[tempEditLength] = '\0';

               utemp = (unsigned) atoi(tempEditText) ;
               if (utemp == 0) {
                  wsprintf(tempEditText, "%u", ticks) ;
                  SetWindowText(hwndSecsPerTick, tempEditText) ;
               }
               else if (utemp != ticks) {
                  ticks = utemp ;
                  wsprintf(tempEditText, "%u", ticks) ;
                  SetWindowText(hwndSecsPerTick, tempEditText) ;
                  // ini_write_tag(my_ini_group, "ticks", ticks) ;
                  // inireg.set_param("ticks", ticks) ;
                  TICK_SPREAD = ticks ;
                  changed = 1 ;
               }

               if (changed) {
                  save_cfg_file();
                  update_slidebar(hwndTrack) ;
               }
            }
            //  overload Updated button
            else {
               count_mode = COUNT_DOWN ;
               SetWindowText(hwndUpdate, "Update variables") ;
               SetWindowText(hwndCountDir, "Count Up") ;
               tcount = 0 ;
               update_timer_count(hwnd) ;

            }
            break;
         }  //lint !e744  end switch(target)
      }  //lint !e744  end switch(cmd)
      }  //  end WM_COMMAND context
      break;

   //********************************************************************
   //  application shutdown handlers
   //********************************************************************
   case WM_CLOSE:
      // syslog("Main window WM_DESTROY\n") ;
      if (timerID != 0) {
         KillTimer(hwnd, timerID) ;
         timerID = 0 ;
      }
      // release_led_images() ;
      DestroyWindow(hwnd);
      break;

   case WM_DESTROY:
      PostQuitMessage(0);
      break;

   default:
      return false;
   }  //  switch(iMsg) 

   return true;
}  //lint !e715

//***********************************************************************
//lint -esym(818, szCmdLine, hPrevInstance)

int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                  LPSTR szCmdLine, int iCmdShow)   //lint !e1784
{//lint !e1784
   g_hinst = hInstance;

   load_exec_filename() ;     //  get our executable name
   init_config();

   //**************************************************************************
   //  create main dialog box
   //**************************************************************************
   HWND hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, (DLGPROC) WndProc);
   if (hwnd == NULL) {
      syslog("CreateDialog (main): %s [%u]\n", get_system_message(), GetLastError()) ;
      return false;
   }
   ShowWindow (hwnd, SW_SHOW) ;
   UpdateWindow(hwnd);

   MSG msg ;
   while (GetMessageA(&msg, NULL, 0, 0)) {
      if (!IsDialogMessage(hwnd, &msg)) {
         TranslateMessage(&msg) ;
         DispatchMessage(&msg) ;
      }
   }  
   return (int) msg.wParam ;
}  //lint !e715

