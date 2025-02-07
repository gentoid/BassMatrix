# BassMatrix

A Roland TB-303 clone

## Release procedure

New features to implement

* A randomized pattern at startup.

Bugs
* Om MacOS when deleting a BassMatrix and the start a new instance. The sequencer buttons, octav buttons and pattern buttons are off. This does not happen on Ableton Live on Windows.

Release procedure

* Write release information in README.md
* Update version number in the GUI
* Update version in SerializeState() and UnserializeState()
* Build **release** version of Component on Mac
* Zip the component directory and mail it to ola@witech.se
* Rename the zip to BassMatrix-au-2.40.zip
* Build **release** version of 64 bit VST3 on Windows
* Zip the BassMatrix.vst3 directory.
* Rename the zip to BassMatrix-vst3-2.40.zip
