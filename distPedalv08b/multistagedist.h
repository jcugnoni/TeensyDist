/* Multistage waveshaping distortion addition to Audio Library for Teensy 3.X
 * Copyright (c) 2016, Joel Cugnoni (joel.cugnoni@gmail.com) */


#ifndef multistagedist_h_
#define multistagedist_h_

// import tube response
#include "tubetable.h"

//#define DEBUGDIST
//#define PRINT_TIMING

#define OVERSAMPLING 2     // either 1 and 2

#define BLOCKSIZE (OVERSAMPLING*AUDIO_BLOCK_SAMPLES)

#define MSDMAXSTAGES 4

#define MSDTABLESIZE (TUBETABLE_LEN*4) 

#define MSDTABLESTEP (65535.0/(MSDTABLESIZE-1))

#define MSDTABLEBND 32767.0

// algorithm structure: 
#define MULTIPASS   // use multiple passes algo, favoring a more vectorial processing form
#define VECTORIZED  // use vectorized  interpolation.
// table interpolation: choose one...  fastest is non MULTIPASS, non VECTORIZED, LINEAR. Best quality/speed is  MULTIPASS,  VECTORIZED, HERMITE or CUBIC.
//#define LINEAR_INTERP   // fast ARM DSP linear interpolation, quick, quite good.
#define HERMITE_INTERP   // activate Hermite (cubic) interpolation: computationnally intensive but smooth
//#define CUBIC_INTERP   // another cubic interpolation ...
//#define QUAD_INTERP // quadratic interpolation, to test.
//
//#define ADAPTIVE_INTERP // tries to adapt the level of interpolation by using smooth cubic interpolation on the first two stages to limit approximation artefacts

#define CLIPSCALE 0.25   // scaling factor for the tube tables = 1/4 divider for tube output

#include "Arduino.h"
#include <AudioStream_F32.h>
#include "arm_math.h"

template <typename type>
type sign(type value) {
 return type((value>0)-(value<0));
}

class AudioEffectMultiStageDist : public AudioStream_F32 {
public:
  AudioEffectMultiStageDist(void) : AudioStream_F32(1, inputQueueArray_f32) {
      // set all stages tables to a simple linear gain, just in case someone forgets to initialize one stage before use
      for(uint8_t i=0; i<MSDMAXSTAGES; i++) setLinear(1.0,i); 
      arm_biquad_cascade_df1_init_f32  (&antialias1, 2, antialiasCoefs1,antialiasState1);
      arm_biquad_cascade_df1_init_f32  (&antialias2, 2, antialiasCoefs2,antialiasState2); 
      };
       
  void setTanh(float b[5], uint8_t stage);

  void setPow(float b[5], uint8_t stage);

  void setCubic(float b[4], uint8_t stage);

  void setLinear(float gain1, uint8_t stage);
  
  void setSoftClipping(float gain1, uint8_t stage);
  
  void setExp(float b[4], uint8_t stage);

  void setAsym(float b[4], uint8_t stage);

  void setSoftPower(float b[4], uint8_t stage);
  
  void setChebyshev(float b[5], uint8_t stage);

  void setPreGain(float gain, uint8_t stage);

  void setPostGain(float gain, uint8_t stage);

  void setBias(float bias,uint8_t stage);
 
  void setTube(uint8_t type, uint8_t stage);

  void setTubePushPull(uint8_t type, uint8_t stage, float bias);

  void enableStage(uint8_t stage);

  void bypassStage(uint8_t stage);
 
  void updateOpPoint(uint8_t stage);

  void updateGains();

  void setLowpass(float freq,uint8_t stage);

  void setHighpass(float R,uint8_t stage);

  uint8_t activeStages();

  void printStatus() {
    Serial.print("MSDTABLESIZE= ");
    Serial.print(MSDTABLESIZE);
    Serial.print(", MSDTABLEBND= ");
    Serial.print(-MSDTABLEBND);
    Serial.print(", MSDTABLESTEP= ");
    Serial.println(MSDTABLESTEP);
    for(uint8_t stage=0;stage<MSDMAXSTAGES;stage++) {
      Serial.print("Stage= ");
      Serial.print(stage);
      Serial.print(", Table start= ");
      Serial.print(this->wavetables[stage][0]);
      Serial.print(", Table end= ");
      Serial.print(this->wavetables[stage][MSDTABLESIZE-1]);
      Serial.print(", Gain= ");
      Serial.print(this->gains[stage]);
      Serial.print(", Bias= ");
      Serial.print(this->bias[stage]);
      Serial.print(", Op Point= ");
      Serial.println(this->oppoint[stage]);
    }
  };
  
  virtual void update(void);
  
private:
  const float gainAdapt=32767.; // post gain to adapt to output block: distortion block expects signals between +/-32767
  bool isenabled=false; // global bypass
  bool enabled[MSDMAXSTAGES]={false,false,false,false};  // stage state on or bypass
  float coefs[6]={0,0,0,0,0,0};  // temp vector for distortion params
  bool topology[MSDMAXSTAGES]={false,false,false,false}; // default singled ended
  // waveshaping tables of each stage
  float wavetables[MSDMAXSTAGES][MSDTABLESIZE];
  // pregain of each stage, last one is output gain
  float pregains[MSDMAXSTAGES]={1.,1.,1.,1.};
  float postgains[MSDMAXSTAGES+1]={1.,1.,1.,1.,1.};
  float gains[MSDMAXSTAGES+1]={1., 1., 1., 1., 1.}; 
  // bias of each stage
  float bias[MSDMAXSTAGES]={0.,0.,0.,0.};
  // DC operation point corresponding to bias
  float oppoint[MSDMAXSTAGES]={0,0,0,0};
  // stage level low pass filters, frequency, lpfa=a1 and lpfb=b0=b1
  float lpf[MSDMAXSTAGES]={9000,9000,9000,9000};
  float lpfa[MSDMAXSTAGES]={-0.14527,-0.14527,-0.14527,-0.14527};
  float lpfb[MSDMAXSTAGES]={0.42737,0.42737,0.42737,0.42737};
  // high pass
  float hpf[MSDMAXSTAGES]={30,30,30,30};
  float hpfr[MSDMAXSTAGES]={0.995,0.995,0.995,0.995};
  float antialiasState1[8]={0.};
  float antialiasState2[8]={0.};
  float antialiasCoefs1[10]={0.08315986993,0.1663197399,0.08315986993,1.03517121,-0.3678106895,0.08315986993,0.1663197399,0.08315986993,1.03517121,-0.3678106895};
  float antialiasCoefs2[10]={0.08315986993,0.1663197399,0.08315986993,1.03517121,-0.3678106895,0.08315986993,0.1663197399,0.08315986993,1.03517121,-0.3678106895};
  arm_biquad_casd_df1_inst_f32 antialias1;
  arm_biquad_casd_df1_inst_f32 antialias2;
  audio_block_f32_t *inputQueueArray_f32[1];
  

  float interpolate(uint16_t tablesize, float minx, float maxx, float invstepx, float table[], float x) {
   if(x>=maxx) return table[tablesize-1];
   if(x<=minx) return table[0];
   float dx=x-minx;
   float divis=dx*invstepx;
   float rem;
   float delta=modff(divis,&rem);
   uint16_t idx=(int)rem;
   return table[idx+1]*(delta)+table[idx]*(1-delta);
  };
  
  // src http://musicdsp.org/archive.php?classid=5#157, laurent de soras
  inline float hermite4(float frac_pos, float xm1, float x0, float x1, float x2)
  {
   const float    c     = (x1 - xm1) * 0.5f;
   const float    v     = x0 - x1;
   const float    w     = c + v;
   const float    a     = w + v + (x2 - x0) * 0.5f;
   const float    b_neg = w + a;

   return ((((a * frac_pos) - b_neg) * frac_pos + c) * frac_pos + x0);
  }
  
  float interpolateHermite(uint16_t tablesize, float minx, float maxx, float invstepx, const float table[], float x) {
//   float maxx=minx+stepx*(tablesize-1);
//   if(x>=maxx) return table[tablesize-1];
//   if(x<=minx) return table[0];
//   float dx=x-minx;
//   float divis=dx/stepx;
//   uint16_t idx=floor(divis);
//   float delta=divis-idx;
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
  
  float interpolateCubic(uint16_t tablesize, float minx, float maxx, float invstepx, const float table[], float x) {
//   float maxx=minx+stepx*(tablesize-1);
//   if(x>=maxx) return table[tablesize-1];
//   if(x<=minx) return table[0];
//   float dx=x-minx;
//   float divis=dx/stepx;
//   uint16_t idx=floor(divis);
//   float delta=divis-idx;
   if(x>=maxx) return table[tablesize-1];
   if(x<=minx) return table[0];
   float dx=x-minx;
   float divis=dx*invstepx;
   float rem;
   float delta=modff(divis,&rem);
   uint16_t idx=(int)rem;
   if((idx>3)&&(idx<tablesize-3)) {
     return cubicinterp(delta,table[idx-1],table[idx],table[idx+1],table[idx+2]);
   } else {
     return table[idx+1]*delta+table[idx]*(1-delta);
   }
  };

  float interpolateQuad(uint16_t tablesize, float minx, float maxx, float invstepx, const float table[], float x) {
   if(x>=maxx) return table[tablesize-1];
   if(x<=minx) return table[0];
   float dx=x-minx;
   float divis=dx*invstepx;
   float rem;
   float delta=modff(divis,&rem);
   uint16_t idx=(int)rem;
   if((idx>3)&&(idx<tablesize-3)) {
     if(delta>0.5) {idx++; delta-=1.0;};
     return quadinterp(delta,table[idx-1],table[idx],table[idx+1]);
   } else {
     return table[idx+1]*delta+table[idx]*(1-delta);
   }
  };

 // interpolate in range finpos= -0.5 to 0.5 interval around t0; smooth derivative but does not pass through x0
  inline float quadinterp(float finpos, float xm1, float x0, float x1) {
     const float a=(-2.0*x0+xm1+x1)*0.5;
     const float b=-(xm1-x1)*0.5;
     const float c=(6.0*x0+xm1+x1)*0.125;
     return (a*finpos+b)*finpos+x0;
  }

  inline float cubicinterp(float finpos, float xm1, float x0, float x1, float x2) {
    const float a = (3.0 * (x0-x1) - xm1 + x2)*0.5;
    const float b = 2.0*x1 + xm1 - (5.0*x0 + x2)*0.5;
    const float c = (x1 - xm1)*0.5;
    return (((a * finpos) + b) * finpos + c) * finpos + x0;
  }
    //  direct upsampling by linear interpolation, checked!
    void upsample(uint8_t oversampling,uint16_t nsamplein,float bufin[], float bufout[]) {
     float ratio=1.0f/oversampling;
     uint16_t k=0; // k = output vector index
     for (uint16_t i=0; i < nsamplein-1; i++) {
        // i=input vector index
        float ym=bufin[i];
        float yp=bufin[i+1];
        // set 1st pt: copy
        bufout[k++]=ym;  
        float delta=(yp-ym)*ratio; // increment btw pts
        for (uint16_t j=1;j<oversampling;j++) { ym+=delta; bufout[k++]=ym; }
      }
      // finish by adding last pt
      bufout[nsamplein*oversampling-1]=bufin[nsamplein-1];
   };
   
   void upsampleIIR(uint8_t oversampling,uint16_t nsamplein,float bufin[], float bufout[]) {
     float ratio=1.0f/oversampling;
     uint16_t k=0; // k = output vector index
     for (uint16_t i=0; i < nsamplein-1; i++) {
        // i=input vector index
        float ym=bufin[i];
        float yp=bufin[i+1];
        // set 1st pt: copy
        bufout[k++]=ym;  
        float delta=(yp-ym)*ratio; // increment btw pts
        for (uint16_t j=1;j<oversampling;j++) { ym+=delta; bufout[k++]=ym; }
      }
      // finish by adding last pt
      bufout[nsamplein*oversampling-1]=bufin[nsamplein-1];
      // post processing by IIR lowpass filter 
      const float a0 = 0.08315961784060605;
      const float a1 = 0.1663192356812121;
      const float a2 = 0.08315961784060605;
      const float b1 = -1.0351680717394027;
      const float b2 = 0.3678065431018269;
      float s0=0;
      static float s1=0;
      static float s2=0;
      for (uint32_t i=0; i<nsamplein*oversampling; i++) {
        s0 = bufout[i] - b1 * s1 - b2 * s2;
        bufout[i] = a0 * s0 + a1 * s1 + a2 * s2;
        s2=s1;
        s1=s0;
      }
   };
   
   // sample and hold upsampler... for debug purpose only
   void upsample0(uint8_t oversampling,uint16_t nsamplein,float bufin[], float bufout[]) {
     for (uint16_t i=0; i < nsamplein; i++) {
        uint16_t k=oversampling*i;
        float ym=bufin[i];
        //int32_t yp=bufin[i+1];
        //bufout[k]=ym;
        //delta=(yp-ym)<<8;
        for (uint8_t j=0;j<oversampling;j++) bufout[k+j]=ym;
      }
   };


   //  direct downsampling by moving average,  for debug purpose
   void downsample(uint8_t downsampling,uint16_t nsampleout,float bufin[], float bufout[]) {
     float ratio=1/downsampling;
     uint16_t k=0;
     for (uint16_t i=0; i < nsampleout; i++) {
        float sum=0;
        for (uint8_t j=0;j<downsampling;j++) sum+=bufin[k+j];
        bufout[i]=sum*ratio;
        k+=downsampling;
      }
   };

   //  direct downsampling by cascading IIR filtering 
   void downsampleIIR(uint8_t downsampling,uint16_t nsampleout,float bufin[], float bufout[]) {
     static float s1[6]={0,0,0,0,0,0};
     static float s2[6]={0,0,0,0,0,0};
     const float a0 = 0.07047279910232168;
     const float a1 = 0.14094559820464336;
     const float a2 = 0.07047279910232168;
     const float b1 = -1.1431976057301103;
     const float b2 = 0.4250888021393969;
     const uint8_t nloop=2;
     
     uint32_t nsample=nsampleout*downsampling;
     // IIR filtering: n pass lowpass biquad
     for (uint16_t j=0; j<nloop; j++) {  // j = loop index
       for (uint32_t i=0; i<nsample; i++) {  // i = sample index
        //printf("%10f", bufin[i]);
        //s0[j] = bufin[i] - bb[0] * s1[j] - bb[1] * s2[j];
        //bufin[i] = aa[0] * s0[j] + aa[1] * s1[j] + aa[2] * s2[j];
        //s2[j]=s1[j];
        //s1[j]=s0[j];
        float in=bufin[i];
        float out=in * a0 + s1[j];
        bufin[i]=out;
        s1[j] = in * a1 + s2[j] - b1 * out;
        s2[j] = in * a2 - b2 * out;
       }
     }
     // decimation
     uint16_t k=0;
     for (uint32_t i=0; i < nsampleout; i++) {
        bufout[i]=bufin[k];
        k+=downsampling;
      }
   };
};


#endif

