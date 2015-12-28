#ifndef SYNTRAX_H
#define SYNTRAX_H

#include <stdint.h>

//----------------------------defines--------------------------

#define SE_NROFEFFECTS          18
#define SE_MAXCHANS             16
#define SE_NROFFINETUNESTEPS    16
                            
#define SE_OVERLAP              100
                            
#define BUFFERLENGTH            4096
#define CHANNELS                2
#define BITRATE                 16
#define SAMPLEFREQUENCY         44100
#define RESAMPLE_POINTS         4

//some constants for controlling stuff
//I've no idea what the abreviations stand for
#define SE_PM_SONG              0
#define SE_PM_PATTERN           1


//---------------------------structures------------------------
//don't pack
struct VoiceEffect
{
    int     VMBNMTNBQU;
    int     TIPUANVVR;
    double  MDTMBBIQHRQ;
    double  YLKJB;
    double  DQVLFV;
    int     MFATTMREMVP;
    double  ILHG;
    double  RKF;
    int     SPYK;
    int     QOMCBTRPXF;
    int     ABJGHAUY;
};

//don't pack
struct Voice
{
    int     wavelength;
    int     gainDelay;
    int     delta;
    int     freq;
    int     isSample;
    int     isPlayingBackward;
    int     smpLoopEnd;
    int     PPMXBYLQ; //unused
    int     gain;
    int16_t *waveBuff;
    int     hasBidiLoop;
    int     synthPos;
    int     gainRight;
    int     smpLoopStart;
    int     bkpSynthPos;
    int     sampPos;
    int     gainLeft;
    int     hasLoop;
    int     smpLength:;
    int     gainDelayRight;
    int     gainDelayLeft;
    int     hasLooped;
};

//don't pack
struct TuneChannel
{
    int         JOEEPJCI;
    int         BNWIGU;
    int         isPlayingBackward;
    int         ELPHLDR;
    int         panning;
    int         TVORFCC;
    int         EYRXAB;          //rather unused
    int         UHYDBDDI;
    int         XESAWSO;
    int         hasBidiLoop;
    int         fmDelay;
    int         volume;
    int         ACKCWV;
    int         sampPos;
    int         insNum;
    int         EQMIWERPIF;
    int         freq;
    int         HFRLJCG;
    int         VNVJPDIWAJQ;
    int         hasLoop;
    int         LJHG;
    int16_r     *sampleBuffer;
    int16_t     *synthBuffers;       //16 * 0x100 + 1
    int         smpLoopStart;
    int         smpLoopEnd;
    int         hasLooped;
    VoiceEffect effects[4];
};

//almost packable
struct Subsong
{
    uint32_t UNK00[16];
    //UNK00 is used for something. No idea what.
    //There is a sequence to the data in it.
    //zeroing it out with hex editor doesn't seem to break stuff with Jaytrax
    //it could as well be uninitialized memory
    uint8_t     mutedChans[16];
    uint32_t    tempo;
    uint32_t    groove;
    uint32_t    startPosCoarse;
    uint32_t    startPosFine;
    uint32_t    endPosCoarse;
    uint32_t    endPosFine;
    uint32_t    loopPosCoarse;
    uint32_t    loopPosFine;
    int16_t     isLooping;
    char        m_Name[32];
    int16_t     channelNumber;
    uint16_t    delayTime;
    uint8_t     chanDelayAmt[16];
    int16_t     amplification;
    int16_t     UNK01;
    int16_t     UNK02;
    int16_t     UNK03;
    int16_t     UNK04;
    int16_t     UNK05;
    int16_t     UNK06;
    int16_t     UNK07;
    //move me
    Order *orders;
};

//almost packable
struct Subsong
{
    uint16_t    version;
    uint16_t    UNK00;
    uint32_t    patNum;
    uint32_t    subsongNum;
    uint32_t    instrNum;
    uint32_t    UNK01;
    int16_t     UNK02;
    int16_t     UNK03;
    int16_t     UNK04;
    int16_t     UNK05;
    int16_t     UNK06;
    int16_t     UNK07;
    int16_t     UNK08;
    int16_t     UNK09;
    int16_t     UNK0A;
    int16_t     UNK0B;
    int16_t     UNK0C;
    int16_t     UNK0D;
    int16_t     UNK0E;
    int16_t     UNK0F;
    int16_t     UNK10;
    int16_t     UNK11;
        
    //move me
    public var rows:Vector.<Row>;                   //*Row[]
    public var patNameSizes:Vector.<int>;           //*uint32le[]    -----\
    public var patternNames:Vector.<String>;        //*char[]        -----/
    public var instruments:Vector.<Instrument>;     //*Instrument[]
    public var subsongs:Vector.<Subsong>;           //*Subsong[]
    public var arpTable:Vector.<int>;               //*int8[]
};

//packable
struct Order
{
    int16_t patIndex;   //0 means empty
    int16_t patLen;
};

//packable
struct Row
{
    uint8_t note;
    uint8_t dest;
    uint8_t instr;
    int8_t  spd;
    uint8_t command;
};

//almost packable
struct Instrument
{
    int16_t             version;
    char                name[32];
    int16_t             waveform;
    int16_t             wavelength;
    int16_t             masterVolume;
    int16_t             amWave;
    int16_t             amSpeed;
    int16_t             amLoopPoint;
    int16_t             finetune;
    int16_t             fmWave;
    int16_t             fmSpeed;
    int16_t             fmLoopPoint;
    int16_t             fmDelay;
    int16_t             arpIndex;
    uint8_t             m_ResetWave[16];
    int16_t             panWave;
    int16_t             panSpeed;
    int16_t             panLoopPoint;
    int16_t             UNK00;
    int16_t             UNK01;
    int16_t             UNK02;
    int16_t             UNK03;
    int16_t             UNK04;
    int16_t             UNK05;
    InstrumentEffect    effects[4];
    char                smpFullImportPath[192]; //why do we even need to store a full path?
    uint32_t            UNK06;
    uint32_t            UNK07;
    uint32_t            UNK08;
    uint32_t            UNK09;
    uint32_t            UNK0A;
    uint32_t            UNK0B;
    uint32_t            UNK0C;
    uint32_t            UNK0D;
    uint32_t            UNK0E;
    uint32_t            UNK0F;
    uint32_t            UNK10;
    uint32_t            UNK11;
    int16_t             UNK12;
    int16_t             shareSmpDataFromInstr;           //0 is off
    int16_t             hasLoop;
    int16_t             hasBidiLoop;
    uint32_t            smpStartPoint;
    uint32_t            smpLoopPoint;
    uint32_t            smpEndPoint;
    uint32_t            hasSample;
    uint32_t            smpLength;
    int16_t             synthBuffers[16][0x100];
    //move me
    int16_t *sampleBuffer;
};

//packable
struct InstrumentEffect
{
    uint32_t    destWave;
    uint32_t    srcWave1;
    uint32_t    srcWave2;
    uint32_t    oscWave;
    uint32_t    variable1;
    uint32_t    variable2;
    uint32_t    fxSpeed;
    uint32_t    oscSpeed;
    uint32_t    effectType;
    int8_t      in5oscSelect;
    int8_t      resetEffect;
    int16_t     UNK00;
};
#endif       