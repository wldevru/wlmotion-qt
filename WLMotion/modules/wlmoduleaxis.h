#ifndef WLMODULEAXIS_H
#define WLMODULEAXIS_H

#include <QObject>
#include <QDataStream>
#include <QTimer>
#include "wlaxis.h"
#include "wlmodule.h"
#include "wlmoduleioput.h"


enum typeInputMAXIS{MAXIS_inEMGStop,MAXIS_inSDStop};

class WLModuleAxis : public WLModule
{
    Q_OBJECT

public:
    enum FlagMAxis{
        //  MAF_latchProbe2=1<<0,
        //  MAF_latchProbe3=1<<1
    };

public:
    WLModuleAxis(WLDevice *_Device);
    ~WLModuleAxis();


    void resetAllAxis() {for(int i=0;i<Axis.size();i++) Axis[i]->reset();}

    bool Init(int size);

    WLAxis *getAxis(int m_index);

private:
    QList <WLAxis*> Axis;

private:
    WLIOPut *inEMGStop;
    WLIOPut *inSDStop;

public:
    WLIOPut *getInput(typeInputMAXIS getTypeModule);

    void setInEMGStop(int m_index);
    void setInSDStop(int m_index);

    int getSizeAxis() {return Axis.size();}

private:
    WLFlags Flags;
    float Fmax;

public:
    Q_INVOKABLE	 float getFmax()   {return Fmax;}

private slots:

    void callDataAxis();
    void callTrackAxis();

public slots:
    virtual void update();
    virtual void backup();

public:
    bool setInputMAxis(typeIOPutAXIS getTypeModule,quint8 num);

    Q_INVOKABLE void addSumAxis(quint8 iA,quint8 iAsum,float k=1.0);
    Q_INVOKABLE void clearSumAxis(quint8 iA);
    Q_INVOKABLE void trackPosAxis(quint8 iA,long pos,float F,quint8 mask=MASK_abs);
    Q_INVOKABLE void setPosAxis(quint8 iA,long pos);

signals:
    void changedInEMGStop();
    void changedInSDStop();


public:

    virtual void writeXMLData(QXmlStreamWriter &stream);
    virtual void  readXMLData(QXmlStreamReader &stream);
    virtual void  readCommand(QByteArray data);
};

#endif // WLMODULEAXIS_H
