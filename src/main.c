

#define WIN32_LEAN_AND_MEAN // for stripping windows.h include

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stddef.h>

#include <windows.h>  // for mixer stream
#include <mmsystem.h> // for mixer stream
#include <conio.h>    // keyboard input

#include "syntrax\syntrax.h"

#define BUFFNUM 8
#define RATE    44100
#define BUFFLEN ((RATE*2*2)/50)

HWAVEOUT     hWaveOut = INVALID_HANDLE_VALUE; /* Device handle */
WAVEFORMATEX wfx;
LPSTR        audblock;
char audiobuffer[BUFFNUM][BUFFLEN];

Song *sang = NULL;
Player *player = NULL;
syntrax_info info;
int max_channels = 0;

HANDLE eventh;

 /* Standard error macro for reporting API errors */ 
 #define PERR(bSuccess, api){if(!(bSuccess)) printf("%s:Error %d from %s \ 
    on line %d\n", __FILE__, GetLastError(), api, __LINE__);}

void cls( HANDLE hConsole )
{
    COORD coordScreen = { 0, 0 };    /* here's where we'll home the
                                        cursor */ 
    BOOL bSuccess;
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */ 
    DWORD dwConSize;                 /* number of character cells in
                                        the current buffer */ 

    /* get the number of character cells in the current buffer */ 

    bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
    PERR( bSuccess, "GetConsoleScreenBufferInfo" );
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    /* fill the entire screen with blanks */ 

    bSuccess = FillConsoleOutputCharacter( hConsole, (TCHAR) ' ',
       dwConSize, coordScreen, &cCharsWritten );
    PERR( bSuccess, "FillConsoleOutputCharacter" );

    /* get the current text attribute */ 

    bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
    PERR( bSuccess, "ConsoleScreenBufferInfo" );

    /* now set the buffer's attributes accordingly */ 

    bSuccess = FillConsoleOutputAttribute( hConsole, csbi.wAttributes,
       dwConSize, coordScreen, &cCharsWritten );
    PERR( bSuccess, "FillConsoleOutputAttribute" );

    /* put the cursor at (0, 0) */ 

    bSuccess = SetConsoleCursorPosition( hConsole, coordScreen );
    PERR( bSuccess, "SetConsoleCursorPosition" );
return;
}


void pressAny(void) {
    printf("Clicky key, get continue!\n");
    getchar();
}

BOOL init( char *name )
{
  //MMRESULT result;

  wfx.nSamplesPerSec  = RATE;
  wfx.wBitsPerSample  = 16;
  wfx.nChannels       = 2;

  wfx.cbSize          = 0;
  wfx.wFormatTag      = WAVE_FORMAT_PCM;
  wfx.nBlockAlign     = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
  wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
  
  sang = File_loadSong( name );
  if ( !sang ) return FALSE;
    
  player = playerCreate( RATE );
  if( !player ) return FALSE;
    
  if ( loadSong( player, sang ) < 0 ) return FALSE;

  eventh = CreateEvent(
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("WriteEvent")  // object name
        );

  if( waveOutOpen( &hWaveOut, WAVE_MAPPER, &wfx, (unsigned int)eventh, 0, CALLBACK_EVENT ) != MMSYSERR_NOERROR )
  {
    printf( "Unable to open waveout\n" );
    return FALSE;
  }

  return TRUE;
}

void shut( void )
{
  //if( synSong ) free( synSong );
  if( hWaveOut != INVALID_HANDLE_VALUE ) waveOutClose( hWaveOut );
}

void updateScreen(void)
{
    playerGetInfo(player, &info);
    cls( GetStdHandle( STD_OUTPUT_HANDLE ));
    printf("Syntrax test player v0.0001 || %i/%i\n", info.selectedSubs+1, info.totalSubs);
    printf("\ro: %3u - r: %2u - c: %2u (%2u)\n", info.coarse, info.fine, info.channelsPlaying, info.channelsPlaying > max_channels ? max_channels = info.channelsPlaying : max_channels);
    printf("Title: %s\n", info.subsongName);
}

int main(int argc, char *argv[])
{
  WAVEHDR header[BUFFNUM];
  int nextbuf = 0;
  //sigset_t base_mask, waiting_mask;

  if( argc < 2 )
  {
    //My F2 gets stuck, lol
    printf( "Usage: syntrax-c <tune.jxs>\n" );
    printf( "F3 and F4 keys change subtune.\n" );
    printf( "ESC closes.\n" );
    printf( "\n" );
    system("pause");
    return 0;
  }

  if( init( argv[1] ) )
  {
    int i;
    updateScreen();

    for ( i=0; i<BUFFNUM; i++ ){
        memset( &header[i], 0, sizeof( WAVEHDR ) );
        header[i].dwBufferLength = BUFFLEN;
        header[i].lpData         = (LPSTR)audiobuffer[i];
    }
    for ( i=0; i<BUFFNUM-1; i++ ){
        mixChunk(player, audiobuffer[nextbuf], BUFFLEN/4);
        waveOutPrepareHeader( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
        waveOutWrite( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
        nextbuf = (nextbuf+1)%BUFFNUM;
    }
    for(;;)
    {
      mixChunk(player, audiobuffer[nextbuf], BUFFLEN/4);
      waveOutPrepareHeader( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
      waveOutWrite( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
      nextbuf = (nextbuf+1)%BUFFNUM;

      
      while( waveOutUnprepareHeader( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) ) == WAVERR_STILLPLAYING ){
        if (_kbhit()) {
            int subnum;
            switch (_getch()) {
                case 0:  /* introduces an extended key */
                case 227:  /* this also happens on Win32 (I think) */
                    switch (_getch()) {  /* read the extended key code */
                        case 61:    //F3, subtune--
                            subnum = info.selectedSubs;
                            --subnum;
                            if (subnum < 0) subnum = info.totalSubs - 1;
                            
                            if (info.selectedSubs != subnum) initSubsong(player, subnum);
                            max_channels = 0;
                            updateScreen();
                            break;
                        case 62:    //F4, subtune++
                            subnum = info.selectedSubs;
                            subnum = ++subnum % info.totalSubs;
                            
                            if (info.selectedSubs != subnum) initSubsong(player, subnum);
                            max_channels = 0;
                            updateScreen();
                            break;
                        case 72:    //up arrow press
                            break;
                        case 75:    //left arrow press
                            break;
                        case 77:    //right arrow press
                            break;
                        case 80:    //down arrow press
                            break;
                        /* etc */
                    }
                    break;
                case 27:    //escape, exit
                    goto GTFO;
                    break;

                case 'H': /* capital 'H' key press */ break;
              /* etc */
            }
        }
        updateScreen();
        WaitForSingleObject(eventh, INFINITE);
      }
      ResetEvent(eventh);
    }

  }
  GTFO:
  //TODO: properly close WaveOut
  //too lazy
  playerDestroy(player);
  File_freeSong(sang);

  return 0;
}