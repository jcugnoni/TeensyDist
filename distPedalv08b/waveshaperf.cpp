/* Waveshaper-based distortion addition to Audio Library for Teensy 3.X
 * Copyright (c) 2016, Joel Cugnoni (joel.cugnoni@gmail.com)
 * 
 * Inspired by the Bitcrusher effect in Audio Library and many docs from CCRMA & DSP community
 * 
 */



#include "waveshaperf.h"


void AudioEffectWaveshaperF::setTanh(float b[5]) {
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
      this->wavex[i]=DISTTABLEBND*x;
      float y=coefs[0] * tanhf(coefs[1] * (x+ coefs[2])) + coefs[3]*x+coefs[4];
      this->wavetable[i]=DISTTABLEBND * sign(y)*min(1.0,fabs(y));
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

void AudioEffectWaveshaperF::setLinear(float gain1) {
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
      this->wavex[i]=(DISTTABLEBND*x);
      float y=gain1*x;
      this->wavetable[i]=(DISTTABLEBND * (sign(y)*min(1.0,fabs(y))));
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
void AudioEffectWaveshaperF::setSoftClipping(float gain1) {
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
      this->wavex[i]=(DISTTABLEBND*x);
      float y=(gain1+1)*x/(1+fabs(gain1*x));
      this->wavetable[i]=(DISTTABLEBND * (sign(y)*min(1.0,fabs(y))));
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

void AudioEffectWaveshaperF::setExp(float b[4]) {
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
      float x1=(DISTTABLEBND*x);
      this->wavex[i]=x1;
      float xp=x+coefs[2]; 
      float y=coefs[0]*sign(xp)*(1-expf(coefs[1]*fabs(xp)))+coefs[3];
      this->wavetable[i]=(DISTTABLEBND * (sign(y)*min(1.0,fabs(y))));
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


void AudioEffectWaveshaperF::setPow(float b[5]) {
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
      this->wavex[i]=(DISTTABLEBND*x);
      float y=b[0] * sign(b[1] * x+ b[2]) * powf( fabs(b[1] * x+ b[2]), b[3]) + b[4];
      this->wavetable[i]=(DISTTABLEBND * (sign(y)*min(1.0,fabs(y))));
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

void AudioEffectWaveshaperF::setCubic(float b[4]) {
    // set the polynomial coefficients as float: waveshaper define as follows
    //  input range +/-1; output range -1 to 1; 
    // polynomial: 
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
      float y=(coefs[0]*(fabs(2*xb)-x2)*sign(xb)+coefs[3]); //y=postgain*((abs(2*pregain*(x+bias))-(pregain*(x+bias))^2)*sign(x+bias))+offset   wavetable[i]=(DISTTABLEBND * (sign(y)*min(1.0,abs(y))));
      this->wavex[i]=(DISTTABLEBND*x);
      this->wavetable[i]=(DISTTABLEBND * (sign(y)*min(1.0,fabs(y))));
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

void AudioEffectWaveshaperF::setAsym(float b[4]) {
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
      if(xb<-0.08905) {
        y=-3./4.*(1 - powf(1-(fabs(xb)-0.032848),12) + 1./3.*(fabs(xb)-0.032848) )-0.01;
      } else if (xb<0.320018) {
        y=-6.153*x2+3.9375*xb;
      } else {
        y=0.630035;
      }
      this->wavex[i]=(DISTTABLEBND*x);
      this->wavetable[i]=(DISTTABLEBND * (sign(y)*min(1.0,fabs(coefs[0]*y))));
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


void AudioEffectWaveshaperF::setSoftPower(float b[4]) {
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
    float yoffset=coefs[2]/powf(1+powf(fabs(coefs[2]),coefs[3]),1/coefs[3]);
    for(int i=0; i<DISTTABLESIZE; i++) {
     float x=((float)(-DISTTABLEBND+i*DISTTABLESTEP))/( (float) DISTTABLEBND);
     float xb=coefs[1]*(x+coefs[2]);
     y=xb/powf(1+powf(fabs(xb),coefs[3]),1/coefs[3])-yoffset;
     this->wavex[i]=(DISTTABLEBND*x);
     this->wavetable[i]=(DISTTABLEBND * (sign(y)*min(1.0,fabs(coefs[0]* y))));
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

void AudioEffectWaveshaperF::setChebyshev(float b[6]) {
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
      wavetable[i]=(DISTTABLEBND * (sign(y)*min(1.0,fabs(y))));
      this->wavex[i]=(DISTTABLEBND*x);
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
};


// AUDIO PROCESSING TASK, 2 versions: with and without Oversampling

#if DISTOVERSAMPLING<2  
void AudioEffectWaveshaperF::update(void)
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
  arm_linear_interp_instance_f32 S = {DISTTABLESIZE, -DISTTABLEBND, DISTTABLESTEP, &this->wavetable[0]};
  for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
//      float x;
//      float ym,yp,xm;
//      // uint32 so sign extension will not occur, fills with zeroes.
//      x = block->data[i];
//      //if(x>maxx) maxx=x;
//      // find addresses
//      //uint16_t idx=((uint16_t) x)>>DISTSHIFTBIT;
//      uint16_t idx=floor(max(0,min(x/DISTTABLESTEP+DISTTABLESIZE/2,DISTTABLESIZE)));
//      uint16_t idx2=floor(max(0,min(idx+1,DISTTABLESIZE)));
//      ym=this->wavetable[idx];
//      yp=this->wavetable[idx2];
//      xm=this->wavex[idx];
//      float fact1=1/DISTTABLESTEP;
//      float dx=(x-xm)*fact1 ;
//      float y= ym * dx + yp * (1-dx) ;  // VERY SLOW!!!
//      //int16_t y=integinterp(ym,yp,dx);
        block->data[i]=floor( arm_linear_interp_f32(&S, block->data[i]) ); 
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
 // Serial.println(maxx);
   #ifdef PRINT_TIMING 
    Serial.println(t1);
  #endif
  transmit(block);
  release(block);
}

#else  

// OVERSAMPLED VERSION
void AudioEffectWaveshaperF::update(void)
{
  audio_block_t *block;
  float inbuffer[AUDIO_BLOCK_SAMPLES];
  elapsedMicros timing1=0;
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
  
  //int16_t max0=0;
  //for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) max0=max(max0,abs(block->data[i])); 
  
  // convert input to float
  //float maxi=-99999;
  //for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) { 
  //  inbuffer[i]=block->data[i]; 
    //if(inbuffer[i]>maxi) maxi=inbuffer[i] ; 
  //}

  // ---- Upsampling -----
  
  //Serial.println(maxi);
  float dataup[DISTOVERSAMPLING*AUDIO_BLOCK_SAMPLES];
  // upsampling by linear interpolation, to check!
  timing1=0;
  upsample(DISTOVERSAMPLING,AUDIO_BLOCK_SAMPLES,block->data,dataup);
  //upsample0(DISTOVERSAMPLING,AUDIO_BLOCK_SAMPLES,block->data,dataup);
  //arm_fir_interpolate_f32( &upsampler, inbuffer, dataup, AUDIO_BLOCK_SAMPLES ); 
  //int16_t maxx=0;
  #ifdef PRINT_TIMING 
    Serial.println(timing1);
  #endif
 
  timing1=0;
  arm_linear_interp_instance_f32 S = {DISTTABLESIZE, -DISTTABLEBND, DISTTABLESTEP, &this->wavetable[0]};
  // ----  non linear waveshaping effect ----
  float fact1=1/DISTTABLESTEP;
  for (uint16_t i=0; i < DISTOVERSAMPLING*AUDIO_BLOCK_SAMPLES; i++) {
//      float x, ym,yp,xm;
//      x = dataup[i];
//      uint16_t idx=floor(max(0,min(x/DISTTABLESTEP+DISTTABLESIZE/2,DISTTABLESIZE)));
//      uint16_t idx2=floor(max(0,min(idx+1,DISTTABLESIZE)));
//      ym=this->wavetable[idx];
//      yp=this->wavetable[idx2];
//      xm=this->wavex[idx];
//      float dx=(x-xm)*fact1 ;
//      float y= ym * dx + yp * (1-dx) ;  // VERY SLOW!!!
//      //int16_t y=integinterp(ym,yp,dx);
    
      dataup[i]=arm_linear_interp_f32(&S, dataup[i]); 
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
   #ifdef PRINT_TIMING 
    Serial.println(timing1);
   #endif   
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
  
  // ---  downsampling by x ----
  
//  float maxx=-99999;
//  float minx=99999; 
//  float sumx=0.;
//  for (uint16_t i=0; i < DISTOVERSAMPLING*AUDIO_BLOCK_SAMPLES; i++) {
//    if(dataup[i]>maxx) maxx=dataup[i];
//    if(dataup[i]<minx) minx=dataup[i];
//    sumx+=dataup[i];
//  }
//  Serial.println(maxx);
//  Serial.println(minx);
//  Serial.println(sumx/128/DISTOVERSAMPLING);
//  Serial.println("---");
  timing1=0;
  arm_fir_decimate_f32(&downsampler,dataup,inbuffer,DISTOVERSAMPLING*AUDIO_BLOCK_SAMPLES);     
  //downsample(DISTOVERSAMPLING,AUDIO_BLOCK_SAMPLES,dataup,inbuffer);
  //downsampleIIR(DISTOVERSAMPLING,AUDIO_BLOCK_SAMPLES,dataup,inbuffer);
  
  #ifdef PRINT_TIMING 
    Serial.println(timing1);   
  #endif
  // OUTPUT
  for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
     block->data[i]=floor(inbuffer[i]); 
//    if(inbuffer[i]>maxx) maxx=inbuffer[i];
  }
  //Serial.println(maxx);

  /*int16_t maxy=0;
  for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) maxy=max(maxy,abs(block->data[i])); 
  Serial.print("Dist. peaks: in = ");
  Serial.print(max0);
  Serial.print(" up = ");
  Serial.print(maxx);
  Serial.print(" out = ");
  Serial.println(maxy);*/
   #ifdef PRINT_TIMING 
    Serial.println(t1);
    Serial.println("--------");
  #endif
  // let it go further...
  transmit(block);
  release(block);
}
#endif
