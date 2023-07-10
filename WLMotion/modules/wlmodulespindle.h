#ifndef WLModuleSpindle_H
#define WLModuleSpindle_H

#include <QObject>
#include <QDebug>
#include <QStringList>

#include "wlmodule.h"
#include "wlspindle.h"


class WLModuleSpindle : public WLModule
{
    Q_OBJECT

public:
    WLModuleSpindle(WLDevice *_Device);
    ~WLModuleSpindle();

    bool Init(int _size);

private:
    QList <WLSpindle*> Spindle;

private:
    bool InitSpindle(int size);

public:
    WLSpindle* getSpindle(int m_index);
    int getSizeSpindle() {return Spindle.size();}
public:

public slots:
    virtual void update();
    virtual void backup();

signals:

public:

    virtual void writeXMLData(QXmlStreamWriter &stream);
    virtual void  readXMLData(QXmlStreamReader &stream);
    virtual void readCommand(QByteArray data);
};

#endif // WLModuleSpindle_H

