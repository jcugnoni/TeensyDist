
#include "ILI9341_t3.h"
#include "gui.h"
#include <Encoder.h>
#include <Bounce.h>

 // GLOBAL objects

 extern ILI9341_t3 tft;
 extern Encoder myEnc;
 extern bool GUI_Change;
 
// --- styles ----
Style MenuStyle={ILI9341_WHITE, ILI9341_DARKGREEN};
Style MenuStyleSelect={ILI9341_WHITE, ILI9341_GREEN};
Style MenuStyleEdit={ILI9341_WHITE, ILI9341_BLUE};

Style StyleLabelNormal={ILI9341_WHITE,ILI9341_BLACK};
Style StyleValueBarNormal={ILI9341_BLUE,ILI9341_BLACK};
Style StyleValueTxtNormal={ILI9341_LIGHTGREY,ILI9341_BLACK};

Style StyleLabelSelect={ILI9341_BLACK,ILI9341_DARKGREY};
Style StyleValueBarSelect={ILI9341_BLUE,ILI9341_DARKGREY};
Style StyleValueTxtSelect={ILI9341_BLACK,ILI9341_DARKGREY};

Style StyleLabelEdit={ILI9341_BLACK,ILI9341_ORANGE};
Style StyleValueBarEdit={ILI9341_RED,ILI9341_DARKGREY};
Style StyleValueTxtEdit={ILI9341_BLACK,ILI9341_DARKGREY};

//   avoid using pins with LEDs attached

// --- class Widget implementation
 
    Widget::Widget(const char labeltxt[GUI_LABEL_LEN],  float minv, float maxv, float stepv, float init) {
      type=FLOAT; 
      state=NORMAL;
      mini=minv; 
      maxi=maxv;
      step=stepv; 
      value=init; 
      strncpy(label,labeltxt,GUI_LABEL_LEN); 
    }; 
    
    Widget::Widget(const char txt[GUI_LABEL_LEN]) {
      type=BIGTEXT; 
      state=NORMAL;
      strncpy(label,txt,GUI_LABEL_LEN); 
    };
    
    Widget::Widget( const char labeltxt[GUI_LABEL_LEN], int n_items, char ** list_items, int deflt) {
      type=LIST; 
      state=NORMAL; 
      mini=0; 
      maxi=n_items-1;
      step=1.0/GUI_ENCODER_DIVIDER;
      value=deflt;
      listItems=list_items;
      strncpy(label,labeltxt,GUI_LABEL_LEN);
      };

    void Widget::setLabel(const char labeltxt[GUI_LABEL_LEN]) {
      strncpy(label,labeltxt,GUI_LABEL_LEN);
    };
    
    void Widget::change(int increment) { 
      value+=increment*step; 
      doConstrain(); 
      refresh();
     };
    
    void Widget::doConstrain() {
      if(value>maxi) value=maxi; 
      if(value<mini) value=mini;
      };
    
    void Widget::getText(char txt[GUI_LABEL_LEN]) {
      doConstrain(); 
      if(type==LIST) { 
        int index=floor(value); 
        char *listitem=listItems[index];
        strncpy(txt,listitem,GUI_LABEL_LEN); 
       } 
     };
    
    void Widget::select() {
      if(type!=BIGTEXT) {
        state=SELECTED; 
        refresh();
      }
    };
    
    void Widget::edit() {
      if(type!=BIGTEXT) {
        state=EDITING; 
        refresh();
      }
    };
    
    void Widget::validate() {
      if(type!=BIGTEXT) {
         state=SELECTED; 
         GUI_Change=true;
      }
      refresh();
    };
    
    void Widget::deselect() {
      if(type!=BIGTEXT) {
        state=NORMAL; 
        refresh();
      }
    };
    
    void Widget::refresh() { 
      draw(); 
      };
    
    void Widget::draw() {
      draw(cx,cy);
      };
    
    void Widget::draw(int cornerx, int cornery) {
        cx=cornerx; cy=cornery;
        tft.setCursor(cornerx,cornery);
        Style styleLbl,styleBar,styleVal;
        switch(state) {
          case NORMAL:
           styleLbl=StyleLabelNormal; styleBar=StyleValueBarNormal; styleVal=StyleValueTxtNormal; break;
          case SELECTED:
           styleLbl=StyleLabelSelect; styleBar=StyleValueBarSelect; styleVal=StyleValueTxtSelect; break;
          case EDITING:
           styleLbl=StyleLabelEdit; styleBar=StyleValueBarEdit; styleVal=StyleValueTxtEdit; break;
          default:
           styleLbl=StyleLabelNormal; styleBar=StyleValueBarNormal; styleVal=StyleValueTxtNormal; break;
        }
        // LABEL: clear bg and print
        if(type!=BIGTEXT) {
          tft.fillRect(cornerx,cornery,GUI_SEP,GUI_CHAR_HEIGHT,styleLbl.bg);
          tft.setTextSize(GUI_TEXT_SIZE);
          tft.setTextColor(styleLbl.fg, styleLbl.bg);
          tft.print(label);
        } else {
          tft.fillRect(cornerx,cornery,GUI_WIDTH,GUI_CHAR_HEIGHT*5,ILI9341_BLUE);
          tft.setTextSize(GUI_TEXT_SIZE*5);
          tft.setTextColor(ILI9341_WHITE,ILI9341_BLUE );
          tft.setCursor(cornerx+5,cornery+5);
          tft.print(label);
          tft.setTextSize(GUI_TEXT_SIZE);
        }
        // BAR
        if(type==FLOAT) {
           tft.fillRect(cornerx+GUI_SEP,cornery,GUI_BAR_WIDTH,GUI_CHAR_HEIGHT,styleBar.bg);
           tft.fillRect(cornerx+GUI_SEP,cornery,floor(GUI_BAR_WIDTH*(value-mini)/(maxi-mini)),GUI_CHAR_HEIGHT,styleBar.fg);
           tft.setTextSize(GUI_TEXT_SIZE);
           tft.setTextColor(styleVal.fg);
           tft.setCursor(cornerx+GUI_SEP+GUI_BAR_WIDTH/2-2*GUI_CHAR_WIDTH,cornery);
           tft.print(value);
        } else if(type==LIST) {  // LIST: print item
           tft.fillRect(cornerx+GUI_SEP,cornery,GUI_BAR_WIDTH,GUI_CHAR_HEIGHT,styleBar.bg);
           //tft.fillRect(cornerx+GUI_SEP,cornery,floor(GUI_BAR_WIDTH*(value-mini)/(maxi-mini)),GUI_CHAR_HEIGHT,styleBar.fg);
           tft.setTextSize(GUI_TEXT_SIZE);
           tft.setTextColor(styleVal.fg);
           tft.setCursor(cornerx+GUI_SEP+GUI_BAR_WIDTH/2-4*GUI_CHAR_WIDTH,cornery);
           char txt[GUI_LABEL_LEN];
           getText(txt);
           tft.print(txt);
        } 
        };

// MENU implementation

    //Menu::Menu(char labeltxt[GUI_LABEL_LEN]) { strncpy(labeltxt,label,GUI_LABEL_LEN);};   

    Menu::Menu(const char labeltxt[GUI_LABEL_LEN], Widget ** widgetLst, int n) { 
      strncpy(label,labeltxt,GUI_LABEL_LEN); 
      n_items=min(n,GUI_N_ITEMS); 
      widgets=widgetLst; 
     };   
    
    void Menu::drawTitle() { // full redraw of title bar
        // Draw menu label
        Style style=MenuStyle;
        if(selected<0) style=MenuStyleSelect;
        if(selected<0 && editing) style=MenuStyleEdit;
        tft.fillRect(0,0,GUI_WIDTH,GUI_CHAR_HEIGHT,style.bg);
        tft.setCursor(GUI_CENTER_X-5*GUI_CHAR_WIDTH,0);
        tft.setTextColor(style.fg,style.bg);
        tft.print(label);
    };
    
    void Menu::draw() { // full redraw, for example when menu changes
        // Draw menu label
         drawTitle();
        // Draw each widget in order
        for(int ii=0; ii<n_items; ii++) {
          int cornerx=0;
          int cornery=GUI_CHAR_HEIGHT*(ii+1);
          Widget *w=widgets[ii];
          w->draw(cornerx,cornery);
        }
    };
    
    void Menu::refresh() { // partial redraw; redraws previously selected item and current selection
       if(lastselected!=selected) {
        if(lastselected<0) {
          drawTitle();
        } else {
          Widget *w=widgets[lastselected];
          w->refresh();
        }
       }
       if(selected<0) {
          drawTitle();
        } else {
          Widget *w=widgets[selected];
          w->refresh();
        }
    };
    
    void Menu::select(int sel) {
      lastselected=selected; 
      selected=constrain(sel,-1,n_items-1);
      if(lastselected>=0) widgets[lastselected]->deselect(); 
      if(selected>=0) widgets[selected]->select(); 
      refresh();
      };
    
    void Menu::switchEdit(bool onoff) {
      editing=onoff; 
      if(selected>=0 && editing) widgets[selected]->edit(); 
      if(selected>=0 && !editing) widgets[selected]->validate(); 
      updated=true; 
      refresh();
      };
    
    //void Menu::addWidget(Widget *w) { if (n_items<GUI_N_ITEMS-1) { widgets[n_items]=w; n_items++; }; }; // untested
    
    // Widget * Menu::curWidget() { if (selected>=0 && selected <n_items) {return widgets[selected]; } else {return NULL;} }; //not tested
    
    void Menu::changeVal(int inc) {
      if(selected>=0) widgets[selected]->change(inc); 
      };
    
    void Menu::changeSel(int inc) { 
      lastselected=selected; 
      select(constrain(selected+inc,-1,n_items-1));
      };
    
    void Menu::change(int inc) {
      if(editing) { 
        changeVal(inc); 
      } else {
        changeSel(-inc); 
      } 
     };
    
    void Menu::toggleEdit() {
      switchEdit(!editing); 
    };
    

// GUI implementation

    Gui::Gui() {
      n_menus=0; 
      curMenuId=-1;
    };
    
    Menu* Gui::curMenu() {
      //if(curMenuId<0 || (curMenuId > (n_menus-1))) return NULL;
      return menus[curMenuId]; 
     };
    
    void Gui::nextMenu() {
      if(curMenuId==n_menus-1) return; 
      curMenuId=min(max(curMenuId+1,0),n_menus-1);//=(curMenuId+1) % n_menus;
      Serial.println(curMenuId);
      delay(2);
      tft.fillScreen(ILI9341_BLACK);
      draw(); 
    };
    
    void Gui::prevMenu() {
      if(curMenuId==0) return; 
      curMenuId=min(max(curMenuId-1,0),n_menus-1);
      Serial.println(curMenuId);
      delay(2);
      tft.fillScreen(ILI9341_BLACK);
      draw(); 
    };
    
    void Gui::add( Menu * menu) {
      if (n_menus>=GUI_N_MENUS) return; 
      menus[n_menus]=menu; 
      n_menus++; 
      if(curMenuId<0) curMenuId=0;
    };
    
    void Gui::refresh() { 
      if(curMenuId<0 || curMenuId>n_menus-1 || curMenu()==NULL) return;
      this->curMenu()->refresh();
    };
    
    void Gui::draw() { 
      if(curMenuId<0 || curMenuId>n_menus-1 || curMenu()==NULL) return; 
      Serial.println("Draw");
      Serial.println(curMenuId);
      this->curMenu()->draw(); 
    };
    
    void Gui::change(int inc) { 
      Menu * m = curMenu();
      if(m==NULL) return;
      if(m->selected<0 && m->editing){
          if(inc>1) {
            //for(uint8_t i=0;i<inc/2;i++) 
            nextMenu();
          }
          if(inc<1)  {
            //for(uint8_t i=0;i<(-inc)/2;i++) 
            prevMenu();
          }
      } else { // selection mode
        m->change(inc);
      }
      delay(2);
    };
    
    void Gui::btnpress() { 
      if(curMenu()!=NULL) curMenu()->toggleEdit(); 
    };

