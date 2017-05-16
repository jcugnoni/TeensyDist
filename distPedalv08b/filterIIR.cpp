/*
 * IIR filter for floating point 32bit , adapted from OpenAudio Library by Chip Audette
 */
#include "Arduino.h"
#include "AudioStream_F32.h"
#include "arm_math.h"
#include "filterIIR.h"


    void filterIIR::setBlockDC(void) {
      //https://www.keil.com/pack/doc/CMSIS/DSP/html/group__BiquadCascadeDF1.html#ga8e73b69a788e681a61bccc8959d823c5
      //Use matlab to compute the coeff for HP at 40Hz: [b,a]=butter(2,40/(44100/2),'high'); %assumes fs_Hz = 44100
      float32_t b[] = {8.173653471988667e-01,  -1.634730694397733e+00,   8.173653471988667e-01};  //from Matlab
      float32_t a[] = { 1.000000000000000e+00,   -1.601092394183619e+00,  6.683689946118476e-01};  //from Matlab
      setFilterCoeff_Matlab(b, a);
    }
    
    void filterIIR::setFilterCoeff_Matlab(float32_t b[], float32_t a[]) { //one stage of N=2 IIR
      //https://www.keil.com/pack/doc/CMSIS/DSP/html/group__BiquadCascadeDF1.html#ga8e73b69a788e681a61bccc8959d823c5
      //Use matlab to compute the coeff, such as: [b,a]=butter(2,20/(44100/2),'high'); %assumes fs_Hz = 44100
      hp_coeff[0] = b[0];   hp_coeff[1] = b[1];  hp_coeff[2] = b[2]; //here are the matlab "b" coefficients
      hp_coeff[3] = -a[1];  hp_coeff[4] = -a[2];  //the DSP needs the "a" terms to have opposite sign vs Matlab 
      uint8_t n_stages = 1;
      arm_biquad_cascade_df1_init_f32(&iir_inst, n_stages, hp_coeff,  &StateF32[0]);     
      enabled=true;
     
    }
    void filterIIR::setFilterCoeff_Matlab(uint8_t stage, float32_t b[], float32_t a[]) { //one stage of N=2 IIR
      //https://www.keil.com/pack/doc/CMSIS/DSP/html/group__BiquadCascadeDF1.html#ga8e73b69a788e681a61bccc8959d823c5
      //Use matlab to compute the coeff, such as: [b,a]=butter(2,20/(44100/2),'high'); %assumes fs_Hz = 44100
      uint8_t offst=stage*5; // stage numbered from 0
      hp_coeff[0+offst] = b[0];   hp_coeff[1+offst] = b[1];  hp_coeff[2+offst] = b[2]; //here are the matlab "b" coefficients
      hp_coeff[3+offst] = -a[0];  hp_coeff[4+offst] = -a[1];  //the DSP needs the "a" terms to have opposite sign vs Matlab 
      uint8_t n_stages = min(FILT_IIR_MAX_STAGES,maxstage);
//      Serial.print("Biquad stages: "); Serial.println(maxstage);
//      Serial.print("HP coefs: ");
//      for(uint8_t i=0;i< FILT_IIR_MAX_STAGES*5; i++) {Serial.print(hp_coeff[i],6); Serial.print(", "); }
//      Serial.println("");
      arm_biquad_cascade_df1_init_f32(&iir_inst, n_stages, hp_coeff,  &StateF32[0]);     
      enabled=true;
   }

    void filterIIR::setBiQuadEq(uint8_t stage,char type,float fc, float param1, float param2) {
    /* 
     *   !! stages are numbered from 0 to FILT_IIR_MAX_STAGES-1 !!
    % Compute several classical eq biquad coefficient sets from the Audio Eq Cookbook by R. Bristow-Johnson 
    % 
    % return biquad filter coefficients for a given type of EQ section
    % filterval = [b0,b1,b2,a1,a2] defines the IIR biquad coefs where a0=1.
    % 
    %           b0 + b1*z^-1 + b2*z^-2
    %    H(z) = ------------------------ 
    %            a0 + a1*z^-1 + a2*z^-2
    % 
    % type= 'L' or 'H' for low and high pass filters of frequency fc, param1=S (slope parameter) and param2=gaindb
    % type = 'B' & 'b' bandpass with freq center fc, param1=Q, B has 0db gain at fc and 'b' has gain=Q and constant skirt gain;
    % type = 'N' notchfilter at  frequency fc with a param1=Q. param2 unused
    % type = 'A' all pass filter , param1=Q
    % type = 'P' peak filter at fc, with param1=Q, param2=gain in db
    % type = 'l' low shelf with frequency cut fc , param1 = Q, param2= gaindb
    % type = 'h' high shelf with frequency cut fc , param1 = Q, param2= gaindb 
    */
    float b0,b1,b2,a0,a1,a2;
    float S,gaindb,A,alpha,Q; 
    float pi=3.1415926;
    float w0=2*pi*fc/44117.0;
    
    switch(type) { 
      case 'L': 
        S=param1;
        gaindb=param2;
        A=sqrt( pow(10,(gaindb/20)) );
        alpha=sin(w0)/2 * sqrt( (A + 1/A)*(1/S - 1) + 2 );
        b0 =  (1 - cos(w0))/2;
        b1 =   1 - cos(w0);
        b2 =  (1 - cos(w0))/2;
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha;
        break;
      case 'H':
        S=param1;
        gaindb=param2;
        A=sqrt( pow(10,(gaindb/20.0)) );
        alpha=sin(w0)/2.0 * sqrt( (A + 1.0/A)*(1/S - 1.0) + 2.0 );
        b0 =  (1. + cos(w0))/2.;
        b1 = -(1. + cos(w0));
        b2 =  (1. + cos(w0))/2.;
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha;
//        Serial.print(" High pass: S, gaindb,A,alpha: "); Serial.print(S); Serial.print(" "); Serial.print(gaindb); Serial.print(" ");Serial.print(A); Serial.print(" ");Serial.print(alpha); Serial.println(" ");
//        Serial.print(" Coefs: "); 
//        Serial.print(b0,8); Serial.print(", ");  
//        Serial.print(b1,8); Serial.print(", ");
//        Serial.print(b2,8); Serial.print(", ");
//        Serial.print(a0,8); Serial.print(", ");
//        Serial.print(a1,8); Serial.print(", ");
//        Serial.println(a2,8);
        break;
      case 'B':
        Q=param1;
        alpha = sin(w0)/(2*Q);
        b0 =   sin(w0)/2;
        b1 =   0;
        b2 =  -sin(w0)/2; 
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha;
        break;
      case 'b':
        Q=param1;
        alpha = sin(w0)/(2*Q);
        b0 =   alpha;
        b1 =   0;
        b2 =  -alpha;
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha;
        break;
      case 'N' :
        Q=param1;
        alpha = sin(w0)/(2*Q);
        b0 =   1;
        b1 =  -2*cos(w0);
        b2 =   1;
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha;
        break;
      case 'A':  
        Q=param1;
        alpha = sin(w0)/(2*Q);
        b0 =   1 - alpha;
        b1 =  -2*cos(w0);
        b2 =   1 + alpha;
        a0 =   1 + alpha;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha;
        break;
      case 'P':
        Q=param1;
        gaindb=param2;
        A=sqrt( pow(10,(gaindb/20)) );
        alpha = sin(w0)/(2*Q);
        b0 =   1 + alpha*A;
        b1 =  -2*cos(w0);
        b2 =   1 - alpha*A;
        a0 =   1 + alpha/A;
        a1 =  -2*cos(w0);
        a2 =   1 - alpha/A;
        break;
      case 'l':
        Q=param1;
        gaindb=param2;
        A=sqrt( pow(10,(gaindb/20)) );
        alpha = sin(w0)/(2*Q);
        b0 =    A*( (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha );
        b1 =  2*A*( (A-1) - (A+1)*cos(w0)                   );
        b2 =    A*( (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha );
        a0 =        (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha;
        a1 =   -2*( (A-1) + (A+1)*cos(w0)                   );
        a2 =        (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha;
        break;
      case 'h': 
        Q=param1;
        gaindb=param2;
        A=sqrt( pow(10,(gaindb/20)) );
        alpha = sin(w0)/(2*Q);
        b0 =    A*( (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha );
        b1 = -2*A*( (A-1) + (A+1)*cos(w0)                   );
        b2 =    A*( (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha );
        a0 =        (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha;
        a1 =    2*( (A-1) - (A+1)*cos(w0)                   );
        a2 =        (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha;
        break;
    }
    
    // normalize to a0=1
    float bcoefs[3]={b0/a0,b1/a0,b2/a0};
    float acoefs[2]={a1/a0,a2/a0};
    maxstage=max(maxstage,stage+1);
    setFilterCoeff_Matlab(stage, bcoefs, acoefs);
//    Serial.print("Biquad type: "); Serial.print(type); Serial.print(" fc="), Serial.print(fc,5),Serial.print(" param1="), Serial.print(param1,5),Serial.print(" param2="), Serial.println(param2,5);  
//    Serial.print(" Coefs: "); 
//    Serial.print(b0,7); Serial.print(", ");  
//    Serial.print(b1,7); Serial.print(", ");
//    Serial.print(b2,7); Serial.print(", ");
//    Serial.print(a0,7); Serial.print(", ");
//    Serial.print(a1,7); Serial.print(", ");
//    Serial.println(a2,7);
   };
    

void filterIIR::update(void)
{
  audio_block_f32_t *block;

  block = AudioStream_F32::receiveWritable_f32();
  if (!block) return;

   // do passthru
  if (!enabled ) {
    // Just passthrough
    AudioStream_F32::transmit(block);
    AudioStream_F32::release(block);
    return;
  }

  // do IIR
  arm_biquad_cascade_df1_f32(&iir_inst, block->data, block->data, block->length);
  AudioStream_F32::transmit(block); // send the IIR output
  AudioStream_F32::release(block);
}
