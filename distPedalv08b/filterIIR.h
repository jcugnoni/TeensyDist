/*
 * IIR filter for floating point 32bit , adapted from OpenAudio Library by Chip Audette
 */

#ifndef filter_iir_h
#define filter_iir_h

#include "Arduino.h"
#include "AudioStream_F32.h"
#include "arm_math.h"

// Indicates that the code should just pass through the audio
// without any filtering (as opposed to doing nothing at all)
//#define IIR_F32_PASSTHRU ((const float32_t *) 1)

#define FILT_IIR_MAX_STAGES 4  //max 4 stages of IIR biquad filters per block

class filterIIR : public AudioStream_F32
{
  //GUI: inputs:1, outputs:1  //this line used for automatic generation of GUI node
  public:
    filterIIR(void): AudioStream_F32(1,inputQueueArray) {  };
    
    void begin(const float32_t *cp, int n_stages) {
      // Initialize IIR instance (ARM DSP Math Library)
       //https://www.keil.com/pack/doc/CMSIS/DSP/html/group__BiquadCascadeDF1.html
        arm_biquad_cascade_df1_init_f32(&iir_inst, n_stages, (float32_t *)cp,  &StateF32[0]);
        enabled = true;
    };
    void end(void) {
     enabled=false;
    }
    
    bool enabled=false;
    int maxstage=-1;
    void setBlockDC(void) ;
    void setFilterCoeff_Matlab(float32_t b[], float32_t a[]) ;
    void setFilterCoeff_Matlab(uint8_t stage, float32_t b[], float32_t a[]);

    void setBiQuadEq(uint8_t stage,char type,float fc, float param1, float param2);
    virtual void update(void);
   
  private:
    audio_block_f32_t *inputQueueArray[1];
    float32_t hp_coeff[5 * FILT_IIR_MAX_STAGES] = {1.0, 0.0, 0.0, 0.0, 0.0,1.0, 0.0, 0.0, 0.0, 0.0,1.0, 0.0, 0.0, 0.0, 0.0,1.0, 0.0, 0.0, 0.0, 0.0}; //no filtering. actual filter coeff set later
    
    // ARM DSP Math library filter instance
    arm_biquad_casd_df1_inst_f32 iir_inst;
    float32_t StateF32[4*FILT_IIR_MAX_STAGES];
};

#endif


