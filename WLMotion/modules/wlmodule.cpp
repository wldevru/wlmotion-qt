#include <QMetaEnum>
#include "wlmodule.h"
#include "wldevice.h"

WLModule::WLModule(WLDevice *_Device)
    :WLElement(_Device)
{
    m_type=typeDevice;
    m_ready=false;

    m_Device=_Device;
}


WLModule::~WLModule()
{

}

QString WLModule::getTypeModuleStr()
{
    return QString::fromUtf8(QMetaEnum::fromType<WLModule::typeModule>().valueToKey(getTypeModule()));
}

void WLModule::setDevice(WLDevice *Device)
{
    m_Device = Device;
}

QString WLModule::getErrorStr(QString str,int ierr)
{
    QString ret;
    QStringList list=str.split(",");

    for(int i=0;i<list.size();i+=2)
        if(list[i].toInt()==ierr)
        {
            ret="("+list[i+1]+")";
            break;
        }
    return ret;
}

void WLModule::sendCommand(QByteArray data)
{
    data.prepend(m_type);

    m_Device->sendCommand(data);
}

void WLModule::callProp()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comModule_getProp;

    sendCommand(data);
}

void WLModule::setReady(bool ready)
{
    if(m_ready!=ready){
        m_ready=ready;
        emit changedReady(m_ready);
    }
}


void WLModule::reset()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comModule_reset;

    sendCommand(data);
}

