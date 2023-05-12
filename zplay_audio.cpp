//**********************************************************************************
// This is an audio-file player implemented using the libZPlay library.
// https://libzplay.sourceforge.net/
// 
// build: g++ -Wall -O2 play_once.cpp -o play_once.exe -lzplay
//**********************************************************************************

// #define  STAND_ALONE    1

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

// include libzplay header file
#include <libzplay.h>

//lint -esym(534, libZPlay::ZPlay::SetEchoParam, SleepEx, libZPlay::ZPlay::Close)
//lint -esym(526, CreateZPlay)  Symbol not defined
//lint -e755   global macro not referenced
//lint -e757   global declarator not referenced
//lint -e758   global enum not referenced
//lint -e768   global struct member not referenced
//lint -e769   global enumeration constant not referenced
//lint -e1716  Virtual member function not referenced

// use libZPlay namespace
using namespace libZPlay;

//***************************************************************************
//  play mp3 (and other) file via zplay library
//***************************************************************************
int zplay_audio_file(char const * const mp3_file)
{
   if(mp3_file == NULL) {
      return 1 ;
   }
   // create class instance
   ZPlay *player = CreateZPlay();

   // chek if we have class instance
   if(player == 0)
   {
      // printf("Error: Can't create class instance !\nPress key to exit.\n");
      return 2;
   }

   // get library version
   int ver = player->GetVersion();
   // check if we have version 1.90 and above ( 1.90 if PF (pattent free) version, 2.00 version is full version
   if(ver < 190)
   {
      // printf("Error: Need library version 2.00 and above !\nPress key to exit.\r\n");
      player->Release();
      return 3;
   }

#ifdef  STAND_ALONE
   // display version info
   printf("libZPlay v.%i.%02i\n", ver / 100, ver % 100);
#endif   

   // check input arguments
      // open file using input argument as filename
   if(player->OpenFile(mp3_file, sfAutodetect) == 0)
   {
      // printf("OpenFile Error: %s\n", player->GetError());
      player->Release();
      return 4;
   }

   // program some echo efffect
   TEchoEffect effect[2];

   effect[0].nLeftDelay = 1000;
   effect[0].nLeftEchoVolume = 20;
   effect[0].nLeftSrcVolume = 80;
   effect[0].nRightDelay = 500;
   effect[0].nRightEchoVolume = 20;
   effect[0].nRightSrcVolume = 80;

   effect[1].nLeftDelay = 300;
   effect[1].nLeftEchoVolume = 20;
   effect[1].nLeftSrcVolume = 0;
   effect[1].nRightDelay = 300;
   effect[1].nRightEchoVolume = 20;
   effect[1].nRightSrcVolume = 0;

   // set echo effects
   player->SetEchoParam(effect, 2);
      
   // get stream info
   TStreamInfo pInfo;
   player->GetStreamInfo(&pInfo);

   // display stream info
#ifdef  STAND_ALONE
   printf("\r\n%s %i Hz %s  Channel: %i  Length: %02u:%02u:%02u:%02u\r\n\r\n",
       pInfo.Description,
       pInfo.SamplingRate,
       (pInfo.VBR) ? "VBR" : "CBR",
       pInfo.ChannelNumber,
       pInfo.Length.hms.hour,
       pInfo.Length.hms.minute,
       pInfo.Length.hms.second,
       pInfo.Length.hms.millisecond);
#endif       

   // start playing
   if(player->Play() == 0) {
      player->Release(); // delete ZPlay class
      return 5;
   }
   // SleepEx(pInfo.Length.hms.second, false);
   unsigned total_secs = (pInfo.Length.hms.second+1) * 1000 ;
   SleepEx(total_secs, false);

   player->Close();  // close open stream
   player->Release(); // destroy class
   return 0;
}

#ifdef  STAND_ALONE
//***************************************************************************
void usage(void)
{
   puts("Usage: play_once mp3_file_name");
}

//***************************************************************************
#define  MAX_PATH_LEN   1024

//lint -esym(818, argv)  Pointer parameter could be declared as pointing to const
int main(int argc, char **argv)
{
   char mp3_file[MAX_PATH_LEN+1] = "" ;
   int idx ;
   for (idx=1; idx<argc; idx++) {
      strncpy(mp3_file, argv[idx], MAX_PATH_LEN);
      mp3_file[MAX_PATH_LEN] = 0 ;
   }
   
   if (mp3_file[0] == 0) {
      usage() ;
      return 1 ;
   }
   int result = zplay_audio_file(mp3_file);
   if (result != 0) {
      printf("Error %d playing %s\n", result, mp3_file);
   }

   return 0;
}

#endif

