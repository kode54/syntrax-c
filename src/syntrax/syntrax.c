#include <stdint.h>

#include "syntrax.h"
#include "file.h"

void advanceTick()
{
    int i;
    ABH();
    
    for (i = 0; i < channelNumber; i++) {
        channelSomething(i);
        Voice *v = voices[i];
        if (tuneChannels[i].insNum != -1) {
            channelSomethingElse(i);
            instrEffect(i);
        }
    }
}

void mixChunk(int16_t *outBuff, uint playbackBufferSize)
{
    
    int i, j;
    uint sampleNum;
    int amp, smp, pos:int;
    int16_t audioMainR, audioMainL;
    int16_t audioDelayR, audioDelayL;
    uint otherDelayTime;
    Voice *v
    TuneChannel *tc;
    
    //We just don't know!
    uint dword_6632774C = 0;
    
    if ( channelNumber > 0 )
    {
        if ( playbackBufferSize & 1) playbackBufferSize--;
        if ( playbackBufferSize <= 0 ) return;
        
        while ( playbackBufferSize > 0 )
        {
            if ( otherSamplesPerBeat >= playbackBufferSize )
            {
                otherSamplesPerBeat = otherSamplesPerBeat - playbackBufferSize;
                sampleNum = playbackBufferSize;
            }
            else
            {
                sampleNum = otherSamplesPerBeat;
                otherSamplesPerBeat = samplesPerBeat * SAMPLEFREQUENCY / 44100;
            }
            playbackBufferSize -= sampleNum;
            
            for (i=0; i < channelNumber; ++i )
            {
                v = voices[i]; tc = tuneChannels[i];
                
                int insNum = tc.insNum;
                if ( insNum == -1 )
                {
                    v.waveBuff = silentBuffer;
                    v.isSample = 0;
                }
                else if ( !tc.sampleBuffer )
                {
                    int waveNum  = instruments[insNum].waveform;
                    v.wavelength = (instruments[insNum].wavelength << 8) - 1;
                    v.waveBuff = tc.synthBuffers[waveNum];
                    v.isSample = 0;
                }
                else
                {
                    v.waveBuff = tc.sampleBuffer;
                    v.sampPos = tc.sampPos;
                    v.smpLoopStart = tc.smpLoopStart;
                    v.smpLoopEnd = tc.smpLoopEnd;
                    v.hasLoop = tc.hasLoop;
                    v.hasBidiLoop = tc.hasBidiLoop;
                    v.isPlayingBackward = tc.isPlayingBackward;
                    v.hasLooped = tc.hasLooped;
                    v.isSample = 1;
                }
                
                if ( tc.freq < 10 )
                    tc.freq = 10;
                
                v.gain = (tc.volume + 10000) / 39;
                v.delta = (tc.freq << 8) / SAMPLEFREQUENCY;
                
                if ( v.gain > 0x100 )
                    v.gain = 0x100;
                if ( tc.panning )
                {
                    if ( tc.panning <= 0 )
                    {
                        v.gainRight = 0x100;
                        v.gainLeft = 0x100 + tc.panning;
                    }
                    else
                    {
                        v.gainLeft = 0x100;
                        v.gainRight = 0x100 - tc.panning;
                    }
                }
                else
                {
                    v.gainRight = 0x100;
                    v.gainLeft = 0x100;
                }
                if ( dword_6632774C )
                {
                    //v.gainDelay = word_6632B9F4[i];
                }
                else
                {
                    v.gainDelay = curSubsong.chanDelayAmt[i];
                }
                v.gainRight      = (v.gain      * v.gainRight) >> 8;
                v.gainLeft       = (v.gain      * v.gainLeft) >> 8;
                v.gainDelayRight = (v.gainDelay * v.gainRight) >> 8;
                v.gainDelayLeft  = (v.gainDelay * v.gainLeft) >> 8;
            }
            if ( dword_6632774C )
            {
                //amp = word_6632B964;
                //otherDelayTime = word_6632BB24;
            }
            else
            {
                amp = curSubsong.amplification;
                otherDelayTime = curSubsong.delayTime / (44100 / SAMPLEFREQUENCY);
            }
            if ( outBuff )
            {
                if ( sampleNum > 0 )
                {
                    for(i = 0; i < sampleNum; i++)
                    {
                        audioMainR = 0;
                        audioMainL = 0;
                        audioDelayR = 0;
                        audioDelayL = 0;
                        if ( channelNumber > 0 )
                        {
                            for (j = 0; j < channelNumber; ++j)
                            {
                                v = voices[j];
                                if ( v.isSample == 1 )
                                {
                                    if ( v.sampPos != -1 )
                                    {
                                        //interpolation
                                        //smp = intp.interpSamp(v);
                                        smp = v.waveBuff[v.sampPos>>8];
                                        
                                        audioMainR  += (smp * v.gainRight) >> 8;
                                        audioMainL  += (smp * v.gainLeft) >> 8;
                                        audioDelayR += (smp * v.gainDelayRight) >> 8;
                                        audioDelayL += (smp * v.gainDelayLeft) >> 8;
                                        if ( v.isPlayingBackward )
                                        {
                                            v.sampPos -= v.delta;
                                            if ( v.sampPos <= v.smpLoopStart )
                                            {
                                                v.isPlayingBackward = 0;
                                                v.sampPos += v.delta;
                                            }
                                        }
                                        else
                                        {
                                            v.sampPos += v.delta;
                                            if ( v.sampPos >= v.smpLoopEnd )
                                            {
                                                if ( v.hasLoop )
                                                {
                                                    v.hasLooped = 1;
                                                    if ( v.hasBidiLoop )
                                                    {
                                                        v.isPlayingBackward = 1;
                                                        v.sampPos -= v.delta;
                                                    }
                                                    else
                                                    {
                                                        v.sampPos += v.smpLoopStart - v.smpLoopEnd;
                                                    }
                                                }
                                                else
                                                {
                                                    v.sampPos = -1;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    //interpolation
                                    //smp = intp.interpSynt(v);
                                    smp = v.waveBuff[v.synthPos>>8];
                                    
                                    audioMainR  += (smp * v.gainRight) >> 8;
                                    audioMainL  += (smp * v.gainLeft) >> 8;
                                    audioDelayR += (smp * v.gainDelayRight) >> 8;
                                    audioDelayL += (smp * v.gainDelayLeft) >> 8;
                                    v.synthPos += v.delta;
                                    v.synthPos &= v.wavelength;
                                }
                            }
                        }
                        
                        audioMainL = (delayBufferL[delayPos] + audioMainL / channelNumber) / 2;
                        audioMainR = (delayBufferR[delayPos] + audioMainR / channelNumber) / 2;
                        audioMainR = audioMainR * amp / 100;
                        audioMainL = audioMainL * amp / 100;
                        //clip audio
                        if ( audioMainR < -32760 ) audioMainR = -32760;
                        if ( audioMainR >  32760 ) audioMainR = 32760;
                        if ( audioMainL < -32760 ) audioMainL = -32760;
                        if ( audioMainL >  32760 ) audioMainL = 32760;
                        
                        //interleaved buffer
                        if ( overlapPos < SE_OVERLAP )
                        {
                            audioMainR  = overlapPos * audioMainR / 100;
                            audioMainR += (SE_OVERLAP - overlapPos) * overlapBuff[overlapPos*2]   / 100;
                            audioMainL  = overlapPos * audioMainL / 100;
                            audioMainL += (SE_OVERLAP - overlapPos) * overlapBuff[overlapPos*2+1] / 100;
                            ++overlapPos;
                        }
                        
                        //output
                        *outbuff++ = audioMainR;
                        *outbuff++ = audioMainL;
                        
                        delayBufferL[delayPos] = (((audioDelayL / channelNumber) + delayBufferL[delayPos]) / 2);
                        delayBufferR[delayPos] = (((audioDelayR / channelNumber) + delayBufferR[delayPos]) / 2);
                        delayPos = ++delayPos % otherDelayTime;
                    }
                }
            }
            
            if ( channelNumber > 0 )
            {
                for (i = 0; i < channelNumber; ++i)
                {
                    v = voices[i]; tc = tuneChannels[i];
                    if ( v.isSample )
                    {
                        tc.sampPos = v.sampPos;
                        tc.isPlayingBackward = v.isPlayingBackward;
                        tc.hasLooped = v.hasLooped;
                    }
                }
            }
            if ( otherSamplesPerBeat == (samplesPerBeat * SAMPLEFREQUENCY) / 44100 )
            {
                bkpDelayPos = delayPos;
                for (i = 0; i < channelNumber; i++) voices[i].bkpSynthPos = voices[i].synthPos;
                
                overlapPos = 0;
                if ( outBuff )
                {
                    for (i = 0; i < SE_OVERLAP; i++)
                    {
                        audioMainR = 0;
                        audioMainL = 0;
                        audioDelayR = 0;
                        audioDelayL = 0;
                        if ( channelNumber > 0 )
                        {
                            for (j = 0; j < channelNumber; j++)
                            {
                                v = voices[j];
                                if ( v.isSample == 1 )
                                {
                                    if ( v.sampPos != -1 )
                                    {
                                        //interpolation
                                        //smp = intp.interpSamp(v);
                                        smp = v.waveBuff[v.sampPos>>8];
                                        
                                        audioMainR  += (smp * v.gainRight) >> 8;
                                        audioMainL  += (smp * v.gainLeft) >> 8;
                                        audioDelayR += (smp * v.gainDelayRight) >> 8;
                                        audioDelayL += (smp * v.gainDelayLeft) >> 8;
                                        if ( v.isPlayingBackward )
                                        {
                                            v.sampPos -= v.delta;
                                            if ( v.sampPos <= v.smpLoopStart )
                                            {
                                                v.isPlayingBackward = 0;
                                                v.sampPos += v.delta;
                                            }
                                        }
                                        else
                                        {
                                            v.sampPos += v.delta;
                                            if ( v.sampPos >= v.smpLoopEnd )
                                            {
                                                if ( v.hasLoop )
                                                {
                                                    v.hasLooped = 1;
                                                    if ( v.hasBidiLoop )
                                                    {
                                                        v.isPlayingBackward = 1;
                                                        v.sampPos -= v.delta;
                                                    }
                                                    else
                                                    {
                                                        v.sampPos += v.smpLoopStart - v.smpLoopEnd;
                                                    }
                                                }
                                                else
                                                {
                                                    v.sampPos = -1;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    //interpolation
                                    //smp = intp.interpSynt(v);
                                    smp = v.waveBuff[v.synthPos>>8];
                                    
                                    audioMainR  += (smp * v.gainRight) >> 8;
                                    audioMainL  += (smp * v.gainLeft) >> 8;
                                    audioDelayR += (smp * v.gainDelayRight) >> 8;
                                    audioDelayL += (smp * v.gainDelayLeft) >> 8;
                                    v.synthPos += v.delta;
                                    v.synthPos &= v.wavelength;
                                }
                            }
                        }
                        
                        audioMainL = (delayBufferL[delayPos] + audioMainL / channelNumber) / 2;
                        audioMainR = (delayBufferR[delayPos] + audioMainR / channelNumber) / 2;
                        audioMainR = audioMainR * amp / 100;
                        audioMainL = audioMainL * amp / 100;
                        //clip audio
                        if ( audioMainR < -32760 ) audioMainR = -32760;
                        if ( audioMainR >  32760 ) audioMainR = 32760;
                        if ( audioMainL < -32760 ) audioMainL = -32760;
                        if ( audioMainL >  32760 ) audioMainL = 32760;
                        
                        overlapBuff[i * 2] = audioMainR;
                        overlapBuff[i*2+1] = audioMainL;
                        
                        delayPos = ++delayPos % otherDelayTime;
                    }
                }
                delayPos = bkpDelayPos;
                for (i = 0; i < channelNumber; i++) voices[i].synthPos = voices[i].bkpSynthPos;
                
                //dword_66327200 = 2 * sampleNum;
                advanceTick();
            }
        }
        return;
    }
    if ( playbackBufferSize <= 0 ) return;
    //blank write to playback buffer
    for ( i = 0; i < playbackBufferSize; i++ )
    {
        *outbuff++ = 0;
        *outbuff++ = 0;
    }
}
        
void pausePlay(void)
{
    isPaused = 1;
}

void resumePlay(void)
{
    isPaused = 0;
}

