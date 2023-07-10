#include "wldcan.h"
#include "wlmoduledcan.h"

WLDCan::WLDCan(WLModuleDCan *_ModuleDCan)
    : WLElement(_ModuleDCan)
{
    setTypeElement(typeEDCan);
    adrCan=0;
}

WLDCan::~WLDCan()
{

}

bool WLDCan::setAdrCan(quint16 _adrCan) 
{
    if(_adrCan==0) return false;

    adrCan=_adrCan;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDCan_setAddress<<(quint8)getIndex()<<adrCan;

    sendCommand(data);
    return true;
}

bool WLDCan::addRemoteElement(WLRemElement remElement)
{
    if(remElement.typeE==typeEEmpty) return false;

    remEList+=remElement;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDCan_addRemElement<<(quint8)getIndex()
         <<(quint8)remElement.typeE
        <<(quint8)remElement.indexLocal
       <<(quint8)remElement.indexRemote;

    sendCommand(data);
    return true;
}

bool WLDCan::clearAllRemoteElement()
{
    remEList.clear();

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDCan_clearAllRemElement<<(quint8)getIndex();

    sendCommand(data);
    return true;
}

bool WLDCan::setEnable(bool enable)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    m_enable=enable;

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDCan_setEnable<<(quint8)getIndex()<<(quint8)m_enable;

    sendCommand(data);
    return true;
}

bool WLDCan::sendGetData()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDCan_getData<<getIndex();

    sendCommand(data);
    return true;
}

void WLDCan::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("enable", QString::number(isEnable()).toUpper());
    stream.writeAttribute("adr", QString::number(getAdrCan(),16).toUpper());

    stream.writeStartElement("remoteElements");

    if(remEList.isEmpty())
    {
        WLRemElement remElement;
        remElement.writeXMLData(stream);
    }
    else
        for(int i=0;i<remEList.size();i++)
            remEList[i].writeXMLData(stream);

    stream.writeEndElement();

}

void WLDCan::readXMLData(QXmlStreamReader &stream)
{
    bool ok;
    if(!stream.attributes().value("adrCan").isEmpty())	setAdrCan(stream.attributes().value("adrCan").toString().toInt(&ok,16));
    if(!stream.attributes().value("adr").isEmpty())	setAdrCan(stream.attributes().value("adr").toString().toInt(&ok,16));

    if(!stream.attributes().value("enable").isEmpty())	{
        setEnable(stream.attributes().value("enable").toString().toInt());
    }else{  //old version
        setEnable(getAdrCan()!=0);
    }

    clearAllRemoteElement();

    while(!stream.atEnd())
    {
        qDebug()<<stream.name();
        stream.readNextStartElement();
        qDebug()<<stream.name();
        if(stream.name()==metaObject()->className()) break;
        if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

        if(stream.name()=="remoteElements")
        {
            while(!stream.atEnd())
            {
                qDebug()<<stream.name();
                stream.readNextStartElement();
                qDebug()<<stream.name();
                if(stream.name()=="remoteElements") break;
                if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

                WLRemElement remElement;
                remElement.readXMLData(stream);

                addRemoteElement(remElement);
            }
        }
    }
}

