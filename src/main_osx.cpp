#include <stdio.h>
#include <stdint.h>
#include <signal.h>

#include "audio_output.h"

#include "file.h"

#define SAMPLE_RATE 44100 /* 22050 - test low-pass capability of blep synthesizer */

static int running;

static Player * player;
static volatile int fade_start, fade_length;
static volatile int max_channels;

void signal_handler(int sig)
{
	running = 0;
    signal(sig, signal_handler);
}

void fade_buffer(signed short *buffer, unsigned int count, int fade_start, int fade_length)
{
    unsigned int i;
    for (i = 0; i < count; i++)
    {
        if (fade_start < fade_length)
        {
            buffer[ i * 2 + 0 ] = (int64_t)((int64_t)buffer[ i * 2 + 0 ] * ( fade_length - fade_start )) / fade_length;
            buffer[ i * 2 + 1 ] = (int64_t)((int64_t)buffer[ i * 2 + 1 ] * ( fade_length - fade_start )) / fade_length;
            fade_start++;
        }
        else
        {
            buffer[ i * 2 + 0 ] = 0;
            buffer[ i * 2 + 1 ] = 0;
        }
    }
}

void render(void * unused, short * samples, uint32_t sampleCount)
{
    syntrax_info info;
    mixChunk(player, samples, sampleCount);
    if (playerGetSongEnded(player)) running = 0;
    if (playerGetLoopCount(player) >= 2)
    {
        fade_buffer( samples, sampleCount, fade_start, fade_length );
        fade_start += sampleCount;
    }
    playerGetInfo(player, &info);
    fprintf(stderr, "\ro: %3u - r: %2u - c: %2u (%2u)", info.coarse, info.fine, info.channelsPlaying, info.channelsPlaying > max_channels ? max_channels = info.channelsPlaying : max_channels);
}

int main(int argc, const char* const* argv)
{
    Song * song;
    CoreAudioStream * output;

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

    output = new CoreAudioStream(render, 0, SAMPLE_RATE);
    
	if ( output )
	{
        fade_start = 0; fade_length = SAMPLE_RATE * 10;
        max_channels = 0;
        running = 1;
        output->start();
        while ( running && fade_start < fade_length )
		{
            usleep(10000);
		}
        output->close();
        fprintf(stderr, "\n");
	}

    delete output;
    
    playerDestroy(player);
    File_freeSong(song);

	return 0;
}
