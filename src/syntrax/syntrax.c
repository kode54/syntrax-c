#include <stdint..h>

#include "syntrax.h"
#include "file.h"

void advanceTick()
{
    int i;
    ABH();
    
    for (i = 0; i < channelNumber; i++) {
        channelSomething(i);
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
                v = &voices[i]; tc = &tuneChannels[i];
                
                int insNum = tc->insNum;
                if ( insNum == -1 )
                {
                    v->waveBuff = &silentBuffer;
                    v->isSample = 0;
                }
                else if ( !tc->sampleBuffer )
                {
                    int waveNum  = instruments[insNum].waveform;
                    v->wavelength = (instruments[insNum].wavelength << 8) - 1;
                    v->waveBuff = &tc->synthBuffers[waveNum];
                    v->isSample = 0;
                }
                else
                {
                    v->waveBuff = &tc->sampleBuffer;
                    v->sampPos = tc->sampPos;
                    v->smpLoopStart = tc->smpLoopStart;
                    v->smpLoopEnd = tc->smpLoopEnd;
                    v->hasLoop = tc->hasLoop;
                    v->hasBidiLoop = tc->hasBidiLoop;
                    v->isPlayingBackward = tc->isPlayingBackward;
                    v->hasLooped = tc->hasLooped;
                    v->isSample = 1;
                }
                
                if ( tc->freq < 10 )
                    tc->freq = 10;
                
                v->gain = (tc->volume + 10000) / 39;
                v->delta = (tc->freq << 8) / SAMPLEFREQUENCY;
                
                if ( v->gain > 0x100 )
                    v->gain = 0x100;
                if ( tc->panning )
                {
                    if ( tc->panning <= 0 )
                    {
                        v->gainRight = 0x100;
                        v->gainLeft = 0x100 + tc->panning;
                    }
                    else
                    {
                        v->gainLeft = 0x100;
                        v->gainRight = 0x100 - tc->panning;
                    }
                }
                else
                {
                    v->gainRight = 0x100;
                    v->gainLeft = 0x100;
                }
                if ( dword_6632774C )
                {
                    //v->gainDelay = word_6632B9F4[i];
                }
                else
                {
                    v->gainDelay = curSubsong->chanDelayAmt[i];
                }
                v->gainRight      = (v->gain      * v->gainRight) >> 8;
                v->gainLeft       = (v->gain      * v->gainLeft) >> 8;
                v->gainDelayRight = (v->gainDelay * v->gainRight) >> 8;
                v->gainDelayLeft  = (v->gainDelay * v->gainLeft) >> 8;
            }
            if ( dword_6632774C )
            {
                //amp = word_6632B964;
                //otherDelayTime = word_6632BB24;
            }
            else
            {
                amp = curSubsong->amplification;
                otherDelayTime = curSubsong->delayTime / (44100 / SAMPLEFREQUENCY);
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
                                v = &voices[j];
                                if ( v->isSample == 1 )
                                {
                                    if ( v->sampPos != -1 )
                                    {
                                        //interpolation
                                        //smp = intp->interpSamp(v);
                                        smp = v->waveBuff[v->sampPos>>8];
                                        
                                        audioMainR  += (smp * v->gainRight) >> 8;
                                        audioMainL  += (smp * v->gainLeft) >> 8;
                                        audioDelayR += (smp * v->gainDelayRight) >> 8;
                                        audioDelayL += (smp * v->gainDelayLeft) >> 8;
                                        if ( v->isPlayingBackward )
                                        {
                                            v->sampPos -= v->delta;
                                            if ( v->sampPos <= v->smpLoopStart )
                                            {
                                                v->isPlayingBackward = 0;
                                                v->sampPos += v->delta;
                                            }
                                        }
                                        else
                                        {
                                            v->sampPos += v->delta;
                                            if ( v->sampPos >= v->smpLoopEnd )
                                            {
                                                if ( v->hasLoop )
                                                {
                                                    v->hasLooped = 1;
                                                    if ( v->hasBidiLoop )
                                                    {
                                                        v->isPlayingBackward = 1;
                                                        v->sampPos -= v->delta;
                                                    }
                                                    else
                                                    {
                                                        v->sampPos += v->smpLoopStart - v->smpLoopEnd;
                                                    }
                                                }
                                                else
                                                {
                                                    v->sampPos = -1;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    //interpolation
                                    //smp = intp->interpSynt(v);
                                    smp = v->waveBuff[v->synthPos>>8];
                                    
                                    audioMainR  += (smp * v->gainRight) >> 8;
                                    audioMainL  += (smp * v->gainLeft) >> 8;
                                    audioDelayR += (smp * v->gainDelayRight) >> 8;
                                    audioDelayL += (smp * v->gainDelayLeft) >> 8;
                                    v->synthPos += v->delta;
                                    v->synthPos &= v->wavelength;
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
                    v = &voices[i]; tc = &tuneChannels[i];
                    if ( v->isSample )
                    {
                        tc->sampPos = v->sampPos;
                        tc->isPlayingBackward = v->isPlayingBackward;
                        tc->hasLooped = v->hasLooped;
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
                                v = &voices[j];
                                if ( v->isSample == 1 )
                                {
                                    if ( v->sampPos != -1 )
                                    {
                                        //interpolation
                                        //smp = intp->interpSamp(v);
                                        smp = v->waveBuff[v->sampPos>>8];
                                        
                                        audioMainR  += (smp * v->gainRight) >> 8;
                                        audioMainL  += (smp * v->gainLeft) >> 8;
                                        audioDelayR += (smp * v->gainDelayRight) >> 8;
                                        audioDelayL += (smp * v->gainDelayLeft) >> 8;
                                        if ( v->isPlayingBackward )
                                        {
                                            v->sampPos -= v->delta;
                                            if ( v->sampPos <= v->smpLoopStart )
                                            {
                                                v->isPlayingBackward = 0;
                                                v->sampPos += v->delta;
                                            }
                                        }
                                        else
                                        {
                                            v->sampPos += v->delta;
                                            if ( v->sampPos >= v->smpLoopEnd )
                                            {
                                                if ( v->hasLoop )
                                                {
                                                    v->hasLooped = 1;
                                                    if ( v->hasBidiLoop )
                                                    {
                                                        v->isPlayingBackward = 1;
                                                        v->sampPos -= v->delta;
                                                    }
                                                    else
                                                    {
                                                        v->sampPos += v->smpLoopStart - v->smpLoopEnd;
                                                    }
                                                }
                                                else
                                                {
                                                    v->sampPos = -1;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    //interpolation
                                    //smp = intp->interpSynt(v);
                                    smp = v->waveBuff[v->synthPos>>8];
                                    
                                    audioMainR  += (smp * v->gainRight) >> 8;
                                    audioMainL  += (smp * v->gainLeft) >> 8;
                                    audioDelayR += (smp * v->gainDelayRight) >> 8;
                                    audioDelayL += (smp * v->gainDelayLeft) >> 8;
                                    v->synthPos += v->delta;
                                    v->synthPos &= v->wavelength;
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
    memset(outbuff, 0, playbackBufferSize*2 * 2);
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
        memset(delayBufferL, 0, 65536 * 2);
        memset(delayBufferR, 0, 65536 * 2);
    }
    if (tuneChannels){
        
        for (i = 0; i < SE_MAXCHANS; i++) {
            TuneChannel *tc: = &tuneChannels[i];
            
            tc->EQMIWERPIF = 0;
            tc->LJHG = 0;
            tc->insNum = -1;
            tc->HFRLJCG = 0;
            tc->ACKCWV = 0;
            tc->ELPHLDR = 0;
            tc->TVORFCC = 0;
            tc->freq = 0;
            tc->BNWIGU = 0;
            tc->UHYDBDDI = 0;
            tc->XESAWSO = 0;
            tc->JOEEPJCI = 0;
            tc->fmDelay = 0;
            tc->sampleBuffer = NULL;
            tc->smpLoopEnd = 0;
            //tc->smpLength = 0;
            tc->sampPos = 0;
            tc->EYRXAB = 0;
            tc->volume = 0;
            tc->panning = 0;
            tc->VNVJPDIWAJQ = 0;
            tc->smpLoopStart = 0;
            tc->hasLoop = 0;
            tc->hasBidiLoop = 0;
            tc->isPlayingBackward = 0;
            tc->hasLooped = 0;
            
            for (j = 0; j < 4; j++) {
                VoiceEffect *vc = &tc->effects[j];
                
                vc->QOMCBTRPXF = 0;
                vc->TIPUANVVR = 0;
                vc->MFATTMREMVP = 0;
                vc->MDTMBBIQHRQ = 0;
                vc->RKF = 0;
                vc->DQVLFV = 0;
                vc->ILHG = 0;
                vc->YLKJB = 0;
                vc->VMBNMTNBQU = 0;
                vc->ABJGHAUY = 0;
                vc->SPYK = 0;
                
            }
            
            memset(tc->synthBuffers, 0, 0x100*16 * 2 + 2);
        }
    }
}
        
        /*void newSong(void)
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
            AMVM = 0x100;
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
        }*/

void playInstrument(int chanNum, int instrNum, int note) //note: 1-112
{
    int j;
    int i;
    
    if (instrNum > synSong.h.instrNum){
        return;
    }
    if ((((tuneChannels[chanNum].insNum == -1)) && ((instrNum == 0)))){
        return;
    }
    
    TuneChannel *tc = tuneChannels[chanNum];
    Voice *v        = voices[chanNum];
    
    tc->ACKCWV = 0;
    tc->HFRLJCG = 0;
    tc->ELPHLDR = 0;
    tc->JOEEPJCI = 0;
    tc->TVORFCC = note;
    tc->freq = 0;
    tc->VNVJPDIWAJQ = note;
    tc->BNWIGU = 0;
    tc->UHYDBDDI = 0;
    tc->XESAWSO = 0;
    m_LastNotes[chanNum] = note;
    
    Instrument *ins;
    if (instrNum != 0) {
        ins = &instruments[instrNum - 1];
        
        if (ins->shareSmpDataFromInstr == 0){
            tc->sampleBuffer = samples[instrNum - 1];
        } else {
            tc->sampleBuffer = samples[ins->shareSmpDataFromInstr - 1];
        }
        tc->sampPos           = ins->smpStartPoint << 8;
        tc->smpLoopStart      = ins->smpLoopPoint << 8;
        tc->smpLoopEnd        = ins->smpEndPoint << 8;
        tc->hasLoop           = ins->hasLoop;
        tc->hasBidiLoop       = ins->hasBidiLoop;
        tc->hasLooped         = 0;
        tc->isPlayingBackward = 0;
        tc->EYRXAB            = -1;
        tc->fmDelay           = ins->fmDelay;
        
        for (i = 0; i < 16; i++) {
            if (ins->m_ResetWave[i]){
                //ins->synthBuffers[i].copyTo(tc.synthBuffers[i]);
                memcpy(&tc.synthBuffers[i], &ins->synthBuffers[i], 0x100 * 2);
            }
        }
        tc->insNum = instrNum - 1;
    }
    ins = &instruments[tc->insNum];
    
    for (j = 0; j < 4; j++) {
        if (ins->effects[j].effectType != 0){
            if (ins->effects[j].resetEffect) {
                VoiceEffect *ve = &tc->effects[j];
                
                ve->MFATTMREMVP = 0;
                ve->QOMCBTRPXF = 0;
                ve->TIPUANVVR = 0;
                ve->YLKJB = 0;
                ve->ILHG = 0;
                ve->VMBNMTNBQU = 0;
                ve->ABJGHAUY = 0;
                ve->SPYK = 0;
            }
        }
    }
}

        void instrEffect(int chanNum)
        {
            //TODO: minimize all the vars
            //too many of them
            int i, j;
            int _local3;
            int destWave;
            int16_t *destBuff;
            int pos;
            int srcWave1;
            int _local10;
            int srcWave2;
            int16_t *srcBuff1;
            int16_t *srcBuff2;
            double _local16;
            double _local17;
            double _local18;
            int oscWave;
            int16_t *oscBuff;
            int _local21;
            int _local22;
            int _local23;
            int _local25;
            int _local26;
            int _local27;
            int var1;
            int var2;
            double _local30;
            double _local31;
            int _local32;
            int _local33;
            int _local34;
            int _local35;
            int _local36;
            int _local37;
            int _local38;
            int _local39;
            int _local40;
            int _local43;
            
            TuneChannel *tc  = &tuneChannels[chanNum];
            Instrument  *ins = &instruments[tc.insNum];
            
            for (i = 0; i < 4; i++ ) {
                InstrumentEffect *ie = &ins->effects[i];
                VoiceEffect *ve      = &tc->effects[i];
                
                ve->MFATTMREMVP = (ve->MFATTMREMVP + ie->oscSpeed);
                ve->MFATTMREMVP = (ve->MFATTMREMVP & 0xFF);
                switch (ie->effectType) {
                    
                    //NONE
                    case 0:
                        break;
                        
                    //NEGATE
                    case 1:
                        destWave = ie->destWave;
                        _local3 = ie->fxSpeed;
                        pos = ve->QOMCBTRPXF;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        for (j = 0; j < _local3; j++ ) {
                            pos++;
                            pos = (pos & 0xFF);
                            destBuff[pos] = (0 - destBuff[pos]);
                            
                        }
                        ve->QOMCBTRPXF = pos;
                        break;
                        
                    //SWEEP
                    case 2:
                        destWave = ie->destWave;
                        _local3 = ie->fxSpeed;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        
                        for (pos = 0, j = 0; j < 0x0100; j++ ) {
                            destBuff[j] = (destBuff[j] + pos);
                            destBuff[j] = (destBuff[j] + 0x8000);
                            destBuff[j] = (destBuff[j] & 0xFFFF);
                            destBuff[j] = (destBuff[j] - 0x8000);
                            pos = (pos + _local3);
                            
                        }
                        break;
                        
                    //AVERAGER
                    case 3:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        destBuff = &tc->synthBuffers[destWave];
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        _local3 = ie->fxSpeed;
                        pos = 0;
                        if (_local3 > 12){
                            _local3 = 12;
                        }
                        
                        for (_local10 = 0; _local10 < _local3; _local10++ ) {
                            destBuff[0] = ((srcBuff1[0xFF] + srcBuff1[1]) >> 1);
                            
                            for (j = 1; j < 0xFF; j++ ) {
                                destBuff[j] = ((srcBuff1[j - 1] + srcBuff1[j + 1]) >> 1);
                                
                            }
                            destBuff[0xFF] = ((srcBuff1[254] + srcBuff1[0]) >> 1);
                            
                        }
                        break;
                        
                    //WAVEMIX
                    case 4:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        srcWave2 = ie->srcWave2;
                        destBuff = &tc->synthBuffers[destWave];
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        srcBuff2 = &tc->synthBuffers[srcWave2];
                        _local3 = ie->fxSpeed;
                        ve->QOMCBTRPXF = (ve->QOMCBTRPXF + _local3);
                        ve->QOMCBTRPXF = (ve->QOMCBTRPXF & 0xFF);
                        pos = ve->QOMCBTRPXF;
                        
                        for (j = 0; j < 0x0100; j++ ) {
                            destBuff[j] = ((srcBuff1[j] + srcBuff2[pos]) >> 1);
                            pos++;
                            pos = (pos & 0xFF);
                            
                        }
                        break;
                        
                    //FILTER
                    case 5:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        srcWave2 = ie->oscWave - 1;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        srcBuff2 = srcWave2 >= 0 ? &tc->synthBuffers[srcWave2] : NULL;
                        pos = ve->MFATTMREMVP;
                        
                        if (ie->oscWave == 0){
                            _local16 = double((ie->variable1 * 20));
                            _local17 = double((ie->variable2 * 16));
                        } else {
                            if (ie->oscSelect){
                                _local16 = double((ie->variable1 * 20));
                                _local17 = (double((srcBuff2[pos] + 0x8000)) / 16);
                            } else {
                                _local16 = (double((srcBuff2[pos] + 0x8000)) / 13);
                                _local17 = double((ie->variable2 * 16));
                            }
                        }
                        ve->DQVLFV = Math.exp((-((2 * Math.PI)) * (_local17 / 22000)));
                        ve->RKF = (((-4 * ve->DQVLFV) / (1 + ve->DQVLFV)) * Math.cos(((2 * Math.PI) * (_local16 / 22000))));
                        ve->MDTMBBIQHRQ = ((1 - ve->DQVLFV) * Math.sqrt((1 - ((ve->RKF * ve->RKF) / (4 * ve->DQVLFV)))));
                        
                        for (j = 0; j < 0x0100; j++) {
                            _local18 = (((ve->MDTMBBIQHRQ * (double(srcBuff1[j]) / 0x8000)) - (ve->RKF * ve->ILHG)) - (ve->DQVLFV * ve->YLKJB));
                            ve->YLKJB = ve->ILHG;
                            ve->ILHG = _local18;
                            if (_local18 > 0.9999){
                                _local18 = 0.9999;
                            }
                            if (_local18 < -0.9999){
                                _local18 = -0.9999;
                            }
                            destBuff[j] = (_local18 * 0x8000);
                            
                        }
                        break;
                        
                    //FILTWHISTLE
                    case 6:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        srcWave2 = ie->oscWave - 1;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        srcBuff2 = srcWave2 >= 0 ? &tc->synthBuffers[srcWave2] : NULL;
                        pos = ve->MFATTMREMVP;
                        
                        if (ie->oscWave == 0){
                            _local16 = double((ie->variable1 * 20));
                            _local17 = double((ie->variable2 * 16));
                        } else {
                            if (ie->oscSelect){
                                _local16 = double((ie->variable1 * 20));
                                _local17 = (double((srcBuff2[pos] + 0x8000)) / 16);
                            } else {
                                _local16 = (double((srcBuff2[pos] + 0x8000)) / 13);
                                _local17 = double((ie->variable2 * 16));
                            }
                        }
                        ve->DQVLFV = Math.exp((-((2 * Math.PI)) * (_local17 / 22000)));
                        ve->RKF = (((-4 * ve->DQVLFV) / (1 + ve->DQVLFV)) * Math.cos(((2 * Math.PI) * (_local16 / 22000))));
                        ve->MDTMBBIQHRQ = ((1 - ve->DQVLFV) * Math.sqrt((1 - ((ve->RKF * ve->RKF) / (4 * ve->DQVLFV)))));
                        ve->DQVLFV = (ve->DQVLFV * 1.2);
                        
                        for (j = 0; j < 0x0100; j++ ) {
                            _local18 = (((ve->MDTMBBIQHRQ * (double(srcBuff1[j]) / 0x8000)) - (ve->RKF * ve->ILHG)) - (ve->DQVLFV * ve->YLKJB));
                            ve->YLKJB = ve->ILHG;
                            ve->ILHG = _local18;
                            if (_local18 > 0.9999){
                                _local18 = 0.9999;
                            }
                            if (_local18 < -0.9999){
                                _local18 = -0.9999;
                            }
                            destBuff[j] = (_local18 * 0x8000);
                            
                        }
                        break;
                        
                    //MORPH
                    case 7:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        srcWave2 = ie->srcWave2;
                        oscWave  = ie->oscWave - 1;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        srcBuff2 = &tc->synthBuffers[srcWave2];
                        oscBuff  = oscWave >= 0 ? &tc->synthBuffers[oscWave] : NULL;
                        pos = ve->MFATTMREMVP;
                        
                        if (ie->oscWave == 0){
                            _local21 = ie->variable1;
                        } else {
                            if (ie->oscSelect){
                                _local21 = ie->variable1;
                            } else {
                                _local21 = ((oscBuff[pos] + 0x8000) / 0x0100);
                            }
                        }
                        _local22 = (0xFF - _local21);
                        
                        for (j = 0; j < 0x0100; j++) {
                            _local23 = (((srcBuff1[j] * _local21) / 0x0100) + ((srcBuff2[j] * _local22) / 0x0100));
                            destBuff[j] = _local23;
                            
                        }
                        break;
                        
                    //DYNAMORPH
                    case 8:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        srcWave2 = ie->srcWave2;
                        oscWave  = ie->oscWave - 1;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        srcBuff2 = &tc->synthBuffers[srcWave2];
                        oscBuff = oscWave >= 0 ? &tc->synthBuffers[oscWave] : NULL;
                        pos = ve->MFATTMREMVP;
                        
                        if (ie->oscWave == 0){
                            _local25 = ie->variable1;
                        } else {
                            if (ie->oscSelect){
                                _local25 = ie->variable1;
                            } else {
                                _local25 = ((oscBuff[pos] + 0x8000) / 0x0100);
                            }
                        }
                        
                        for (j = 0; j < 0x0100; j++) {
                            _local21 = ((dynamorphTable[_local25] >> 8) + 128);
                            _local22 = (0xFF - _local21);
                            _local23 = (((srcBuff1[j] * _local21) / 0x0100) + ((srcBuff2[j] * _local22) / 0x0100));
                            destBuff[j] = _local23;
                            _local25++;
                            _local25 = (_local25 & 0xFF);
                            
                        }
                        break;
                        
                    //DISTORTION
                    case 9:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        oscWave  = ie->oscWave - 1;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        oscBuff  = oscWave >= 0 ? &tc->synthBuffers[oscWave] : NULL;
                        pos = ve->MFATTMREMVP;
                        
                        if (ie->oscWave == 0){
                            _local21 = ie->variable1;
                        } else {
                            if (ie->oscSelect){
                                _local21 = ie->variable1;
                            } else {
                                _local21 = ((oscBuff[pos] + 0x8000) / 0x0100);
                            }
                        }
                        
                        for (j = 0; j < 0x0100; j++) {
                            _local23 = ((srcBuff1[j] * _local21) / 16);
                            _local23 = (_local23 + 0x8000);
                            if (_local23 < 0){
                                _local23 = -(_local23);
                            }
                            _local23 = (_local23 % 131072);
                            if (_local23 > 0xFFFF){
                                _local23 = (131071 - _local23);
                            }
                            _local23 = (_local23 & 0xFFFF);
                            _local23 = (_local23 - 0x8000);
                            destBuff[j] = _local23;
                            
                        }
                        break;
                        
                    //SCROLL LEFT
                    case 10:
                        destWave = ie->destWave;
                        destBuff = &tc->synthBuffers[destWave];
                        _local10 = destBuff[0];
                        
                        for (j = 0; j < 0xFF; j++) {
                            destBuff[j] = destBuff[j + 1];
                            
                        }
                        destBuff[0xFF] = _local10;
                        break;
                        
                    //UPSAMPLE
                    case 11:
                        pos = ve->QOMCBTRPXF;
                        if (pos != 0){
                            ve->QOMCBTRPXF--;
                            break;
                        }
                        ve->QOMCBTRPXF = ie->variable1;
                        destWave = ie->destWave;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        for (j = 0; j < 128; j++) {
                            destBuff[j] = destBuff[j * 2];
                            
                        }
                        
                        for (j = 0; j < 128; j++) {
                            destBuff[j + 128] = destBuff[j];
                            
                        }
                        break;
                        
                    //CLIPPER
                    case 12:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        oscWave  = ie->oscWave - 1;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        oscBuff  = oscWave >= 0 ? &tc->synthBuffers[oscWave] : NULL;
                        pos = ve->MFATTMREMVP;
                        
                        if (ie->oscWave == 0){
                            _local21 = ie->variable1;
                        } else {
                            if (ie->oscSelect){
                                _local21 = ie->variable1;
                            } else {
                                _local21 = ((oscBuff[pos] + 0x8000) / 0x0100);
                            }
                        }
                        
                        for (j = 0; j < 0x0100; j++) {
                            _local23 = ((srcBuff1[j] * _local21) / 16);
                            if (_local23 < -32767){
                                _local23 = -32767;
                            }
                            if (_local23 > 32767){
                                _local23 = 32767;
                            }
                            destBuff[j] = _local23;
                            
                        }
                        break;
                        
                    //LOWPASS
                    case 13:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        srcWave2 = ie->oscWave - 1;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        srcBuff2 = srcWave2 >= 0 ? &tc->synthBuffers[srcWave2] : NULL;
                        pos = ve->MFATTMREMVP;
                        
                        if (ie->oscWave == 0){
                            var1 = ie->variable1;
                            var2 = ie->variable2;
                            var1 = var1 * 16;
                        } else {
                            if (ie->oscSelect){
                                var1 = ie->variable1;
                                var2 = ((srcBuff2[pos] + 0x8000) >> 8);
                                var1 = (var1 * 16);
                            } else {
                                //_local28 = ((_local14->data[_local7] + 0x8000) / 16);
                                var1 = ((srcBuff2[pos] + 0x8000) >> 4);
                                var2 = ie->variable2;
                            }
                        }
                        _local30 = (var1 - 920);
                        _local31 = (228 + var1);
                        _local26 = int(((2 * Math.PI) * _local31));
                        _local27 = (707 + ((1000 * var2) / 128));
                        _local36 = ve->ABJGHAUY;
                        _local37 = ve->SPYK;
                        _local38 = ve->VMBNMTNBQU;
                        _local40 = 8;
                        
                        for (j = 0; j < 0x0100; j++) {
                            _local32 = ((_local26 * _local40) / 100);
                            _local39 = srcBuff1[j];
                            _local33 = ((((_local36 * 1000) / _local27) - _local37) + _local39);
                            _local34 = (_local36 - ((_local32 * (_local38 / 100)) / 100));
                            _local35 = (_local37 - ((_local32 * (_local36 / 100)) / 100));
                            _local38 = _local33;
                            _local36 = _local34;
                            _local37 = _local35;
                            _local3 = _local37;
                            if (_local3 > 32767){
                                _local3 = 32767;
                            }
                            if (_local3 < -32767){
                                _local3 = -32767;
                            }
                            destBuff[j] = _local3;
                            
                        }
                        ve->ABJGHAUY = _local36;
                        ve->SPYK = _local37;
                        ve->VMBNMTNBQU = _local38;
                        break;
                        
                    //HIGHPASS
                    case 14:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        srcWave2 = ie->oscWave - 1;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        srcBuff2 = srcWave2 >= 0 ? &tc->synthBuffers[srcWave2] : NULL;
                        pos = ve->MFATTMREMVP;
                        
                        if (ie->oscWave == 0){
                            var1 = ie->variable1;
                            var2 = ie->variable2;
                            var1 = (var1 * 32);
                        } else {
                            if (ie->oscSelect) {
                                var1 = ie->variable1;
                                var2 = ((srcBuff2[pos] + 0x8000) >> 8);
                                var1 = (var1 * 32);
                            } else {
                                //checked with IDA against windows ver. of Syntrax(v1.03)
                                //It's possible that the effect has changed along the way(v2.xx)
                                //same for lowpass
                                //_local28 = ((_local14->data[_local7] + 0x8000) / 16);
                                var1 = ((srcBuff2[pos] + 0x8000) >> 3);
                                var2 = ie->variable2;
                            }
                        }
                        _local30 = (var1 - 920);
                        _local31 = (228 + var1);
                        _local26 = int(((2 * Math.PI) * _local31));
                        _local27 = (707 + ((1000 * var2) / 128));
                        _local36 = ve->ABJGHAUY;
                        _local37 = ve->SPYK;
                        _local38 = ve->VMBNMTNBQU;
                        _local40 = 8;
                        
                        for (j = 0; j < 0x0100; j++) {
                            _local32 = ((_local26 * _local40) / 100);
                            _local39 = srcBuff1[j];
                            _local33 = ((((_local36 * 1000) / _local27) - _local37) + _local39);
                            _local34 = (_local36 - ((_local32 * (_local38 / 100)) / 100));
                            _local35 = (_local37 - ((_local32 * (_local36 / 100)) / 100));
                            _local38 = _local33;
                            _local36 = _local34;
                            _local37 = _local35;
                            _local3 = _local38;
                            if (_local3 > 32767){
                                _local3 = 32767;
                            }
                            if (_local3 < -32767){
                                _local3 = -32767;
                            }
                            destBuff[j] = _local3;
                            
                        }
                        ve->ABJGHAUY = _local36;
                        ve->SPYK = _local37;
                        ve->VMBNMTNBQU = _local38;
                        break;
                        
                    //BANDPASS
                    case 15:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        srcWave2 = ie->oscWave - 1;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        srcBuff2 = srcWave2 >= 0 ? &tc->synthBuffers[srcWave2] : NULL;
                        pos = ve->MFATTMREMVP;
                        
                        if (ie->oscWave == 0){
                            var1 = ie->variable1;
                            var2 = ie->variable2;
                            var1 = var1 * 16;
                        } else {
                            if (ie->oscSelect){
                                var1 = ie->variable1;
                                var2 = ((srcBuff2[pos] + 0x8000) >> 8);
                                var1 = var1 * 16;
                            } else {
                                var1 = ((srcBuff2[pos] + 0x8000) / 16);
                                var2 = ie->variable2;
                            }
                        }
                        _local30 = (var1 - 920);
                        _local31 = (228 + var1);
                        _local26 = int(((2 * Math.PI) * _local31));
                        _local27 = (707 + ((1000 * var2) / 128));
                        _local36 = ve->ABJGHAUY;
                        _local37 = ve->SPYK;
                        _local38 = ve->VMBNMTNBQU;
                        _local40 = 8;
                        
                        for (j = 0; j < 0x0100; j++) {
                            _local32 = ((_local26 * _local40) / 100);
                            _local39 = srcBuff1[j];
                            _local33 = ((((_local36 * 1000) / _local27) - _local37) + _local39);
                            _local34 = (_local36 - ((_local32 * (_local38 / 100)) / 100));
                            _local35 = (_local37 - ((_local32 * (_local36 / 100)) / 100));
                            _local38 = _local33;
                            _local36 = _local34;
                            _local37 = _local35;
                            _local3 = _local36;
                            if (_local3 > 32767){
                                _local3 = 32767;
                            }
                            if (_local3 < -32767){
                                _local3 = -32767;
                            }
                            destBuff[j] = _local3;
                            
                        }
                        ve->ABJGHAUY = _local36;
                        ve->SPYK = _local37;
                        ve->VMBNMTNBQU = _local38;
                        break;
                        
                    //METALNOISE
                    case 16:
                        destWave = ie->destWave;
                        destBuff = &tc->synthBuffers[destWave];
                        for (j = 0; j < 0x0100; j++ ) {
                            //Something very bad happens here
                            //I think it's fixed now.
                            destBuff[j] = ((Math.random() * 65530) - 0x8000);
                            
                        }
                        break;
                        
                    //SQUASH
                    case 17:
                        destWave = ie->destWave;
                        srcWave1 = ie->srcWave1;
                        oscWave  = ie->oscWave - 1;
                        destBuff = &tc->synthBuffers[destWave];
                        
                        srcBuff1 = &tc->synthBuffers[srcWave1];
                        
                        oscBuff = oscWave >= 0 ? &tc->synthBuffers[oscWave] : NULL;
                        pos = ve->MFATTMREMVP;
                        
                        if (ie->oscWave == 0){
                            var1 = ie->variable1;
                            var2 = ie->variable2;
                        } else {
                            if (ie->oscSelect){
                                var1 = ie->variable1;
                                var2 = ((oscBuff[pos] + 0x8000) >> 8);
                            } else {
                                var1 = ((oscBuff[pos] + 0x8000) >> 8);
                                var2 = ie->variable2;
                            }
                        }
                        
                        var2 = (var2 << 8);
                        var1 = (var1 + var2);
                        _local22 = 0;
                        
                        int butt, ron, pat, buf2, buf1;
                        for (j = 0; j < 0x0100; j++ ) {
                            //Hex Rays decompiler is lovely tool.
                            //butt        = (butt & 0xFFFF0000) | (_local22 & 0x0000FFFF);
                            butt        = _local22 & 0xFFFF;
                            _local22   += var1;
                            //ron         = (ron  & 0xFFFFFF00) | (  butt   & 0x000000FF);
                            ron         = butt & 0xFF;
                            butt      >>= 8;
                            buf1        = srcBuff1[butt];
                            //overflow warning
                            buf2        = srcBuff1[butt + 1];
                            pat         = (255 - ron) * buf1;
                            destBuff[j] = (ron * buf2 >> 8) + (pat >> 8);
                        }
                        break;
                }
            }
        }

        
//<kode54> int16_t ** samples;
//<kode54> then you (re)alloc sizeof(int16_t*)*n for samples, then sizeof(int16_t)*n for samples[y]