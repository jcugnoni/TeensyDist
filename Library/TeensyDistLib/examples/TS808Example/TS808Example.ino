/*
 * Distortion pedal modeling using Teensy 3.5/3.6 and i2s audio codec
 * 
 * Copyright, J.Cugnoni, 2017
 *  
 * Licence GNU GPLv3
 * 
 * Example implementation of a Tube screamer type pedal. 
 * 
 *  Please note that the parameters are not optimized for fidelity as this example is just to show you how to implement the typical blocks found in a pedal     
 */
 

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <OpenAudio_ArduinoLibrary.h>   // 32 bit floating point extension to Teensy Audio lib
#include <TeensyDist.h>

// --- GENERAL CODE OPTIONS -----

// select your codec below:
//#define SGTL5000
#define WM8731

#define FS 44117
//----------------------------

// -- instanciate AUDIO OBJECTS ---

// Audio Input / Output
#ifdef SGTL5000
AudioInputI2S             audioIn;           
AudioOutputI2S            audioOut;          
AudioControlSGTL5000      codec;     
#endif
#ifdef WM8731
AudioInputI2Sslave        audioIn;
AudioOutputI2Sslave       audioOut;
AudioControlWM8731master  codec;
#endif

// Audio processing 
AudioConvert_I16toF32     convert1;
filterIIR                 eq1;  // pre eq
AudioFilterFIR_F32        fir1;     // pre FIR filter
AudioEffectMultiStageDist dist;  // distortion block
AudioConvert_F32toI16     convert2;         
AudioFilterFIR            fir2;     // post FIR filter
AudioFilterBiquad         eq2;  // post eq        

// peak detector input & output
AudioAnalyzePeak         peakIn;
AudioAnalyzePeak         peakOut;          

// audio connections between objects
AudioConnection          patchCord1(audioIn, 0, convert1, 0);
AudioConnection          patchCord1b(audioIn, 0, peakIn, 0);
AudioConnection          patchCord2(convert1, 0, eq1, 0 );
AudioConnection_F32      patchCord3(eq1, 0, fir1, 0 );
AudioConnection_F32      patchCord4(fir1, 0, dist, 0 );
AudioConnection_F32      patchCord5(dist, 0,convert2,0);
AudioConnection          patchCord6(convert2,0,fir2,0);
AudioConnection          patchCord7(fir2, 0, eq2, 0);
AudioConnection          patchCord8(eq2, 0, audioOut, 0);
AudioConnection          patchCord8b(eq2, 0, audioOut, 1);
AudioConnection          patchCord12(eq2, 0, peakOut, 0);

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
 
  // Audio connections require memory to work.  Optimize the number of blocks by looking at the stats printed during execution. 
  AudioNoInterrupts(); 
  AudioMemory(12);
  AudioMemory_F32(12); 

  // input and output level adjustment, tune depending on your input / output signal levels
  #ifdef SGTL5000
  codec.enable();
  codec.inputSelect(myInput);
  codec.volume(0.6);
  codec.lineInLevel(1);   // 2.63Vpp
  codec.lineOutLevel(29); // 1.29Vpp
  #endif
  #ifdef WM8731
  codec.enable();
  codec.volume(0.90);
  codec.inputLevel(.70);   
  #endif
   
  // init audio blocks

  // pre eq: 3 stage equalizer (0 to 2) as : high pass (type = 'H')=> mid peak(type 'P') => low pass (type = 'L')
  float eq1_lowcut=120;
  float eq1_midf=400;
  float eq1_midq=1.5;
  float eq1_midgain=2.0;
  float eq1_highcut=3500;
  eq1.setBiQuadEq(0,'H',eq1_lowcut,1,0);
  eq1.setBiQuadEq(1,'P',eq1_midf,eq1_midq,eq1_midgain);
  eq1.setBiQuadEq(2,'L',eq1_highcut,1,0);
  Serial.println("Setting input EQ");

  // pre FIR: impulse response of the preamp. Using a TS808 impulse pre defined in TD_filterf.h,
  // TD_FIR_TS is a c preprocessor variable defined in TD_filterf.h that defines the array index corresponding to a tube screamer impulse. 
  // to use your own FIR, just follow the same structure as TD_filter.h and create another custom header file...
  int fir_id=TD_FIR_TS;
  setInputFIR(fir_id);
  Serial.println("Setting input FIR filter");

  // distortion model: one single stage with softclipping
  // parameters: stage=0, pregain=2.0, bias=0.0, distgain=5.0, postgain=2.0, mastervolume=1.0, lowcut=80Hz, highcut=5000Hz
  int stage=0;
  float pregain=2.0;
  float bias=0.0;
  float distgain=5.0;
  float postgain=2.0;
  float mastervol=1.0;
  float lowcut=80;
  float highcut=5000;
  dist.setSoftClipping(distgain, stage); 
  dist.enableStage(stage);
  dist.setBias(bias*32767.,stage);
  dist.setPreGain(pregain,stage);
  dist.setPostGain(postgain,stage);
  dist.setLowpass(highcut,stage);
  dist.setHighpass(lowcut,stage);
  dist.setPostGain(mastervol,MSDMAXSTAGES);
  Serial.println("Distortion clipping mode : SoftClipping");
  
  // post eq, low cut => mid peak => high cut. Set as follow low cut=80, high cut=7000, mid freq 200 Hz, Q=1.5 mid gain 2.0)
  eq2.setHighpass(0, 80.0, 0.8);
  // compute mid peak filter coefficients (formula from http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt)
  double midcoefs2[5];
  getBiquadPeakCoefs(200.0, 1.5, 2.0, midcoefs2);
  eq2.setCoefficients(1,midcoefs2);
  eq2.setLowpass(2, 7000, 0.8);
  Serial.println("Set Post EQ");
    
  // post FIR filter, set as neutral in this model
  fir2.begin(  FIR_PASSTHRU, 0);
  
  // embedded equalizer in SGTL5000 (cool features if you want to spare some cpu shifting the post equalization to the audio chip itself)
  #ifdef SGTL5000
  codec.audioPostProcessorEnable();
  codec.eqSelect(3); // activate graphic eq
  codec.eqBands(-0.2, 0., -0., -0.4, -1);// bands:  115Hz, 330Hz, 990Hz, 3kHz, and 9.9kHz
  // freeze adc offset to remove further input noise, optionnal.
  delay(100);
  codec.adcHighPassFilterFreeze(); 
  #endif
    
  // ready to go, enable audio processing!
  AudioInterrupts(); 
}

//...................
// helper functions
//..................
void setInputFIR(uint8_t firid) {
  Serial.print("Setting input FIR to ");
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

// compute mid peak biquad params
void getBiquadPeakCoefs(double freq,double Q,double midgaindb,double midcoefs[5]) {
  double w0=2.0*3.1415926*freq/44117.0;
  double alpha = sin(w0)/(2*Q);
  double A  = sqrt( pow(10,midgaindb/20) );
  double a0= 1 + alpha/A;
  midcoefs[0]=   (1 + alpha*A)/a0;
  midcoefs[1] =  (-2*cos(w0))/a0;
  midcoefs[2] =   (1 - alpha*A)/a0;
  midcoefs[3] =  (-2*cos(w0))/a0;
  midcoefs[4] =   (1 - alpha/A)/a0;
};

//----------------------------
// -----MAIN UPDATE LOOP -----
elapsedMillis volmsec=0;

void loop() {
  //  print peak levels and stats
  if (volmsec > 66) {
    float v_peakIn = peakIn.read();
    float v_peakOut = peakOut.read();
    Serial.print("Peak In: ");
    Serial.print(v_peakIn*100);
    Serial.print(", ");
    Serial.print("Peak Out: ");
    Serial.print(v_peakOut*100);;
    Serial.print(", ");
    Serial.print("  CPU%=");
    Serial.print(AudioProcessorUsage());
    Serial.print("(");
    Serial.print(AudioProcessorUsageMax());
    Serial.print(") Mem%= ");
    Serial.print(AudioMemoryUsage());
    Serial.print(",");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    volmsec=0;
  }
}


