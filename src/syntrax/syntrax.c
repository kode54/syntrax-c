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
    memset(outbuff, 0, playbackBufferSize * 2);
}
        
void pausePlay(void)
{
    isPaused = 1;
}

void resumePlay(void)
{
    isPaused = 0;
}

void reset(void)
{
    int i, j;
    
    //dem assumptions
    if (delayBufferL && delayBufferR){
        memset(delayBufferL, 0, 65536);
        memset(delayBufferR, 0, 65536);
    }
    if (tuneChannels){
        
        for (i = 0; i < SE_MAXCHANS; i++) {
            TuneChannel *tc: = tuneChannels[i];
            
            tc.EQMIWERPIF = 0;
            tc.LJHG = 0;
            tc.insNum = -1;
            tc.HFRLJCG = 0;
            tc.ACKCWV = 0;
            tc.ELPHLDR = 0;
            tc.TVORFCC = 0;
            tc.freq = 0;
            tc.BNWIGU = 0;
            tc.UHYDBDDI = 0;
            tc.XESAWSO = 0;
            tc.JOEEPJCI = 0;
            tc.fmDelay = 0;
            tc.sampleBuffer = null;
            tc.smpLoopEnd = 0;
            //tuneChan.smpLength = 0;
            tc.sampPos = 0;
            tc.EYRXAB = 0;
            tc.volume = 0;
            tc.panning = 0;
            tc.VNVJPDIWAJQ = 0;
            tc.smpLoopStart = 0;
            tc.hasLoop = 0;
            tc.hasBidiLoop = 0;
            tc.isPlayingBackward = 0;
            tc.hasLooped = 0;
            
            for (j = 0; j < 4; j++) {
                VoiceEffect *voiceEffect = tc.effects[j];
                
                voiceEffect.QOMCBTRPXF = 0;
                voiceEffect.TIPUANVVR = 0;
                voiceEffect.MFATTMREMVP = 0;
                voiceEffect.MDTMBBIQHRQ = 0;
                voiceEffect.RKF = 0;
                voiceEffect.DQVLFV = 0;
                voiceEffect.ILHG = 0;
                voiceEffect.YLKJB = 0;
                voiceEffect.VMBNMTNBQU = 0;
                voiceEffect.ABJGHAUY = 0;
                voiceEffect.SPYK = 0;
                
            }
            
            memset(tc.synthBuffers, 0, 0x100 * SE_MAXCHANS + 1);
        }
    }
}
        
        void newSong(void)
        {
            var _local1:int;
            var i:int;
            var j:int;
            var _local4:Order;
            var _local6:Vector.<Order>;
            var _local7:Row;
            
            reset();
            AMYGPFQCHSW = 1;
            selectedSubsong = 0;
            WDTECTE = 8;
            AMVM = 0x0100;
            synSong.h.subsongNum = 1;
            synSong.h.version = 3457;
            subsongs = new Vector.<Subsong>();
            subsongs.push(new Subsong());
            var subs0:Subsong = subsongs[0];
            curSubsong = subsongs[selectedSubsong];
            
            subs0.tempo = 120;
            subs0.groove = 0;
            subs0.startPosCoarse = 0;
            subs0.startPosFine = 0;
            subs0.loopPosCoarse = 0;
            subs0.loopPosFine = 0;
            subs0.endPosCoarse = 1;
            subs0.endPosFine = 0;
            subs0.channelNumber = 4;
            subs0.delayTime = 0x8000;
            subs0.amplification = 400;
            subs0.chanDelayAmt = new Vector.<int>(SE_MAXCHANS, true);

            subs0.m_Name = "Empty";
            subs0.mutedChans  = new Vector.<int>(SE_MAXCHANS, true);
            
            
            subs0.orders = Tools.malloc_2DVector(Order, SE_MAXCHANS, 0x0100, true, true);
            for (i = 0; i < SE_MAXCHANS; i++) {
                _local6 = subs0.orders[i];
                for (j = 0; j < 0x0100; j++) {
                    _local6[j].patIndex = 0;
                    _local6[j].patLen = 0;
                }
            }
            
            synSong.h.patNum = 2;
            synSong.rows = Tools.malloc_1DVector(Row, 64 * synSong.h.patNum);
            
            for (i = 0; i < (64 * synSong.h.patNum); i++) {
                rows[i].dest = 0;
                rows[i].note = 0;
                rows[i].instr = 0;
                rows[i].command = 0;
                rows[i].spd = 0;
                
            }
            patternNames = new Vector.<String>();
            patternNames.push("Empty");
            patternNames.push("Pat1");
            synSong.h.instrNum = 1;
            synSong.instruments = new Vector.<Instrument>();
            synSong.instruments.push(new Instrument());
            
            mutedChans = new Vector.<int>(SE_MAXCHANS, true);
        }