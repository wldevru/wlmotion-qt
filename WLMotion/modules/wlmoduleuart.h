#ifndef WLModuleUART_H
#define WLModuleUART_H

#include <QObject>
#include <QDebug>
#include <QStringList>
#include "wlmodule.h"
#include "wluart.h"


class WLModuleUART : public WLModule
{
    Q_OBJECT

public:
    WLModuleUART(WLDevice *_Device);
    ~WLModuleUART();

    bool Init(int sizeUART);

private:
    QList <WLUART*> UART;

public:

    int getSizeUART() {return UART.size();}

    WLUART* getUART(int m_index);

public:

    Q_INVOKABLE void setBaudrate(int index,quint32 _baudrate) {if(index>=UART.size()) {return;} UART[index]->setBaudrate(_baudrate);}
    Q_INVOKABLE void setEnable(int index,bool enable);
    Q_INVOKABLE void transmitData(int index,QString data);

    Q_INVOKABLE void clearRecive(int index);
    Q_INVOKABLE QString getReciveStr(int index,int len=-1);

    Q_INVOKABLE double  getReciveByte(int index,bool unsig=false);
    Q_INVOKABLE double  getRecive2Byte(int index,bool unsig=false);
    Q_INVOKABLE double  getRecive4Byte(int index,bool unsig=false);
    Q_INVOKABLE double  getReciveFloat(int index);
    Q_INVOKABLE double  getReciveDouble(int index);
    //Q_INVOKABLE double  getReciveNum(int index);
    Q_INVOKABLE bool isEmptyRecive(int index);
    //Q_INVOKABLE QString getReciveNum(int index,int type,int n);

signals:
    void changedRecive(int index);

public slots:

    virtual void update();
    virtual void backup();

public:

    virtual void writeXMLData(QXmlStreamWriter &stream);
    virtual void  readXMLData(QXmlStreamReader &stream);
    virtual void readCommand(QByteArray data);
};

#endif // WLModuleUART_H

