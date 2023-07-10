#include "wlfreq.h"
#include "wlmodulefreq.h"

WLFreq::WLFreq(WLModuleFreq *_ModuleFreq)
    : WLElement(_ModuleFreq)
{
    setTypeElement(typeEFreq);

    Freq=1000;
    flag=0;
    error=0;
}

WLFreq::~WLFreq()
{

}

bool WLFreq::setMaxOut(float Fmax)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comFreq_setMaxOut<<getIndex()<<Fmax;

    sendCommand(data);
    return true;
}

bool WLFreq::sendGetData()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<((quint8)comFreq_getDataOut)<<getIndex();

    sendCommand(data);
    return true;
}	

bool WLFreq::setEnable(bool enable)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comFreq_setEnableOut<<getIndex()<<(quint8)enable;

    sendCommand(data);
    return true;
}
