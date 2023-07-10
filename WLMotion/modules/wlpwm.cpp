#include "wlpwm.h"
#include "wlmodulepwm.h"

WLPWM::WLPWM(WLModulePWM *_ModulePWM)
    : WLElement(_ModulePWM)
{
    setTypeElement(typeEOutPWM);
    m_value=0;
    m_Freq=0;
    error=0;
}

WLPWM::~WLPWM()
{

}


bool WLPWM::setOut(float value)
{
    if(value<0||value>1) return false;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPWM_setOut<<getIndex()<<value;

    sendCommand(data);
    return true;
}

bool WLPWM::setInv(bool inv)
{
    Flags.set(PWMF_inv,inv);

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPWM_setInvOut<<getIndex()<<(quint8)inv;

    sendCommand(data);

    //sendGetData();
    return true;
}

bool WLPWM::setEnable(bool enable)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    qDebug()<<data.size();

    Stream<<(quint8)comPWM_setEnableOut<<getIndex()<<enable;

    sendCommand(data);
    return true;
}

bool WLPWM::sendGetData(typeDataPWM type)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPWM_getData<<getIndex()<<(quint8)type;

    sendCommand(data);

    return true;
}

bool WLPWM::sendGetData()
{
    sendGetData(dataPWM_Fpwm);
    sendGetData(dataPWM_flag);
    sendGetData(dataPWM_value);

    return true;
}

bool WLPWM::setFreq(float F)
{
    if(F<=0) return false;

    m_Freq=F;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPWM_setFOut<<getIndex()<<m_Freq;

    sendCommand(data);

    qDebug()<<"WLPWM::setFreq"<<getIndex()<<F;
    return true;
}

void WLPWM::setData(QDataStream &Stream)
{
    quint8 ui1;

    Stream>>ui1;

    switch(ui1)
    {
    case dataPWM_Fpwm:    Stream>>m_Freq;   emit changedFreq(m_Freq);   emit changed(getIndex()); break;
    case dataPWM_value:   Stream>>m_value;  emit changedValue(m_value); emit changed(getIndex()); break;
    case dataPWM_flag:    Stream>>ui1;
        Flags.m_Data=ui1; emit changed(getIndex()); break;
    default: break;
    }

}

void WLPWM::update()
{
    sendGetData();
}

void WLPWM::backup()
{
    setInv(isInv());
    setFreq(getFreq());
    setOut(getValue());
}

void WLPWM::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("Freq",QString::number(getFreq()));
    stream.writeAttribute("inv", QString::number(isInv()));

}

void WLPWM::readXMLData(QXmlStreamReader &stream)
{
    if(!stream.attributes().value("Freq").isEmpty())
        setFreq(stream.attributes().value("Freq").toString().toFloat());
    if(!stream.attributes().value("inv").isEmpty())
        setInv(stream.attributes().value("inv").toString().toInt());
}
