/* Multistage Waveshaper-based distortion addition to Audio Library for Teensy 3.X
 * Copyright (c) 2016, Joel Cugnoni (joel.cugnoni@gmail.com) */

#include "TD_multistagedist.h"

void AudioEffectMultiStageDist::setTanh(float b[5], uint8_t stage) {
    // set the waveshaper as follows
    //  input range +/-1.0; output range -1.0 to 1.0; 
    //  y=b[0]*tanh(b[1]*(x+b[2]))+b[3]*x+b[4];
    if(stage>=MSDMAXSTAGES) return;
    coefs[0]=CLIPSCALE*b[0];
    coefs[1]=b[1];
    coefs[2]=b[2]/b[1];
    coefs[3]=b[3];
    coefs[4]=b[4];
    // define wave table
    for(int i=0; i<MSDTABLESIZE; i++) {
      float x=((float)(-MSDTABLEBND+i*MSDTABLESTEP))/( (float) MSDTABLEBND);
      float y=coefs[0] * tanhf(coefs[1] * (x+ coefs[2])) + coefs[3]*x+coefs[4];
      this->wavetables[stage][i]=MSDTABLEBND * sign(y)*min(1.0,fabs(y));
    }
    this->topology[stage]=false;
    updateOpPoint(stage);

    #ifdef DEBUGDIST
    Serial.println("Setting waveshaperf table:");
    for(int i=0; i<MSDTABLESIZE; i++) {
      Serial.println(this->wavetables[stage][i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
}

void AudioEffectMultiStageDist::setLinear(float gain1,uint8_t stage) {
    // set the waveshaper as follows
    //  input range +/-1.0; output range -1.0 to 1.0; 
    //  y=gain*x;
    if(stage>=MSDMAXSTAGES) return;
    coefs[0]=gain1;
    coefs[1]=0;
    coefs[2]=0;
    coefs[3]=0;
    coefs[4]=0;
    // define wave table
    for(int i=0; i<MSDTABLESIZE; i++) {
      float x=((float)(-MSDTABLEBND+i*MSDTABLESTEP))/( (float) MSDTABLEBND);
      float y=CLIPSCALE*gain1*x;
      this->wavetables[stage][i]=(MSDTABLEBND * (sign(y)*min(1.0,fabs(y))));
    }
    this->topology[stage]=false;
    updateOpPoint(stage);

    #ifdef DEBUGDIST
    Serial.println("Setting waveshaperf table:");
    for(int i=0; i<MSDTABLESIZE; i++) {
      Serial.println(this->wavetables[stage][i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
}
void AudioEffectMultiStageDist::setSoftClipping(float gain1,uint8_t stage) {
    // set the waveshaper as follows
    //  input range +/-1.0; output range -1.0 to 1.0; 
    //  y=(gain+1)*x/(1+abs(gain*x));
    if(stage>=MSDMAXSTAGES) return;
    coefs[0]=gain1;
    coefs[1]=0;
    coefs[2]=0;
    coefs[3]=0;
    coefs[4]=0;
    this->topology[stage]=false;
    // define wave table
    for(int i=0; i<MSDTABLESIZE; i++) {
      float x=((float)(-MSDTABLEBND+i*MSDTABLESTEP))/( (float) MSDTABLEBND);
      float y=CLIPSCALE*(gain1+1)*x/(1+fabs(gain1*x));
      this->wavetables[stage][i]=(MSDTABLEBND * (sign(y)*min(1.0,fabs(y))));
    }
    updateOpPoint(stage);

    #ifdef DEBUGDIST
    Serial.println("Setting waveshaperf table:");
    for(int i=0; i<MSDTABLESIZE; i++) {
      Serial.println(this->wavetables[stage][i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
}

void AudioEffectMultiStageDist::setExp(float b[4],uint8_t stage) {
    // set the waveshaper as follows
    //  input range +/-1.0; output range -1.0 to 1.0; 
    //  y=b[0]*sign(x+b[2])*(1-exp(b[1]*abs(x+b[2]))+b[3];
    if(stage>=MSDMAXSTAGES) return;
    coefs[0]=b[0];
    coefs[1]=b[1];
    coefs[2]=b[2]/b[1];
    coefs[3]=b[3];
    coefs[4]=0;
    // define wave table
    for(int i=0; i<MSDTABLESIZE; i++) {
      float x=((float)(-MSDTABLEBND+i*MSDTABLESTEP))/( (float) MSDTABLEBND);
      float x1=(MSDTABLEBND*x);
      float xp=x+coefs[2]; 
      float y=CLIPSCALE*coefs[0]*sign(xp)*((1-expf(coefs[1]*fabs(xp)))+coefs[3]);
      this->wavetables[stage][i]=(MSDTABLEBND * (sign(y)*min(1.0,fabs(y))));
    }
    this->topology[stage]=false;
    updateOpPoint(stage);

    #ifdef DEBUGDIST
    Serial.println("Setting waveshaperf table:");
    for(int i=0; i<MSDTABLESIZE; i++) {
      Serial.println(this->wavetables[stage][i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
}


void AudioEffectMultiStageDist::setPow(float b[5],uint8_t stage) {
    // set the waveshaper as follows
    //  input range +/-1.0; output range -1.0 to 1.0; 
    //  y=b[0]*sign(x)*pow(abs(b[1]*x+b[2],b[3])+b[4];
    if(stage>=MSDMAXSTAGES) return;
    coefs[0]=b[0];
    coefs[1]=b[1];
    coefs[2]=b[2];
    coefs[3]=b[3];
    coefs[4]=b[4];
    // define wave table
    for(int i=0; i<MSDTABLESIZE; i++) {
      float x=((float)(-MSDTABLEBND+i*MSDTABLESTEP))/( (float) MSDTABLEBND);
      float y=CLIPSCALE*(b[0] * sign(b[1] * x+ b[2]) * powf( fabs(b[1] * x+ b[2]), b[3]) + b[4]);
      this->wavetables[stage][i]=(MSDTABLEBND * (sign(y)*min(1.0,fabs(y))));
    }
    this->topology[stage]=false;
    updateOpPoint(stage);

    #ifdef DEBUGDIST
    Serial.println("Setting waveshaperf table:");
    for(int i=0; i<MSDTABLESIZE; i++) {
      Serial.println(this->wavetables[stage][i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
}

void AudioEffectMultiStageDist::setCubic(float b[4], uint8_t stage) {
    // set the polynomial coefficients as float: waveshaper define as follows
    //  input range +/-1; output range -1 to 1; 
    // polynomial: 
    // y=postgain*((abs(2*pregain*(x+bias))-(pregain*(x+bias))^2)*sign(x+bias))+offsety=coefs[3]*x3+coefs[2]*x2+coefs[1]*x+coefs[0]
    if(stage>=MSDMAXSTAGES) return;
    coefs[0]=b[0];
    coefs[1]=b[1];
    coefs[2]=b[2]/b[1];
    coefs[3]=b[3];
    // define wave table
    for(int i=0; i<MSDTABLESIZE; i++) {
      float x=((float)(-MSDTABLEBND+i*MSDTABLESTEP))/( (float) MSDTABLEBND);
      float xb=min(1.,max(-1.,coefs[1]*(x+coefs[2])));
      float x2=xb*xb;
      float y=CLIPSCALE*(coefs[0]*(fabs(2*xb)-x2)*sign(xb)+coefs[3]); //y=postgain*((abs(2*pregain*(x+bias))-(pregain*(x+bias))^2)*sign(x+bias))+offset   wavetable[i]=(MSDTABLEBND * (sign(y)*min(1.0,abs(y))));
      this->wavetables[stage][i]=(MSDTABLEBND * (sign(y)*min(1.0,fabs(y))));
    }
    this->topology[stage]=false;
    updateOpPoint(stage);

    #ifdef DEBUGDIST
    Serial.println("Setting waveshaperf table:");
    for(int i=0; i<MSDTABLESIZE; i++) {
      Serial.println(this->wavetables[stage][i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
}

void AudioEffectMultiStageDist::setAsym(float b[4],uint8_t stage) {
    // set an asymmetric waveshaper taken from Doidic/Line 6 patent (formulas in Pakarinen & Yeh 2009) 
    //  input range +/-1; output range -1 to 1;  
    if(stage>=MSDMAXSTAGES) return;
    coefs[0]=b[0]; // postgain
    coefs[1]=b[1]; // pregain
    coefs[2]=b[2]/b[1]; // bias
    coefs[3]=0;
    coefs[4]=0;
    coefs[6]=0;
    // define wave table
    float y=0.;
    for(int i=0; i<MSDTABLESIZE; i++) {
      float x=((float)(-MSDTABLEBND+i*MSDTABLESTEP))/( (float) MSDTABLEBND);
      float xb=min(1.,max(-1.,coefs[1]*(x+coefs[2])));
      float x2=xb*xb;
      if(xb<-0.08905) {
        y=-3./4.*(1 - powf(1-(fabs(xb)-0.032848),12) + 1./3.*(fabs(xb)-0.032848) )-0.01;
      } else if (xb<0.320018) {
        y=-6.153*x2+3.9375*xb;
      } else {
        y=0.630035;
      }
      y=CLIPSCALE*y;
      this->wavetables[stage][i]=(MSDTABLEBND * (sign(y)*min(1.0,fabs(coefs[0]*y))));
    }
    this->topology[stage]=false;
    updateOpPoint(stage);

    #ifdef DEBUGDIST
    Serial.println("Setting waveshaperf table:");
    for(int i=0; i<MSDTABLESIZE; i++) {
      Serial.println(this->wavetables[stage][i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
}


void AudioEffectMultiStageDist::setSoftPower(float b[4],uint8_t stage) {
    // set a power law modified soft clipper  
    //  input range +/-1; output range -1 to 1;  
    if(stage>=MSDMAXSTAGES) return;
    coefs[0]=b[0]; // postgain
    coefs[1]=b[1]; // pregain
    coefs[2]=b[2]/b[1]; // bias
    coefs[3]=b[3];  // power exponent
    coefs[4]=0;
    coefs[6]=0;
    // define wave table
    float y=0.;
    float yoffset=coefs[2]/powf(1+powf(fabs(coefs[2]),coefs[3]),1/coefs[3]);
    for(int i=0; i<MSDTABLESIZE; i++) {
     float x=((float)(-MSDTABLEBND+i*MSDTABLESTEP))/( (float) MSDTABLEBND);
     float xb=coefs[1]*(x+coefs[2]);
     y=CLIPSCALE*(xb/powf(1+powf(fabs(xb),coefs[3]),1/coefs[3])-yoffset);
     this->wavetables[stage][i]=(MSDTABLEBND * (sign(y)*min(1.0,fabs(coefs[0]* y))));
    } 
    this->topology[stage]=false;
    updateOpPoint(stage);

    #ifdef DEBUGDIST
    Serial.println("Setting waveshaperf table:");
    for(int i=0; i<MSDTABLESIZE; i++) {
      Serial.println(this->wavetables[stage][i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
}

void AudioEffectMultiStageDist::setChebyshev(float b[6],uint8_t stage) {
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
    if(stage>=MSDMAXSTAGES) return;
    
    coefs[0]=b[0];
    coefs[1]=b[1];
    coefs[2]=b[2];
    coefs[3]=b[3];
    coefs[4]=b[4];
    coefs[5]=b[5];
    
    // define wave table
    for(int i=0; i<MSDTABLESIZE; i++) {
      float x=((float)(-MSDTABLEBND+i*MSDTABLESTEP))/( (float) MSDTABLEBND);
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
      y=CLIPSCALE*y;
      this->wavetables[stage][i]=(MSDTABLEBND * (sign(y)*min(1.0,fabs(y))));
    }
    this->topology[stage]=false;
    updateOpPoint(stage);

    #ifdef DEBUGDIST
    Serial.println("Setting waveshaperf table:");
    for(int i=0; i<MSDTABLESIZE; i++) {
      Serial.println(this->wavetables[stage][i]);
    }
    Serial.println("DONE");
    delay(1000);
    #endif
};

void AudioEffectMultiStageDist::setTube(uint8_t type, uint8_t stage) {
  if(stage>=MSDMAXSTAGES) return;
  if(type>TUBETABLE_N-1) { 
    Serial.println("Error: tube table does not exist for this type");
    return;
  }
  for(int i=0; i<MSDTABLESIZE; i++) {
    this->wavetables[stage][i]=interpolateHermite(TUBETABLE_LEN,-MSDTABLEBND, MSDTABLEBND, (TUBETABLE_LEN-1)/(2.0*MSDTABLEBND), &tubetable[type][0],-MSDTABLEBND+MSDTABLESTEP*i);
  }
  this->topology[stage]=false;
  updateOpPoint(stage);
};

void AudioEffectMultiStageDist::setTubePushPull(uint8_t type, uint8_t stage, float bias) {
  bias/=5.0;
  if(stage>=MSDMAXSTAGES) return;
  if(type>TUBETABLE_N-1) { 
    Serial.println("Error: tube table does not exist for this type");
    return;
  }
  for(int i=0; i<MSDTABLESIZE; i++) {
    float in=-MSDTABLEBND+MSDTABLESTEP*i;
    float v0=-interpolateHermite(TUBETABLE_LEN,-MSDTABLEBND, MSDTABLEBND, (TUBETABLE_LEN-1)/(2.0*MSDTABLEBND), &tubetable[type][0],-in-bias);
    float v1=interpolateHermite(TUBETABLE_LEN,-MSDTABLEBND, MSDTABLEBND, (TUBETABLE_LEN-1)/(2.0*MSDTABLEBND), &tubetable[type][0],in-bias);
    //float v=interpolateHermite(TUBETABLE_LEN,-MSDTABLEBND, (2.0*MSDTABLEBND)/(TUBETABLE_LEN-1), &tubetable[type][0],in);
    this->wavetables[stage][i]=(v0+v1);
  }
  this->topology[stage]=true;
//
//  Serial.println("Waveshaper table:");
//  for(int i=0; i<MSDTABLESIZE; i++) {
//      Serial.println(this->wavetables[stage][i]);
//  }
//  Serial.println("DONE");
//  delay(10000);
//  
  updateOpPoint(stage);
};

void AudioEffectMultiStageDist::setPreGain(float gain, uint8_t stage) {
  if(stage>=MSDMAXSTAGES) return;
  this->pregains[stage]=gain;
  updateGains();
};

void AudioEffectMultiStageDist::setPostGain(float gain, uint8_t stage) {
  if(stage>MSDMAXSTAGES) return;
  this->postgains[stage]=gain;
  updateGains();
};

void AudioEffectMultiStageDist::updateGains() {
  // reset default gains = unity
  for(uint8_t stage=0;stage<=MSDMAXSTAGES;stage++) this->gains[stage]=1.0;
  // multiply gains for pre gains
  for(uint8_t stage=0;stage<MSDMAXSTAGES;stage++) {
    if(this->enabled[stage]) this->gains[stage] *= this->pregains[stage];
  }
  // apply postgains on the next enabled stage (or last master post gain)
  for(uint8_t stage=0;stage<MSDMAXSTAGES;stage++) {
    if(this->enabled[stage]) {
      for(uint8_t stage2=stage+1; stage2 <= MSDMAXSTAGES; stage2++) {
         if((this->enabled[stage2])||stage2==MSDMAXSTAGES) {
          this->gains[stage2] *= this->postgains[stage];
          break;
         }
      }
    }
  }
  // apply master gains control
  this->gains[MSDMAXSTAGES] *= this->postgains[MSDMAXSTAGES];
  // correct for gain adaptation
  this->gains[0] *=this->gainAdapt;
  this->gains[MSDMAXSTAGES] /= this->gainAdapt;
};

void AudioEffectMultiStageDist::setBias(float bias,uint8_t stage) {
  if(stage>=MSDMAXSTAGES) return;
  this->bias[stage]=bias;
  updateOpPoint(stage);
};

void AudioEffectMultiStageDist::enableStage(uint8_t stage){
  if(stage>=MSDMAXSTAGES) return;
  this->enabled[stage]=true;
  this->isenabled=true;
  updateGains();
};

void AudioEffectMultiStageDist::bypassStage(uint8_t stage){
  if(stage>=MSDMAXSTAGES) return;
  this->enabled[stage]=false;
  if(this->activeStages()==0) this->isenabled=false;
  updateGains();
};

uint8_t AudioEffectMultiStageDist::activeStages() {
  uint8_t out=0;
  for(uint8_t i=0; i<MSDMAXSTAGES; i++) {
    if(this->enabled[i]) out = out | (1 << i);
  }
  return(out);
};

void AudioEffectMultiStageDist::updateOpPoint(uint8_t stage) {  // set DC operation point corresponding to bias for given stage
    if(stage>=MSDMAXSTAGES) return;
    //arm_linear_interp_instance_f32 S1={MSDTABLESIZE,-MSDTABLEBND,(2*MSDTABLEBND+1)/MSDTABLESIZE,this->wavetables[stage][0]};
    float x= this->bias[stage];
    this->oppoint[stage]=this->interpolate(MSDTABLESIZE,-MSDTABLEBND, MSDTABLEBND, 1.0/MSDTABLESTEP, &this->wavetables[stage][0], x);   //arm_linear_interp_f32(&S1, this->bias[stage]);
//    #ifdef DEBUGDIST
//    Serial.print("updateOpPoint: Bias= ");
//    Serial.print(this->bias[stage]);
//    Serial.print(", Op Point= ");
//    Serial.println(this->oppoint[stage]);
//    #endif
};

void AudioEffectMultiStageDist::setLowpass(float freq,uint8_t stage) {
   if(stage>=MSDMAXSTAGES) return;
   float omegac=2*3.1415926*freq/(44117.0*OVERSAMPLING);
   float K=tan(omegac/2);
   float alpha=1+K;
   this->lpf[stage]=freq;
   this->lpfa[stage]=-(1-K)/alpha;
   this->lpfb[stage]=K/alpha;
};

void AudioEffectMultiStageDist::setHighpass(float freq,uint8_t stage) {
   if(stage>=MSDMAXSTAGES) return;
   float omegac=2*3.1415926*freq/(44117.0*OVERSAMPLING);
   float K=tan(omegac/2);
   float alpha=1+K;
   this->hpf[stage]=freq;
   this->hpfr[stage]=(1-K)/alpha;
};


// AUDIO PROCESSING TASK
void AudioEffectMultiStageDist::update(void)
{
  // io management
  audio_block_f32_t *block;
  elapsedMicros t1=0;
  
  if (not(isenabled)) {
    // nothing to do. Output is sent through clean, then exit the function
    block = AudioStream_F32::receiveReadOnly_f32();
    if (!block) return;
    AudioStream_F32::transmit(block);
    AudioStream_F32::release(block);
    return;
  }
  block = AudioStream_F32::receiveWritable_f32();
  if (!block) return;

  // start of processing functions. 
  // signal flow:  gain0 => stage0 => filter0 => gain1 => stage1 => filter1 => ... => postgain  
  // each stage is : input => biasing => waveshaping => dc_removal => output
  // each filter stage is a simple first order lowpass/highpass
  // gain0 should be defined as gain0=fullrange/32767 to map the input range +/-32767 to a grid voltage range of +/-fullrangeall gains for i>0 

  // DONE: implement basic the process without feedback but with customised filters & gains for each stage.
  // TODO: implement a feedback loop (for tubes only) and check if it adds "dynamics", else try to "fake it" by implementing additionnal sag and tube compression models
  
  /* Source of inspiration:
  *  sample of Guitarix tubestage model in Faust language (guitarix.lib) => cool!, should try to make a bridge with FAUST code one day..
  *  we note that the basic tube stage model has a 40x output divider with 250V Vplus 
  *  fck represents the low pass frequency (Hz) cut for the change in input bias VkC 
  *  
  *  Guitarix tube stage model                        
  *  -------------------------
  *  
  *                          VkC-Vplus
  *                             |
  *  input => (-)->(+)-->Ftube()-->(+)->+-->(/) => output 
  *            |    |                   |    |
  *            Vk0  +<-lowpass(fck)-(*)-+  divider
  *                                 |
  *                               Rp/Rk
  *  
  *  Faust Code:
  *  
    tubestageF(tb,vplus,divider,fck,Rk,Vk0) = anti_aliase : tube : hpf with {
    lpfk = lowpass(1,fck);
    anti_aliase = lowpass(3,SR/2.1);
    Rp = 100.0e3;
    VkC = Vk0 * (Rp/Rk);
    tube = (+ : -(Vk0) : Ftube(tb) : +(VkC-vplus)) ~ (*(Rk/Rp) : lpfk) : /(divider);
    hpf = highpass(1,31.0);
  };
  tubestage(tb,fck,Rk,Vk0)  = tubestageF(tb,250.0,40.0,fck,Rk,Vk0);
  *
  */

  // interpolator
  arm_linear_interp_instance_f32 S[MSDMAXSTAGES] = {{MSDTABLESIZE, -MSDTABLEBND, MSDTABLESTEP, &this->wavetables[0][0]},
                                                    {MSDTABLESIZE, -MSDTABLEBND, MSDTABLESTEP, &this->wavetables[1][0]},
                                                    {MSDTABLESIZE, -MSDTABLEBND, MSDTABLESTEP, &this->wavetables[2][0]},
                                                    {MSDTABLESIZE, -MSDTABLEBND, MSDTABLESTEP, &this->wavetables[3][0]}};
  // processing variables
  float x, y0;
  // static states
  static float y1[MSDMAXSTAGES], y1p[MSDMAXSTAGES], y[MSDMAXSTAGES], yp[MSDMAXSTAGES];
  static float y2[MSDMAXSTAGES], y2p[MSDMAXSTAGES];
  float postg=gains[MSDMAXSTAGES];

  float maxx=0; 
  float max0=0;

  #ifndef MULTIPASS
  // processing loop
  //Serial.println("MSD: standard");
  for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
     x=block->data[i];
     if( (i==0) && (x>max0)) max0=x;
     for(uint8_t stage=0; stage<MSDMAXSTAGES; stage++) {
       
       if(enabled[stage]) { 
         float curbias=this->topology[stage]?0.0:this->bias[stage];  
         // prescaling & biasing
         x=gains[stage]*x-curbias;
         
         // nonlinear transformation
         #ifdef HERMITE_INTERP
         y0=interpolateHermite(MSDTABLESIZE, -MSDTABLEBND, MSDTABLEBND, 1.0/MSDTABLESTEP, &this->wavetables[stage][0], x) - this->oppoint[stage];
         #endif 
         #ifdef CUBIC_INTERP
          y0=interpolateCubic(MSDTABLESIZE, -MSDTABLEBND, MSDTABLEBND, 1.0/MSDTABLESTEP, &this->wavetables[stage][0], x) - this->oppoint[stage];
         #endif
         #ifdef QUAD_INTERP
          y0=interpolateQuad(MSDTABLESIZE, -MSDTABLEBND, MSDTABLEBND, 1.0/MSDTABLESTEP, &this->wavetables[stage][0], x) - this->oppoint[stage];
         #endif
         #ifdef ADAPTIVE_INTERP
          if(stage<2) {
           y0=interpolateHermite(MSDTABLESIZE, -MSDTABLEBND, MSDTABLEBND, 1.0/MSDTABLESTEP, &this->wavetables[stage][0], x) - this->oppoint[stage];
          } else {
           y0=arm_linear_interp_f32(&S[stage], x) - this->oppoint[stage];
          }
         #endif
         #ifdef LINEAR_INTERP
         y0=arm_linear_interp_f32(&S[stage], x)-this->oppoint[stage];
         #endif

         // filtering
         y1[stage]=y0;
         y2[stage]=this->hpfr[stage]*(y1[stage]-y1p[stage]+y2p[stage]);
         // Lowpass filter: todo, reduce number of static vars and copies
         y[stage]=-this->lpfa[stage]*yp[stage]+this->lpfb[stage]*(y2[stage]+y2p[stage]);
         // store state
         y1p[stage]=y1[stage]; 
         y2p[stage]=y2[stage];
         yp[stage]=y[stage];
         // store input for next stage
         x=y[stage];
       }
     }
     // post gain
     x*=postg;
    //if(x>32767.0) x=32767.0;
    //if(x<-32767.0) x=-32767.0;
    if(x>maxx) maxx=x;
     // save and loop
     block->data[i]=x; //floor(min(32767.0,max(x,-32767.0))); // sign(x)*min(32767.0,fabs(x)) ); => !! THIS IS VERY TIME CONSUMING ... don't see why. 
  }
  //Serial.print("Input: "); Serial.print(max0); Serial.print(", Output: "), Serial.println(maxx);
  #else
  // processing loop, multipass version, much faster. TODO: use ARM M4 DSP to speed up? (for filters for ex). 
  
  float tmp[BLOCKSIZE+1];
  
  #if OVERSAMPLING<2
    for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) tmp[i]=block->data[i];
  #else
     uint16_t ii=0;
     { 
       uint16_t i=0;
      //float tmp2[BLOCKSIZE];
//       static float prev1=0,prev2=0;prev3=0;
//       tmp[0]=prev1;
//       tmp[1]=hermite4(0.5, prev3, prev2, prev1, block->data[0]);
//       tmp[2]=block->data[0];
//       tmp[3]=hermite4(0.5, prev2, prev1, block->data[0],block->data[1]);
//       tmp[4]=block->data[1];
//       tmp[5]=hermite4(0.5, prev1, block->data[0],block->data[1],  block->data[2]);
//       //tmp[6]=block->data[2];
//       //tmp[7]=hermite4(0.5,block->data[0],block->data[1],block->data[2],block->data[3]);
//       //tmp[8]=block->data[3];
//       ii=6;
//       for (i=2; i < AUDIO_BLOCK_SAMPLES-1; i++) {tmp[ii]=block->data[i]; tmp[ii+1]=hermite4(0.5, block->data[i-2], block->data[i-1], block->data[i], block->data[i+1]); ii+=2;};
//       tmp[]=block->data[i]; tmp[ii+1]=0.5*(block->data[i]+block->data[i+1]); ii+=2; i++;
//       tmp[ii]=block->data[i]; tmp[ii+1]=0.5*(block->data[i]+block->data[i+1]); ii+=2; i++;
//       prev1=block->data[AUDIO_BLOCK_SAMPLES-1];
//       prev2=block->data[AUDIO_BLOCK_SAMPLES-2];
//       prev3=block->data[AUDIO_BLOCK_SAMPLES-3];
//       for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) {tmp[ii]=block->data[i]; tmp[ii+1]=0.5*(block->data[i]+block->data[i+1]); ii+=2;};  // fast linear 2x interpolator
         static float prev=0;
         tmp[0]=prev;
         tmp[1]=0.5*(prev+block->data[0]);
         ii=2;
         for (uint32_t i=0; i < (AUDIO_BLOCK_SAMPLES-1); i++) {tmp[ii]=block->data[i]; tmp[ii+1]=0.5*(block->data[i]+block->data[i+1]); ii+=2;};
         prev=block->data[AUDIO_BLOCK_SAMPLES-1];
       //upsample(OVERSAMPLING,AUDIO_BLOCK_SAMPLES,block->data,tmp);
       //upsampleIIR(OVERSAMPLING,AUDIO_BLOCK_SAMPLES,&block->data[0], &tmp[0]);
       //arm_biquad_cascade_df1_f32  (&antialias1,tmp2,tmp,BLOCKSIZE );
     }
  #endif
  for(uint8_t stage=0; stage<MSDMAXSTAGES; stage++) {
    if(enabled[stage]) {
     
     float curbias=this->topology[stage]?0.0:this->bias[stage]; // in pushpull the bias is applied on the waveshaping curve directly, not on the computed signals
     
     // prescaling & biasing
     //x=gains[stage]*x-curbias;
     #ifndef VECTORIZED
     arm_scale_f32(&tmp[0], gains[stage], &tmp[0], BLOCKSIZE);
     arm_offset_f32(&tmp[0], -curbias, &tmp[0], BLOCKSIZE);
     #endif
     
     // interpolation, hermite interpolation using arm dsp vector operations
     
    /*  Standard algorithm:
       
      float interpolateHermite(uint16_t tablesize, float minx, float maxx, float invstepx, const float table[], float x) {
       if(x>=maxx) return table[tablesize-1];
       if(x<=minx) return table[0];
       float dx=x-minx;
       float divis=dx*invstepx;
       float rem;
       float delta=modff(divis,&rem);
       uint16_t idx=(int)rem;
       if((idx>3)&&(idx<tablesize-3)) {
         return hermite4(delta,table[idx-1],table[idx],table[idx+1],table[idx+2]);
       } else {
         return table[idx+1]*(delta)+table[idx]*(1-delta);
       }
      };
      inline float hermite4(float frac_pos, float xm1, float x0, float x1, float x2)
      {
       const float    c     = (x1 - xm1) * 0.5f;
       const float    v     = x0 - x1;
       const float    w     = c + v;
       const float    a     = w + v + (x2 - x0) * 0.5f;
       const float    b_neg = w + a;
    
       return ((((a * frac_pos) - b_neg) * frac_pos + c) * frac_pos + x0);
      
      }
      */ 
    #ifdef VECTORIZED
    //Serial.println("MSD: Vectorized");
    // Vectorized version:
    // compute integer and fractional parts
    const float invstepx=1.0/MSDTABLESTEP;
    const float gainmod=gains[stage]*invstepx;
    const float offst=(MSDTABLEBND-curbias)*invstepx;
    // merged scaling & offset operations for gain, bias & table index computation
    // tmp[i]=gain*input[i]-curbias;
    // tmp[i]=dx=x[i]-minx; 
    // tmp[i]=divis=invstepx*dx[i];
    // is equivalent to : 
    arm_scale_f32(tmp, gainmod, tmp, BLOCKSIZE);
    arm_offset_f32(tmp, offst , tmp, BLOCKSIZE);
    
    for (uint16_t i=0; i < BLOCKSIZE; i++) {
     #ifdef HERMITE_INTERP
       if(tmp[i]>=MSDTABLESIZE) { tmp[i]=this->wavetables[stage][MSDTABLESIZE-1]; continue;}
       if(tmp[i]<=0.) { tmp[i]=this->wavetables[stage][0]; continue; }
       float rem;
       float delta=modff(tmp[i],&rem);
       uint16_t idx=(int)rem;
       if((idx>3)&&(idx<MSDTABLESIZE-3)) {
          tmp[i]=hermite4(delta,this->wavetables[stage][idx-1],this->wavetables[stage][idx],this->wavetables[stage][idx+1],this->wavetables[stage][idx+2]);
       } else {
          tmp[i]=this->wavetables[stage][idx+1]*(delta)+this->wavetables[stage][idx]*(1-delta);
       }
     #endif
     #ifdef CUBIC_INTERP
       if(tmp[i]>=MSDTABLESIZE) { tmp[i]=this->wavetables[stage][MSDTABLESIZE-1]; continue;}
       if(tmp[i]<=0.) { tmp[i]=this->wavetables[stage][0]; continue; }
       float rem;
       float delta=modff(tmp[i],&rem);
       uint16_t idx=(int)rem;
       if((idx>3)&&(idx<MSDTABLESIZE-3)) {
          tmp[i]=cubicinterp(delta,this->wavetables[stage][idx-1],this->wavetables[stage][idx],this->wavetables[stage][idx+1],this->wavetables[stage][idx+2]);
       } else {
          tmp[i]=this->wavetables[stage][idx+1]*(delta)+this->wavetables[stage][idx]*(1-delta);
       }
     #endif
     #ifdef QUAD_INTERP
       if(tmp[i]>=MSDTABLESIZE) { tmp[i]=this->wavetables[stage][MSDTABLESIZE-1]; continue;}
       if(tmp[i]<=0.) { tmp[i]=this->wavetables[stage][0]; continue; }
       float rem;
       float delta=modff(tmp[i],&rem);
       uint16_t idx=(int)rem;
       if((idx>3)&&(idx<MSDTABLESIZE-3)) {
         if(delta>0.5) {idx++; delta-=1.0;};
         tmp[i]=quadinterp(delta,this->wavetables[stage][idx-1],this->wavetables[stage][idx],this->wavetables[stage][idx+1]);
       } else {
         tmp[i]=this->wavetables[stage][idx+1]*(delta)+this->wavetables[stage][idx]*(1-delta);
       }
     #endif
     #ifdef LINEAR_INTERP
       if(tmp[i]>=MSDTABLESIZE) { tmp[i]=this->wavetables[stage][MSDTABLESIZE-1]; continue;}
       if(tmp[i]<=0.) { tmp[i]=this->wavetables[stage][0]; continue; }
       float rem;
       const float delta=modff(tmp[i],&rem);
       uint16_t idx=(int)rem;
       tmp[i]=this->wavetables[stage][idx+1]*(delta)+this->wavetables[stage][idx]*(1-delta);
      #endif   // linear
    }
    
    #else
    //Serial.println("MSD: Multipass std");
     for (uint16_t i=0; i < BLOCKSIZE; i++) {
       x=tmp[i];    
       // nonlinear transformation
       #ifdef HERMITE_INTERP
         y0=interpolateHermite(MSDTABLESIZE, -MSDTABLEBND, MSDTABLEBND,  1.0/MSDTABLESTEP, &this->wavetables[stage][0], x) - this->oppoint[stage];
       #endif
       #ifdef CUBIC_INTERP
         y0=interpolateCubic(MSDTABLESIZE, -MSDTABLEBND, MSDTABLEBND, 1.0/MSDTABLESTEP, &this->wavetables[stage][0], x) - this->oppoint[stage];
       #endif
       #ifdef QUAD_INTERP
         y0=interpolateQuad(MSDTABLESIZE, -MSDTABLEBND, MSDTABLEBND, 1.0/MSDTABLESTEP, &this->wavetables[stage][0], x) - this->oppoint[stage];
       #endif
       #ifdef ADAPTIVE_INTERP
         if(stage<2) {
           y0=interpolateHermite(MSDTABLESIZE, -MSDTABLEBND, MSDTABLEBND, 1.0/MSDTABLESTEP, &this->wavetables[stage][0], x) - this->oppoint[stage];
         } else {
           y0=arm_linear_interp_f32(&S[stage], x) - this->oppoint[stage];
         }
       #endif
       #ifdef LINEAR_INTERP
         //y0=interpolate(MSDTABLESIZE, -MSDTABLEBND, MSDTABLEBND, 1.0/MSDTABLESTEP, &this->wavetables[stage][0], x) - this->oppoint[stage];
         y0=arm_linear_interp_f32(&S[stage], x) - this->oppoint[stage];
       #endif
       tmp[i]=y0;
     }
   #endif   // end ifdef VECTORIZED
     
     // filtering
     for (uint16_t i=0; i < BLOCKSIZE; i++) {
       y1[stage]=tmp[i];
       y2[stage]=this->hpfr[stage]*(y1[stage]-y1p[stage]+y2p[stage]);
       // Lowpass filter: todo, reduce number of static vars and copies
       y[stage]=-this->lpfa[stage]*yp[stage]+this->lpfb[stage]*(y2[stage]+y2p[stage]);
       // store state
       y1p[stage]=y1[stage]; 
       y2p[stage]=y2[stage];
       yp[stage]=y[stage];
       // store input for next stage
       tmp[i]=y[stage];
     }
    }
  }
  // save
  #if OVERSAMPLING <2
    float * ptr=&tmp[0];
  #else
    float * ptr=&block->data[0];
     ii=0;
     //arm_biquad_cascade_df1_f32  (&antialias2,tmp,tmp,BLOCKSIZE );      
     //for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) {block->data[i]=tmp[ii]; ii+=2;};  // fast 2x hold decimator
     for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) {block->data[i]=0.5*(tmp[ii]+tmp[ii+1]); ii+=2;}; // moving average 2x decimator
     //downsample(OVERSAMPLING,AUDIO_BLOCK_SAMPLES,tmp,block->data);
  #endif
  
  //for (uint16_t i=0; i < AUDIO_BLOCK_SAMPLES; i++) block->data[i]=floor(gains[MSDMAXSTAGES]*tmp[i]); //floor(min(32767.0,max(x,-32767.0))); // sign(x)*min(32767.0,fabs(x)) ); => !! THIS IS VERY TIME CONSUMING ... don't see why. 
  float postgn=gains[MSDMAXSTAGES]/8;  // output gain: adjusted to give a factor of 2 safety...
  arm_scale_f32(ptr,postgn,block->data,AUDIO_BLOCK_SAMPLES);
  #endif     // end ifdef MULTIPASS

   #ifdef PRINT_TIMING 
    Serial.println(t1);
   #endif
  AudioStream_F32::transmit(block);
  AudioStream_F32::release(block);
}

