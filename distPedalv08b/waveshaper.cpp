/* Waveshaper-based distortion addition to Audio Library for Teensy 3.X
 * Copyright (c) 2016, Joel Cugnoni (joel.cugnoni@gmail.com)
 * 
 * Inspired by the Bitcrusher effect in Audio Library and many docs from CCRMA & DSP community
 * 
 */

 /* TODO LIST 
  *  - migrate to Float 
  *  - implement TFT GUI for main parameters: waveshaper, gains, 
  *  - implement a compression block
  *  - implement sag/squishiness by variable input/output compression or 2D interpolation...
  *  - implement interpolation using arm dsp function? benchmark! higher order interpolation?
  *  - implement a tube stage model for multistage distortion for asymmetric clipping, even order harmonics
  *  - implement a power amp model for symmetric waveshaping  / odd order harmonics
  */

#include "waveshaper.h"


void AudioEffectWaveshaper::setTanh(float b[5]) {
    // set the waveshaper as follows
    //  input range +/-1.0; output range -1.0 to 1.0; 
    //  y=b[0]*tanh(b[1]*(x+b[2]))+b[3]*x+b[4];
    coefs[0]=b[0];
    coefs[1]=b[1];
    coefs[2]=b[2]/b[1];
    coefs[3]=b[3];
    coefs[4]=b[4];
    // define wave table
    for(int i=0; i<DISTTABLESIZE; i++) {
      float x=((float)(-DISTTABLEBND+i*DISTTABLESTEP))/( (float) DISTTABLEBND);
      this->wavex[i]=floor(DISTTABLEBND*x);
      float y=coefs[0] * tanh(coefs[1] * (x+ coefs[2])) + coefs[3]*x+coefs[4];
      this->wavetable[i]=floor(DISTTABLEBND * sign(y)*min(1.0,abs(y)));
    }
    #ifdef DEBUGDIST
    Serial.println("Setting waveshaper table:");
    for(int i=0; i<DISTTABLESIZE; i++) {
      Serial.print(this->wavex[i]);
      Serial.print(", ");
      Serial.println(this->wavetable[i]);
    }
    Serial.println("DONE");
    delay(10000);
    #endif
    isenabled=true;
}

void AudioEffectWaveshaper::setLinear(float gain1) {
    // set the waveshaper as follows
    //  input range +/-1.0; output range -1.0 to 1.0; 
    //  y=gain*x;
    coefs[0]=gain1;
    coefs[1]=0;
    coefs[2]=0;
    coefs[3]=0;
    coefs[4]=0;
    // define wave table
    for(int i=0; i<DISTTABLESIZE; i++) {
      float x=((float)(-DISTTABLEBND+i*DISTTABLESTEP))/( (float) DISTTABLEBND);
      this->wavex[i]=floor(DISTTABLEBND*x);
      float y=gain1*x;
      this->wavetable[i]=floor(DISTTABLEBND * (sign(y)*min(1.0,abs(y))));
    }
    #ifdef DEBUGDIST
    Serial.println("Setting waveshaper table:");
    for(int i=0; i<DISTTABLESIZE; i++) {
      Serial.print(this->wavex[i]);
      Serial.print(", ");
      Serial.println(this->wavetable[i]);
    }
    Serial.println("DONE");
    delay(10000);
    #endif

    isenabled=true;
}

void AudioEffectWaveshaper::setSoftClipping(float gain1) {
    // set the waveshaper as follows
    //  input range +/-1.0; output range -1.0 to 1.0; 
    //  y=(gain+1)*x/(1+abs(gain*x));
    coefs[0]=gain1;
    coefs[1]=0;
    coefs[2]=0;
    coefs[3]=0;
    coefs[4]=0;
    // define wave table
    for(int i=0; i<DISTTABLESIZE; i++) {
      float x=((float)(-DISTTABLEBND+i*DISTTABLESTEP))/( (float) DISTTABLEBND);
      this->wavex[i]=floor(DISTTABLEBND*x);
      float y=((gain1+1)*x/(1+abs(gain1*x)));
      this->wavetable[i]=floor(DISTTABLEBND * (sign(y)*min(1.0,abs(y))));
    }
    #ifdef DEBUGDIST
    Serial.println("Setting waveshaper table:");
    for(int i=0; i<DISTTABLESIZE; i++) {
      Serial.print(this->wavex[i]);
      Serial.print(", ");
      Serial.println(this->wavetable[i]);
    }
    Serial.println("DONE");
    delay(10000);
    #endif
    isenabled=true;
}

void AudioEffectWaveshaper::setExp(float b[4]) {
    // set the waveshaper as follows
    //  input range +/-1.0; output range -1.0 to 1.0; 
    //  y=b[0]*sign(x+b[2])*(1-exp(b[1]*abs(x+b[2]))+b[3];
    coefs[0]=b[0];
    coefs[1]=b[1];
    coefs[2]=b[2]/b[1];
    coefs[3]=b[3];
    coefs[4]=0;
    // define wave table
    for(int i=0; i<DISTTABLESIZE; i++) {
      float x=((float)(-DISTTABLEBND+i*DISTTABLESTEP))/( (float) DISTTABLEBND);
      float x1=floor(DISTTABLEBND*x);
      this->wavex[i]=x1;
      float xp=x+coefs[2]; 
      float y=coefs[0]*sign(xp)*(1-exp(coefs[1]*abs(xp)))+coefs[3];
      this->wavetable[i]=floor(DISTTABLEBND * (sign(y)*min(1.0,abs(y))));
    }
    #ifdef DEBUGDIST
    Serial.println("Setting waveshaper table:");
    for(int i=0; i<DISTTABLESIZE; i++) {
      Serial.print(this->wavex[i]);
      Serial.print(", ");
      Serial.println(this->wavetable[i]);
    }
    Serial.println("DONE");
    delay(10000);
    #endif
    isenabled=true;
}


void AudioEffectWaveshaper::setPow(float b[5]) {
    // set the waveshaper as follows
    //  input range +/-1.0; output range -1.0 to 1.0; 
    //  y=b[0]*sign(x)*pow(abs(b[1]*x+b[2],b[3])+b[4];
    coefs[0]=b[0];
    coefs[1]=b[1];
    coefs[2]=b[2];
    coefs[3]=b[3];
    coefs[4]=b[4];
    // define wave table
    for(int i=0; i<DISTTABLESIZE; i++) {
      float x=((float)(-DISTTABLEBND+i*DISTTABLESTEP))/( (float) DISTTABLEBND);
      this->wavex[i]=floor(DISTTABLEBND*x);
      float y=b[0] * sign(b[1] * x+ b[2]) * pow( abs(b[1] * x+ b[2]), b[3]) + b[4];
      this->wavetable[i]=floor(DISTTABLEBND * (sign(y)*min(1.0,abs(y))));
    }
    #ifdef DEBUGDIST
    Serial.println("Setting waveshaper table:");
    for(int i=0; i<DISTTABLESIZE; i++) {
      Serial.print(this->wavex[i]);
      Serial.print(", ");
      Serial.println(this->wavetable[i]);
    }
    Serial.println("DONE");
    delay(10000);
    #endif

    isenabled=true;
}

void AudioEffectWaveshaper::setCubic(float b[4]) {
    // set the polynomial coefficients as float: waveshaper define as follows
    //  input range +/-1 output range -1 to 1; 
    // y=postgain*((abs(2*pregain*(x+bias))-(pregain*(x+bias))^2)*sign(x+bias))+offsety=coefs[3]*x3+coefs[2]*x2+coefs[1]*x+coefs[0]
    coefs[0]=b[0];
    coefs[1]=b[1];
    coefs[2]=b[2]/b[1];
    coefs[3]=b[3];
    // define wave table
    for(int i=0; i<DISTTABLESIZE; i++) {   
      float x=((float)(-DISTTABLEBND+i*DISTTABLESTEP))/( (float) DISTTABLEBND);
      float xb=min(1.,max(-1.,coefs[1]*(x+coefs[2])));
      float x2=xb*xb;
      float y=DISTTABLEBND*(coefs[0]*(abs(2*xb)-x2)*sign(xb)+coefs[3]); //y=postgain*((abs(2*pregain*(x+bias))-(pregain*(x+bias))^2)*sign(x+bias))+offset
      wavetable[i]=floor(DISTTABLEBND * (sign(y)*min(1.0,abs(y))));
      this->wavex[i]=floor(DISTTABLEBND*x);
    }
    #ifdef DEBUGDIST
    Serial.println("Setting waveshaper table:");
    for(int i=0; i<DISTTABLESIZE; i++) {
      Serial.print(this->wavex[i]);
      Serial.print(", ");
      Serial.println(this->wavetable[i]);
    }
    Serial.println("DONE");
    delay(10000);
    #endif
    isenabled=true;
}

void AudioEffectWaveshaper::setAsym(float b[4]) {
    // set an asymmetric waveshaper taken from Doidic/Line 6 patent (formulas in Pakarinen & Yeh 2009) 
    //  input range +/-1; output range -1 to 1;  
    coefs[0]=b[0]; // postgain
    coefs[1]=b[1]; // pregain
    coefs[2]=b[2]/b[1]; // bias
    coefs[3]=0;
    coefs[4]=0;
    coefs[6]=0;
    // define wave table
    float y=0.;
    for(int i=0; i<DISTTABLESIZE; i++) {
      float x=((float)(-DISTTABLEBND+i*DISTTABLESTEP))/( (float) DISTTABLEBND);
      float xb=min(1.,max(-1.,coefs[1]*(x+coefs[2])));
      float x2=xb*xb;
      float x3=xb*x2;
      if(xb<-0.08905) {
        y=-3./4.*(1 - pow(1-(abs(xb)-0.032847),12) + 1./3.*(abs(xb)-0.032847))+0.01;
      } else if (xb<0.320018) {
        y=-6.153*x3+3.9375*xb;
      } else {
        y=0.630035;
      }
     this->wavex[i]=floor(DISTTABLEBND*x);
     this->wavetable[i]=floor(DISTTABLEBND * (sign(y)*min(1.0,abs(coefs[0]*y))));
    }
    #ifdef DEBUGDIST
    Serial.println("Setting waveshaperf table:");
    for(int i=0; i<DISTTABLESIZE; i++) {
      Serial.print(this->wavex[i]);
      Serial.print(", ");
      Serial.println(this->wavetable[i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
    isenabled=true;
}


void AudioEffectWaveshaper::setSoftPower(float b[4]) {
    // set a power law modified soft clipper  
    //  input range +/-1; output range -1 to 1;  
    coefs[0]=b[0]; // postgain
    coefs[1]=b[1]; // pregain
    coefs[2]=b[2]/b[1]; // bias
    coefs[3]=b[3];  // power exponent
    coefs[4]=0;
    coefs[6]=0;
    // define wave table
    float y=0.;
    float yoffset=coefs[2]/pow(1+pow(abs(coefs[2]),coefs[3]),1/coefs[3]);
    for(int i=0; i<DISTTABLESIZE; i++) {
     float x=((float)(-DISTTABLEBND+i*DISTTABLESTEP))/( (float) DISTTABLEBND);
     float xb=coefs[1]*(x+coefs[2]);
     y=xb/pow(1+pow(abs(xb),coefs[3]),1/coefs[3])-yoffset;
     this->wavex[i]=floor(DISTTABLEBND*x);
     this->wavetable[i]=floor(DISTTABLEBND * (sign(y)*min(1.0,abs(coefs[0]* y))));
    }
    #ifdef DEBUGDIST
    Serial.println("Setting waveshaperf table:");
    for(int i=0; i<DISTTABLESIZE; i++) {
      Serial.print(this->wavex[i]);
      Serial.print(", ");
      Serial.println(this->wavetable[i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
    isenabled=true;
}

void AudioEffectWaveshaper::setChebyshev(float b[6]) {
    // set a waveshaper define as a series of polynomials of Chebyshev of the 1st kind 
    // input range +/-1; output range -1 to 1; 
    // polynomial: y=sum_over_i b[i]*Ti(x)  where Ti is ith order Chebyshev polynomial
    /*
     * T_0(x) = 1 
       T_1(x)  = x 
       T_2(x)  = 2x^2-1  
       T_3(x)  = 4x^3-3x 
       T_4(x)  = 8x^4-8x^2+1 
       T_5(x)  = 16x^5-20x^3+5x  
     */
    coefs[0]=b[0];
    coefs[1]=b[1];
    coefs[2]=b[2];
    coefs[3]=b[3];
    coefs[4]=b[4];
    coefs[5]=b[5];
    
    // define wave table
    for(int i=0; i<DISTTABLESIZE; i++) {
      float x=((float)(-DISTTABLEBND+i*DISTTABLESTEP))/( (float) DISTTABLEBND);
      float x2=x*x;
      float x3=x2*x;
      float x4=x3*x;
      float x5=x4*x;
      float y=0;
      y+=b[0];
      y+=b[1]*x;
      y+=b[2]*(2*x2-1);
      y+=b[3]*(4*x3-3*x);
      y+=b[4]*(8*x4-8*x2+1);
      y+=b[5]*(16*x5-20*x3+5*x);
      wavetable[i]=floor(DISTTABLEBND * (sign(y)*min(1.0,abs(y))));
      this->wavex[i]=floor(DISTTABLEBND*x);
    }
    #ifdef DEBUGDIST
    Serial.println("Setting waveshaper table:");
    for(int i=0; i<DISTTABLESIZE; i++) {
      Serial.print(this->wavex[i]);
      Serial.print(", ");
      Serial.println(this->wavetable[i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
    isenabled=true;
};

// AUDIO PROCESSING TASK, 2 versions: with and without Oversampling

#if DISTOVERSAMPLING<2  
void AudioEffectWaveshaper::update(void)
{
  audio_block_t *block;
  elapsedMicros t1=0;       
  if (not(isenabled)) {
    // nothing to do. Output is sent through clean, then exit the function
    block = receiveReadOnly();
    if (!block) return;
    transmit(block);
    release(block);
    return;
  }
  
  // start of processing functions. Could be more elegant based on external
  // functions but left like this to enable code optimisation later.
  block = receiveWritable();
  if (!block) return;
  int16_t maxx=0;
 
  for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
      int16_t x;
      int16_t ym,yp;
      // uint32 so sign extension will not occur, fills with zeroes.
      x = block->data[i];
      // find addresses
      //uint16_t idx=((uint16_t) x)>>DISTSHIFTBIT;
      uint16_t idx=max(0,min(x/DISTTABLESTEP+DISTTABLESIZE/2,DISTTABLESIZE));
      uint16_t idx2=max(0,min(idx+1,DISTTABLESIZE));
      int16_t dx=x % DISTTABLESTEP;
      ym=this->wavetable[idx];
      yp=this->wavetable[idx2];
      //float yinterp= ym * ((float) dx)/DISTTABLESTEP + yp * (1-( (float) dx/DISTTABLESTEP) ) ;  // VERY SLOW!!!
      int16_t y=integinterp(ym,yp,dx);
      block->data[i]=y; 
//      #ifdef DEBUGDIST
//      Serial.print("x=");
//      Serial.print(x);
//      Serial.print(" idx=");
//      Serial.print(idx);
//      Serial.print(" y=");
//      Serial.print(y);
//      Serial.print(" x(idx)=");
//      Serial.println(this->wavex[idx]);
//      #endif
     }
   #ifdef DEBUGDIST
   static int count=0;
   if(count<10) {
      count++;
     
     Serial.println("***CHECK WAVETABLE***");
     for(int i=0; i<DISTTABLESIZE; i+=2) {
        Serial.print(i);
        Serial.print(", ");
        Serial.print(this->wavex[i]);
        Serial.print(", ");
        Serial.println(this->wavetable[i]);
     }
     Serial.println("-------");
   }
   #endif
  #ifdef PRINT_TIMING 
    Serial.println(t1);
  #endif
  transmit(block);
  release(block);
}
#else  
void AudioEffectWaveshaper::update(void)
{
  audio_block_t *block;
        
  if (not(isenabled)) {
    // nothing to do. Output is sent through clean, then exit the function
    block = receiveReadOnly();
    if (!block) return;
    transmit(block);
    release(block);
    return;
  }
  
  // start of processing functions. Could be more elegant based on external
  // functions but left like this to enable code optimisation later.
  block = receiveWritable();
  if (!block) return;
  
  //int16_t max0=0;
  //for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) max0=max(max0,abs(block->data[i])); 

  int16_t dataup[DISTOVERSAMPLING*AUDIO_BLOCK_SAMPLES];
  arm_fir_interpolate_q15( &upsampler, block->data, dataup, AUDIO_BLOCK_SAMPLES ); 
    
  //upsample0(DISTOVERSAMPLING,AUDIO_BLOCK_SAMPLES,block->data,dataup);
  //int16_t maxx=0;
  
  // non linear effect
  for (uint16_t i=0; i < DISTOVERSAMPLING*AUDIO_BLOCK_SAMPLES; i++) {
      int16_t x;
      int16_t ym,yp;
      // uint32 so sign extension will not occur, fills with zeroes.
      #if DISTPRESCALESHIFT!=0
      x = dataup[i]<<DISTPRESCALESHIFT;
      #else
      x = dataup[i];
      #endif
      //maxx=max(maxx,abs(x));
      // find addresses
      //uint16_t idx=((uint16_t) x)>>DISTSHIFTBIT;
      uint16_t idx=max(0,min(x/DISTTABLESTEP+DISTTABLESIZE/2,DISTTABLESIZE));
      uint16_t idx2=max(0,min(idx+1,DISTTABLESIZE));
      int16_t dx=x % DISTTABLESTEP;
      ym=this->wavetable[idx];
      yp=this->wavetable[idx2];
      //float yinterp= ym * ((float) dx)/DISTTABLESTEP + yp * (1-( (float) dx/DISTTABLESTEP) ) ;  // VERY SLOW!!!
      int16_t y=integinterp(ym,yp,dx);
      dataup[i]=y; 
//      #ifdef DEBUGDIST
//      Serial.print("x=");
//      Serial.print(x);
//      Serial.print(" idx=");
//      Serial.print(idx);
//      Serial.print(" y=");
//      Serial.print(y);
//      Serial.print(" x(idx)=");
//      Serial.println(this->wavex[idx]);
//      #endif
     }
//   #ifdef DEBUGDIST
//   static int count=0;
//   if(count<10) {
//      count++;
//     
//     Serial.println("***CHECK WAVETABLE***");
//     for(int i=0; i<DISTTABLESIZE; i+=2) {
//        Serial.print(i);
//        Serial.print(", ");
//        Serial.print(this->wavex[i]);
//        Serial.print(", ");
//        Serial.println(this->wavetable[i]);
//     }
//     Serial.println("-------");
//   }
//   #endif
  
  // downsampling 4x
  //arm_fir_decimate_q15(&downsampler,dataup,block->data,DISTOVERSAMPLING*AUDIO_BLOCK_SAMPLES);     
  //downsample(DISTOVERSAMPLING,AUDIO_BLOCK_SAMPLES,dataup,block->data);
  downsampleIIR(DISTOVERSAMPLING,AUDIO_BLOCK_SAMPLES,dataup,block->data);
  /*int16_t maxy=0;
  for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) maxy=max(maxy,abs(block->data[i])); 
  Serial.print("Dist. peaks: in = ");
  Serial.print(max0);
  Serial.print(" up = ");
  Serial.print(maxx);
  Serial.print(" out = ");
  Serial.println(maxy);*/

  // let it go further...
  transmit(block);
  release(block);
}
#endif
