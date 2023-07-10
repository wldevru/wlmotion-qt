#ifndef WLDCAN_H
#define WLDCAN_H

#include <QObject>
#include <QDataStream>
#include <QTimer>
#include <QDebug>
#include <QStringList>
#include "wlmodule.h"

#define comDCan_empty 0

#define comDCan_setOutputs 1
#define comDCan_setPWM     2  

#define comDCan_setAddress  10
#define comDCan_initInput  11
#define comDCan_initOutput 12
#define comDCan_setEnable  13
#define comDCan_addRemElement 14
#define comDCan_clearAllRemElement 15
#define comDCan_getData 16

#define comDCan_getInputsData      50

#define sendDCan_inputsData 100
//#define sendDCan_error 255

#define DCF_changeInput   1<<0
#define DCF_changeOutput  1<<1
#define DCF_enable        1<<2


#define defTimerTx 100
#define defTimerRx 150

#define error_sizecan    1
#define error_connectcan 2

const QString errorDCan(QT_TR_NOOP("0,no error"
                                   ",1,size CAN"
                                   ",2,error connect"));

class WLModuleDCan;

class WLDCan : public WLElement
{
    Q_OBJECT

public:

    WLDCan(WLModuleDCan *_ModuleDCan);
    ~WLDCan();

private:
    quint8  error=0;
    quint8  flag=0;
    quint16 adrCan=0;
    bool    m_enable=false;

    QList<WLRemElement> remEList;

public:
    quint16 getAdrCan() {return adrCan;}
    bool setAdrCan(quint16 _adrCan);


    void setError(quint8 _error) {emit changedError(error=_error);}

signals:

    void changedError(quint8);

public:

    QList<WLRemElement> getRemElementList() {return remEList;}

    bool addRemoteElement(WLRemElement remE);
    bool clearAllRemoteElement();

    bool setEnable(bool enable);
    bool isEnable() {return m_enable;}

    bool sendGetData();


public:

    virtual void writeXMLData(QXmlStreamWriter &stream);
    virtual void  readXMLData(QXmlStreamReader &stream);
};



#endif // WLDCAN_H
