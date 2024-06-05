# BassMatrix readme

## Version 1.10
Fixed problem when key sync mode didn't read the tempo from host.

Added a kind of smarter randomize algorithm. It will adapt to the pattern you have choosed. If you for
example standing on pattern E on Octav 3 you will get a pattern using E and the fifth of E and the E one octav up
in the variation, and similar for the other keys. This will hold for all patterns exept patterns from f# on octav 3
and above, since the sequencer doesn't cover the upper fifth of those keys.

### Known problems
The sequencer sometime loses it's position when DAW transport is moved.
 * Workaround this by put restart the melody on a bar.

The size of gui is not saved and restored when restarting DAW.

When you move the bar during play, the last note of the sequencer is played

Sometimes when working on a pattern on Octav 3 the corresponding pattern in Octav 2
is overwritten.

Octave up/down/neutral buttons does not mirror the actual values in the sequencer.

The sequecer leds are not allways in sync with the transport bar.



## Version 1.00
Every known issues is fixed.

