#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "syntrax.h"
#include "file.h"


int16_t *silentBuffer;
uint8_t m_LastNotes[SE_MAXCHANS];

uint32_t *freqTable;
int16_t *dynamorphTable;

int isPaused;
Song *synSong;
TuneChannel *tuneChannels;
Voice *voices;

int samplesPerBeat;
int otherSamplesPerBeat;
int someCounter;
int channelNumber;
int sePmSong;
int16_t *overlapBuff;
int16_t *delayBufferR;
int16_t *delayBufferL;

int bkpDelayPos;
int delayPos;
int gainPos;
int overlapPos;

int ISWLKT;
int WDTECTE;
int PQV;
int AMVM;
int PENIS;
int posCoarse;
int AMYGPFQCHSW;
int posFine;
int8_t mutedChans[SE_MAXCHANS];

int selectedSubsong;
Subsong *curSubsong;

//local pointers to song structures
Row *rows;
char **patternNames;
Instrument *instruments;
Subsong *subsongs;
int8_t *arpTable;
int16_t **samples;

uint bufflen;

int noAutoplay = 1;





void reset(void)
{
    int i, j;

    if (delayBufferL && delayBufferR){
        memset(delayBufferL, 0, 65536 *2);
        memset(delayBufferR, 0, 65536 *2);
    }
    if (tuneChannels){

        for (i = 0; i < SE_MAXCHANS; i++) {
            TuneChannel *tc = &tuneChannels[i];

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
            memset(&tc->synthBuffers, 0, 0x100 * SE_MAXCHANS *2 + 2);
        }
    }
}

void generateTables(void)
{
    int i, j;

    dynamorphTable = malloc(0x100 *2);
    for (i = 0; i < 0x0100; i++ ) {
        dynamorphTable[i] = (sin(((M_PI * i) / 128)) * 32760);
    }

    //debug Asscilloscope says 0xF8 to 0x61FFB
    //we probably don't have uint24_t at our disposal
    //uint32_t it is, then
    freqTable = malloc(SE_NROFFINETUNESTEPS * 128 *4);
    for (i = 0; i < SE_NROFFINETUNESTEPS; i++) {
        double x;
        for (j = 0; j < 128; j++) {
            x = (((j + 3) * 16) - i);
            x = (x / 192);
            x = pow(2, x);
            x = (x * 220) + 0.5;
            freqTable[i* 128 + j] = int(x);
        }
    }
}

void constructor(void)
{
    int i, j;
    
    srand(time(NULL));
    
    bufflen = BUFFERLENGTH;
    generateTables();
    
    overlapPos = 0;

    silentBuffer = malloc(0x0100 *2);
    memset(silentBuffer, 0, 0x0100 *2);

    ISWLKT = 0;
    posCoarse = 0;
    posFine = 0;
    bkpDelayPos = 0;
    PENIS = 0;
    AMYGPFQCHSW = 0;
    selectedSubsong = 0;
    curSubsong = NULL;
    PQV = 0;
    isPaused = 0;
    someCounter = 0;
    WDTECTE = 0;
    sePmSong = SE_PM_SONG;
    AMVM = 0x0100;
    channelNumber = 0;
    delayPos = 0;

    otherSamplesPerBeat = 2200;
    samplesPerBeat = 2200;

    overlapBuff  = malloc(SE_OVERLAP * 2 *2 + 2);
    delayBufferL = malloc(65536 *2);
    delayBufferR = malloc(65536 *2);

    tuneChannels = malloc(SE_MAXCHANS *sizeof(TuneChannel));
    voices = malloc(SE_MAXCHANS *sizeof(Voice));
    reset();

    //synSong = malloc(sizeof(Song));
}

/*
        public function destructor():void
        {
            var _local1:int;
            arpTable = null;
            silentBuffer = null;
            overlapBuff = null;
            delayBufferL = null;
            delayBufferR = null;
            freqTable = null;
            tuneChannels = null;
            silentBuffer = null;
            arpTable = null;
        }

        private function clearSongData():void
        {
            synSong.rows = null;
            synSong.patternNames = null;
            synSong.instruments = null;
            synSong.subsongs = null;
        }
*/

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
    Instrument  *ins = &instruments[tc->insNum];

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
                ve->DQVLFV = exp((-((2 * M_PI)) * (_local17 / 22000)));
                ve->RKF = (((-4 * ve->DQVLFV) / (1 + ve->DQVLFV)) * cos(((2 * M_PI) * (_local16 / 22000))));
                ve->MDTMBBIQHRQ = ((1 - ve->DQVLFV) * sqrt((1 - ((ve->RKF * ve->RKF) / (4 * ve->DQVLFV)))));

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
                ve->DQVLFV = exp((-((2 * M_PI)) * (_local17 / 22000)));
                ve->RKF = (((-4 * ve->DQVLFV) / (1 + ve->DQVLFV)) * cos(((2 * M_PI) * (_local16 / 22000))));
                ve->MDTMBBIQHRQ = ((1 - ve->DQVLFV) * sqrt((1 - ((ve->RKF * ve->RKF) / (4 * ve->DQVLFV)))));
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
                _local26 = int(((2 * M_PI) * _local31));
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
                _local26 = int(((2 * M_PI) * _local31));
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
                _local26 = int(((2 * M_PI) * _local31));
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
                    destBuff[j] = int((double(rand()) / double(RAND_MAX)) * 65530 - 0x8000);

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

void channelSomethingElse(int chanNum)
{
    int _local3;
    int _local4;
    int _local5;
    int _local6;

    TuneChannel *tc  = &tuneChannels[chanNum];
    Instrument  *ins = &instruments[tc->insNum];

    if (ins->amWave == 0){
        _local3 = 0;
    } else {
        tc->HFRLJCG = (tc->HFRLJCG + ins->amSpeed);
        if (tc->HFRLJCG >= 0x0100){
            tc->HFRLJCG = (tc->HFRLJCG - 0x0100);
            tc->HFRLJCG = (tc->HFRLJCG + ins->amLoopPoint);
            if (tc->HFRLJCG >= 0x0100){
                tc->HFRLJCG = ins->amLoopPoint;
            }
        }
        _local3 = tc->synthBuffers[ins->amWave - 1][tc->HFRLJCG];
        _local3 = (_local3 + 0x8000);
        _local3 = (_local3 / 6);
        _local3 = -(_local3);
        if (_local3 < -10000){
            _local3 = -10000;
        }
    }
    _local3 = (_local3 + 10000);
    _local3 = (_local3 * ins->masterVolume);
    _local3 = (_local3 >> 8);
    _local3 = (_local3 * AMVM);
    _local3 = (_local3 >> 8);
    _local3 = (_local3 - 10000);
    tc->volume = _local3;
    if (ins->panWave == 0){
        _local5 = 0;
    } else {
        tc->ELPHLDR = (tc->ELPHLDR + ins->panSpeed);
        if (tc->ELPHLDR >= 0x0100){
            tc->ELPHLDR = (tc->ELPHLDR - 0x0100);
            tc->ELPHLDR = (tc->ELPHLDR + ins->panLoopPoint);
            if (tc->ELPHLDR >= 0x0100){
                tc->ELPHLDR = ins->panLoopPoint;
            }
        }
        _local5 = tc->synthBuffers[ins->panWave - 1][tc->ELPHLDR];
        _local5 = (_local5 >> 7);
    }
    tc->panning = _local5;
    _local6 = 0;
    _local6 = arpTable[(ins->arpIndex * 16) + tc->ACKCWV];
    tc->ACKCWV++;
    tc->ACKCWV = (tc->ACKCWV & 15);
    _local4 = freqTable[ins->finetune*128 + (_local6 + tc->TVORFCC)];
    if (tc->fmDelay){
        tc->fmDelay--;
    } else {
        if (ins->fmWave != 0){
            tc->JOEEPJCI = (tc->JOEEPJCI + ins->fmSpeed);
            if (tc->JOEEPJCI >= 0x0100){
                tc->JOEEPJCI = (tc->JOEEPJCI - 0x0100);
                tc->JOEEPJCI = (tc->JOEEPJCI + ins->fmLoopPoint);
                if (tc->JOEEPJCI >= 0x0100){
                    tc->JOEEPJCI = ins->fmLoopPoint;
                }
            }
            _local4 = _local4 - tc->synthBuffers[ins->fmWave - 1][tc->JOEEPJCI];
        }
    }
    _local4 = (_local4 + tc->BNWIGU);
    tc->freq = _local4;
    if (tc->XESAWSO != 0){
        if (tc->XESAWSO > 0){
            if (tc->BNWIGU < tc->UHYDBDDI){
                tc->BNWIGU = (tc->BNWIGU + tc->XESAWSO);
                if (tc->BNWIGU > tc->UHYDBDDI){
                    tc->BNWIGU = tc->UHYDBDDI;
                }
            }
        } else {
            if (tc->BNWIGU > tc->UHYDBDDI){
                tc->BNWIGU = (tc->BNWIGU + tc->XESAWSO);
                if (tc->BNWIGU < tc->UHYDBDDI){
                    tc->BNWIGU = tc->UHYDBDDI;
                }
            }
        }
    }
}

void playInstrument(int chanNum, int instrNum, int note) //note: 1-112
{
    int j;
    int i;

    if (instrNum > synSong->h.instrNum){
        return;
    }
    if ((((tuneChannels[chanNum].insNum == -1)) && ((instrNum == 0)))){
        return;
    }

    TuneChannel *tc = &tuneChannels[chanNum];
    Voice *v        = &voices[chanNum];

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
                memcpy(&tc->synthBuffers[i], &ins->synthBuffers[i], 0x100 *2);
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

void patEffect(int note, int command, int dest, int spd, int chanNum)
{
    TuneChannel *tc = &tuneChannels[chanNum];
    Instrument *ins = &instruments[tc->insNum];

    if (tc->insNum == -1) return;
    int off;
    double tempo;
    switch (command) {

        //NONE
        case 0:
        default:
            return;

        //PITCHBEND
        case 1:
            if (tc->VNVJPDIWAJQ){
                off = freqTable[ins->finetune*128 + tc->VNVJPDIWAJQ];
                tc->TVORFCC = tc->VNVJPDIWAJQ;
            } else {
                off = freqTable[ins->finetune*128 + note];
            }
            tc->BNWIGU = 0;
            tc->UHYDBDDI = (freqTable[ins->finetune*128 + dest] - off);
            tc->XESAWSO = (spd * 20);
            tc->VNVJPDIWAJQ = dest;
            break;

        //CHNG WAVEFORM
        case 2:
            if (dest > 15){
                dest = 15;
            }
            ins->waveform = dest;
            break;

        //CHNG WAVELENGTH
        case 3:
            if (dest > 192){
                dest = 0x0100;
            } else {
                if (dest > 96){
                    dest = 128;
                } else {
                    if (dest > 48){
                        dest = 64;
                    } else {
                        dest = 32;
                    }
                }
            }
            ins->wavelength = dest;
            break;

        //CHNG MASTER VOL
        case 4:
            ins->masterVolume = dest;
            break;

        //CHNG AMWAVE
        case 5:
            if (dest > 15){
                dest = 15;
            }
            ins->amWave = dest;
            break;

        //CHNG AMSPD
        case 6:
            ins->amSpeed = dest;
            break;

        //CHNG AMLPPOINT
        case 7:
            ins->amLoopPoint = dest;
            break;

        //CHNG FINETUNE
        case 8:
            if (dest > 15){
                dest = 15;
            }
            ins->finetune = dest;
            break;

        //CHNG FMWAVE
        case 9:
            if (dest > 15){
                dest = 15;
            }
            ins->fmWave = dest;
            break;

        //CHNG FMSPD
        case 10:
            ins->fmSpeed = dest;
            break;

        //CHNG FMLPPOINT
        case 11:
            ins->fmLoopPoint = dest;
            break;

        //CHNG FMDELAY
        case 12:
            ins->fmDelay = dest;
            break;

        //CHNG ARPEGGIO
        case 13:
            if (dest > 15){
                dest = 15;
            }
            ins->arpIndex = dest;
            break;

        //CHNG EFF#1 DESTWAVE
        case 14:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[0].destWave = dest;
            break;

        //CHNG EFF#1 SRCWAVE1
        case 15:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[0].srcWave1 = dest;
            break;

        //CHNG EFF#1 SRCWAVE2
        case 16:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[0].srcWave2 = dest;
            break;

        //CHNG EFF#1 OSCWAVE
        case 17:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[0].oscWave = dest;
            break;

        //CHNG EFF#1 VARIABLE1
        case 18:
            ins->effects[0].variable1 = dest;
            break;

        //CHNG EFF#1 VARIABLE2
        case 19:
            ins->effects[0].variable2 = dest;
            break;

        //CHNG EFF#1 FXSPEED
        case 20:
            ins->effects[0].fxSpeed = dest;
            break;

        //CHNG EFF#1 OSCSPEED
        case 21:
            ins->effects[0].oscSpeed = dest;
            break;

        //CHNG EFF#1 OSCSELECT
        case 22:
            if (dest > 1){
                dest = 1;
            }
            ins->effects[0].oscSelect = dest;
            break;

        //CHNG EFF#1 TYPE
        case 23:
            if (dest >= SE_NROFEFFECTS){
                dest = (SE_NROFEFFECTS - 1);
            }
            ins->effects[0].effectType = dest;
            break;

        //CHNG EFF#1 RESETEFFECT
        case 24:
            if (dest > 1){
                dest = 1;
            }
            ins->effects[0].resetEffect = dest;
            break;

        //CHNG EFF#2 DESTWAVE
        case 25:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[1].destWave = dest;
            break;

        //CHNG EFF#2 SRCWAVE1
        case 26:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[1].srcWave1 = dest;
            break;

        //CHNG EFF#2 SRCWAVE2
        case 27:
            if (dest > 15){
                dest = 15;
            }
            //Good god! SrcEffect2 is left unplugged. I could guess what it was.
            //Luckly, I'm saved by one of the later effects.
            ins->effects[1].srcWave2 = dest;
            break;

        //CHNG EFF#2 OSCWAVE
        case 28:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[1].oscWave = dest;
            break;

        //CHNG EFF#2 VARIABLE1
        case 29:
            ins->effects[1].variable1 = dest;
            break;

        //CHNG EFF#2 VARIABLE2
        case 30:
            ins->effects[1].variable2 = dest;
            break;

        //CHNG EFF#2 FXSPEED
        case 31:
            ins->effects[1].fxSpeed = dest;
            break;

        //CHNG EFF#2 OSCSPEED
        case 32:
            ins->effects[1].oscSpeed = dest;
            break;

        //CHNG EFF#2 OSCSELECT
        case 33:
            if (dest > 1){
                dest = 1;
            }
            ins->effects[1].oscSelect = dest;
            break;

        //CHNG EFF#2 TYPE
        case 34:
            if (dest >= SE_NROFEFFECTS){
                dest = (SE_NROFEFFECTS - 1);
            }
            ins->effects[1].effectType = dest;
            break;

        //CHNG EFF#2 RESETEFFECT
        case 35:
            if (dest > 1){
                dest = 1;
            }
            ins->effects[1].resetEffect = dest;
            break;

        //CHNG EFF#3 DESTWAVE
        case 36:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[2].destWave = dest;
            break;

        //CHNG EFF#3 SRCWAVE1
        case 37:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[2].srcWave1 = dest;
            break;

        //CHNG EFF#3 SRCWAVE2
        case 38:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[2].srcWave2 = dest;
            break;

        //CHNG EFF#3 OSCWAVE
        case 39:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[2].oscWave = dest;
            break;

        //CHNG EFF#3 VARIABLE1
        case 40:
            ins->effects[2].variable1 = dest;
            break;

        //CHNG EFF#3 VARIABLE2
        case 41:
            ins->effects[2].variable2 = dest;
            break;

        //CHNG EFF#3 FXSPEED
        case 42:
            ins->effects[2].fxSpeed = dest;
            break;

        //CHNG EFF#3 OSCSPEED
        case 43:
            ins->effects[2].oscSpeed = dest;
            break;

        //CHNG EFF#3 OSCSELECT
        case 44:
            if (dest > 1){
                dest = 1;
            }
            ins->effects[2].oscSelect = dest;
            break;

        //CHNG EFF#3 TYPE
        case 45:
            if (dest >= SE_NROFEFFECTS){
                dest = (SE_NROFEFFECTS - 1);
            }
            ins->effects[2].effectType = dest;
            break;

        //CHNG EFF#3 RESETEFFECT
        case 46:
            if (dest > 1){
                dest = 1;
            }
            ins->effects[2].resetEffect = dest;
            break;

        //CHNG EFF#4 DESTWAVE
        case 47:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[3].destWave = dest;
            break;

        //CHNG EFF#4 SRCWAVE1
        case 48:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[3].srcWave1 = dest;
            break;

        //CHNG EFF#4 SRCWAVE2
        case 49:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[3].srcWave2 = dest;
            break;

        //CHNG EFF#4 OSCWAVE
        case 50:
            if (dest > 15){
                dest = 15;
            }
            ins->effects[3].oscWave = dest;
            break;

        //CHNG EFF#4 VARIABLE1
        case 51:
            ins->effects[3].variable1 = dest;
            break;

        //CHNG EFF#4 VARIABLE2
        case 52:
            ins->effects[3].variable2 = dest;
            break;

        //CHNG EFF#4 FXSPEED
        case 53:
            ins->effects[3].fxSpeed = dest;
            break;

        //CHNG EFF#4 OSCSPEED
        case 54:
            ins->effects[3].oscSpeed = dest;
            break;

        //CHNG EFF#4 OSCSELECT
        case 55:
            if (dest > 1){
                dest = 1;
            }
            ins->effects[3].oscSelect = dest;
            break;

        //CHNG EFF#4 TYPE
        case 56:
            if (dest >= SE_NROFEFFECTS){
                dest = (SE_NROFEFFECTS - 1);
            }
            ins->effects[3].effectType = dest;
            break;

        //CHNG EFF#4 RESETEFFECT
        case 57:
            if (dest > 1){
                dest = 1;
            }
            ins->effects[3].resetEffect = dest;
            break;

        //CHNG RESET WAVE #1
        case 58:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[0] = dest;
            break;

        //CHNG RESET WAVE #2
        case 59:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[1] = dest;
            break;

        //CHNG RESET WAVE #3
        case 60:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[2] = dest;
            break;

        //CHNG RESET WAVE #4
        case 61:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[3] = dest;
            break;

        //CHNG RESET WAVE #5
        case 62:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[4] = dest;
            break;

        //CHNG RESET WAVE #6
        case 63:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[5] = dest;
            break;

        //CHNG RESET WAVE #7
        case 64:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[6] = dest;
            break;

        //CHNG RESET WAVE #8
        case 65:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[7] = dest;
            break;

        //CHNG RESET WAVE #9
        case 66:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[8] = dest;
            break;

        //CHNG RESET WAVE #10
        case 67:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[9] = dest;
            break;

        //CHNG RESET WAVE #11
        case 68:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[10] = dest;
            break;

        //CHNG RESET WAVE #12
        case 69:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[11] = dest;
            break;

        //CHNG RESET WAVE #13
        case 70:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[12] = dest;
            break;

        //CHNG RESET WAVE #14
        case 71:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[13] = dest;
            break;

        //CHNG RESET WAVE #15
        case 72:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[14] = dest;
            break;

        //CHNG RESET WAVE #16
        case 73:
            if (dest > 1){
                dest = 1;
            }
            ins->m_ResetWave[15] = dest;
            break;

        //CHNG BPM
        case 74:
            if (dest <= 10){
                dest = 10;
            }
            if (dest > 220){
                dest = 220;
            }
            curSubsong->tempo = dest;
            tempo = double(curSubsong->tempo);
            tempo = (tempo / 60);
            tempo = (tempo * 32);
            samplesPerBeat = int(44100 / tempo);
            break;

        //CHNG GROOVE
        case 75:
            if (dest > 3){
                dest = 3;
            }
            curSubsong->groove = dest;
            break;

        //FIRE EXTERNAL EVENT
        case 76:
            //this effect is for controlling external code
            //like animation and whatnot
            //similar to how 8xx is used in Travolta's 'testlast' .mod

            //do take note of audio latency when dealing with this.

            //This is called UserEvent in ocx player doc, I think
            break;
    }
}

void channelSomething(int chanNum)
{
    int _local2;
    int _local3;
    int note;
    int dest;
    int contrScript;
    int spd;

    if (isPaused)           return;
    if (!PQV)               return;
    if (someCounter != WDTECTE) return;

    if (sePmSong == SE_PM_PATTERN){
        if (chanNum > 0) return;
        _local2 = AMYGPFQCHSW;
        _local3 = ISWLKT;
    } else {
        if (curSubsong->mutedChans[chanNum] == 1) return;

        _local3 = tuneChannels[chanNum].LJHG;
        _local2 = curSubsong->orders[chanNum][tuneChannels[chanNum].EQMIWERPIF].patIndex;
    }
    note = rows[(_local2 * 64) + _local3].note;
    if (note != 0) {
        playInstrument(chanNum, rows[(_local2 * 64) + _local3].instr, note);
    }
    contrScript = rows[(_local2 * 64) + _local3].command;
    dest = rows[(_local2 * 64) + _local3].dest;
    spd = rows[(_local2 * 64) + _local3].spd;
    patEffect(note, contrScript, dest, spd, chanNum);
}

void ABH(void)
{
    int i, j0, j1;
    int _local2;
    Order *_local3;
    int _local4;
    bool _local5;
    int _local6;
    Order *_local9;
    int _local10;
    int _local11;

    if (!PQV)     return;
    if (isPaused) return;

    someCounter--;
    if (someCounter == 0){
        if (sePmSong == SE_PM_PATTERN){
            if (!(ISWLKT & 1)){
                WDTECTE = (8 - curSubsong->groove);
            } else {
                WDTECTE = (8 + curSubsong->groove);
            }
        } else {
            if (!(posFine & 1)){
                WDTECTE = (8 - curSubsong->groove);
            } else {
                WDTECTE = (8 + curSubsong->groove);
            }
        }
        someCounter = WDTECTE;
        if (sePmSong == SE_PM_PATTERN){
            ISWLKT++;
            ISWLKT = (ISWLKT % PENIS);
        } else {

            for (i = 0; i < channelNumber; i++) {
                TuneChannel *tc = &tuneChannels[i];

                tc->LJHG++;
                _local3 = &curSubsong->orders[i];
                _local4 = tc->EQMIWERPIF;
                if (_local4 == -1){
                    _local4 = 0;
                }
                _local2 = &_local3[_local4].patLen;
                if ((((tc->LJHG == _local2)) || ((tc->EQMIWERPIF == -1)))){
                    tc->LJHG = 0;
                    tc->EQMIWERPIF++;
                    curSubsong->mutedChans[i] = mutedChans[i];
                }

            }
            posFine++;
            if (posFine == 64){
                posFine = 0;
                posCoarse++;
            }
            if ((((posCoarse == curSubsong->endPosCoarse)) && ((posFine == curSubsong->endPosFine)))){
                if (curSubsong->isLooping){
                    _local5 = false;

                    for (j0 = 0; j0 < SE_MAXCHANS; j0++) {
                        _local6 = 0;

                        _local11 = 0;
                        _local9 = &curSubsong->orders[j0];
                        _local10 = ((curSubsong->loopPosCoarse * 64) + curSubsong->loopPosFine);
                        for (j1 = 0; j1 < 0x0100; j1++) {
                            if (_local6 > _local10){
                                if (j1 != _local10){
                                    j1--;
                                }
                                break;
                            }
                            _local11 = _local6;
                            _local6 = (_local6 + _local9[j1].patLen);

                        }
                        if (j1 == 0x0100){
                            PQV = 0;
                            _local5 = true;
                            break;
                        }
                        _local10 = (_local10 - _local11);
                        _local10 = (_local10 & 63);
                        tuneChannels[j0].EQMIWERPIF = j1;
                        tuneChannels[j0].LJHG = _local10;

                    }
                    if (_local5 == false){
                        posCoarse = curSubsong->loopPosCoarse;
                        posFine = curSubsong->loopPosFine;
                    }
                } else {
                    PQV = 0;
                    isPaused = 0;
                    sePmSong = SE_PM_SONG;
                    posCoarse = curSubsong->startPosCoarse;
                    posFine = curSubsong->startPosFine;
                }
            }
        }
    }
}

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
    int amp, smp, pos;
    int16_t audioMainR, audioMainL;
    int16_t audioDelayR, audioDelayL;
    uint otherDelayTime;
    Voice *v;
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
                        *outBuff++ = audioMainR;
                        *outBuff++ = audioMainL;

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
    memset(outBuff, 0, playbackBufferSize * 2 *2);
}

void pausePlay(void)
{
    isPaused = 1;
}

void resumePlay(void)
{
    isPaused = 0;
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

void AAUCAPQW(void)
{
    //What is this even for?

    PQV = 0;
    isPaused = 0;
    sePmSong = SE_PM_SONG;
    if (subsongs){
        posCoarse = curSubsong->startPosCoarse;
        posFine = curSubsong->startPosFine;
    }
}

void IMXFLSSMB(int _arg1)
{
    PQV = 1;
    AMYGPFQCHSW = _arg1;
    ISWLKT = 63;
    someCounter = 1;
    sePmSong = SE_PM_PATTERN;
    WDTECTE = subsongs[0].tempo - subsongs[0].groove;
}

void initSubsong(int num)
{
    int _local2;
    int i, j;
    Order *_local5;
    bool _local6;
    int _local7;
    int _local8;
    double tempo;

    if (num >= synSong->h.subsongNum) return;

    selectedSubsong = num;
    curSubsong = &subsongs[selectedSubsong];
    channelNumber = curSubsong->channelNumber;
    reset();
    _local6 = false;

    for (i = 0; i < SE_MAXCHANS; i++) {
        m_LastNotes[i] = 0;
        _local2 = 0;

        _local8 = 0;
        _local5 = &curSubsong->orders[i];
        _local7 = (((curSubsong->startPosCoarse * 64) + curSubsong->startPosFine) - 1);
        for (j = 0; j < 0x0100; j++) {
            if (_local2 >= _local7){
                if (j != _local7){
                    j--;
                }
                break;
            }
            _local8 = _local2;
            _local2 = (_local2 + _local5[j].patLen);

        }
        if (j == 0x0100){
            _local6 = true;
            break;
        }
        _local7 = (_local7 - _local8);
        _local7 = (_local7 & 63);
        tuneChannels[i].EQMIWERPIF = j;
        tuneChannels[i].LJHG = _local7;
        curSubsong->mutedChans[i] = mutedChans[i];

    }
    if (_local6 == false){
        someCounter = 1;
        sePmSong = SE_PM_SONG;
        PQV = 1;
        isPaused = noAutoplay;
        WDTECTE = (8 + curSubsong->groove);
        if (curSubsong->tempo){
            tempo = double(curSubsong->tempo);
            tempo = (tempo / 60);
            tempo = (tempo * 32);
            samplesPerBeat = int((44100 / tempo));
            otherSamplesPerBeat = samplesPerBeat;
        }
        if (curSubsong->startPosFine == 0){
            posCoarse = curSubsong->startPosCoarse - 1;
        } else {
            posCoarse = curSubsong->startPosCoarse;
        }
        posFine = curSubsong->startPosFine - 1;
        posFine = posFine & 63;
    }
}

Song* loadSongFromFile(char *path)
{
    int i;

    AAUCAPQW();
    reset();
    //clearSongData();
    synSong = File_loadSong(path);
    if (!synSong) return NULL;

    //pass things locally
    //not much purpouse here
    //nor in AS3
    //why did I do it
    rows = synSong->rows;
    patternNames = synSong->patternNames;
    instruments = synSong->instruments;
    subsongs = synSong->subsongs;
    arpTable = synSong->arpTable;
    samples  = synSong->samples;

    for (i = 0; i < SE_MAXCHANS; i++) {
        mutedChans[i] = synSong->subsongs[0].mutedChans[i];
    }
    initSubsong(0);

    return synSong;
}

//<kode54> int16_t ** samples;
//<kode54> then you (re)alloc sizeof(int16_t*)*n for samples, then sizeof(int16_t)*n for samples[y]
