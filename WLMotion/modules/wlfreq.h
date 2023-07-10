#ifndef WLFREQ_H
#define WLFREQ_H

#include <QObject>
#include <QDataStream>
#include <QTimer>
#include <QDebug>

#include "wlmodule.h"


#define PWMF_enable 1<<0 
#define PWMF_lock   1<<1
#define PWMF_inv    1<<2

//Freq connect
#define comFreq_setOut       1
#define comFreq_setEnableOut 2
#define comFreq_setMaxOut   4

#define comFreq_getDataOut 100

#define sendFreq_dataOut 200

const QString errorFreq(QT_TR_NOOP("0,no error"));

class WLModuleFreq;

class WLFreq : public WLElement
{
    Q_OBJECT

public:

    WLFreq(WLModuleFreq *_ModuleFreq);
    ~WLFreq();

private:

    float Freq;
    quint8 error;
    quint8 flag;

public:
    void setData(float F) {Freq=F;}

    float getFreq() {return Freq;}

    void setError(quint8 err)  {emit changedError(error=err);}

signals:

    void changedError(quint8);
    void changedFreq(float);

public:

    bool setOut(float F);
    bool setEnable(bool enable);
    bool setMaxOut(float Fmax);

    bool sendGetData();


public:

    //virtual void writeXMLData(QXmlStreamWriter &stream);
    //virtual void  readXMLData(QXmlStreamReader &stream);
};



#endif // WLFREQ_H
