#ifndef WLModuleFreq_H
#define WLModuleFreq_H

#include <QObject>
#include <QDebug>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QStringList>
#include "wlmodule.h"
#include "wlfreq.h"


class WLModuleFreq : public WLModule
{
    Q_OBJECT

public:
    WLModuleFreq(WLDevice *_Device);
    ~WLModuleFreq();

    bool Init(int _sizeOutFreq);

private:
    QList <WLFreq*> outFreq;

public:	
    int getSizeOutFreq() {return outFreq.size();}
    WLFreq* getOutFreq(int m_index);

public slots:
    virtual void update();
    virtual void backup();

public:
    //virtual void callProp();
    //virtual void writeXMLData(QXmlStreamWriter &stream);
    //virtual void  readXMLData(QXmlStreamReader &stream);

    virtual void readCommand(QByteArray data);
};

#endif // WLModuleFreq_H

