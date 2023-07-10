#ifndef WLDevice_H
#define WLDevice_H

#include <QObject>
#include <QCoreApplication>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>
#include <QString>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QUdpSocket>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QFile>
#include <QTextCodec>
#include <QNetworkInterface>
#include "modules/wlmodule.h"
#include "modules/wlmoduleconnect.h"
#include "wlflags.h"

//WLDevice
#define comDev_resetAll    1 //reset all
#define comDev_getVersion  2 //call version
#define comDev_getProp     3 //call prop
#define comDev_getModule   4 //call modules
#define comDev_getUID      5 //call	UID
#define comDev_getStatus   6 //call status

#define comDev_setStatus   10 //set status
#define comDev_reboot      11 //rebooting

#define comDev_writeDataUser   13 //G43enable ethernet ack
#define comDev_readDataUser    14 //enable ethernet ack

#define sendDev_prop     100
#define sendDev_Module   101 //send modules
#define sendDev_UID      102 //send uid
#define sendDev_status   103 //send status
#define sendDev_version  104 //send version

#define sendDev_userData 110 //send userData

#define errorDevice_nomodule 20

#define DEFINE_TIMERWAITUSB  2
#define DEFINE_TIMERWAITETH  1

#ifndef ETH_UDPPORT
#define ETH_UDPPORT 2020
#endif

#define ETH_MINTIMEOUTACK 2 //2-4-8-16-32-64-128-256
#define ETH_MAXTIMEOUTACK 256
#define ETH_MAXETHPACK 256-1

const QString errorDevice(QT_TR_NOOP("0,no error"
                                     ",20,eror no module"));

struct WLVersion{
    quint32 ver;

    bool isNulll(){
        return ver==0;
    }

    bool isValidVersion(WLVersion min,WLVersion max)
    {
        if(min.isNulll())
            min=*this;

        if(max.isNulll())
            max=*this;

        if(min.ver>ver
                ||max.ver<ver) return false;

        return true;
    }

    WLVersion(quint8 _ver3=0,quint8 _ver2=0,quint8 _ver1=0,quint8 _ver0=0){
        ver= _ver3; ver<<=8;
        ver|=_ver2; ver<<=8;
        ver|=_ver1; ver<<=8;
        ver|=_ver0;
    }

    QString toString(){

        quint8 ver3=ver>>24;
        quint8 ver2=ver>>16;
        quint8 ver1=ver>>8;
        quint8 ver0=ver;

        QString ret=QString("%1").arg(ver3);

        if(ver2!=0) ret+=QString(".%1").arg(ver2);
        if(ver1!=0) ret+=QString(".%1").arg(ver1);
        if(ver0!=0) ret+=QString(".%1").arg(ver0);

        return ret;
    }

};

struct WLDeviceInfo
{
    QString           name;
    QString         typeFW;
    QString        comPort;
    QHostAddress        HA;
    QString          UID96;
    WLVersion      version;
    QList<quint8>  modules;

    bool isValid(WLDeviceInfo info,QString _typeFW){
        return name==info.name
                &&UID96==info.UID96
                &&  (_typeFW==typeFW
                     || _typeFW.isEmpty());
    }
};



class WLDevice: public WLModule
{
    Q_OBJECT

public:

    enum statusDevice{DEVICE_empty,DEVICE_init,DEVICE_connect};

    enum flag{fl_openconnect = 1<<0
              ,fl_connect = 1<<1
              // ,fl_ready   = 1<<2
              ,fl_waitack = 1<<3};

    WLDevice(QObject *parent=nullptr);
    ~WLDevice();

    void off();

    static WLVersion getMinVersion() {return  minVersion;}
    static WLVersion getMaxVersion() {return  maxVersion;}

private:

    static WLVersion maxVersion;
    static WLVersion minVersion;

    QList <WLModule*> m_modules;

    WLDeviceInfo m_deviceInfo;

    QString   m_UID96;
    WLVersion m_version;

    QMutex     inBufMutex;
    QMutex    outBufMutex;

    QMutex  ethernetMutex;
    QMutex   connectMutex;

    enum statusDevice status;

    QByteArray outBuf;
    QByteArray inBuf;

    QString m_nameDevice;
    QString m_typeFW;

    QSerialPort m_serialPort;

    QHostAddress  m_HA;
    QUdpSocket   m_udpSocket;

    WLFlags Flags;

    QString m_prop;

    quint32 error;

    QByteArray m_bufEth;
    QTimer    *m_timerEth;
    quint8    m_countTxPacket;
    quint8    m_countRxPacket;

    QTimer *m_timerSend;

    quint16 m_timeoutAck;

public:

    bool isValidVersion() {return m_version.isValidVersion(minVersion,maxVersion);}

    bool initSerialPort(QString portName="");
    bool initUdpSocket(QHostAddress HA);

    bool openConnect();
    bool waitForReady(int timeout=500);

    WLModule* getModule(typeModule getTypeModule);
    void addModule(WLModule *module);

    QList<WLModule *> getModules() const;

    WLModuleConnect* getModuleConnect() {return static_cast<WLModuleConnect*>(getModule(typeMConnect));}

    void setNameDevice(QString _name) {if(m_nameDevice.isEmpty()||!_name.isEmpty()) m_nameDevice=_name;}
    QString getNameDevice() {return m_nameDevice;}

    QString getTypeFW() {return m_typeFW;}

    bool initFromFile(QString nameFile);

    bool writeToFile(QString nameFile);
    bool writeToDir(QString dir) {qDebug()<<"WLDevice::writeToDir";
                                  return writeToFile(dir+"\\"+getNameDevice()+".xml");}

    bool isOpenConnect() {return Flags.get(fl_openconnect);}
    bool isConnect() {return Flags.get(fl_connect);}

    statusDevice getStatus() {return status;}

    WLVersion getVersion()    {return m_version;}

    void  setVersion(WLVersion _ver);

    void clearBuf();

    bool writeDataUser(quint8 offset, quint8 size,quint8 *buf);
    bool readDataUser(quint8 offset, quint8 size);

public:
    virtual WLModule *createModule(QString name);
    virtual WLModule *createModule(typeModule getTypeModule);

public:
    virtual void callPropModules();
    virtual void callProp();
    virtual void reset();

public:

private slots:	

    void sendEthData();
    void updateReady();

    virtual	void readSlot();

    void onErrorSerialPort(QSerialPort::SerialPortError serialPortError);

    void sendData();

public slots:
    void removeModules();
    void callModules();
    void callVersion();
    void reconnectSerialPort();

    void closeConnect();

    void update(); //get current data
    void backup(); //backup current data

    void sendCommand(QByteArray);

public slots:
    virtual void callStatus();

signals:
    void readDataDevice();
    void createdModules();

    void reconnected();

    void changedConnect(bool);
    void changedModules(int);
    void changedProp(QString);
    void changedStatus(statusDevice);
    void changedUID96(QString);
    void changedVersion(WLVersion);
    void changedVersionProtocol(quint16);

private:

    void init(QXmlStreamReader &stream);
    void decodeInputData();
public:

    static QList<WLDeviceInfo> availableDevices();

    void reboot(uint8_t getTypeModule);

    WLDeviceInfo getInfo();

    void setInfo(WLDeviceInfo info);

    QHostAddress getHA() {return m_HA;}

    QString getPortName()  {return m_serialPort.portName();}
    QString getProp()  {return m_prop;}
    QString getUID96() {return m_UID96;}
    void setUID96(QString);
    void setStatus(enum statusDevice);

private slots:

    void callUID();

public:

    virtual void writeXMLData(QXmlStreamWriter &stream);
    virtual  void readXMLData(QXmlStreamReader &stream);


};

#endif //WLDEVICE_H
