#include "menu.h"

#include <Audio.h>

#include "filterIIR.h"
#include "codeOptions.h"
#include "multistagedist.h"
#include "compressor.h"

#define WM8731

// external objects
extern Encoder myEnc;
extern ILI9341_t3 tft;
extern Gui mygui;
#ifdef SGTL5000
extern AudioControlSGTL5000     codec; 
#endif    
#ifdef WM8731
extern AudioControlWM8731master     codec; 
#endif    
//extern AudioFilterBiquad        biquad1;
extern filterIIR        biquad1;
extern AudioFilterBiquad        biquad2;
extern AudioEffectCompressor compr;
extern bool GUI_Change;
extern void setInputFIR(uint8_t firid);
extern void setOutputFIR(uint8_t firid);
extern AudioEffectMultiStageDist  dist;

// MACROS
#define getW(menuid,widgetid) (mygui.menus[menuid]->widgets[widgetid]->value)
#define setW(menuid,widgetid,val) ( mygui.menus[menuid]->widgets[widgetid]->value=val )
#define robustRead(pin,val) val=0.; for(uint8_t i=0; i<4; i++) val+=analogRead(pin); val/=4;

void buildGui() {
  // Menu definitions (need to be global variables) 
  // MENU 1 : levels and compression

  //  widget: input Level 
  const static char inLvlLbl[GUI_LABEL_LEN]="In. Level";
  static Widget inLvlWidget(inLvlLbl,0,1.,0.05,1.0);
  //  widget: Output Level 
  const static char outLvlLbl[GUI_LABEL_LEN]="Out.Level";
  static Widget outLvlWidget(outLvlLbl,0,1,0.05,0.85);
  //  widget: post gain 
  const static char postGainLbl[GUI_LABEL_LEN]="MasterLvl";
  static Widget postGainWidget(postGainLbl,0,5.,0.1,1.);
  // compressor gain & threshold / ratio , attack , release times 
  const static char comprGainLbl[GUI_LABEL_LEN]="Comp.Gain";
  static Widget comprGainWidget(comprGainLbl,0.1,10,0.1,1.0);
  const static char comprThrLbl[GUI_LABEL_LEN]="Comp.Thr";
  static Widget comprThrWidget(comprThrLbl,0.1,1,0.01,1.);
  const static char comprRatioLbl[GUI_LABEL_LEN]="CompRatio";
  static Widget comprRatioWidget(comprRatioLbl,1,10,0.5,4);
  const static char comprAttLbl[GUI_LABEL_LEN]="Comp.Att ";
  static Widget comprAttWidget(comprAttLbl,10,600,10,40);
  const static char comprRelLbl[GUI_LABEL_LEN]="Comp.Rel ";
  static Widget comprRelWidget(comprRelLbl,25,5000,25,400);
  // noise gate (input)
  const static char noiseThrLbl[GUI_LABEL_LEN]="Noise.Thr";
  static Widget noiseThrWidget(noiseThrLbl,0,200,5,0);
  const static char noiseRateLbl[GUI_LABEL_LEN]="NoiseRate";
  static Widget noiseRateWidget(noiseRateLbl,2,100,2,10);

   // 1st Menu: list of widget & label
  static Widget *menuLst1[]={&inLvlWidget,&outLvlWidget,&postGainWidget,&comprGainWidget,&comprThrWidget,&comprRatioWidget,&comprAttWidget,&comprRelWidget,&noiseThrWidget,&noiseRateWidget};
  const static char men1Lbl[GUI_LABEL_LEN]="Lvl&Comp";
  static Menu Menu1(men1Lbl,menuLst1,10);
  mygui.add( &Menu1 );

  // menu 2-5 : drive

  // Enabled?
  const static char enabledLbl[GUI_LABEL_LEN]="Enabled  ";
  static char *listEnabled[]={"   Off   ","   On    "};
  static Widget enabledWidget1(enabledLbl,2,listEnabled,0);
  static Widget enabledWidget2(enabledLbl,2,listEnabled,0);
  static Widget enabledWidget3(enabledLbl,2,listEnabled,0);
  static Widget enabledWidget4(enabledLbl,2,listEnabled,0);
  
  // Clipping / Non linear table widget
  const static char clippingLbl[GUI_LABEL_LEN]="Type  ";
  static char *listTypeNL[]={" Linear  ","  Cubic  "," SoftClip","   Power "," Exponent", " Hyp. Tan" , "Asymmetr.", "SoftPower", "  12AU7  ","  12AT7  ", "12AX7-68", "12AX7-250", " 6V6-68k", " 6V6-250k","KT88-68k ","KT88-250k"};
  static Widget clippingWidget1(clippingLbl,16,listTypeNL,0);
  static Widget clippingWidget2(clippingLbl,16,listTypeNL,0);
  static Widget clippingWidget3(clippingLbl,16,listTypeNL,0);
  static Widget clippingWidget4(clippingLbl,16,listTypeNL,0);

 // Topology 
  const static char topoLbl[GUI_LABEL_LEN]="Topology ";
  static char *listTopo[]={" Single ","Push-Pull"};
  static Widget topoWidget1(topoLbl,2,listTopo,0);
  static Widget topoWidget2(topoLbl,2,listTopo,0);
  static Widget topoWidget3(topoLbl,2,listTopo,0);
  static Widget topoWidget4(topoLbl,2,listTopo,0);
 
  // Pre Gain
  const static char pregainLbl[GUI_LABEL_LEN]="Pre Gain ";
  static Widget pregainWidget1(pregainLbl,0,20,0.25,2);
  static Widget pregainWidget2(pregainLbl,0,20,0.25,2);
  static Widget pregainWidget3(pregainLbl,0,20,0.25,2);
  static Widget pregainWidget4(pregainLbl,0,20,0.25,2);
   
  // Bias
  const static char biasLbl[GUI_LABEL_LEN]="Bias     ";
  static Widget biasWidget1(biasLbl,-0.5,0.5,0.01,0.0);
  static Widget biasWidget2(biasLbl,-0.5,0.5,0.01,0.0);
  static Widget biasWidget3(biasLbl,-0.5,0.5,0.01,0.0);
  static Widget biasWidget4(biasLbl,-0.5,0.5,0.01,0.0);

  // High cut
  const static char highcutLbl[GUI_LABEL_LEN]="High Cut ";
  static Widget highcutWidget1(highcutLbl,1500,15000,500,6000);
  static Widget highcutWidget2(highcutLbl,1500,15000,500,6000);
  static Widget highcutWidget3(highcutLbl,1500,15000,500,6000);
  static Widget highcutWidget4(highcutLbl,1500,15000,500,6000);

  // Low cut
  const static char lowcutLbl[GUI_LABEL_LEN]="Low Cut  ";
  static Widget lowcutWidget1(lowcutLbl,10,100,5,30);
  static Widget lowcutWidget2(lowcutLbl,10,100,5,30);
  static Widget lowcutWidget3(lowcutLbl,10,100,5,30);
  static Widget lowcutWidget4(lowcutLbl,10,100,5,30);
  
  // Distortion Drive
  const static char driveLbl[GUI_LABEL_LEN]="DistDrive";
  static Widget driveWidget1(driveLbl,1,100,1,1.0);
  static Widget driveWidget2(driveLbl,1,100,1,1.0);
  static Widget driveWidget3(driveLbl,1,100,1,1.0);
  static Widget driveWidget4(driveLbl,1,100,1,1.0);
  
  // Exponent
  const static char exponentLbl[GUI_LABEL_LEN]="DistExpon";
  static Widget exponentWidget1(exponentLbl,0.2,5,0.1,1.0);
  static Widget exponentWidget2(exponentLbl,0.2,5,0.1,1.0);
  static Widget exponentWidget3(exponentLbl,0.2,5,0.1,1.0);
  static Widget exponentWidget4(exponentLbl,0.2,5,0.1,1.0);
  
  // Dry mix
  const static char dryMixLbl[GUI_LABEL_LEN]="Dry Mix  ";
  static Widget dryMixWidget1(dryMixLbl,0.,1.,0.05,0.);
  static Widget dryMixWidget2(dryMixLbl,0.,1.,0.05,0.);
  static Widget dryMixWidget3(dryMixLbl,0.,1.,0.05,0.);
  static Widget dryMixWidget4(dryMixLbl,0.,1.,0.05,0.);
  
  // Post Gain
  const static char postgainLbl[GUI_LABEL_LEN]="PostGain ";
  static Widget postgainWidget1(postgainLbl,0,10,0.1,1);
  static Widget postgainWidget2(postgainLbl,0,10,0.1,1);
  static Widget postgainWidget3(postgainLbl,0,10,0.1,1);
  static Widget postgainWidget4(postgainLbl,0,10,0.1,1);
 
   // Drive Menus: list of widget & label
  static Widget *menuLst2[]={&enabledWidget1,&clippingWidget1,&topoWidget1,&pregainWidget1,&biasWidget1,&highcutWidget1,&lowcutWidget1,&driveWidget1,&exponentWidget1,&dryMixWidget1,&postgainWidget1};
  const static char men2Lbl[GUI_LABEL_LEN]=" Drive 1 ";
  static Menu Menu2(men2Lbl,menuLst2,11);
  mygui.add( &Menu2 );
  static Widget *menuLst3[]={&enabledWidget2,&clippingWidget2,&topoWidget2,&pregainWidget2,&biasWidget2,&highcutWidget2,&lowcutWidget2,&driveWidget2,&exponentWidget2,&dryMixWidget2,&postgainWidget2};
  const static char men3Lbl[GUI_LABEL_LEN]=" Drive 2 ";
  static Menu Menu3(men3Lbl,menuLst3,11);
  mygui.add( &Menu3 );
  static Widget *menuLst4[]={&enabledWidget3,&clippingWidget3,&topoWidget3,&pregainWidget3,&biasWidget3,&highcutWidget3,&lowcutWidget3,&driveWidget3,&exponentWidget3,&dryMixWidget3,&postgainWidget3};
  const static char men4Lbl[GUI_LABEL_LEN]=" Drive 3 ";
  static Menu Menu4(men4Lbl,menuLst4,11);
  mygui.add( &Menu4 );
  static Widget *menuLst5[]={&enabledWidget4,&clippingWidget4,&topoWidget4,&pregainWidget4,&biasWidget4,&highcutWidget4,&lowcutWidget4,&driveWidget4,&exponentWidget4,&dryMixWidget4,&postgainWidget4};
  const static char men5Lbl[GUI_LABEL_LEN]=" Drive 4 ";
  static Menu Menu5(men5Lbl,menuLst5,11);
  mygui.add( &Menu5 );

  // 6th MENU: pre/post eq parameters

  // PreFIR
  const static char preFIRLbl[GUI_LABEL_LEN]="PreImpuls";
  static char *listPreFIR[]={" Bypass  "," Morpher ", "   RAT   ","   OCD   ","Fuzz Face","   DS1   "," Big Muff", "   Klon  ","  TS808  ", "Marshall ", "   Vox   ", "  Fender "};
  static Widget preFIRWidget(preFIRLbl,12,listPreFIR,0);
   // PostFIR
  const static char postFIRLbl[GUI_LABEL_LEN]="PostImpls";
  static char *listPostFIR[]={" Bypass  "," Morpher ", "   RAT   ","   OCD   ","Fuzz Face","   DS1   "," Big Muff","   Klon  ","  TS808  ", "Marshall ", "   Vox   ", "  Fender "};
  static Widget postFIRWidget(postFIRLbl,12,listPostFIR,0);
  // Pre HP or lowcut
  const static char preHPLbl[GUI_LABEL_LEN]="PreLowCut";
  static Widget preHPWidget(preHPLbl,10,400,2,80);
  // Pre Mid param
  const static char preMidGainLbl[GUI_LABEL_LEN]="PreMidGn ";
  static Widget preMidGainWidget(preMidGainLbl,-10,10,0.25,0);
  const static char preMidFreqLbl[GUI_LABEL_LEN]="PreMidFrq";
  static Widget preMidFreqWidget(preMidFreqLbl,100,5000,50,800);
  const static char preMidQLbl[GUI_LABEL_LEN]="PreMidQ ";
  static Widget preMidQWidget(preMidQLbl,0,2,0.1,0.7);
  //  pre LP or high cut
  static char preLPLbl[GUI_LABEL_LEN]="PreHiCut ";
  static Widget preLPWidget(preLPLbl,1500,12000,100,5000);

  // Post HP or lowcut
  const static char postHPLbl[GUI_LABEL_LEN]="PosLowCut";
  static Widget postHPWidget(postHPLbl,10,400,2,80);
  // Pre Mid param
  const static char postMidGainLbl[GUI_LABEL_LEN]="PosMidGn ";
  static Widget postMidGainWidget(postMidGainLbl,-10,10,0.25,0);
  const static char postMidFreqLbl[GUI_LABEL_LEN]="PosMidFrq";
  static Widget postMidFreqWidget(postMidFreqLbl,100,5000,50,800);
  const static char postMidQLbl[GUI_LABEL_LEN]="PostMidQ ";
  static Widget postMidQWidget(postMidQLbl,0,2,0.1,0.7);
  //  pre LP or high cut
  static char postLPLbl[GUI_LABEL_LEN]="PostHiCut";
  static Widget postLPWidget(postLPLbl,1500,12000,100,5000);

   // Menu: eq parameters
  static Widget *menuLst6[]={&preFIRWidget,&postFIRWidget,&preHPWidget,&preMidGainWidget,&preMidFreqWidget,&preMidQWidget,&preLPWidget,
                             &postHPWidget,&postMidGainWidget,&postMidFreqWidget,&postMidQWidget,&postLPWidget};
  static char men6Lbl[GUI_LABEL_LEN]="Eq&Impuls";
  static Menu Menu6(men6Lbl,menuLst6,12);
  mygui.add( &Menu6 );// 6th MENU: pre/post eq parameters

  // 7th menu: presets

  // Bank
  const static char bankLbl[GUI_LABEL_LEN]="Bank     ";
  static char *listBank[]={" Bank 1  "," Bank 2  "," Bank 3  "," Bank 4  "};
  static Widget bankWidget(bankLbl,4,listBank,0);
  // Preset
  const static char presetLbl[GUI_LABEL_LEN]="Preset   ";
  static char *listPreset[]={"   1A   ","   1B   ","   2A   ","   2B   ","   3A   ","   3B   ","   4A   ","   4B   "};
  static Widget presetWidget(presetLbl,8,listPreset,0);
  // here we will just recall the widgets defined earlier for level, gain, treble, mid & bass
  // postGainWidget
  // pregainWidget1
  // postLPWidget
  // postMidGainWidget
  // postHPWidget

  // next widgets are for preset management
  // Preset
  const static char actionLbl[GUI_LABEL_LEN]="Action   ";
  static char *listAction[]={"  None   ","   Save  ","   Load  ","  Swap   "};
  static Widget actionWidget(actionLbl,4,listAction,0);
  static char *listPreset2[]={"  B1-1A  ","  B1-1B  ","  B1-2A  ","  B1-2B  ","  B1-3A  ","  B1-3B  ","  B1-4A  ","  B1-4B  ",
                              "  B2-1A  ","  B2-1B  ","  B2-2A  ","  B2-2B  ","  B2-3A  ","  B2-3B  ","  B2-4A  ","  B2-4B  ",
                              "  B3-1A  ","  B3-1B  ","  B3-2A  ","  B3-2B  ","  B3-3A  ","  B3-3B  ","  B3-4A  ","  B3-4B  ",
                              "  B4-1A  ","  B4-1B  ","  B4-2A  ","  B4-2B  ","  B4-3A  ","  B4-3B  ","  B4-4A  ","  B4-4B  "};
  static Widget presetWidget2(presetLbl,32,listPreset2,0);
  const static char bigLbl[GUI_LABEL_LEN]="        ";
  static Widget bigWidget(bigLbl);
  
   // Menu: presets
  static Widget *menuLst7[]={&bankWidget,&presetWidget,&postGainWidget,&pregainWidget1,&postLPWidget,&postMidGainWidget,&postHPWidget,&actionWidget,&presetWidget2,&bigWidget};
  static char men7Lbl[GUI_LABEL_LEN]="Presets ";
  static Menu Menu7(men7Lbl,menuLst7,10);
  mygui.add( &Menu7 );
  
  mygui.draw();
};

// compute mid biquad params
void getBiquadPeakCoefs(double freq,double Q,double midgaindb,double midcoefs[5]) {
  double w0=2.0*3.1415926*freq/44117.0;
  double alpha = sin(w0)/(2*Q);
  double A  = sqrt( pow(10,midgaindb/20) );
  double a0= 1 + alpha/A;
  midcoefs[0]=   (1 + alpha*A)/a0;
  midcoefs[1] =  (-2*cos(w0))/a0;
  midcoefs[2] =   (1 - alpha*A)/a0;
  midcoefs[3] =  (-2*cos(w0))/a0;
  midcoefs[4] =   (1 - alpha/A)/a0;
};

// read all GUI parameters from Widgets
void getGUIparameters(Gui_parameters &p) {
  // menu1: levels
  p.inlevel=getW(0,0);
  p.outlevel=getW(0,1);
  p.masterGain=getW(0,2);
  p.compGain=getW(0,3);
  p.compThr=getW(0,4);
  p.compRatio=getW(0,5);
  p.compAttack=getW(0,6);
  p.compRelease=getW(0,7);
  p.noiseThr=getW(0,8);
  p.noiseRate=getW(0,9);
  // menus 2-5 drives
  p.enabled[0]=floor(getW(1,0));
  p.enabled[1]=floor(getW(2,0));
  p.enabled[2]=floor(getW(3,0));
  p.enabled[3]=floor(getW(4,0));
  p.clipping[0]=floor(getW(1,1));
  p.clipping[1]=floor(getW(2,1));
  p.clipping[2]=floor(getW(3,1));
  p.clipping[3]=floor(getW(4,1));
  p.topology[0]=floor(getW(1,2));
  p.topology[1]=floor(getW(2,2));
  p.topology[2]=floor(getW(3,2));
  p.topology[3]=floor(getW(4,2));
  p.pregain[0]=getW(1,3);
  p.pregain[1]=getW(2,3);
  p.pregain[2]=getW(3,3);
  p.pregain[3]=getW(4,3);
  p.bias[0]=getW(1,4);
  p.bias[1]=getW(2,4);
  p.bias[2]=getW(3,4);
  p.bias[3]=getW(4,4); 
  p.hicut[0]=getW(1,5);
  p.hicut[1]=getW(2,5);
  p.hicut[2]=getW(3,5);
  p.hicut[3]=getW(4,5);
  p.lowcut[0]=getW(1,6);
  p.lowcut[1]=getW(2,6);
  p.lowcut[2]=getW(3,6);
  p.lowcut[3]=getW(4,6);
  p.drive[0]=getW(1,7);
  p.drive[1]=getW(2,7);
  p.drive[2]=getW(3,7);
  p.drive[3]=getW(4,7);
  p.exponent[0]=getW(1,8);
  p.exponent[1]=getW(2,8);
  p.exponent[2]=getW(3,8);
  p.exponent[3]=getW(4,8);
  p.drymix[0]=getW(1,9);
  p.drymix[1]=getW(2,9);
  p.drymix[2]=getW(3,9);
  p.drymix[3]=getW(4,9);
  p.postgain[0]=getW(1,10);
  p.postgain[1]=getW(2,10);
  p.postgain[2]=getW(3,10);
  p.postgain[3]=getW(4,10); 
  // menu 6: eq & FIR
  p.preFIR=getW(5,0);
  p.postFIR=getW(5,1);
  p.preHP=getW(5,2);
  p.preMidGain=getW(5,3);
  p.preMidFreq=getW(5,4);
  p.preMidQ=getW(5,5);
  p.preLP=getW(5,6);
  p.postHP=getW(5,7);
  p.postMidGain=getW(5,8);
  p.postMidFreq=getW(5,9);
  p.postMidQ=getW(5,10);
  p.postLP=getW(5,11);
}

void setGUIparameters(Gui_parameters &p) {
  // menu1: levels
  setW(0,0,p.inlevel);
  setW(0,1,p.outlevel);
  setW(0,2,p.masterGain);
  setW(0,3,p.compGain);
  setW(0,4,p.compThr);
  setW(0,5,p.compRatio);
  setW(0,6,p.compAttack);
  setW(0,7,p.compRelease);
  setW(0,8,p.noiseThr);
  setW(0,9,p.noiseRate);
  // menus 2-5 drives
  setW(1,0,p.enabled[0]);
  setW(1,1,p.clipping[0]);
  setW(1,2,p.topology[0]);
  setW(1,3,p.pregain[0]);
  setW(1,4,p.bias[0]);
  setW(1,5,p.hicut[0]);
  setW(1,6,p.lowcut[0]);
  setW(1,7,p.drive[0]);
  setW(1,8,p.exponent[0]);
  setW(1,9,p.drymix[0]);
  setW(1,10,p.postgain[0]);
  setW(2,0,p.enabled[1]);
  setW(2,1,p.clipping[1]);
  setW(2,2,p.topology[1]);
  setW(2,3,p.pregain[1]);
  setW(2,4,p.bias[1]);
  setW(2,5,p.hicut[1]);
  setW(2,6,p.lowcut[1]);
  setW(2,7,p.drive[1]);
  setW(2,8,p.exponent[1]);
  setW(2,9,p.drymix[1]);
  setW(2,10,p.postgain[1]);
  setW(3,0,p.enabled[2]);
  setW(3,1,p.clipping[2]);
  setW(3,2,p.topology[2]);
  setW(3,3,p.pregain[2]);
  setW(3,4,p.bias[2]);
  setW(3,5,p.hicut[2]);
  setW(3,6,p.lowcut[2]);
  setW(3,7,p.drive[2]);
  setW(3,8,p.exponent[2]);
  setW(3,9,p.drymix[2]);
  setW(3,10,p.postgain[2]);
  setW(4,0,p.enabled[3]);
  setW(4,1,p.clipping[3]);
  setW(4,2,p.topology[3]);
  setW(4,3,p.pregain[3]);
  setW(4,4,p.bias[3]);
  setW(4,5,p.hicut[3]);
  setW(4,6,p.lowcut[3]);
  setW(4,7,p.drive[3]);
  setW(4,8,p.exponent[3]);
  setW(4,9,p.drymix[3]);
  setW(4,10,p.postgain[3]);
  // menu 6: eq & FIR
  setW(5,0,p.preFIR);
  setW(5,1,p.postFIR);
  setW(5,2,p.preHP);
  setW(5,3,p.preMidGain);
  setW(5,4,p.preMidFreq);
  setW(5,5,p.preMidQ);
  setW(5,6,p.preLP);
  setW(5,7,p.postHP);
  setW(5,8,p.postMidGain);
  setW(5,9,p.postMidFreq);
  setW(5,10,p.postMidQ);
  setW(5,11,p.postLP);
}

#define PRNT(x) Serial.print(x);
#define PRNTLN(x) Serial.println(x);
#define PRNTP(p,name) PRNT(" " #name ": "); PRNTLN(p.name);
void printGUIparameters(Gui_parameters &p) {
  // menu1: levels
  PRNTLN("Preset listing:");
  PRNTP(p,inlevel);
  PRNTP(p,outlevel);
  PRNTP(p,outlevel);
  PRNTP(p,masterGain);
  PRNTP(p,compGain);
  PRNTP(p,compThr);
  PRNTP(p,compRatio);
  PRNTP(p,compAttack);
  PRNTP(p,compRelease);
  PRNTP(p,noiseThr);
  PRNTP(p,noiseRate);
  // menus 2-5 drives
  PRNTP(p,enabled[0]);
  PRNTP(p,clipping[0]);
  PRNTP(p,topology[0]);
  PRNTP(p,pregain[0]);
  PRNTP(p,bias[0]);
  PRNTP(p,hicut[0]);
  PRNTP(p,lowcut[0]);
  PRNTP(p,drive[0]);
  PRNTP(p,exponent[0]);
  PRNTP(p,drymix[0]);
  PRNTP(p,postgain[0]);
  PRNTP(p,enabled[1]);
  PRNTP(p,clipping[1]);
  PRNTP(p,topology[1]);
  PRNTP(p,pregain[1]);
  PRNTP(p,bias[1]);
  PRNTP(p,hicut[1]);
  PRNTP(p,lowcut[1]);
  PRNTP(p,drive[1]);
  PRNTP(p,exponent[1]);
  PRNTP(p,drymix[1]);
  PRNTP(p,postgain[1]);
  PRNTP(p,enabled[2]);
  PRNTP(p,clipping[2]);
  PRNTP(p,topology[2]);
  PRNTP(p,pregain[2]);
  PRNTP(p,bias[2]);
  PRNTP(p,hicut[2]);
  PRNTP(p,lowcut[2]);
  PRNTP(p,drive[2]);
  PRNTP(p,exponent[2]);
  PRNTP(p,drymix[2]);
  PRNTP(p,postgain[2]);
  PRNTP(p,enabled[3]);
  PRNTP(p,clipping[3]);
  PRNTP(p,topology[3]);
  PRNTP(p,pregain[3]);
  PRNTP(p,bias[3]);
  PRNTP(p,hicut[3]);
  PRNTP(p,lowcut[3]);
  PRNTP(p,drive[3]);
  PRNTP(p,exponent[3]);
  PRNTP(p,drymix[3]);
  PRNTP(p,postgain[3]);
  // menu 6: eq & FIR
  PRNTP(p,preFIR);
  PRNTP(p,postFIR);
  PRNTP(p,preHP);
  PRNTP(p,preMidGain);
  PRNTP(p,preMidFreq);
  PRNTP(p,preMidQ);
  PRNTP(p,preLP);
  PRNTP(p,postHP);
  PRNTP(p,postMidGain);
  PRNTP(p,postMidFreq);
  PRNTP(p,postMidQ);
  PRNTP(p,postLP);
}

void getHardwareState(hardwareState &h) {
//  Serial.print("Switches Hardware");
//  Serial.print(digitalRead(SWITCH1));
//  Serial.print(" ");
//  Serial.println(digitalRead(SWITCH2));
  h.encButton=digitalRead(BUTTON);
  h.encoder=myEnc.read();
  h.editSwitch=digitalRead(SWITCH1);
  h.toggleSwitch=digitalRead(SWITCH2);
  robustRead(POT1,h.pot1);
  robustRead(POT2,h.pot2);
  robustRead(POT3,h.pot3);
  robustRead(POT4,h.pot4);
  h.timecode=millis();
};

void getPresetState(presetState &p) {
  p.bank=floor(getW(6,0));
  p.bankState=mygui.menus[6]->widgets[0]->state;
  p.preset=floor(getW(6,1));
  p.presetState=mygui.menus[6]->widgets[1]->state;
  p.action=floor(getW(6,7));
  p.actionState=mygui.menus[6]->widgets[7]->state;
  p.target=floor(getW(6,8));
};

uint32_t checksum(Gui_parameters params) {
  byte* b = (byte *) &params;
  uint16_t recordlen=sizeof(params);
  uint32_t sum=314159;
  // read params , skip first 4 bytes which store the checksum.
  for(uint16_t i=0;i<recordlen;i++) {
    sum+=*b;
    b++;
  }
  return sum;
};

uint32_t checksum(hardwareState params) {
  byte* b = (byte *) &params;
  uint16_t recordlen=sizeof(params);
  uint32_t sum=76566;
  // read params , skip first 4 bytes which store the checksum.
  for(uint16_t i=0;i<recordlen;i++) {
    sum+=*b;
    b++;
  }
  return sum;
};

// save a preset to eeprom
bool savePreset(uint8_t presetID,Gui_parameters &params) {
  //Serial.println("Saving preset");
  unsigned int eeAddress = 0; //EEPROM address to start reading from
  if(presetID>32) return false;
  uint32_t checksum1=checksum(params);
  unsigned int recordLen= sizeof(params)+sizeof(checksum1)+sizeof(presetID);
  unsigned int offset0=eeAddress+recordLen*presetID;
  unsigned int offset=offset0;
  // save params
  EEPROM.put(offset,params);
  offset+=sizeof(params);
  // save checksum and ID
  EEPROM.put(offset,checksum1);
  offset+=sizeof(checksum1);
  EEPROM.put(offset,presetID);
  // test read
  Gui_parameters tst;
  offset=offset0;
  EEPROM.get(offset,tst);
  offset+=sizeof(tst);
  uint32_t tstcheck=0; uint8_t tstid=0;
  EEPROM.get(offset,tstcheck);
  offset+=sizeof(tstcheck);
  EEPROM.get(offset,tstid);
  //Serial.println( (String) "Checksum Ref=" + checksum1 + " Verif=" + tstcheck +" Id=" + presetID + " Checkid=" + tstid);
  if(checksum(tst)!=tstcheck) {
    Serial.println("Checksum error save preset!");
  }
  if(tstid!=presetID) {
    Serial.println("ID error save preset!");
  }
  if((checksum(tst)==checksum(params))) { 
    return true;
  } else {
    Serial.print("Invalid preset checksum while writing EEPROM for id ");
    Serial.print(presetID);
    Serial.print(" checksum1: ");
    Serial.print(checksum(tst));
    Serial.print(" checksum2: ");
    Serial.println(checksum(params));
    return false;
  }
}

// read a preset to a parameter structure
bool readPreset(uint8_t presetID,Gui_parameters &params) {
  unsigned int eeAddress = 0; //EEPROM address to start reading from
  uint32_t checksum1=0;
  uint8_t id=0;
  unsigned int recordLen= sizeof(params)+sizeof(checksum1)+sizeof(id);
  unsigned int offset0=eeAddress+recordLen*presetID;
  unsigned int offset=offset0;
  
  //Serial.println("Reading preset");
  if(presetID>32) return false;
  EEPROM.get(offset,params);
  offset+=sizeof(params);
  EEPROM.get(offset,checksum1);
  offset+=sizeof(checksum1);
  EEPROM.get(offset,id);
  if(checksum(params)!=checksum1) {
    Serial.println("Checksum error read preset!");
  }
  if(id!=presetID) {
    Serial.println("ID error read preset!");
  }
   
  if(checksum(params)==checksum1) { 
    return true;
    Serial.println("Read preset: ok");
  } else {
    Serial.print("Invalid preset checksum while reading EEPROM for id ");
    Serial.print(presetID);
    Serial.print(" checksum1: ");
    Serial.print(checksum(params));
    Serial.print(" checksum2: ");
    Serial.println(checksum1);
    return false;
  }
}

// load a saved preset to current parameter set
bool loadPreset(uint8_t presetID) {
  Gui_parameters curp;
  getGUIparameters(curp);
  Gui_parameters p;
  if(readPreset(presetID,p)) {
    PRNTLN("--- GUI params before ---");
    printGUIparameters(curp);
    PRNTLN("--- PRESET params ---");
    printGUIparameters(p);
    setGUIparameters(p);
    getGUIparameters(curp);
    PRNTLN("--- GUI params after ---");
    printGUIparameters(curp);
    
    Serial.println("LoadPreset: Parameters restored");
  } else {
    Serial.print("Failed to load preset id ");
    Serial.println(presetID);
  }
}

// update potentiometer reading with noise rejection
void updatePot(uint8_t potid, uint16_t value) {
  float val1=((float) value) / 1023.;
  switch(potid) {
    case 1:  // level => change masterGain
       setW(0,2, 0.1*(1-val1)+5*val1 );  
       Serial.print("Level set to ");
       Serial.println(val1);
       break;
    case 2:  // gain => change first stage preGain
       setW(1,3, 0.1*(1-val1)+10*val1 );
       Serial.print("preGain set to ");
       Serial.println(val1);
       break;
    case 3:  // treble
       setW(5,11,1500*(1-val1)+12000*val1);
       Serial.print("Treble cut set to ");
       Serial.println(val1);
       break;
    case 4:  // bass
       setW(5,7, 20*(1-val1)+200*val1);
       Serial.print("Bass cut set to ");
       Serial.println(val1);
       break;
  }
};

// refresh the big preset display
void updatePresetDisplay(int bank,int presetid) {
   uint8_t presetNum=presetid/2+1;
   char toggle=((presetid % 2)==0)?'A':'B';
   char txt[GUI_LABEL_LEN]="         ";
   sprintf(txt,"B%1d-%1d%c",bank+1,presetNum,toggle);
   mygui.menus[6]->widgets[9]->setLabel(txt);
    // force refresh
    //mygui.menus[6]->draw();
};

// Gui_validate is responsible to apply the changes of the GUI state to the processing algorithms 
void Gui_validate() {
  Serial.println("Activate paramters (Gui_validate)");
  static Gui_parameters curp;
  static hardwareState hardware;
  Gui_parameters newp;

  // get updates
  getGUIparameters(newp);
  getHardwareState(hardware);
  
  // compare new & old param and update 
  bool setDistCurve[4]={false,false,false,false};
  bool setDistParam[4]={false,false,false,false};
  bool setLevels=false;
  bool setFIR=false;
  bool setPreEq=false;
  bool setPostEq=false;
  bool setComp=false;
  
  // check differences  
  for(uint8_t i=0;i<4;i++) {
    if(newp.clipping[i]!=curp.clipping[i] || newp.drive[i]!=curp.drive[i] || newp.exponent[i]!=curp.exponent[i] || newp.drymix[i]!=curp.drymix[i] || newp.bias[i]!=curp.bias[i] || newp.topology[i]!=curp.topology[i] ) {  
      setDistCurve[i]=true;
    }
  }
  for(uint8_t i=0;i<4;i++) {
    if(newp.enabled[i]!=curp.enabled[i] || newp.pregain[i]!=curp.pregain[i] || newp.postgain[i]!=curp.postgain[i] || newp.bias[i]!=curp.bias[i] || newp.hicut[i]!=curp.hicut[i] || newp.lowcut[i]!=curp.lowcut[i]) {  
      setDistParam[i]=true;
    }
  }
  if(newp.preFIR!=curp.preFIR || newp.postFIR!=curp.postFIR){ setFIR=true; }
  if(newp.preHP!=curp.preHP || newp.preMidGain!=curp.preMidGain|| newp.preMidFreq!=curp.preMidFreq|| newp.preMidQ!=curp.preMidQ|| newp.preLP!=curp.preLP){ 
    setPreEq=true;
  }
  if(newp.postHP!=curp.postHP || newp.postMidGain!=curp.postMidGain|| newp.postMidFreq!=curp.postMidFreq|| newp.postMidQ!=curp.postMidQ|| newp.postLP!=curp.postLP){ 
    setPostEq=true;
  }
  if(newp.inlevel!=curp.inlevel || newp.outlevel!=curp.outlevel || newp.masterGain!=curp.masterGain ) {
    setLevels=true;
  }
  if(newp.compThr!=curp.compThr || newp.compGain!=curp.compGain || newp.compRatio!=curp.compRatio || newp.compAttack!=curp.compAttack || newp.compRelease!=curp.compRelease || newp.noiseThr!=curp.noiseThr|| newp.noiseRate!=curp.noiseRate) {
    setComp=true;
  }

  /*Serial.print("Dist upd? ");
  Serial.println(setDistCurve);
  Serial.println(setDistParam);
  Serial.print("PreEq upd? ");
  Serial.println(setPreEq);
  Serial.print("PostEq upd? ");
  Serial.println(setPostEq);
  Serial.print("Compr upd? ");
  Serial.println(setComp);*/
  
  // APPLY CHANGES
  AudioNoInterrupts();

  // store new parameters
  curp=newp;
  // set levels 
  if(setLevels) {
    Serial.println("Level param update");
    codec.inputLevel(curp.inlevel); 
    codec.volume(curp.outlevel);
    dist.setPostGain(curp.masterGain,MSDMAXSTAGES);
  }
  if(setComp) {
    Serial.println("Compressor param update");
    compr.setGain(curp.compGain);
    compr.setThreshold(curp.compThr);
    compr.setRatio(curp.compRatio);
    compr.setAttack(curp.compAttack);
    compr.setRelease(curp.compRelease);
    compr.setNoiseThr(floor(curp.noiseThr));
    compr.setNoiseRate(curp.noiseRate);
    if((fabs(curp.compGain-1.0)<1e-4) && (curp.compThr>0.999) && (curp.noiseThr<0.1)) {compr.bypass();}  else {compr.enable();};
  }
  // update distortion params
  for(uint8_t stage=0;stage<4;stage++) {
  if(setDistCurve[stage]) {
    Serial.print("Clipping mode ");
    Serial.println(curp.clipping[stage]);
    uint8_t nbdrives=8;  // number of drives before tube in clipping list , to change when a drive is added !!!
    switch( (int) floor(curp.clipping[stage]) ) {
      case 0:  // linear
         dist.setLinear(curp.drive[stage],stage); 
         Serial.print("Clipping mode : Linear");
         break;
      case 1:  // cubic, using a soft clipping symmetric cubic with pregain, modified version of Doidic, y=postgain*((abs(2*pregain*(x+bias))-(pregain*(x+bias))^2)*sign(x+bias))+offset
        { float distparam[4];
        distparam[0]=1.0;
        distparam[1]=curp.drive[stage];
        distparam[2]=0.0;
        distparam[3]=0.0;
        dist.setCubic(distparam,stage);
        Serial.print("Clipping mode : Cubic");
        break;}
      case 2: // softClip
        dist.setSoftClipping(curp.drive[stage],stage); 
        Serial.print("Clipping mode : Soft");
        break;
      case 3: // power law
        { float distparam[5];
        distparam[0]=1.0;
        distparam[1]=curp.drive[stage];
        distparam[2]=0.0;
        distparam[3]=curp.exponent[stage];
        distparam[4]=0.0;
        dist.setPow(distparam,stage); 
        Serial.print("Clipping mode : Power");
        break; }
      case 4: // Exponential
        { float distparam[4];
        distparam[0]=1.0;
        distparam[1]=curp.drive[stage];
        distparam[2]=0.0;
        distparam[3]=0.0;
        dist.setExp(distparam,stage);
        Serial.print("Clipping mode : Exp");
        break; }
      case 5: //Hyperbolic tan
        { float distparam[5];
        distparam[0]=1-curp.drymix[stage];
        distparam[1]=curp.drive[stage];
        distparam[2]=0.0;
        distparam[3]=curp.drymix[stage];
        distparam[4]=0.0;
        dist.setTanh(distparam,stage);
        Serial.print("Clipping mode : Tanh");
        break; }
      case 6: //Asym
        { float distparam[4]; // Asymmetric clipping from Doidic et al.
        distparam[0]=1.0; 
        distparam[1]=curp.drive[stage]/10;  
        distparam[2]=0.0;  
        distparam[3]=0.0;
        dist.setAsym(distparam,stage);
        Serial.print("Clipping mode : Asymmetric Doidic");
        break; }
      case 7: //Soft power
        { float distparam[4]; // soft clipping with powerlaw response
        distparam[0]=1.0; 
        distparam[1]=curp.drive[stage];  
        distparam[2]=0.0;  
        distparam[3]=curp.exponent[stage];
        dist.setSoftPower(distparam,stage);
        Serial.print("Clipping mode : PowerLaw modified soft clipping");
        break; }
      default: // tubes 12au7,12at7,12ax7-68k,12ax7-250k
        {
          uint8_t tubeid=curp.clipping[stage]-nbdrives;
          if(curp.topology[stage]) { // push pull
            dist.setTubePushPull(tubeid,stage, curp.bias[stage]*32767.);
          } else {
            dist.setTube(tubeid,stage);
          }
          Serial.print("Clipping mode : tube ");
          Serial.print(tubeid); 
        break; }
      }
    } // end if setdistcurve
    if(setDistParam[stage]) {
      if(curp.enabled[stage]) {
        dist.enableStage(stage);
      } else {
        dist.bypassStage(stage);
      }
      dist.setBias(curp.bias[stage]*32767.,stage);
      dist.setPreGain(curp.pregain[stage],stage);
      dist.setPostGain(curp.postgain[stage],stage);
      dist.setLowpass(curp.hicut[stage],stage);
      dist.setHighpass(curp.lowcut[stage],stage);
    }
    }// end loop on stages
    
    // update eq params
    if(setPreEq) {
      Serial.println("Set Pre EQ");
      // pre eq
      //biquad1.setHighpass(0,curp.preHP,0.8);
      biquad1.setBiQuadEq(0,'H',curp.preHP,1,0);
      // compute mid peak filter coefficients (formula from http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt)
      double midcoefs1[5];
      //getBiquadPeakCoefs(curp.preMidFreq,curp.preMidQ,curp.preMidGain,midcoefs1);
      //biquad1.setCoefficients(1,midcoefs1);
      biquad1.setBiQuadEq(1,'P',curp.preMidFreq,curp.preMidQ,curp.preMidGain);
      //biquad1.setLowpass(2,curp.preLP,0.8);
      biquad1.setBiQuadEq(2,'L',curp.preLP,1,0);
    }
    if(setPostEq) {
       Serial.println("Set Post EQ");
       // post eq
       biquad2.setHighpass(0,curp.postHP,0.8);
       // compute mid peak filter coefficients (formula from http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt)
       double midcoefs2[5];
       getBiquadPeakCoefs(curp.postMidFreq,curp.postMidQ,curp.postMidGain,midcoefs2);
       biquad2.setCoefficients(1,midcoefs2);
       biquad2.setLowpass(2,curp.postLP,0.8);
    }
    if(setFIR) {
       setInputFIR(curp.preFIR);
       setOutputFIR(curp.postFIR);
    }
    AudioInterrupts();
};

// Gui_update is responsible to detect changes in the interface (graphical menu and hardware) and run the appropriate actions
void Gui_update() {
  static hardwareState prevstate;
  hardwareState curstate;
  static int btnCount=0;
  static uint32_t prevToggle=0;
  bool changed=false;
  bool redraw=false;
  const uint8_t noisefloor=20; 
  static presetState preset;
  presetState newpreset;
  
  // hardware interface update
  getHardwareState(curstate);
  
  // send button press to gui code
  if(curstate.encButton==0) btnCount++;
  if(btnCount>3) {
    btnCount=0;
    AudioNoInterrupts(); 
    mygui.btnpress();
    prevstate.encButton=curstate.encButton;
    AudioInterrupts(); 
  }

  // update encoder 
  long delta=curstate.encoder-prevstate.encoder;
  if (delta!=0) {
    //if(delta>5) delta=5;
    //if(delta<5) delta=-5;
    AudioNoInterrupts(); 
    mygui.change(delta);
    prevstate.encoder=curstate.encoder;
    AudioInterrupts(); 
  } 
//  Serial.print("Switches ");
//  Serial.print(curstate.toggleSwitch);
//  Serial.print(" ");
//  Serial.println(curstate.editSwitch);
  
  // toggle switch : 
  const uint8_t presetMenu=6;
  if(curstate.toggleSwitch!=prevstate.toggleSwitch) {
    uint32_t dt=millis()-prevToggle;
    bool curToggle=curstate.toggleSwitch;
    if((dt>100)&&(dt<700)) { // cycle to next preset and toggle A/B
      //cyclePreset
      Serial.println("Cycle preset");
      int curbank=getW(presetMenu,0);
      int curpreset=floor(getW(presetMenu,1))/2;  // preset number 0,1,2,3
      int newpreset=(curpreset+1)%4;
      int curpresetID=8*curbank+newpreset*2+curToggle;
      setW(presetMenu,1,newpreset*2+curToggle);
      prevToggle=millis();
      loadPreset(curpresetID);
    } else {
      Serial.println("Toggle preset A/B");
      int curbank=getW(presetMenu,0);
      int curpreset=floor(getW(presetMenu,1))/2;  // preset number 0,1,2,3
      int newpreset=curpreset*2+curToggle;
      int curpresetID=8*curbank+newpreset;
      //int newpreset=curpreset+curToggle;
      setW(presetMenu,1,newpreset);
      prevToggle=millis();
      loadPreset(curpresetID);
    }
    prevstate.toggleSwitch=curstate.toggleSwitch;
    changed=true;
    redraw=true;
  }
  
  // edit switch : 
  if(curstate.editSwitch) {
    // if 1 set to preset mode else set to edit mode => force display of preset menu only
    // TODO: improve this "hack" to move this behaviour in the gui object directly
    //if(curstate.editSwitch!=prevstate.editSwitch) {
      const int presetMenuId=7;
      if(mygui.curMenuId!=presetMenuId-1) {
        Serial.println("Edit switch Engaged");
        mygui.curMenuId=presetMenuId-1;
        redraw=true;
        changed=true;
        prevstate.editSwitch=curstate.editSwitch;
      }
    //}
  }
  // pots : 
  if (abs(curstate.pot1-prevstate.pot1)>noisefloor) {
    // send update to gui code
    updatePot(1,curstate.pot1);
//    Serial.print("Pot 1 update: delta=");
//    Serial.print(abs(curstate.pot1-prevstate.pot1));
//    Serial.print(" prev value: ");
//    Serial.print(prevstate.pot1);
//    Serial.print(" current value: ");
//    Serial.println(curstate.pot1);
    changed=true; redraw=true;
    prevstate.pot1=curstate.pot1;
  }
  if (abs(curstate.pot2-prevstate.pot2)>noisefloor) {
    updatePot(2,curstate.pot2);
//    Serial.print("Pot 2 update: delta=");
//    Serial.print(abs(curstate.pot2-prevstate.pot2));
//    Serial.print(" prev value: ");
//    Serial.print(prevstate.pot2);
//    Serial.print(" current value: ");
//    Serial.println(curstate.pot2);
    changed=true; redraw=true;
    prevstate.pot2=curstate.pot2;
  }
  if (abs(curstate.pot3-prevstate.pot3)>noisefloor) {
    updatePot(3,curstate.pot3);
//    Serial.print("Pot 3 update: delta=");
//    Serial.print(abs(curstate.pot3-prevstate.pot3));
//    Serial.print(" prev value: ");
//    Serial.print(prevstate.pot3);
//    Serial.print(" current value: ");
//    Serial.println(curstate.pot3);
    changed=true; redraw=true;
    prevstate.pot3=curstate.pot3;
  }
  if (abs(curstate.pot4-prevstate.pot4)>noisefloor) {
    updatePot(4,curstate.pot4);
//    Serial.print("Pot 4 update: delta=");
//    Serial.print(abs(curstate.pot4-prevstate.pot4));
//    Serial.print(" prev value: ");
//    Serial.print(prevstate.pot4);
//    Serial.print(" current value: ");
//    Serial.println(curstate.pot4);
    changed=true; redraw=true;
    prevstate.pot4=curstate.pot4;
  }
  
  // read Preset Menu state
  getPresetState(newpreset);

  // detect change of active preset in GUI
  if(GUI_Change) {
  // DEBUG
  char txt[130];
  sprintf(txt,"GUI_Change: %1d, Bank: %1d, Preset: %1d, Action: %1d, Target: %2d, States: %1d,%1d,%1d, SW: %1d,%1d",
         GUI_Change,newpreset.bank,newpreset.preset,newpreset.action,newpreset.target,newpreset.bankState,
         newpreset.presetState,newpreset.actionState, curstate.editSwitch, curstate.toggleSwitch);
  Serial.println(txt);
  }
  
  if( GUI_Change && (newpreset.bankState!=0 || newpreset.presetState!=0 )) { 
    // change preset
    // update display:
    Serial.println("Change preset");
    // load preset
    int presetID=newpreset.bank*8+newpreset.preset;
    if(loadPreset(presetID)) { 
      Serial.println("Preset Loaded");
    } else {
      Serial.println("Preset loading failed !");
    }
  }
  
  // update display of preset if needed 
  if(newpreset.preset!=preset.preset  ||  newpreset.bank!=preset.bank || mygui.menus[presetMenu]->widgets[9]->label[0]==' '){
    updatePresetDisplay(newpreset.bank,newpreset.preset);
    redraw=true;
  }
  //  detect activation of action (switching from editing to normal) 
  if( GUI_Change && newpreset.actionState!=0 ) {
    // perform action on preset
    Gui_parameters curp;
    getGUIparameters(curp);
    Serial.print("Preset action handler: action code=");
    Serial.println(newpreset.action);
    switch(newpreset.action) {  // None,   Save  ,  Load  ,  Swap  
      case 0: // none
        break;
      case 1: // save
        if(savePreset(newpreset.target,curp)) {
          Serial.println("Preset Saved");
        } else {
          Serial.println("Preset Save failed!");
        }
        break;
      case 2: // load
        if(loadPreset(newpreset.target)) {
          Serial.println("Preset loaded");
        } else {
          Serial.println("Preset Load failed!");
        }
        break;
      case 3: // swap
        Gui_parameters tmp;
        if(readPreset(newpreset.target,tmp)) {
          savePreset(newpreset.target,tmp);
        }
        break;
    }
    setW(6,7,0);  // reset action widget to None
    newpreset.action=0;
  }
  
  
  // if GUI state has changed , perform the update (marked by global semaphore GUI_change)
  if(changed) {
    Serial.println("Interface: Change detected");
    Gui_validate();
  }
  
  if(GUI_Change) {
    Serial.println("GUI: Change detected");
    Gui_validate();
    GUI_Change=false;
  }
  
  if(redraw) {
    // store state
    Serial.println("Refresh");
    mygui.draw();
  }

  // save current preset for next update
  preset=newpreset;
};


