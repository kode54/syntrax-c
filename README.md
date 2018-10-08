A C port of the AS3 version of Syntrax replayer.

Original C++ and subsequent ActionScript port by rhinoid - [their GitHub](https://github.com/rhinoid) - [their BitBucket](https://bitbucket.org/rhinoid).

Original port from ActionScript and modifications by a friend, who wishes to be divested from this publication. Except I failed to do that. Meh.

Some conventions/helpers/whatever:
* Number becomes double. Decide after if keep it as such or make it.

* int and uint stay as such. Slowly convert everything to stdint as proper size is determined.

* I will be writing malloc()s without worry of free() usage and deal with it after.

* Vars commented as unused are clues of things that were not ported into AS3 player. I say leave them as is for later piecing.

* WaveBuffer is a helper in AS3 and can be replaced with proper C.
  
* When renaming/understanding stuff, Jaytrax' GUI will be of help. Syntrax, not so much because of instability and the mobile UI with design shortcuts taken. For example, Jay's arpeggiator allows you to define 16 steps freely. Syn forces you to use arp presets.

Test vectors uploaded here:

On Amazon S3: [jaytrax demo songs and test vectors.tar.xz](https://f.losno.co/jaytrax_demo_songs_and_test_vectors.tar.xz)

[Post with some songs](http://viewer.scuttlebot.io/%25Be%2BwDtlXz%2BL7fXnCqNUl7eDOGVOJ4gqr%2B2p1REheS98%3D.sha256) on [Scuttlebutt](https://www.scuttlebutt.nz).
