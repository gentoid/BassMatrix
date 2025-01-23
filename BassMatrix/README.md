# BassMatrix

A basic MPE capable synthesier plug-in with IGraphics GUI

## Release procedure

New features to implement

* Listen to Note off massage in pattern changes
* Använd transparant logga på KVR-audio.
* Let UnserializeState() turn off effects if version is < 2.00

Release procedure

* Update version in SerializeState() and UnserializeState()
* Build Component on Mac
* Zip the component directory and mail it to ola@witech.se
* Rename the zip to BassMatrix-au-2.40.zip
* Build 64 bit VST3 on Windows
* Zip the BassMatrix.vst3 directory.
* Rename the zip to BassMatrix-vst3-2.40.zip
