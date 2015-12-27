A C port of the AS3 version of Syntrax replayer.

Some conventions/helpers/whatever:
*Number becomes double. Decide after if keep it as such or make it.
*int and uint stay as such. Slowly convert everything to stdint as proper size is determined.
*I will be writing malloc()s without worry of free() usage and deal with it after.
*Vars commented as unused are clues of things that were not ported into AS3 player. I say leave them as is for later piecing.
*WaveBuffer is a helper in AS3 and can be replaced with proper C.