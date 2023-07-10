#include "wlencoder.h"
#include "wlmoduleencoder.h"

WLEncoder::WLEncoder(WLModuleEncoder *_ModuleEncoder)
    : WLElement(_ModuleEncoder)
{
    setTypeElement(typeEEncoder);
    m_count=0;
    error=0;

    connect(this,&WLEncoder::changedCount,this,[=](){emit changed(getIndex());});
}

WLEncoder::~WLEncoder()
{

}


long WLEncoder::count() {return m_count;}

void WLEncoder::setData(QDataStream &data)
{
    quint8 ui1,type;
    qint32 l;

    data>>type;

    switch((typeDataEncoder)type)
    {
    case dataEncoder_count: data>>l;

        if(m_count!=l)
        {
            m_count=l;

            emit changedCount(m_count);
        }
        break;

    case dataEncoder_flag:  data>>ui1;
        Flags.m_Data=ui1;
        emit changed(getIndex());
        break;
        /*
 case dataAxis_F:      data>>Freq;
                       emit changedFreq(Freq);
                       break;

 case dataAxis_latch2: data>>m_latchPos2;
                       m_validLatch2=true;
                       emit changedLatch2(m_latchPos2);
                       break;

 case dataAxis_latch3: data>>m_latchPos3;
                       m_validLatch3=true;
                       emit changedLatch3(m_latchPos3);
                       break;
                       */
    }
}

bool WLEncoder::setCount(qint32 cnt)
{
    m_count=cnt;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comEnc_setData<<(quint8)getIndex()<<(quint8)dataEncoder_count<<(qint32)m_count;

    sendCommand(data);
    return true;
}

bool WLEncoder::sendGetData()
{
    sendGetData(typeDataEncoder::dataEncoder_count);
    return true;
}

bool WLEncoder::sendGetData(typeDataEncoder type)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comEnc_getData<<(quint8)getIndex()<<(quint8)type;

    sendCommand(data);
    return true;
}

bool WLEncoder::setScale(quint16 _scale)	
{
    if(_scale>0)
    {
        scale=_scale;
        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream<<(quint8)comEnc_setScale<<(quint8)getIndex()<<(quint16)scale;

        sendCommand(data);
        return true;
    }
    else
        return false;
}

bool WLEncoder::setInv(quint16 _inv)
{
    Flags.set(ENCF_inv,_inv);

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comEnc_setInv<<(quint8)getIndex()<<(quint8)_inv;

    sendCommand(data);

    return true;
}

bool WLEncoder::setEnable(bool enable)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comEnc_setEnable<<(quint8)getIndex()<<(quint8)enable;

    sendCommand(data);
    return true;
}

void WLEncoder::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("inv", QString::number(isInv()));
}

void WLEncoder::readXMLData(QXmlStreamReader &stream)
{
    if(!stream.attributes().value("inv").isEmpty())
        setInv(stream.attributes().value("inv").toString().toInt());
}

void WLEncoder::update()
{
    sendGetData();
}

void WLEncoder::backup()
{
    setInv(isInv());
    sendGetData();
}
