#ifndef WLMODULE_H
#define WLMODULE_H

#include <QObject>
#include <QTimer>
#include <QDataStream>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QStringList>
#include "wlelement.h"
#include "wlflags.h"

#define idAxis 0
#define idCut  10

#define idLine  100
#define idCirc  101
#define idBuff  102
#define idWhell 103


#define comModule_getProp 255 //call property module
#define comModule_reset   254 //reset module

#define sendModule_error  255 //send error module
#define sendModule_prop   254 //send property

#define errorModule_index    254

#define startIndexErrorModule 230

const QString errorModule(QT_TR_NOOP("0,no error"
                                     ",254,wrong index element"
                                     ",255,no module"));

enum typeInterface{typeInterfaceNo
                   ,typeInterfaceUSB
                   ,typeInterfaceEthernet
                   ,typeInterfaceCAN
                   ,typeInterfaceUART
                  };

struct WLRemElement
{
    WLElement::typeElement typeE;
    quint16 indexLocal;
    quint16 indexRemote;

    WLRemElement() {indexLocal=indexRemote=0; typeE=WLElement::typeEEmpty;}

    void writeXMLData(QXmlStreamWriter &stream)
    {
        QStringList Names=namesTypeElement.split(",");

        if(typeE<Names.size())
        {
            stream.writeStartElement(Names[typeE]);
            stream.writeAttribute("ilocal",QString::number(indexLocal));
            stream.writeAttribute("iremote",QString::number(indexRemote));
            stream.writeEndElement();
        }

    }

    void readXMLData(QXmlStreamReader &stream)
    {
        QStringList Names=namesTypeElement.split(",");

        for(int i=0;i<Names.size();i++)
            if(stream.name()==Names[i])
            {
                typeE=static_cast<WLElement::typeElement>(i);
                break;
            };

        indexLocal   =stream.attributes().value("ilocal").toString().toInt();
        indexRemote  =stream.attributes().value("iremote").toString().toInt();
    }

};

class WLDevice;


class WLModule : public WLElement
{
    Q_OBJECT
public:
    enum typeModule {typeDevice=0
                     ,typeMAxis//1
                     ,typeMMPG//2
                     ,typeMPWM//3
                     ,typeMFreq//4
                     ,typeMIOPut	 //5
                     ,typeMPlanner//6
                     ,typeMCut//7
                     ,typeMEncoder//8
                     ,typeMConnect//9
                     ,typeMFW//10
                     ,typeMDCan//11
                     ,typeMCKey//12
                     ,typeMAIOPut//13
                     ,typeMUART//14
                     ,typeMDModbus//15
                     ,typeMSpindle//16
                     ,typeMOscp//17
                     ,typeHeart=255
                    };

    Q_ENUM(typeModule)

public:

    WLModule(WLDevice *_Device);
    ~WLModule();

    typeModule getTypeModule() {return m_type;}
    QString getTypeModuleStr();
    void setTypeModule(typeModule _type) {m_type=_type;}
    bool isReady() {return m_ready;}

    WLDevice *getDevice() {return m_Device;}
    void setDevice(WLDevice *Device);

private:
    typeModule m_type;
    bool m_ready=false;

    WLDevice *m_Device=nullptr;

public slots:

    virtual void reset();
    virtual void callProp();

    virtual void setReady(bool);

public slots:
    void sendCommand(QByteArray data);

public:
    virtual void  readCommand(QByteArray)     {}

    static QString getErrorStr(QString str,int);

signals:
    void changedReady(bool);
};

#endif // WLMODULE_H
