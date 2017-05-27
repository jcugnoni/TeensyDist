In this folder, you will find the TeensyDist library which contains all the custom audio processing objects used in the TeensyDist pedal.

To use it, just copy this folder to your Arduino Library folder as usual. 
To use this code, you will need a Teensy 3.5 or 3.6; earlier Teensy model do not have enough floating point capability to run the example code.
If you want to use a Teensy 3.0-3.2, you could try to use the simpler Waveshaper object for distortion which is implemented in integer 16bit arithmetic.

The library uses the Teensy Audio library (https://www.pjrc.com/teensy/td_libs_Audio.html) and the OpenAudio_arduin_library by Chipaudette (https://github.com/chipaudette/OpenAudio_ArduinoLibrary)

Implementation examples are given in the example folder.
( at the moment one example which models approximatelly a Tube Screamer type pedal... more to be added soon).
