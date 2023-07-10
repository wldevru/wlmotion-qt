#ifndef WLModuleDModbus_H
#define WLModuleDModbus_H

#include <QObject>
#include <QDebug>
#include <QStringList>
#include "wlmodule.h"
#include "wldmodbus.h"


class WLModuleDModbus : public WLModule
{
    Q_OBJECT

public:
    WLModuleDModbus(WLDevice *_Device);
    ~WLModuleDModbus();

    bool Init(int sizeDCAN);

private:
    QList <WLDModbus*> DModbus;

    enum typeInterface m_interface;
    quint8 indexUART=0;

public:
    //bool isEnable() {return enable;}
    bool setInterfaceUART(quint8 index);
    int getSizeDModbus() {return DModbus.size();}

    WLDModbus* getDModbus(int m_index);

    bool sendCallData(QByteArray sendData);

signals:

    void timeoutCallData(QByteArray);
    void reciveCallData(QByteArray);

public slots:
    virtual void update();
    virtual void backup();

public:

    virtual void writeXMLData(QXmlStreamWriter &stream);
    virtual void  readXMLData(QXmlStreamReader &stream);
    virtual void readCommand(QByteArray data);
};

#endif // WLModuleDModbus_H

