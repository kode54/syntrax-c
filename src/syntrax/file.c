#include <stdint.h>
#include <stdio.h>
#include <syntrax.h>

size_t filesize;

Song loadSong(char *path)
{
    var i:int, j:int, k:int;
    int songVer;
    Subsong *subs;
    Order *orderCol;
    Order *order;
    Row *row;
    Instrument *instr;
    Song *synSong;
    
    FILE *f;
    if (!(f = fopen(path, "rb"))) return null;
    
    synSong = malloc(sizeof(Song);
    
    /*
    //unused vars
    int8_t _local5[] = [0, 0, 0, 0, 0, 0];
    bool _local2 = false;
    int _local7 = 0;
    bool _local8 = true;
    */
    
    
    fread(synSong->h, sizeof(SongHeader), 1, f);
    songVer = synSong->h.version;
    if ((songVer >= 3456) && (songVer <= 3457)){
        if (synSong->h.subsongNum > 0){
            synSong->subsongs = malloc(synSong->h.subsongNum *sizeof(Subsong));
            fread(synSong->subsongs, sizeof(Subsong), synSong->h.subsongNum, f);
            
            synSong->rows = malloc(synSong->h.patNum * 64 *sizeof(Row));
            fread(synSong->rows, sizeof(Row), synSong->h.patNum * 64, f);
            
            synSong->patNameSizes = malloc(synSong->h.patNum *4);
            synSong->patternNames = malloc(synSong->h.patNum *sizeof(char *));
            
            for (i = 0; i < synSong.h->patNum; i++) {
                fread(&synSong->patNameSizes[i], 4, 1, f);
                synSong->patternNames[i] = malloc(synSong->patNameSizes[i]);
                fread(synSong->patternNames[i], synSong->patNameSizes[i], 1, f);
                synSong->patternNames[i][synSong->patNameSizes[i]] = 0x00;
            }
            
            synSong->instruments = malloc(synSong->h.instrNum * Instrument);
            synSong->samples = malloc(synSong->h.instrNum * sizeof(int16_t *));
            for (i = 0; i < synSong->h.instrNum; i++) {
                instr = &synSong->instruments[i];
                fread(instr, sizeof(Instrument), 1, f);
                if (songVer == 3456){
                    instr->shareSmpDataFromInstr = 0;
                    instr->hasLoop = 0;
                    instr->hasBidiLoop = 0;
                    instr->smpStartPoint = 0;
                    instr->smpLoopPoint = 0;
                    instr->smpEndPoint = 0;
                    if (instr->hasSample){
                        instr->smpStartPoint = 0;
                        instr->smpEndPoint = (instr->smpLength / 2);
                        instr->smpLoopPoint = 0;
                    }
                }
                if (instr->hasSample){
                    //instr->smpLength is in bytes, I think
                    synSong->samples[i] = malloc(instr->smpLength);
                    fread(synSong->samples[i], 2, instr->smpLength / 2, f)
                } else {
                    synSong->samples[i] = NULL;
                }
                
            }
            fread(&synSong->arpTable, 1, 0x100, f);
        } else goto FAIL;
    } else goto FAIL;
    fclose(f);
    return synSong;
    
    FAIL:
    fclose(f);
    free(synSong);
    return NULL;
}