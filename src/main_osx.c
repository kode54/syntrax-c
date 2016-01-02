#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <ao/ao.h>

#include "file.h"

#define SAMPLE_RATE 44100 /* 22050 - test low-pass capability of blep synthesizer */

static int running;

void signal_handler(int sig)
{
	running = 0;
    signal(sig, signal_handler);
}

int main(int argc, const char* const* argv)
{
    Song * song;
	Player * player;
	ao_device * dev;
    ao_sample_format fmt = { 16, SAMPLE_RATE, 2, AO_FMT_NATIVE, 0 };

	signed short sample_buffer[2048 * 2];

	if (argc != 2)
	{
		fprintf(stderr, "Usage:\t%s <song>\n", argv[0]);
		return 1;
	}
    
    song = File_loadSong(argv[1]);
    if (!song)
    {
        fprintf(stderr, "Invalid song:\t%s\n", argv[1]);
        return 1;
    }

    player = playerCreate(SAMPLE_RATE);
    if (!player)
    {
        fprintf(stderr, "Out of memory.\n");
        File_freeSong(song);
        return 1;
    }
    
    if (loadSong(player, song) < 0)
    {
        fprintf(stderr, "Out of memory.\n");
        playerDestroy(player);
        File_freeSong(song);
        return 1;
    }

    initSubsong(player, 0);

	signal(SIGINT, signal_handler);

	ao_initialize();

	dev = ao_open_live( ao_default_driver_id(), &fmt, NULL );

	if ( dev )
	{
        running = 1;
        while ( running )
		{
            mixChunk(player, sample_buffer, 2048);
			ao_play( dev, (char*) sample_buffer, 2048 * 4 );
		}

		ao_close( dev );
	}

	ao_shutdown();

    playerDestroy(player);
    File_freeSong(song);

	return 0;
}
