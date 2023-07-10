#ifndef WLModuleOscp_H
#define WLModuleOscp_H

#include <QObject>
#include <QDebug>
#include <QStringList>

#include "wlmodule.h"

//Osciloscope
#define comMOscp_setRun       1 //
#define comMOscp_setSourceChannel  2 //
#define comMOscp_setPeriod   3 //

#define comMOscp_getAvaSource  4 //


#define sendMOscp_dataCh     4 //
#define sendMOscp_avaSource  5 //

enum typeValueOScp{OSC_empty,OSC_u8,OSC_i8,OSC_u16,OSC_i16,OSC_u32,OSC_i32,OSC_f,OSC_d,OSC_dl};

typedef struct
{
    enum WLElement::typeElement element=WLElement::typeEEmpty;
    uint8_t typeData=255;
}WLSrcChOscp; //source Oscp

typedef struct
{
    uint8_t index=0;

    WLSrcChOscp src;

    uint8_t indexElement;

    double value;

}WLChOscp; //channel Oscp


class WLModuleOscp : public WLModule
{
    Q_OBJECT

public:
    WLModuleOscp(WLDevice *_Device);
    ~WLModuleOscp();

    bool Init(int _size);

private:
    QList <WLChOscp*> chOscp;
    QList <WLSrcChOscp> source;

    uint32_t period=1000;

private:
    bool InitOscp(int size);
    void getAvaibleSource();

public:     
    int getSizeOscp() {return chOscp.size();}
    WLChOscp *getOscp(int index);

public:
    bool setSourceChannel(WLChOscp chOscp);
    bool setSourceChannel(quint8 indexch,enum WLElement::typeElement = typeEEmpty, uint8_t indexElement = 0,uint8_t typeData = 0);
    bool setRun(bool run=true);

    QList <WLSrcChOscp> getSource() {return source;}

public slots:
    virtual void update();
    virtual void backup();

signals:
    void changedValues(double,QList<double>);
    void changedSource(QList <WLSrcChOscp>);

public:

    virtual void writeXMLData(QXmlStreamWriter &stream);
    virtual void  readXMLData(QXmlStreamReader &stream);
    virtual void readCommand(QByteArray data);
};

#endif // WLModuleOscp_H

