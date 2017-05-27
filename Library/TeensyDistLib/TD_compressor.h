/* Audio dynamic compressor / noise gate for Audio Library / Teensy 3.X
 * Copyright (c) 2017, Joel Cugnoni (joel.cugnoni@gmail.com)
 */

#ifndef compressor_h_
#define compressor_h_


#include "Arduino.h"
#include <AudioStream_F32.h>
#include "arm_math.h"

//#define PRINT_TIMING

class AudioEffectCompressor : public AudioStream_F32 {
  
private: 
  
  audio_block_f32_t *inputQueueArray_f32[1];
  float prevEnv=0;
  float prevNoise=0;
  float noisethr=0;
  float gain=1.0;
  float ratio=4.0;
  float prevGain=1.;
  float ratioInv=0.25;
  float thr=32767/8;
  float thrIn=0;
  float grel,gatt=0.0;
  float greln,gattn=0.0;
  float gnoiseRel=0.999;
  float gnoiseAtt=0.005;
  bool enabled=true;
  
public:
  
  AudioEffectCompressor(void): AudioStream_F32(1, inputQueueArray_f32)  { setGainThreshold(2.0 , 0.75*32767); setAttack(10); setRelease(250); setNoiseThr(100);  enabled=true; greln=exp(-1./(44.1*5)); gattn=exp(-1./(44.1*10)); };
  void update();
  void setNoiseThr(int16_t nthr) { noisethr=nthr/32767.0; };
  void setThreshold(int16_t thres) {thr=thres/32767.0; thrIn=thr/gain;};
  void setRatio(float r) { ratio=r; ratioInv=1/ratio;};
  void setGainRatio(float g, float r) {gain=g; ratio=r; ratioInv=1/ratio; thrIn=(ratio-1)/(ratio*gain-1); thr=gain*thrIn; };
  void setGainThreshold(float g, int16_t t) {gain=g; thr=t; thrIn=t/gain; ratio=(1-thrIn)/(1-thr); ratioInv=1/ratio; };  
  void setRelease(float trel) {this->grel= expf(-1.0/(44100.0*(trel/1000.0))); }; 
  void setAttack(float tatt) {this->gatt= expf(-1.0/(44100.0*(tatt/1000.0))); }; 
  void setGain(float g) { this->gain=g; this->thrIn=thr/gain; };
  void setNoiseRate(float t1) {this->gnoiseRel=expf(-1.0/(44100.0*(t1/1000.0))); this->gnoiseAtt=(1.0-this->gnoiseRel)*5;};
  void enable() {this->enabled=true;};
  void bypass() {this->enabled=false;};

private:

  inline float gainCurve(float level, float noise) {
    static float localGain=0.;
    static float noiseGain=1.0;
    if(noise<noisethr) { // below noise thr
      if(noiseGain>0.001) {
        noiseGain*=this->gnoiseRel;
        return noiseGain*this->gain;
      } else {
        return 0.;
      }
    } else if(level<thrIn) { // higher than noise thr
      if(noiseGain<0.999) {
        noiseGain+=this->gnoiseAtt;
        return noiseGain*this->gain;
      } else {
        noiseGain=1.0;
        localGain=this->gain;
        return localGain;
      }
    } else { // larger than noise & threshold
      localGain=(thr+(level-thrIn)*ratioInv)/level;
      return localGain;
    }
  };
  
}; // end class

#endif
