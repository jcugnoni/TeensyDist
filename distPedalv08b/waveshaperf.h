/* A waveshaper distortion addition to Audio Library for Teensy 3.X
 * Copyright (c) 2016, Joel Cugnoni (joel.cugnoni@gmail.com)
 * Inspired by the Bitcrusher effect in Audio Library and available literature (see Pakarinen 2009 as a first ref).
 */

#ifndef waveshaperf_h_
#define waveshaperf_h_


//#define DEBUGDIST
//#define PRINT_TIMING

//#define DISTADDRBITS 15
#define DISTADDRBITS 14
//#define DISTADDRBITS 13
//#define DISTADDRBITS 12
//#define DISTADDRBITS 11
//#define DISTADDRBITS 10
//#define DISTADDRBITS 9
//#define DISTTABLESIZE  32768   //pow(2,DISTADDRBITS))
#define DISTTABLESIZE  16384   //pow(2,DISTADDRBITS))
//#define DISTTABLESIZE  8192   //pow(2,DISTADDRBITS))
//#define DISTTABLESIZE  4096   //pow(2,DISTADDRBITS)) 
//#define DISTTABLESIZE  2048   //pow(2,DISTADDRBITS))
//#define DISTTABLESIZE  1024   //pow(2,DISTADDRBITS))
//#define DISTTABLESIZE  512   //pow(2,DISTADDRBITS))

#define DISTTABLESTEP (65535/(DISTTABLESIZE-1))
#define DISTSHIFTBIT (16-DISTADDRBITS)    // unused
//#define DISTBITMASK  256      // (pow(2,DISTSHIFTBIT-1)) unused
//#define DISTBITMASK  128      // (pow(2,DISTSHIFTBIT-1))
//#define DISTBITMASK  64      // (pow(2,DISTSHIFTBIT-1))
//#define DISTBITMASK  32      // (pow(2,DISTSHIFTBIT-1))
//#define DISTBITMASK  16      // (pow(2,DISTSHIFTBIT-1))

#define DISTTABLEBND 32767

#define DISTOVERSAMPLING 1

#if DISTOVERSAMPLING<2
#define DISTFILTERSNUMTAP1 1
#define FILTER1 {1}
#define FILTER2 {1}
#define DISTFILTERSNUMTAP2 1
#endif

#if DISTOVERSAMPLING==2
#define DISTFILTERSNUMTAP1 20
//#define FILTER1 {-0.00269,  -0.00146,   0.00900,   0.01724,  -0.00095,  -0.04282,  -0.05270,   0.03320,   0.20114,   0.34097,   0.34097,   0.20114,   0.03320,  -0.05270,  -0.04282,  -0.00095,   0.01724,   0.00900,  -0.00146,  -0.00269}
//#define FILTER1 {-88,-48,295,565,-31,-1403,-1727,1088,6591,11173,11173,6591,1088,-1727,-1403,-31,565,295,-48,-88}
#define FILTER1 {-0.002686, -0.001465,  0.009003,  0.017243, -0.000946, -0.042817, -0.052705,  0.033204,  0.201147,  0.340983,  0.340983,  0.201147,  0.033204, -0.052705, -0.042817, -0.000946,  0.017243,  0.009003, -0.001465, -0.002686}
//#define FILTER1 {125,-83,-228,36,350,81,-467,-290, 544,611,-532,-1068,356,1720,141,-2799,-1538,5905,13513,13513,5905,-1538,-2799,141,1720,356,-1068,-532,611,544,-290,-467, 81,350,36,-228,-83,125}
#define DISTFILTERSNUMTAP2 26
//#define FILTER2 {16,-33,28,39,-202,463,-764,971,-879,217,1413,-5075,20194,20194,-5075,1413,217,-879,971,-764,463,-202,39,28,-33,16}
#define FILTER2 {  0.000488, -0.001007,  0.000855,  0.001190, -0.006165,  0.014130, -0.023316,  0.029633, -0.026826,  0.006623,  0.043123, -0.154881,  0.616291,  0.616291, -0.154881,  0.043123,  0.006623, -0.026826,  0.029633, -0.023316,  0.014130, -0.006165,  0.001190,  0.000855, -0.001007,  0.000488}
#endif

#if DISTOVERSAMPLING>=4
//#define DISTFILTERSNUMTAP1 28 
//#define FILTER1 {205,205,88,-159,-489,-784,-888,-645,38,1148,2544,3978,5159,5825,5825,5159,3978,2544,1148,38,-645,-888,-784,-489,-159,88,205,205}
//#define DISTFILTERSNUMTAP1 44
//#define FILTER1 {-10,-17,-18,-4,32,86,144,177,152,43,-153,-405,-641,-761,-657,-245,500,1529,2712,3864,4781,5289,5289,4781,3864,2712,1529,500,-245,-657,-761,-641,-405,-153,43,152,177,144,86,32,-4,-18,-17,-10}
//#define DISTFILTERSNUMTAP2 28
//#define FILTER2 {-127,-260,78,490,147,-706,-627,770,1437,-446,-2777,-910,6179,13000,13000,6179,-910,-2777,-446,1437,770,-627,-706,147,490,78,-260,-127}
//#define DISTFILTERSNUMTAP2 24
//#define FILTER2 {-159,-211,-226,-72,357,1139,2292,3746,5344,6861,8050,8704,8704,8050,6861,5344,3746,2292,1139,357,-72,-226,-211,-159}

#define DISTFILTERSNUMTAP1 32  //quite ok, recommended!
//#define  FILTER1 {-674,-682,-910,-1076,-1120,-986,-627,-18,837,1904,3113,4370,5567,6589,7336,7729,7729,7336,6589,5567,4370,3113,1904,837,-18,-627,-986,-1120,-1076,-910,-682,-674}
#define FILTER1 {-0.02057,  -0.02081,  -0.02777,  -0.03284,  -0.03418,  -0.03009,  -0.01913,  -0.00055,   0.02554,   0.05811,   0.09500,   0.13336,   0.16989,   0.20108,   0.22388,   0.23587,   0.23587,   0.22388,   0.20108,   0.16989,   0.13336,   0.09500,   0.05811,   0.02554,  -0.00055,  -0.01913,  -0.03009,  -0.03418,  -0.03284,  -0.02777,  -0.02081,  -0.02057}
#define DISTFILTERSNUMTAP2 32   // quite ok, recommended
//#define FILTER2 {2,-56,-149,-264,-373,-436,-406,-241,91,593,1242,1984,2740,3418,3930,4205,4205,3930,3418,2740,1984,1242,593,91,-241,-406,-436,-373,-264,-149,-56,2}
#define FILTER2 { 0.00006,  -0.00171,  -0.00455,  -0.00806,  -0.01138,  -0.01331,  -0.01239,  -0.00735,   0.00278,   0.01810,   0.03790,   0.06055,   0.08362,   0.10431,   0.11993,   0.12833,   0.12833,   0.11993,   0.10431,   0.08362,   0.06055,   0.03790,   0.01810,   0.00278,  -0.00735,  -0.01239,  -0.01331,  -0.01138,  -0.00806,  -0.00455,  -0.00171,   0.00006}

//#define DISTFILTERSNUMTAP2 52
//#define FILTER2 {2,15,6,-31,-32,41,83,-23,-151,-47,212,187,-218,-395,108,632,179,-821,-701,835,1520,-463,-2839,-920,6203,13005,13005,6203,-920,-2839,-463,1520,835,-701,-821,179,632,108,-395,-218,187,212,-47,-151,-23,83,41,-32,-31,6,15,2}
//#define DISTFILTERSNUMTAP2 36
//#define  FILTER2 {-139,-210,-330,-454,-552,-587,-517,-301,93,680,1454,2386,3420,4481,5481,6330,6947,7272,7272,6947,6330,5481,4481,3420,2386,1454,680,93,-301,-517,-587,-552,-454,-330,-210,-139}

#endif

#include "Arduino.h"
#include "AudioStream.h"
#include "arm_math.h"

// integer interpolation
inline int16_t integinterp(int16_t a, int16_t b, int16_t dist) {
 int32_t bb=((int32_t) b * (int32_t) dist); 
 int32_t aa=((int32_t) a* ( DISTTABLESTEP - dist )) ;
 return (aa+bb) >> DISTSHIFTBIT; 
}

template <typename type>
type sign(type value) {
 return type((value>0)-(value<0));
}

class AudioEffectWaveshaperF : public AudioStream {
public:
  AudioEffectWaveshaperF(void)
    : AudioStream(1, inputQueueArray) {
       // init up & downsampler objects
       arm_fir_interpolate_init_f32( &upsampler, DISTOVERSAMPLING, DISTFILTERSNUMTAP1, UpsamplerLowPass, upstate,  AUDIO_BLOCK_SAMPLES );     
       arm_fir_decimate_init_f32(&downsampler, DISTFILTERSNUMTAP2, DISTOVERSAMPLING , DownsamplerLowPass, downstate, AUDIO_BLOCK_SAMPLES*DISTOVERSAMPLING);
      }
    
  void setTanh(float b[5]);

  void setPow(float b[5]);

  void setCubic(float b[4]);

  void setLinear(float gain1);
  
  void setSoftClipping(float gain1);
  
  void setExp(float b[4]);

  void setAsym(float b[4]);

  void setSoftPower(float b[4]);
  
  void setChebyshev(float b[5]);
  
  virtual void update(void);
  
private:
  bool isenabled=false;
  float coefs[6]={0,0,0,0,0,0};
  float wavetable[DISTTABLESIZE];
  float wavex[DISTTABLESIZE]; 
  audio_block_t *inputQueueArray[1];
  float upstate[AUDIO_BLOCK_SAMPLES-1 + (DISTFILTERSNUMTAP1 / DISTOVERSAMPLING) ];
  arm_fir_interpolate_instance_f32 upsampler;
  arm_fir_decimate_instance_f32 downsampler;
  float downstate[ DISTFILTERSNUMTAP2 + AUDIO_BLOCK_SAMPLES*DISTOVERSAMPLING - 1 ];
 

  float UpsamplerLowPass[DISTFILTERSNUMTAP1]= FILTER1;

  float DownsamplerLowPass[DISTFILTERSNUMTAP2]= FILTER2; 
 
  //{24   , 38  ,  64  , 110  , 180  , 278  , 405  , 559  , 735  , 925 , 1119 , 1307 , 1475 , 1614 , 1710  ,1764  ,1764,1710,1614,1475,1307,1119,925,735,559,405,278,180,110,64,38,24};
  //{212  , 361  , 780 , 1455,  2298 , 3158 , 3862 , 4258 , 4258  ,3862  ,3158 , 2298 , 1455  , 780 ,  361 ,  212};
  //{-45  ,  21 ,  265 ,  890,  1962,  3319,  4597,  5375,  5375  ,4597  ,3319 , 1962   ,890   ,265   , 21 ,  -45};
  //{211  , 553 , 1576 , 3189 , 4883 , 5972 , 5972 , 4883,  3189,  1576,   553,   211};
  
   //  direct upsampling by linear interpolation, checked!
    void upsample(uint8_t oversampling,uint16_t nsamplein,int16_t bufin[], float bufout[]) {
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
   
   void upsampleIIR(uint8_t oversampling,uint16_t nsamplein,int16_t bufin[], float bufout[]) {
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
   void upsample0(uint8_t oversampling,uint16_t nsamplein,int16_t bufin[], float bufout[]) {
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
     for (uint16_t i=0; i < nsampleout; i++) {
        uint16_t k=downsampling*i;
        float sum=0;
        for (uint8_t j=0;j<downsampling;j++) sum+=bufin[k+j];
        bufout[i]=sum*ratio;
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
     const uint8_t nloop=4;
     
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

