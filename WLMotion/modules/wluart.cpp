#include "wluart.h"
#include "wlmoduleuart.h"

WLUART::WLUART(WLModuleUART *_ModuleUART)
    : WLElement(_ModuleUART)
{
    setTypeElement(typeEDCan);
}

WLUART::~WLUART()
{

}

bool WLUART::setBaudrate(quint32 _baudrate)
{
    if(_baudrate>0) {
        m_baudrate=_baudrate;
    }else {
        return false;
    }

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comUART_setData<<(quint8)dataUART_baudrate<<(quint8)getIndex()<<m_baudrate;

    sendCommand(data);
    return true;
}

bool WLUART::setEnable(bool enable)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comUART_setEnable<<(quint8)getIndex()<<(quint8)enable;

    sendCommand(data);
    return true;
}


bool WLUART::sendGetData()
{
    sendGetData(dataUART_baudrate);

    return true;
}

void WLUART::setReciveData(QByteArray buf)
{
    QMutexLocker locker(&reciveMutex);

    if((m_reciveData.size()+buf.size())<MaxReciveBufSize){
        m_reciveData+=buf;
    }
    else {
        qDebug()<<"WLUART::setReciveData error full buf indexUrart"<<getIndex();
    }
}

bool WLUART::sendGetData(typeDataUART type)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comUART_getData<<getIndex()<<(quint8)type;

    sendCommand(data);

    return true;
}

void WLUART::setData(QDataStream &Stream)
{
    quint8 ui1;

    Stream>>ui1;

    switch(ui1)
    {
    case dataUART_baudrate:   Stream>>m_baudrate;   emit changed(getIndex()); break;
    default: break;
    }

}

bool WLUART::setDelayFrame(quint16 delay)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    m_delayFrame=delay;

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comUART_setDelayFrame<<getIndex()<<m_delayFrame;

    sendCommand(data);

    return true;
}

QByteArray WLUART::takeReciveData()
{
    QByteArray ret=m_reciveData;
    clearReciveData();

    return ret;
}

void WLUART::clearReciveData()
{
    QMutexLocker locker(&reciveMutex);
    m_reciveData.clear();
}

bool WLUART::isEmptyReciveData()
{
    QMutexLocker locker(&reciveMutex);
    return m_reciveData.isEmpty();
}

bool WLUART::transmitData(QByteArray trdata)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comUART_transmit<<getIndex();

    for(int i=0;i<trdata.size();i++)
        Stream<<(quint8)trdata[i];

    sendCommand(data);

    return true;
}

QString WLUART::getReciveStr(int len)
{
    QMutexLocker locker(&reciveMutex);

    if(isEmptyReciveData()){
        return QString();
    }

    QByteArray ba;

    for(int i=0;i<m_reciveData.size();i++)
        qDebug()<<m_reciveData.at(i);

    if(m_reciveData.size()<=len || len<=0) {
        ba=m_reciveData;
        m_reciveData.clear();
    }
    else {
        ba=m_reciveData.mid(0,len);
        m_reciveData=m_reciveData.mid(len);
    }

    return QString::fromUtf8(ba);
}

qint32 WLUART::getReciveBytes(int size, bool *ok)
{
    QMutexLocker locker(&reciveMutex);

    if(size>4
            ||m_reciveData.size()<size)
    {
        if(ok) *ok=false;

        return 0;
    }

    QDataStream stream(m_reciveData.mid(0,size));

    m_reciveData=m_reciveData.mid(size);

    qint32 ret=0;

    switch(size)
    {
    case 1:{
        qint8 i8;
        stream>>i8;
        ret=i8;
    }
        break;
    case 2:{
        qint16 i16;
        stream>>i16;
        ret=i16;
    }
        break;
    case 4:{
        stream>>ret;
    }
        break;
    }

    if(ok) *ok=true;

    return ret;
}

quint32 WLUART::getReciveUBytes(int size, bool *ok)
{
    QMutexLocker locker(&reciveMutex);

    if(size>4
            ||m_reciveData.size()<size)
    {
        if(ok) *ok=false;

        return 0;
    }

    QDataStream stream(m_reciveData.mid(0,size));

    m_reciveData=m_reciveData.mid(size);

    quint32 ret=0;

    switch(size)
    {
    case 1:{
        quint8 i8;
        stream>>i8;
        ret=i8;
    }
        break;
    case 2:{
        quint16 i16;
        stream>>i16;
        ret=i16;
    }
        break;
    case 4:{
        stream>>ret;
    }
        break;
    }

    if(ok) *ok=true;

    return ret;
}

float WLUART::getReciveFloat(bool *ok)
{
    QMutexLocker locker(&reciveMutex);

    int size=4;

    if(m_reciveData.size()<size)
    {
        if(ok) *ok=false;

        return 0;
    }

    QDataStream Stream(m_reciveData.mid(0,size));
    m_reciveData=m_reciveData.mid(size);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    float ret;

    Stream>>ret;

    if(ok) *ok=true;

    return ret;
}

double WLUART::getReciveDouble(bool *ok)
{
    QMutexLocker locker(&reciveMutex);

    int size=8;

    if(m_reciveData.size()<size)
    {
        if(ok) *ok=false;
        return 0;
    }

    QDataStream Stream(m_reciveData.mid(0,size));
    m_reciveData=m_reciveData.mid(size);

    Stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    double ret;

    Stream>>ret;

    if(ok) *ok=true;

    return ret;
}

void WLUART::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("baudrate", QString::number(getBaudrate()));
    stream.writeAttribute("delayFrame", QString::number(getDelayFrame()));
}

void WLUART::readXMLData(QXmlStreamReader &stream)
{
    bool ok;

    if(!stream.attributes().value("baudrate").isEmpty())
        setBaudrate(stream.attributes().value("baudrate").toString().toInt(&ok));

    if(!stream.attributes().value("delayFrame").isEmpty())
        setDelayFrame(stream.attributes().value("delayFrame").toString().toInt(&ok));


}

