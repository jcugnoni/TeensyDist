
#ifndef __MENU_H 
#define __MENU_H

#include "gui.h"
#include "EEPROM.h"

 struct Gui_parameters {
  float inlevel=0.5;
  float outlevel=0.5;
  float masterGain=1.0;
  float compGain=1;
  float compThr=0.5;
  float compRatio=4;
  float compAttack=20;
  float compRelease=200;
  float noiseThr=0.;
  float noiseRate=10;
  float enabled[4]={0,0,0,0};
  float topology[4]={0,0,0,0};
  float clipping[4]={0,0,0,0};
  float pregain[4]={1,1,1,1};
  float postgain[4]={1,1,1,1};
  float bias[4]={0,0,0,0};
  float hicut[4]={10000,10000,10000,10000};
  float lowcut[4]={30,30,30,30};
  float drive[4]={1,1,1,1};
  float exponent[4]={1,1,1,1};
  float drymix[4]={0,0,0,0};
  float preFIR=0;
  float postFIR=0;
  float preHP=80;
  float preMidGain=0.;
  float preMidFreq=800.;
  float preMidQ=0.707;
  float preLP=3000;
  float postHP=40;
  float postMidGain=0.;
  float postMidFreq=800.;
  float postMidQ=0.707;
  float postLP=7000;
} ;

struct hardwareState {
  uint16_t pot1=0;
  uint16_t pot2=0;
  uint16_t pot3=0;
  uint16_t pot4=0;
  bool editSwitch=false;
  bool toggleSwitch=false;
  int32_t encoder=0;
  bool encButton=false; 
  uint32_t timecode=0;
};

struct presetState {
  uint8_t bank=0;
  widgetState bankState=NORMAL;
  uint8_t preset=0;
  widgetState presetState=NORMAL;
  uint8_t action=0;
  widgetState actionState=NORMAL;
  uint8_t target=0;
}; 

// track current Preset
static uint8_t presetId;

void buildGui();  // used to initialize the GUI
void validate();  // custom code to validate / push the gui variables to the DSP code
void Gui_update(); // handle update of physical buttons/encoder and transfer to GUI 
void Gui_validate(); // change audio processing settings according to GUI values
void getPresetState(presetState &p); // get preset selection, not to be saved on EEPROM
void getGUIparameters(Gui_parameters &params); // retrieve all GUI parameters from the GUI widgets
void setGUIparameters(Gui_parameters &params);
void getHardwareState(hardwareState &hardware); // read the current state of hardware interface
bool savePreset(uint8_t presetID,Gui_parameters &params);  // save preset to EEPROM, return true if success
bool readPreset(uint8_t presetID,Gui_parameters &params);  // read preset from EEPROM, return true if OK
uint32_t checksum(Gui_parameters params);  //compute a checksum on the given parameter set (to avoid loading corrupt/empty data)
uint32_t checksum(hardwareState params);  //compute a checksum on the given parameter set (to avoid loading corrupt/empty data)

// helper functions:
void getBiquadPeakCoefs(double freq,double Q,double midgaindb,double midcoefs[5]);

#endif
