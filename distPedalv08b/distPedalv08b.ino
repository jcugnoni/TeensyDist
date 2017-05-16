/*
 * Distortion pedal using Teensy 3.5/3.6 and i2s audio codec
 * with TFT GUI and hardware interface 
 * hadreare: Teensy 3.5/3.6, 4 pots for level, gain, treble and bass, 1 encoder+button for menu navigation, 1 flip switch for edit mode, one toggle switch for presets, and 1 true bypass switch.
 * requires also a high input impedance buffer with approx unity gain (look for Klon buffer for a good starting point) and a clean 5V regulator for Teensy, TFT , codec, buttons & pots 
 * 
 * Copyright, J.Cugnoni, 2017
 *  
 * Licence GNU GPLv3
 *  
 * here are some notes gathered while developing / experimenting :
 * - the Teensy audio board offers great functionnality but in my case did suffer from unnacceptably high noise floor that I did not manage to solve (played with external voltage regulators, 
 *   filters on DC supply or grounds, suppression of possible ground loops (star grounding helped a bit). The noise seems to be caused by the internal filters (resonnating?) in the board causing 
 *   a high pitch (1.5khz+) random noise and very disturbing digital noise during transient load fluctuations (heavy interrupt handling)... The SGTL5000 board gave me a peak noise floor of 0.05 to 
 *   0.09 % while grounding the line input, indepdendtly from the line input gain. Low noise is particularly important for a high gain distortion as 
 *   the signal gets amplified up to 200x in the linear range ! I tried another codec Mikroelectronika WM8731 and the noise floor is 0.02-0.03% in similar conditions. The WM8731 board gives a lower 
 *   frequency noise, closer to the classical hum caught by guitar pickup and cables and the digital noise is gone. Interesting fact: having both the SGTL5000 (powered off) and WM8731 connected on the same source 
 *   an additionnal "resonnating" noise at 1.5+khz (like a high pitched wha filter) could be heard, as if the passive filtering on the Teensy audio board had something to do with noise amplification.  
 *   Thus I strongly recommend the MikroElectronika WM8731 proto board even though it lacks in feature and some support compared to Teensy audio board. It would be nice to improve the design of the audio board to decouple the 
 *   "audio" DC (dedicated regulator & filter?) and improve the onboard grounding scheme and filtering (the capacitor leakage resistor fix as proposed on Sparkfun protopedal page could work. Indeed while touching this cap with my 
 *   finger tips (1MOhm...) some part of the noise was reduced. 
 *-  DONE: the input (and to a lesser extent output) FIR filters, when implemented in int16 cause a drastic gain reduction that needs to be compensated afterwards by an integer multiplication. This process is found to reduce 
 *   effective bit depth of about 5.5 bits, ie, only a 11 bit signal truly remains. Prescaling seems impractical as we use the full signal range on input. Floating point FIR or cascading IIR filters need to be inverstigated. 
 *   => Floating point FIR on input/preamp stage solved the problem. The output FIR does not seem to be such an issue for the moment as it is already at unity gain and does not affect noise amplification in the distortion stages.
 *-  DONE: implement output eq as biquad filter blocks for MikroE board.  
 *-  TODO: rethink the type of eq for pre and post, an how to link it to treble and bass controls
 *-  DONE: implement hardware pots for Pre gain (pregain of first stage), Level (adjust postgain), Bass and Treble (need a stable tone stack first)
 *-  DONE: implement a preset mode (activated by SW2) with its own set of menu. Press once on toggle switch to alternate between A/B and double tap to increment preset
 *-  DONE: the cubic distortion needs to be fixed, weird noise (probably discontinuity in the wave function). The Asymmetric waveshaper is OK now
 *-  DONE: implemented 2x oversampling to reduce distortion artefacts in multi stage distortion block. Can be combined with linear or cubic interpolation of the wave shaping tables.
 *   MAYBE: add an option to select the type of interpolation used for each stage.
 *-  DONE: add more FIR filters for classic pedal and amp tone stacks & evtl speaker response. 
 *-  MAYBE: add sdcard support to store FIR and also serial command to upload/download FIRs.
 *-  MAYBE: Could improve FIR filtering by adding a IIR stage for improved low frequency details. To test but it would be more difficult to identify a proper model from frequency response measurements..
 *-  MAYBE: use log scale on compressor threshold and add attack/release time + knee (any use of DSP methods here??).
 *-  DONE: reduce drive noise by initial gain reduction around origin (but preserving non-linearity), optimize distortion by using CMSIS/Keil arm linear interpolation and reduce oversampling overhead by using DSP functions.
 *         => using Floating point FIR solved the high noise / hiss problem, indeed input FIR had bad gain scaling due to 16bit max coefficient, so was losing 5 bits of input data  
 *-  DONE: implement multistage distortion block with at least 2 to 3 preamp stages (non linear tube gain stage characteristics & low-highpass filters) + 1 power stage.
 *  -TODO: implement sag/squishiness by variable input/output compression or 2D interpolation or variable FIR filtering, or differential equation for supply voltage
 *         This is partially achievable with current version with a slow input compressor and reduced DC blocker cut off (more DC swing on stage input during attack)
 *  -TODO: implement cliping tables for diodes and opamps; if possible we can take tables from Guitarix as a starting point.
 *  -TODO: renormalize correctly the waveshaping tables for tubes (KT88 too low) and adjust clipping tables for diodes & opamps
 *  -TODO: fix noise gate
 *  -TODO: fix saving of presets on banks >1 
 *  -DONE: convert all the pre distorition stages to floating point: codec input i16 -> f32 -> pre eq (IIR) -> pre FIR -> compressor -> multistage distortion -> f32 to i16 -> post FIR -> post eq -> output 
 *  -MAYBE: implement a real Fender-Marshall-Vox and James tonestack in the multistage dist block with selectable position
 *  -DONE: try linear interpolation with 2x oversampling, Teensy 3.5 can even handle it. 
 *  -DONE: migrate code of multistage dist and compression to AudioStream_F32 to accept float as input (reduce conversion between int and float to a minimum)
 */


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <OpenAudio_ArduinoLibrary.h>   // 32 bit floating point extension to Teensy Audio lib
#include "ILI9341_t3.h"
#include "gui.h"
#include <Encoder.h>

// --- GENERAL CODE OPTIONS -----

#include "codeOptions.h"

//----------------------------

// -----  custom includes  and global objects ----- 

// audio processing blocks

#include "filterIIR.h"
#include "filterf.h"
#include "multistagedist.h"
#include "compressor.h"

// physical interface objects

// TFT  (pin number defined in gui.h)
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

// Encoder (pin numbers in gui.h)
//    Change these two numbers to the pins connected to your encoder.
Encoder myEnc(ENCB,ENCA);
bool GUI_Change=false;
  
// global variables to define the GUI
Gui mygui;

// menu definitions
#include "menu.h"

// -- instanciate AUDIO OBJECTS ---
// audio io
#ifdef SGTL5000
AudioInputI2S            audioIn;           
AudioOutputI2S           audioOut;          
AudioControlSGTL5000     codec;     
#endif
#ifdef WM8731
AudioInputI2Sslave       audioIn;
AudioOutputI2Sslave      audioOut;
AudioControlWM8731master codec;
#endif
#ifdef TEENSYADDA
AudioInputAnalog         audioIn;
AudioOutputAnalog        audioOut;
#endif

// filters & mixers
AudioMixer4              mixer1;   //input mixer         
AudioAnalyzeRMS          rms1;
//AudioFilterBiquad        biquad1;  
AudioConvert_I16toF32    convert1;
filterIIR                biquad1;  // pre eq
AudioFilterFIR_F32       fir1;     // pre FIR filter
AudioEffectCompressor    compr;    // compressor
AudioEffectMultiStageDist   dist;     // distortion block
AudioConvert_F32toI16    convert2;         
AudioFilterFIR           fir2;     // post FIR filter
AudioFilterBiquad        biquad2;  // post eq        
AudioMixer4              mixer2;   // output mixer
        
AudioConnection          patchCord1(audioIn, 0, mixer1, 0);
AudioConnection          patchCord6(audioIn, rms1);
//AudioConnection          patchCord1b(mixer1, 0 , convert1, 0 );
//AudioConnection          patchCord1b(mixer1, 0 , biquad1, 0 );
//AudioConnection          patchCord4(biquad1, 0, convert1, 0 );
AudioConnection          patchCord4(mixer1, 0, convert1, 0 );
AudioConnection_F32      patchCord4a(convert1, 0, biquad1, 0 );
AudioConnection_F32      patchCord4aa(biquad1, 0, fir1, 0 );
//AudioConnection_F32      patchCord4a(convert1, 0, fir1, 0 );
AudioConnection_F32      patchCord4b(fir1, 0, compr, 0 );
AudioConnection_F32      patchCordD1(compr,0, dist,0);
AudioConnection_F32      patchCordD2(dist, 0,convert2,0);
AudioConnection          patchCordD2b(convert2,0,fir2,0);
AudioConnection          patchCord7(fir2, 0, biquad2, 0);
AudioConnection          patchCord7b(biquad2, 0, mixer2, 0);
AudioConnection          patchCord8(mixer2, 0, audioOut, 0);

#ifdef TEST_TONE  
  AudioSynthWaveform       waveform1;     
  AudioConnection          patchCord1x(waveform1, 0, mixer1, 2);   
#endif
#ifdef STEREO
AudioConnection          patchCord8b(mixer2, 0, audioOut, 1);
#endif

// USB io
#ifdef USBAUDIO
AudioOutputUSB           usbOUT;        
AudioInputUSB            usbIN;         
AudioConnection          patchCord8c(mixer2, 0, usbOUT , 0);
AudioConnection          patchCord4b(usbIN, 0, mixer1, 1);
#endif


// peak detector input & post dist
AudioAnalyzePeak         peakIn1;
AudioAnalyzePeak         peakFir1;
AudioAnalyzePeak         peakDist;          
AudioAnalyzePeak         peakOut;          
AudioConnection          patchCord9(audioIn, 0, peakIn1, 0);
AudioConnection          patchCord10(convert2, 0, peakDist, 0);
AudioConnection          patchCord11(biquad1, 0, peakFir1, 0);
AudioConnection          patchCord12(mixer2, 0, peakOut, 0);

// queues for signal analysis
#ifdef PLOTOUT
AudioRecordQueue         queue1;         //xy=871,495
AudioRecordQueue         queue2;         //xy=1046,489
AudioConnection          patchCord5(mixer1, queue1);
AudioConnection          patchCordD3(convert2, queue2);
//AudioConnection          patchCordD3(mixer2, queue2);
#endif

// ---- END OF AUDIO OBJECT DEFS

// MAIN VARS
uint32_t counter=0;
uint32_t rmscount=0;
unsigned long t=0;
const int myInput = AUDIO_INPUT_LINEIN;

// Note: deprecrated?
void tests() {
  delay(1000);
  Serial.println("Running tests");
  delay(5000);
}


//-------------------------------
//----- INITIALIZATION ----------

void setup() {
  Serial.begin(115200);
  #ifdef RUNTESTS
  tests();
  #endif

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioNoInterrupts(); 
  AudioMemory(14);
  AudioMemory_F32(14); 
  
  #ifdef SGTL5000
  codec.enable();
  codec.inputSelect(myInput);
  codec.volume(0.4);
  codec.lineInLevel(1);   // 2.63Vpp
  codec.lineOutLevel(29); // 1.29Vpp
  #endif
  #ifdef WM8731
  codec.enable();
  codec.volume(0.55);
  codec.inputLevel(.65);   
  #endif
   
  // init audio blocks
  compr.setGainThreshold(4.0,0.75*32767);
  compr.setNoiseThr(200);
 
  #ifdef TEST_TONE
    waveform1.begin(WAVEFORM_SINE);
    waveform1.frequency(700);
    waveform1.amplitude(0.5);
    mixer1.gain(2,1);
  #endif
  
  #ifdef PRINTWAVE
  delay(3000);
  Serial.println("Wave table1");
  for(int i=0; i<DISTTABLESIZE; i++) {
   Serial.print(dist.wavex[i]);
   Serial.print(", ");
   Serial.println(dist.wavetable[i]);
  }
  delay(3000);
  #endif

  // INIT AUDIO OBJECTS
  fir1.begin( firbypass , 3);
  fir2.begin(  FIR_PASSTHRU, 0);
  mixer1.gain(0,1);
  mixer1.gain(1,1);
  mixer2.gain(0,1);
  #ifdef PLOTOUT
  queue1.begin();
  queue2.begin();
  #endif
  // embedded equalizer in SGTL5000 (cool features in that chip!)
  #ifdef SGTL5000
  codec.audioPostProcessorEnable();
  codec.eqSelect(3); // activate graphic eq
  codec.eqBands(-0.2, 0., -0., -0.4, -1);// bands:  115Hz, 330Hz, 990Hz, 3kHz, and 9.9kHz
  #endif
   
  // freeze adc offset to remove further input noise, optionnal (not found to affect noise that much).
  delay(100);
  //codec.adcHighPassFilterFreeze(); 

  // init GUI
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  pinMode(BUTTON,INPUT_PULLUP);
  pinMode(SWITCH1,INPUT_PULLUP);
  pinMode(SWITCH2,INPUT_PULLUP);
  buildGui();
  Gui_update();
  Gui_validate();
  mygui.draw();

//  // DEBUG TEST EEPROM  !!! THIS ERASES THE PRESETS !!!
//  Serial.println("Testing EEPROM");
//  bool EEPROM_OK=true;
//  for(uint8_t i=0;i<255;i++) EEPROM.put(sizeof(i)*(i-1),i);
//  for(uint8_t i=0;i<255;i++) { 
//    uint8_t tst=0; EEPROM.get(sizeof(i)*(i-1),tst); 
//    if(tst!=i) {
//      Serial.print("EEPROM mismatch: "); Serial.print(i); Serial.print(" != ");Serial.println(tst);
//      EEPROM_OK=false;
//    }
//  }
//  if(EEPROM_OK) Serial.println("EEPROM test pass");
  

  // ready to go!
  AudioInterrupts(); 
}
//...................
// helper functions
//..................
void setInputFIR(uint8_t firid) {
  Serial.print("Setting input FIR to no ");
  Serial.println(firid);
  if(firid>0){
     fir1.begin( &infilter[firid-1][0], INFILTER_NUM_TAPS);
   } else {
     fir1.begin( firbypass , 3);
   }  
};

void setOutputFIR(uint8_t firid) {
  Serial.print("Setting output FIR to no ");
  Serial.println(firid);
   if(firid>0) {
    fir2.begin( &outfilter[firid-1][0], OUTFILTER_NUM_TAPS);
   } else {
    fir2.begin(  FIR_PASSTHRU, 0);
   }
};

//----------------------------
// -----MAIN UPDATE LOOP -----
uint32_t time_gen_upd=0;
uint32_t dt_gen_upd=3000;
uint8_t ifreq=0;
uint8_t iamp=0;
uint8_t namp=6;
uint8_t nfreq=7;
bool toplot=true;
float curvol=1.0;

elapsedMillis volmsec=0;
elapsedMillis guiUpd=0;

void loop() {
  //  peak detect and stats
  if (volmsec > 66) {
    #ifdef TEST_TONE
      /*static float ampl=0.0,amplstep=0.001;
      if(ampl<0.0) {ampl=0.0; amplstep*=-1.0;}
      else if(ampl>0.05) {ampl=0.05; amplstep*=-1.0; }
      else { ampl+=amplstep; } 
      waveform1.amplitude(ampl);*/
    #endif 
    #ifdef PRINTSTAT
    float v_peakIn1 = peakIn1.read();
    float v_peakFir1 = peakFir1.read();
    float v_peakDist = peakDist.read();
    float v_peakOut = peakOut.read();
    Serial.print("RMS in : ");
    Serial.print(rms1.read()*100);
    Serial.print(", ");
    Serial.print("Peaks : ");
    Serial.print(v_peakIn1*100);
    Serial.print(", ");
    Serial.print(v_peakFir1*100);
    Serial.print(", ");
    Serial.print(v_peakDist*100);
    Serial.print(", ");
    Serial.print(v_peakOut*100);
    Serial.print("  CPU%=");
    Serial.print(AudioProcessorUsage());
    Serial.print("(");
    Serial.print(AudioProcessorUsageMax());
    Serial.print(") Mem%= ");
    Serial.print(AudioMemoryUsage());
    Serial.print(",");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    #endif
    volmsec=0;
  }
  // input output queue analysis & plotting
   #ifdef PLOTOUT
  long q1len=queue1.available();
  long q2len=queue2.available();
  if (q1len>1 && q2len>1) {
    for(int ii=0;ii<min(q1len,q2len);ii++){
       int16_t *data1=queue1.readBuffer();
       int16_t *data2=queue2.readBuffer();
       for(uint16_t i=1;i<AUDIO_BLOCK_SAMPLES;i+=2) {
          Serial.print(data1[i]);
          Serial.print(' ');
          Serial.println(data2[i]);
       }
       queue1.freeBuffer();
       queue2.freeBuffer();
    }
    queue1.clear();
    queue2.clear();
  }
  #endif
  // GUI update
  if(guiUpd>77) {
    Gui_update();
    guiUpd=0;
  }
}


