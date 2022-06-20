// =====================================================
// TouchScreen.cpp
// Author: Richard Benear 2022

#include "TouchScreen.h"
#include "../../../lib/tasks/OnTask.h"
#include "../screens/AlignScreen.h"
#include "../screens/CatalogScreen.h"
#include "../screens/DCFocuserScreen.h"
#include "../screens/GotoScreen.h"
#include "../screens/GuideScreen.h"
#include "../screens/HomeScreen.h"
#include "../screens/MoreScreen.h"
#include "../screens/PlanetsScreen.h"
#include "../screens/SettingsScreen.h"
#include "../screens/ExtStatusScreen.h"

#ifdef ODRIVE_MOTOR_PRESENT
  #include "../screens/ODriveScreen.h"
#endif

unsigned char __task_postpone;
void touchWrapper() { touchScreen.touchScreenPoll(display.currentScreen); }

void TouchScreen::init() {
// Start TouchScreen
  if (!ts.begin()) {
    VLF("MSG: TouchScreen, unable to start");
  } else {
    pinMode(TS_IRQ, INPUT_PULLUP); // XPT2046 library doesn't turn on pullup
    ts.setRotation(3); // touchscreen rotation
    VLF("MSG: TouchScreen, started");
  }

  // start touchscreen task
  VF("MSG: Setup, start TouchScreen polling task (rate 300 ms priority 7)... ");
  uint8_t TShandle = tasks.add(400, 0, true, 7, touchWrapper, "TouchScreen");
  if (TShandle)  { VLF("success"); } else { VLF("FAILED!"); }
  tasks.setTimingMode(TShandle, TM_MINIMUM);
}

// Poll the TouchScreen
void TouchScreen::touchScreenPoll(Screen tCurScreen) {
  if (ts.touched()) {
    //if (screenTouched) return; // effectively a debounce

    // tell other screens to process button states...they will clear this flag when done
    screenTouched = true;
    p = ts.getPoint();      

    // Scale from ~0->4000 to tft.width using the calibration #'s
    // VF("x="); V(p.x); VF(", y="); V(p.y); VF(", z="); VL(p.z); // for calibration
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    // VF("x="); V(p.x); VF(", y="); V(p.y); VF(", z="); VL(p.z); //for calibration
    

    // *************** MENU MAP ****************
    // Current Screen |Cur |Col1|Col2|Col3|Col4|
    // Home-----------| Ho | Gu | Fo | GT | Mo |
    // Guide----------| Gu | Ho | Fo | Al | Mo |
    // Focuser--------| Fo | Ho | Gu | GT | Mo |
    // GoTo-----------| GT | Ho | Fo | Gu | Mo |

    //if ODRIVE_PRESENT then use this menu structure
    //  Current Screen |Cur |Col1|Col2|Col3|Col4|
    //  More & (CATs)--| Mo | GT | Se | Od | Al |
    //  ODrive---------| Od | Ho | Se | Al | Xs |
    //  Extended Status| Xs | Ho | Se | Al | Od |
    //  Settings-------| Se | Ho | Xs | Al | Od |
    //  Alignment------| Al | Ho | Fo | Gu | Od |
    //else if not ODRIVE_PRESENT use this menu structure
    //  More & (CATs)--| Mo | GT | Se | Gu | Al |
    //  Extended Status| Xs | Ho | Se | Al | Mo |
    //  Settings-------| Se | Ho | Xs | Al | Mo |
    //  Alignment------| Al | Ho | Fo | Gu | Mo |

    // Detect which Menu Screen is requested
    // skip checking these page menus since they don't have this menu setup
    if ((tCurScreen == CATALOG_SCREEN) || 
        (tCurScreen == PLANETS_SCREEN) ||  
        (tCurScreen == CUST_CAT_SCREEN)) return; 
    
    // Check for any Menu buttons pressed
    // == LeftMost Menu Button ==
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X                   ) && p.x < (MENU_X                    + MENU_BOXSIZE_X)) {
      BEEP;
      switch(tCurScreen) { 
        case HOME_SCREEN:     setCurrentScreen(GUIDE_SCREEN); guideScreen.draw(); break;
        case GUIDE_SCREEN:    setCurrentScreen(HOME_SCREEN);  homeScreen.draw();  break;
        case FOCUSER_SCREEN:  setCurrentScreen(HOME_SCREEN);  homeScreen.draw();  break;
        case GOTO_SCREEN:     setCurrentScreen(HOME_SCREEN);  homeScreen.draw();  break;
        //==============================================
      #ifdef ODRIVE_MOTOR_PRESENT
        case MORE_SCREEN:     setCurrentScreen(GOTO_SCREEN);  gotoScreen.draw();  break;
        case ODRIVE_SCREEN:   setCurrentScreen(HOME_SCREEN);  homeScreen.draw();  break;
        case XSTATUS_SCREEN:  setCurrentScreen(HOME_SCREEN);  homeScreen.draw();  break;
        case SETTINGS_SCREEN: setCurrentScreen(HOME_SCREEN);  homeScreen.draw();  break;
        case ALIGN_SCREEN:    setCurrentScreen(HOME_SCREEN);  homeScreen.draw();  break;
      #else
        case MORE_SCREEN:     setCurrentScreen(GOTO_SCREEN);  gotoScreen.draw();  break;
        case XSTATUS_SCREEN:  setCurrentScreen(HOME_SCREEN);  homeScreen.draw();  break;
        case SETTINGS_SCREEN: setCurrentScreen(HOME_SCREEN);  homeScreen.draw();  break;
        case ALIGN_SCREEN:    setCurrentScreen(HOME_SCREEN);  homeScreen.draw();  break;
      #endif
        default: setCurrentScreen(HOME_SCREEN); homeScreen.draw();
      }
      screenTouched = false;
      return;
    }
    // == Center Left Menu - Column 2 ==
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X +   MENU_X_SPACING) && p.x < (MENU_X +   MENU_X_SPACING + MENU_BOXSIZE_X)) {
      BEEP;
      switch(tCurScreen) {
        case HOME_SCREEN:     setCurrentScreen(FOCUSER_SCREEN); dCfocuserScreen.draw();  break;
        case GUIDE_SCREEN:    setCurrentScreen(FOCUSER_SCREEN); dCfocuserScreen.draw();  break;
        case FOCUSER_SCREEN:  setCurrentScreen(GUIDE_SCREEN);   guideScreen.draw();      break;
        case GOTO_SCREEN:     setCurrentScreen(FOCUSER_SCREEN); dCfocuserScreen.draw();  break;
        //==================================================
      #ifdef ODRIVE_MOTOR_PRESENT
        case MORE_SCREEN:     setCurrentScreen(SETTINGS_SCREEN); settingsScreen.draw();  break;
        case ODRIVE_SCREEN:   setCurrentScreen(SETTINGS_SCREEN); settingsScreen.draw();  break;
        case XSTATUS_SCREEN:  setCurrentScreen(SETTINGS_SCREEN); settingsScreen.draw();  break;
        case SETTINGS_SCREEN: setCurrentScreen(XSTATUS_SCREEN);  extStatusScreen.draw(); break;
        case ALIGN_SCREEN:    setCurrentScreen(FOCUSER_SCREEN);  dCfocuserScreen.draw(); break;
      #else
        case MORE_SCREEN:     setCurrentScreen(SETTINGS_SCREEN); settingsScreen.draw();  break;
        case XSTATUS_SCREEN:  setCurrentScreen(SETTINGS_SCREEN); settingsScreen.draw();  break;
        case SETTINGS_SCREEN: setCurrentScreen(XSTATUS_SCREEN);  extStatusScreen.draw(); break;
        case ALIGN_SCREEN:    setCurrentScreen(FOCUSER_SCREEN);  dCfocuserScreen.draw(); break;
      #endif
        default: setCurrentScreen(HOME_SCREEN); homeScreen.draw();
      }
      screenTouched = false;
      return;
    }
    // == Center Right Menu - Column 3 ==
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X + 2*MENU_X_SPACING) && p.x < (MENU_X + 2*MENU_X_SPACING + MENU_BOXSIZE_X)) {
      BEEP;
      switch(tCurScreen) {
        case HOME_SCREEN:      setCurrentScreen(GOTO_SCREEN);   gotoScreen.draw();   break;
        case GUIDE_SCREEN:     setCurrentScreen(ALIGN_SCREEN);  alignScreen.draw();  break;
        case FOCUSER_SCREEN:   setCurrentScreen(GOTO_SCREEN);   gotoScreen.draw();   break;
        case GOTO_SCREEN:      setCurrentScreen(GUIDE_SCREEN);  guideScreen.draw();  break;
        //==============================================
      #ifdef ODRIVE_MOTOR_PRESENT
        case MORE_SCREEN:      setCurrentScreen(ODRIVE_SCREEN); oDriveScreen.draw(); break;
        case ODRIVE_SCREEN:    setCurrentScreen(ALIGN_SCREEN);  alignScreen.draw();  break;
        case XSTATUS_SCREEN:   setCurrentScreen(ALIGN_SCREEN);  alignScreen.draw();  break;
        case SETTINGS_SCREEN:  setCurrentScreen(ALIGN_SCREEN);  alignScreen.draw();  break;
        case ALIGN_SCREEN:     setCurrentScreen(GUIDE_SCREEN);  guideScreen.draw();  break;
      #else
        case MORE_SCREEN:      setCurrentScreen(GUIDE_SCREEN);  guideScreen.draw();  break;
        case XSTATUS_SCREEN:   setCurrentScreen(ALIGN_SCREEN);  alignScreen.draw();  break;
        case SETTINGS_SCREEN:  setCurrentScreen(ALIGN_SCREEN);  alignScreen.draw();  break;
        case ALIGN_SCREEN:     setCurrentScreen(GUIDE_SCREEN);  guideScreen.draw();  break;
      #endif
        default: setCurrentScreen(HOME_SCREEN); homeScreen.draw();
      }
      screenTouched = false;
      return;
    }
    // == Right Menu - Column 4 ==
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X + 3*MENU_X_SPACING) && p.x < (MENU_X + 3*MENU_X_SPACING + MENU_BOXSIZE_X)) {
      BEEP;   
      switch(tCurScreen) {
        case HOME_SCREEN:     setCurrentScreen(MORE_SCREEN);    moreScreen.draw();     break;
        case GUIDE_SCREEN:    setCurrentScreen(MORE_SCREEN);    moreScreen.draw();     break;
        case FOCUSER_SCREEN:  setCurrentScreen(MORE_SCREEN);    moreScreen.draw();     break;
        case GOTO_SCREEN:     setCurrentScreen(MORE_SCREEN);    moreScreen.draw();     break;
        //==============================================
      #ifdef ODRIVE_MOTOR_PRESENT
        case MORE_SCREEN:     setCurrentScreen(ALIGN_SCREEN);   alignScreen.draw();     break;
        case ODRIVE_SCREEN:   setCurrentScreen(XSTATUS_SCREEN); extStatusScreen.draw(); break;
        case XSTATUS_SCREEN:  setCurrentScreen(ODRIVE_SCREEN);  oDriveScreen.draw();    break;
        case SETTINGS_SCREEN: setCurrentScreen(ODRIVE_SCREEN);  oDriveScreen.draw();    break;
        case ALIGN_SCREEN:    setCurrentScreen(ODRIVE_SCREEN);  oDriveScreen.draw();    break;
      #else 
        case MORE_SCREEN:     setCurrentScreen(ALIGN_SCREEN);   alignScreen.draw();     break;
        case XSTATUS_SCREEN:  setCurrentScreen(MORE_SCREEN);    moreScreen.draw();      break;
        case SETTINGS_SCREEN: setCurrentScreen(MORE_SCREEN);    moreScreen.draw();      break;
        case ALIGN_SCREEN:    setCurrentScreen(MORE_SCREEN);    moreScreen.draw();      break;
      #endif
        default: setCurrentScreen(HOME_SCREEN); homeScreen.draw();
      }
      screenTouched = false;
      return;
    }

    // Check for touchscreen button action on the selected Screen
    // This does not include the Menu Buttons
    switch (tCurScreen) {
      case HOME_SCREEN:     
        if (homeScreen.touchPoll(p.x, p.y)) homeScreen.updateHomeButtons();             break;       
      case GUIDE_SCREEN:       
        if (guideScreen.touchPoll(p.x, p.y)) guideScreen.updateGuideButtons();          break;        
      case FOCUSER_SCREEN:  
        if (dCfocuserScreen.touchPoll(p.x, p.y)) dCfocuserScreen.updateFocuserButtons(); break; 
      case GOTO_SCREEN:      
        if (gotoScreen.touchPoll(p.x, p.y)) gotoScreen.updateGotoButtons();             break;          
      case MORE_SCREEN:       
        if (moreScreen.touchPoll(p.x, p.y)) moreScreen.updateMoreButtons();             break;          
      case SETTINGS_SCREEN:  
        if (settingsScreen.touchPoll(p.x, p.y)) settingsScreen.updateSettingsButtons(); break;
      case ALIGN_SCREEN:     
        if (alignScreen.touchPoll(p.x, p.y)) alignScreen.updateAlignButtons();          break;     
      case CATALOG_SCREEN:   
        if (catalogScreen.touchPoll(p.x, p.y)) catalogScreen.updateCatalogButtons();    break; 
      case PLANETS_SCREEN:   
        if (planetsScreen.touchPoll(p.x, p.y)) planetsScreen.updatePlanetsButtons();    break;
      case XSTATUS_SCREEN: break;

      #ifdef ODRIVE_MOTOR_PRESENT
        case ODRIVE_SCREEN:  
          if (oDriveScreen.touchPoll(p.x, p.y)) oDriveScreen.updateOdriveButtons();     break;
      #endif

      default: VLF("touchscreen error");
    }
    screenTouched = false;
  } 
}

TouchScreen touchScreen;