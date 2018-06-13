#ifndef SYNTRAX_H
#define SYNTRAX_H

#include <stdint.h>

//----------------------------typedefs-------------------------
#define PACKED __attribute__((packed))
typedef unsigned uint;
typedef enum { false, true } bool;

//<monty> hold the fuck up
//<monty> >#define double(x) ((double)x)
//<monty> you're dead to me
//hahahahahahahahaha
//TODO: remove these after
#ifndef __cplusplus
    #define double(x) ((double)x)
    #define float(x)  ((float)x)
    #define uint(x)   ((uint)x)
    #define int(x)    ((int)x)
#endif

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
//player structs
//don't pack these
typedef struct
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
    int     smpLength;
    int     gainDelayRight;
    int     gainDelayLeft;
    int     hasLooped;
} Voice;

typedef struct
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
} VoiceEffect;

typedef struct
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
    int16_t     *sampleBuffer;
    //SQUASH effect overflows into next buffer
    //SE_MAXCHANS * 0x100 + 1 must be allocated
    int16_t     *synthBuffers;
    int         smpLoopStart;
    int         smpLoopEnd;
    int         hasLooped;
    VoiceEffect effects[4];
} TuneChannel;


//data structs

#pragma pack(push,1)
typedef struct
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
} PACKED InstrumentEffect;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
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
    uint8_t             m_ResetWave[SE_MAXCHANS];
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
    //why do we even need to store a full path?
    //only filename appears to be used.
    char                smpFullImportPath[192];
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
    int16_t             shareSmpDataFromInstr; //0 is off
    int16_t             hasLoop;
    int16_t             hasBidiLoop;
    uint32_t            smpStartPoint;
    uint32_t            smpLoopPoint;
    uint32_t            smpEndPoint;
    uint32_t            hasSample;
    uint32_t            smpLength;
    int16_t             synthBuffers[SE_MAXCHANS][0x100];
} PACKED Instrument;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint8_t note;
    uint8_t dest;
    uint8_t instr;
    int8_t  spd;
    uint8_t command;
} PACKED Row;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    int16_t patIndex;   //0 means empty
    int16_t patLen;
} PACKED Order;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t UNK00[16];
    //UNK00 is used for something. No idea what.
    //There is a sequence to the data in it.
    //zeroing it out with hex editor doesn't seem to break stuff with Jaytrax
    //it could as well be uninitialized memory
    uint8_t     mutedChans[SE_MAXCHANS];
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
    uint8_t     chanDelayAmt[SE_MAXCHANS];
    int16_t     amplification;
    int16_t     UNK01;
    int16_t     UNK02;
    int16_t     UNK03;
    int16_t     UNK04;
    int16_t     UNK05;
    int16_t     UNK06;
    int16_t     UNK07;
    //if my eyes don't deceive me, this actually happens
    //waste of space
    Order orders[SE_MAXCHANS][0x100];
} PACKED Subsong;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
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
} PACKED SongHeader;
#pragma pack(pop)


//no need to pack
typedef struct
{
    SongHeader h;
    int8_t arpTable[0x100];

    Row *rows;
    //we don't know what maximum pat name length should be
    //in fact this is probably a buffer overflow target in Syntrax(app crashed on too long name, from UI);
    uint32_t *patNameSizes;
    char **patternNames;
    Instrument *instruments;
    Subsong *subsongs;
    int16_t **samples;
} Song;

//---------------------------prototypes------------------------
void constructor(void);
void mixChunk(int16_t *outBuff, uint playbackBufferSize);
void pausePlay(void);
void resumePlay(void);
void reset(void);
//void newSong(void);
void playInstrument(int chanNum, int instrNum, int note); //could be handy dandy
void initSubsong(int num);
Song loadSongFromFile(char *path);

#endif
