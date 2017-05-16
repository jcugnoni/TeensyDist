#ifndef __GUI_H 
#define __GUI_H

#include "ILI9341_t3.h"
#include <Encoder.h>
//#include <Bounce.h>

// hardware UI
#define POT1 36
#define POT2 37
#define POT3 38
#define POT4 39
#define SWITCH1 28
#define SWITCH2 27
#define BUTTON 24
#define ENCA 26
#define ENCB 25

// TFT
#define TFT_DC      20
#define TFT_CS      21
#define TFT_RST    255  // 255 = unused, connect to 3.3V
#define TFT_MOSI     7
#define TFT_SCLK    14
#define TFT_MISO    12

// GUI params
#define GUI_ENCODER_DIVIDER 8
#define GUI_NLINES 15
#define GUI_NCOLS 26
#define GUI_CHAR_WIDTH 12
#define GUI_CHAR_HEIGHT 16
#define GUI_TEXT_SIZE 2
#define GUI_WIDTH 320
#define GUI_HEIGHT 240
#define GUI_CENTER_X (GUI_WIDTH/2)
#define GUI_SEP 120
#define GUI_LABEL_LEN 10
#define GUI_BAR_WIDTH (GUI_WIDTH-GUI_SEP)
#define GUI_WIDGET_CENTER_X (GUI_SEP+GUI_BAR_WIDTH/2)
#define GUI_MAX_LIST 10

#define GUI_N_MENUS 10
#define GUI_N_ITEMS 14

// --- types ----
typedef struct {
    int fg;
    int bg;  
}  Style;

typedef enum { NORMAL, SELECTED, EDITING} widgetState;

typedef enum { FLOAT, LIST , BIGTEXT } widgetType;



// ---- classes

 class Widget {
    public:
     char label[GUI_LABEL_LEN];
     widgetState state;
     widgetType type;
     float value;
     char ** listItems;
     int cx=0,cy=0;
     float mini,maxi,step;
     Widget(const char labeltxt[GUI_LABEL_LEN],  float minv, float maxv, float stepv, float init);
     Widget(const char labeltxt[GUI_LABEL_LEN], int n_items, char ** list_items, int deflt);
     Widget(const char txt[GUI_LABEL_LEN]);
     void change(int increment);
     void doConstrain();
     void getText(char txt[GUI_LABEL_LEN]);
     void setLabel(const char labeltxt[GUI_LABEL_LEN]);
     void select();
     void edit();
     void validate();
     void deselect();
     void refresh();
     void draw();
     void draw(int cornerx, int cornery); 
 };
 
 class Menu {
    public:
     char label[GUI_LABEL_LEN];
     int n_items;
     Widget **widgets;
     int selected=0,lastselected=0; 
     bool editing=false;
     bool updated=false; 
     Menu( const char labeltxt[GUI_LABEL_LEN], Widget ** widgetLst, int n);   
     void drawTitle();
     void draw();
     void refresh();
     void select(int sel);
     void switchEdit(bool onoff);
     void changeVal(int inc);
     void changeSel(int inc);
     void change(int inc);
     void toggleEdit();
 };  
 
 class Gui {
   public:
     Menu * menus[GUI_N_MENUS];
     int n_menus;
     int curMenuId;
     Gui();
     Menu* curMenu();
     void nextMenu();
     void prevMenu();
     void add(Menu *menu);
     void refresh();
     void change(int inc);
     void draw();
     void btnpress();
 };



#endif
