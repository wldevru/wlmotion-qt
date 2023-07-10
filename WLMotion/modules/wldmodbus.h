#ifndef WLDModbus_H
#define WLDModbus_H

#include <QObject>
#include <QDataStream>
#include <QTimer>
#include <QDebug>
#include <QStringList>
#include "wlmodule.h"

#define comDModbus_callDataM 5

#define comDModbus_setInterfaceM 9

#define comDModbus_setAddress  10
#define comDModbus_setEnable  13
#define comDModbus_addRemElement    14
#define comDModbus_clearAllRemElement 15
#define comDModbus_getData 16

#define sendDModbus_timeoutCallDataM 200
#define sendDModbus_reciveCallDataM 201
/*
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
#define error_connectcan 2*/

const QString errorDModbus(QT_TR_NOOP("0,no error"
                                      ",1,set data"
                                      ",2,error connect"));

class WLModuleDModbus;

class WLDModbus : public WLElement
{
    Q_OBJECT

public:

    WLDModbus(WLModuleDModbus *_ModuleDModbus);
    ~WLDModbus();

private:

    quint8  error=0;
    quint8  flag=0;
    quint8  adrModbus=0;
    bool  m_enable=0;

    QList<WLRemElement> remEList;

public:
    quint8 getAdrModbus() {return adrModbus;}
    bool setAdrModbus(quint8 _adrModbus);

    bool setEnable(bool _enable);
    bool isEnable() {return m_enable;}

    void setError(quint8 _error) {emit changedError(error=_error);}

signals:

    void changedError(quint8);

public:

    QList<WLRemElement> getRemElementList() {return remEList;}

    bool addRemoteElement(WLRemElement remE);
    bool clearAllRemoteElement();

    bool sendGetData();


public:

    virtual void writeXMLData(QXmlStreamWriter &stream);
    virtual void  readXMLData(QXmlStreamReader &stream);
};



#endif // WLDModbus_H
