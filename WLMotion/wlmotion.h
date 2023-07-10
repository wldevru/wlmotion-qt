#ifndef WLMotion_H
#define WLMotion_H

#include "wldevice.h"

#include "modules/wlmoduledcan.h"
#include "modules/wlmoduleioput.h"
#include "modules/wlmoduleaioput.h"
#include "modules/wlmoduleencoder.h"
#include "modules/wlmoduleaxis.h"
#include "modules/wlmodulempg.h"
#include "modules/wlmoduleplanner.h"
#include "modules/wlmodulepwm.h"
#include "modules/wlmoduleuart.h"
#include "modules/wlmoduledmodbus.h"
#include "modules/wlmodulespindle.h"
#include "modules/wlmoduleoscp.h"

class WLMotion: public WLDevice
{
    Q_OBJECT

public:

    WLMotion();
    ~WLMotion();

public:
    WLModulePlanner* getModulePlanner() {return static_cast<WLModulePlanner*>(getModule(typeMPlanner));}
    WLModuleOscp*    getModuleOscp()    {return static_cast<WLModuleOscp*>(getModule(typeMOscp));}
    WLModuleSpindle* getModuleSpindle() {return static_cast<WLModuleSpindle*>(getModule(typeMSpindle));}
    WLModuleAxis*    getModuleAxis()    {return static_cast<WLModuleAxis*>(getModule(typeMAxis));}
    WLModuleIOPut*   getModuleIOPut()   {return static_cast<WLModuleIOPut*>(getModule(typeMIOPut));}
    WLModuleAIOPut*  getModuleAIOPut()  {return static_cast<WLModuleAIOPut*>(getModule(typeMAIOPut));}
    WLModuleMPG*     getModuleMPG()     {return static_cast<WLModuleMPG*>(getModule(typeMMPG));}
    WLModulePWM*     getModulePWM()     {return static_cast<WLModulePWM*>(getModule(typeMPWM));}
    WLModuleEncoder* getModuleEncoder() {return static_cast<WLModuleEncoder*>(getModule(typeMEncoder));}
    WLModuleUART*    getModuleUART()    {return static_cast<WLModuleUART*>(getModule(typeMUART));}
    WLModuleDCan*    getModuleDCan()    {return static_cast<WLModuleDCan*>(getModule(typeMDCan));}
    WLModuleDModbus* getModuleDModbus() {return static_cast<WLModuleDModbus*>(getModule(typeMDModbus));}

public:
    virtual WLModule *createModule(QString name);
    virtual WLModule *createModule(typeModule getTypeModule);

public:
    bool getIn(int index);
    bool getOut(int index);
    void setOut(int index, bool set);
    void setOutPulse(int index, bool set, quint32 time);
    void setOutTog(int index);

    void  setAOut(int index, float value);
    float getAIn(int index);
    float getAOut(int index);
};

#endif //WLMOTION_H
