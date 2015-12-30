

#define WIN32_LEAN_AND_MEAN // for stripping windows.h include

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stddef.h>

#include <windows.h>  // for mixer stream
#include <mmsystem.h> // for mixer stream

#include "syntrax\syntrax.h"

#define BUFFNUM 8

HWAVEOUT     hWaveOut = INVALID_HANDLE_VALUE; /* Device handle */
WAVEFORMATEX wfx;
LPSTR        audblock;
char audiobuffer[BUFFNUM][((44100*2*2)/50)];

Song *sang = NULL;

HANDLE eventh;

BOOL init( char *name )
{
  //MMRESULT result;

  wfx.nSamplesPerSec  = 44100;
  wfx.wBitsPerSample  = 16;
  wfx.nChannels       = 2;

  wfx.cbSize          = 0;
  wfx.wFormatTag      = WAVE_FORMAT_PCM;
  wfx.nBlockAlign     = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
  wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

  constructor();

  sang = loadSongFromFile(name);
  resumePlay();
  if( !sang ) return FALSE;

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

int main(int argc, char *argv[])
{
  WAVEHDR header[BUFFNUM];
  int nextbuf = 0;
  //sigset_t base_mask, waiting_mask;

  printf( "Syntrax test player v0.000000001\n" );

  if( argc < 2 )
  {
    printf( "Usage: play_hvl <tune>\n" );
    return 0;
  }

  if( init( argv[1] ) )
  {
    int i;

    for ( i=0; i<BUFFNUM; i++ ){
        memset( &header[i], 0, sizeof( WAVEHDR ) );
        header[i].dwBufferLength = ((44100*2*2)/50);
        header[i].lpData         = (LPSTR)audiobuffer[i];
    }

    for ( i=0; i<BUFFNUM-1; i++ ){
        mixChunk(audiobuffer[nextbuf], 1024);
        waveOutPrepareHeader( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
        waveOutWrite( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
        nextbuf = (nextbuf+1)%BUFFNUM;
    }

    for(;;)
    {
      mixChunk(audiobuffer[nextbuf], 1024);
      waveOutPrepareHeader( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
      waveOutWrite( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) );
      nextbuf = (nextbuf+1)%BUFFNUM;

      // Don't do this in your own player or plugin :-)
      //while( waveOutUnprepareHeader( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) ) == WAVERR_STILLPLAYING ) ;
      while( waveOutUnprepareHeader( hWaveOut, &header[nextbuf], sizeof( WAVEHDR ) ) == WAVERR_STILLPLAYING ){
        WaitForSingleObject(eventh, INFINITE);
      }
      ResetEvent(eventh);
    }

  }
  shut();

  return 0;
}

