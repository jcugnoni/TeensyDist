# TeensyDist
Teensy 3.5/3.6 multistage distortion modeling pedal

## What is it?

TeensyDist is a DYI Teensy 3-5/3.6 based distortion pedal and firmware that features :

_Hardware:_

* An high impedance input buffer of your choice (mine is a DIY "Klon" type buffer = transparent opamp buffer, see here : http://tagboardeffects.blogspot.ch/2011/04/klon-buffer.html
). 
* An audio codec supported by Teensy Audio Library (mine is a MicroE WM8731 but PJRC Audio board should work as well with a little bit more noise though). Links:  https://shop.mikroe.com/add-on-boards/audio-voice/audio-codec-proto or https://www.pjrc.com/store/teensy3_audio.html
* A Teensy 3.5 (the one I use now) or 3.6 (better, but I fried mine...). Links: https://www.pjrc.com/store/teensy36.html or https://www.pjrc.com/store/teensy35.html
* A 2.2'' 320x240 ILI9341 TFT for GUI. Link: https://www.pjrc.com/store/display_ili9341.html
* Two latching foot switches: one for True bypass, the other for preset switching.
* One simple switch to trigger preset or editing mode
* 4x 100k linear rotary potentiometers for master volume, gain, treble and bass controls
* one rotary encoder + switch for menu navigation and editing (type EC11 or similar).

_Software (in the order of the signal path):_

* a pre-eq & pre-impulse (FIR) filter for tone shaping before distortion
* a compressor and noise gate (pre distortion)
* up to 4 distortion stages (single or push-pull tube stage or waveshaper based stage) each with their own pre- and post gains as well as bias, low and high pass filters. All distortion and pre filters are implemented in 32bit floating point for maximum dynamics and distortion can run up to 2x oversampling with Hermite or cubic interpolation of the waveshaping tables on Teensy 3.5 in real time.
* a post FIR and post EQ for modeling a tone stack or final shaping of the sound.
* optional support for USB audio input  and output to act as a sound card (both mixed with physical inputs and outputs).
* 4 banks of 4 dual (A/B) presets, for a total of 32 presets stored in Teensy's EEPROM

Here is how it looks like (this is my "dirty" prototype...):
![Image of TeensyDist pedal proto](https://github.com/jcugnoni/TeensyDist/blob/master/TeensyDistPicture.png)

## Aim
TeensyDist is a personnal project that I carried out to discover and research how amp and pedal modeling technology works and is thus provided as is , without any warranty or support. The goal here is to share with you my trials and possibly inspire new developments in the open source / open hardware community. But be carefull, if you are like me, you might spend a LOT of time playing and hacking this thing to get the ultimate sound ;-)

In terms of implementation, TeensyDist is based on  Paul Stoffregen's great Teensy Audio Library (https://www.pjrc.com/teensy/td_libs_Audio.html) and Chipaudette's OpenAudio Library which extends it to floating point.

## Implementations
My implementation uses as much as possible the ARM Cortex M4 DSP instructions and is pretty much optimized. I have tested many variants of the algorithms to choose the best quality/performance ratio. When relevant, the code can be tuned by chosing different options (#define) at compile time.

The current signal flow is shown in the image below:
![Image of the DSP signal flow](https://github.com/jcugnoni/TeensyDist/blob/master/TeensyDistSignalFlow.png)

This code provides also new objects for the Teensy/OpenAudio audio libraries:
* a single stage waveshaper distortion block with variable oversampling and different interpolation types (my first steps, unused in latest version)
* a multistage distortion object with many waveshaping tables for tubes (12AX7, 12AT7, 12AU7, 6V6, KT88), and other classic mathematical waveshaping functions (hyperbolic tangent, cubic and asymmetric laws of Doidic et al, power or exponential laws, soft clipping). Each stage can be eitehr single ended or push-pull and bias can be adjusted on the fly to generate different levels of odd/even harmonics. The DSP code performance can be adjusted to select different types of table interpolation (linear, quadratic, cubic spline of Hermite), change the number of point of the wave tables or choose between 1x or 2x oversampling. All the code is floating point 32bit.
* a simple compressor/noise gate block.
* a very efficient and easy to use floating point IIR equalizer block with automatic calculation of coeficients for high or low pass, peak or shelving filters.

## Documentation
The current documentation is sparse and mostly targeted at an experienced audiance but it consists in:
* A user manual: https://github.com/jcugnoni/TeensyDist/blob/master/TeensyDistmanual.pdf
* The general flow chart above or in PDF https://github.com/jcugnoni/TeensyDist/blob/master/doc/distPedal08-block-diagram.pdf
* A detailed table of the Teensy pin connections  in https://github.com/jcugnoni/TeensyDist/blob/master/doc/teensypinMap2.xls
* a user documentation and some presets/example tones will probably follow... 

## Extending it / further development
I have developed several tools for impulse and distortion model identifications in Octave (profiling). At the moment, it works for simple distortion pedals (tried on a Suhr Riot clone). But my ultimate goal would be to implement and automate those methods on the Teensy to be able to "clone" a real distortion pedal or amp: capture its overall impulse response (FIR) and optimize distortion parameters to match the measure harmonic profile of the real pedal. 

Also several tools were developed to identify FIR or IIR filters for simple cabinets or tone stack models. Finally some difficult parts have been debugged by implementing the algorithms in an Octave MEX file for quick evaluation. 
I will probably upload my Octave codes when I have managed to make it work more reliably and cleaned all the mess...

Finally, the waveshaping tables used for tube modeling have been derived from the code base of the mighty Guitarix open source amp modeling plugin which uses Koren's equations and parameters. Further additions to the list of tube transfer functions would be welcome, for example EL84 or 6L6 tubes.


