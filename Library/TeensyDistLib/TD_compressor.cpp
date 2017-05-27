#include "TD_compressor.h"

void AudioEffectCompressor::update(void)
{
  audio_block_f32_t *block;
  elapsedMicros t1=0;
  
  if (not(enabled)) {
    // nothing to do. Output is sent through clean, then exit the function
    block = AudioStream_F32::receiveReadOnly_f32();
    if (!block) return;
    AudioStream_F32::transmit(block);
    AudioStream_F32::release(block);
    return;
  }
  
  // start of processing functions. Could be more elegant based on external
  // functions but left like this to enable code optimisation later.
  block = AudioStream_F32::receiveWritable_f32();
  if (!block) return;

  // processing
  float envelope=prevEnv;
  float noise=prevNoise;
  float gainSmooth=prevGain;
  float g=0;
  float xabs=0;
  for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i+=2) {
   // compute envelope
   float x=block->data[i];
   xabs=fabs(x);
   // compute envelope
   if(envelope>xabs) {
    envelope-=grel*(envelope-xabs); 
   } else {
    envelope+=gatt*(xabs-envelope); 
   }
   // compute noise lvl
   if(noise>xabs) {
    noise-=greln*(noise-xabs);
   } else {
    noise+=gattn*(xabs-noise);
   }  
   // compute regularized gain (secant gain)
   g=gainCurve(envelope,noise);
   gainSmooth+=0.01*(g-gainSmooth);
   // compute output through gain
   block->data[i]=x*g;
  }
  // save for later
  prevEnv=envelope; 
  prevNoise=noise; 
  prevGain=gainSmooth;
  AudioStream_F32::transmit(block);
  AudioStream_F32::release(block);
  /*Serial.print(prevEnv);
  Serial.print(",");
  Serial.print(3000*g);
  Serial.print(",");
  Serial.print(noise);
  Serial.print(",");
  Serial.println(xabs);*/
  
  #ifdef PRINT_TIMING
    Serial.println(t1);
  #endif
  
  return;
}
