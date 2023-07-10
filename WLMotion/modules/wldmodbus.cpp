#include "wldmodbus.h"
#include "wlmoduledmodbus.h"

WLDModbus::WLDModbus(WLModuleDModbus *_ModuleDModbus)
    : WLElement(_ModuleDModbus)
{
    setTypeElement(typeEDModbus);
}

WLDModbus::~WLDModbus()
{

}

bool WLDModbus::setAdrModbus(quint8 _adrModbus)
{
    adrModbus=_adrModbus;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDModbus_setAddress<<(quint8)getIndex()<<adrModbus;

    sendCommand(data);
    return true;
}

bool WLDModbus::addRemoteElement(WLRemElement remElement)
{
    if(remElement.typeE==typeEEmpty) return false;

    remEList+=remElement;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDModbus_addRemElement<<(quint8)getIndex()
         <<(quint8)remElement.typeE
        <<(quint8)remElement.indexLocal
       <<(quint16)remElement.indexRemote;

    sendCommand(data);
    return true;
}

bool WLDModbus::clearAllRemoteElement()
{
    remEList.clear();

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDModbus_clearAllRemElement<<(quint8)getIndex();

    sendCommand(data);
    return true;
}

bool WLDModbus::setEnable(bool enable)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    m_enable=enable;

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDModbus_setEnable<<(quint8)getIndex()<<(quint8)m_enable;

    sendCommand(data);
    return true;
}


bool WLDModbus::sendGetData()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDModbus_getData<<getIndex();

    sendCommand(data);
    return true;
}

void WLDModbus::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("enable", QString::number(isEnable()).toUpper());
    stream.writeAttribute("adr", QString::number(getAdrModbus(),16).toUpper());

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

void WLDModbus::readXMLData(QXmlStreamReader &stream)
{
    bool ok;
    if(!stream.attributes().value("adr").isEmpty())	setAdrModbus(stream.attributes().value("adr").toString().toInt(&ok,16));

    if(!stream.attributes().value("enable").isEmpty())	{
        setEnable(stream.attributes().value("enable").toString().toInt());
    }else{  //old version
        setEnable(getAdrModbus()!=0);
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

