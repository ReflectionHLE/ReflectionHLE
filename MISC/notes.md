This directory should have a small patch to the recreated Super 3-D Noah's Ark code, making sure MIDI tracks are played back with the correct tempos (more-or-less). An EXE built from these sources+patch should also be found.

Note that this describes a DIFFERENCE from the original EXE for DOS as released in the 90s in terms of behaviors.

Originally, the cast to long for the middle byte was done in the wrong place, leading to an integer overflow in case *(midiData+1), an unsigned 8-bit byte, had the value of 128 or greater. While (*midiData+1) may internally be casted to int, a 16-bit signed integer in this case, for the shift by 8 bits to the left, this isn't sufficient for the later cast to long, since a sign extension is done here.

Example: if (*midiData+1) is, in binary, the value of 11000100b, then after shifting we get the 16-bit value of 1100010000000000b. By casting from a 16-bit SIGNED int to a 32-bit signed long int (i.e. doing a signed extension to 32-bit), we get the bit pattern of 11111111111111111100010000000000b. This is not the expected result, though, as it should've really been just 0000000000000000100010000000000b stored in a long int.

The multiplication of midiTimeScale by 1.1 is probably an attempt to get the correct tempo, at least for certain musical tracks, as the actual cause of the bug was apparently not found back in the 90s.
