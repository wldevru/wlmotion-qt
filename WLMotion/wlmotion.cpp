#include <QMetaEnum>
#include "wlmotion.h"

WLMotion::WLMotion()
{

}

WLMotion::~WLMotion()
{

}

WLModule *WLMotion::createModule(QString name)
{
    WLModule *Module=WLDevice::createModule(name);

    if(Module==nullptr)
    {
        if(name=="WLModuleDModbus") return createModule(typeMDModbus);
        if(name=="WLModuleDCan")    return createModule(typeMDCan);
        if(name=="WLModuleIOPut")   return createModule(typeMIOPut);
        if(name=="WLModuleAIOPut")  return createModule(typeMAIOPut);
        if(name=="WLModuleEncoder") return createModule(typeMEncoder);
        if(name=="WLModuleWhell"
                ||name=="WLModuleMPG")     return createModule(typeMMPG);
        if(name=="WLModulePlanner") return createModule(typeMPlanner);
        if(name=="WLModuleAxis")    return createModule(typeMAxis);
        if(name=="WLModulePWM")     return createModule(typeMPWM);
        if(name=="WLModuleUART")    return createModule(typeMUART);
        if(name=="WLModuleSpindle") return createModule(typeMSpindle);
        if(name=="WLModuleOscp")    return createModule(typeMOscp);

        bool ok;

        WLDevice::typeModule type=static_cast<WLDevice::typeModule>(QMetaEnum::fromType<WLDevice::typeModule>().keyToValue(name.toUtf8(),&ok));

        if(ok)
            return createModule(type);
    }

    return Module;
}

WLModule *WLMotion::createModule(typeModule type)
{
    WLModule *Module=WLDevice::createModule(type);

    if(Module==nullptr)
    {
        switch(type)
        {
        case typeMDModbus:Module=new WLModuleDModbus(this);
            break;

        case typeMDCan:   Module=new WLModuleDCan(this);
            break;

        case typeMIOPut:  Module=new WLModuleIOPut(this);
            break;

        case typeMAIOPut: Module=new WLModuleAIOPut(this);
            break;

        case typeMMPG:    Module=new WLModuleMPG(this);
            break;

        case typeMEncoder:Module=new WLModuleEncoder(this);
            break;

        case typeMPlanner:if(createModule(WLDevice::typeMIOPut)
                             &&createModule(WLDevice::typeMSpindle)){
                Module=new WLModulePlanner(this);
            }
            break;

        case typeMSpindle:if(createModule(WLDevice::typeMIOPut)){
                Module=new WLModuleSpindle(this);
            }
            break;

        case typeMOscp:   Module=new WLModuleOscp(this);
            break;

        case typeMAxis:   if(createModule(WLDevice::typeMIOPut)){
                Module=new WLModuleAxis((this));
            }
            break;

        case typeMPWM:    Module=new WLModulePWM(this);
            break;

        case typeMUART:   Module=new WLModuleUART(this);
            break;

        default: break;
        }

        addModule(Module);
    }

    return Module;
}



bool WLMotion::getIn(int index)
{
    if(getModuleIOPut()
            &&getModuleIOPut()->getInput(index))
        return getModuleIOPut()->getInput(index)->getNow();

    return false;
}

bool WLMotion::getOut(int index)
{
    if(getModuleIOPut()
            &&getModuleIOPut()->getOutput(index))
        return getModuleIOPut()->getOutput(index)->getNow();

    return false;
}

void WLMotion::setOut(int index, bool set)
{
    if(getModuleIOPut()
            &&getModuleIOPut()->getOutput(index))
        getModuleIOPut()->getOutput(index)->setOut(set);
}

void WLMotion::setOutPulse(int index, bool set, quint32 time)
{
    if(getModuleIOPut()
            &&getModuleIOPut()->getOutput(index))
        getModuleIOPut()->getOutput(index)->setOutPulse(set,time);
}

void WLMotion::setOutTog(int index)
{
    if(getModuleIOPut()
            &&getModuleIOPut()->getOutput(index))
        getModuleIOPut()->getOutput(index)->setTog();
}

void WLMotion::setAOut(int index, float value)
{
    if(getModuleAIOPut()
            &&getModuleAIOPut()->getOutput(index))
        getModuleAIOPut()->getOutput(index)->setValue(value);
}

float WLMotion::getAOut(int index)
{
    if(getModuleAIOPut()
            &&getModuleAIOPut()->getOutput(index))
        return getModuleAIOPut()->getOutput(index)->getValue();

    return 0.0;
}

float WLMotion::getAIn(int index)
{
    if(getModuleAIOPut()
            &&getModuleAIOPut()->getInput(index))
        return getModuleAIOPut()->getInput(index)->getValue();

    return 0.0;
}
