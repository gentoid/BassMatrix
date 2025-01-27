# BassMatrix

A basic MPE capable synthesier plug-in with IGraphics GUI

## Release procedure

New features to implement

* Listen to Note off massage in pattern changes
* Let UnserializeState() turn off effects if version is < 2.00
* Check that saving settings works on Mac
* A more fun pattern at startup.
* Choose HOST SYNC mode if start up from DAW, else choose INT.SYNC
* On Mac Randomize doesn't select any note as gated.

Bugs
* Gui don't show octav 3 change from octav 2 on the midi in.

Release procedure

* Update version number in the GUI
* Update version in SerializeState() and UnserializeState()
* Build **release** version of Component on Mac
* Zip the component directory and mail it to ola@witech.se
* Rename the zip to BassMatrix-au-2.40.zip
* Build **release** version of 64 bit VST3 on Windows
* Zip the BassMatrix.vst3 directory.
* Rename the zip to BassMatrix-vst3-2.40.zip
