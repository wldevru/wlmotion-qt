#include "wlmoduledmodbus.h"

WLModuleDModbus::WLModuleDModbus(WLDevice *_Device)
    : WLModule(_Device)
{
    setTypeModule(typeMDModbus);
    Init(1);
}

WLModuleDModbus::~WLModuleDModbus()
{
    while(DModbus.isEmpty())
        delete DModbus.takeLast();
}
bool WLModuleDModbus::Init(int sizeDModbus)

{
    if(sizeDModbus<1
            ||DModbus.size()== sizeDModbus
            ||isReady()) return false;

    WLDModbus *dmodbus;

    if(sizeDModbus>DModbus.size())
        for(int i=DModbus.size();i<sizeDModbus;i++ )
        {
            dmodbus = new WLDModbus(this);
            dmodbus->setIndex(i);
            dmodbus->setParent(this);
            DModbus+=dmodbus;
        }
    else
        while(DModbus.size()!= sizeDModbus)
        {
            dmodbus=DModbus.takeLast();
            delete dmodbus;
        }

    return true;
}

WLDModbus *WLModuleDModbus::getDModbus(int index)
{
    Q_ASSERT((index<getSizeDModbus())&&(index<255));

    return index<getSizeDModbus()? DModbus[index]:nullptr;
}

bool WLModuleDModbus::sendCallData(QByteArray sendData)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);


    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDModbus_callDataM;

    for(int i=0;i<sendData.size();i++)
        Stream<<(quint8)sendData[i];

    sendCommand(data);
    return true;
}

void WLModuleDModbus::update()
{
    foreach(WLDModbus *dmodbus,DModbus)
        dmodbus->sendGetData();
}

void WLModuleDModbus::backup()
{
    switch(m_interface)
    {
    case typeInterfaceUART: setInterfaceUART(indexUART);break;
    default: break;
    }

    foreach(WLDModbus *dmodbus,DModbus)
    {
        dmodbus->setAdrModbus(dmodbus->getAdrModbus());
        dmodbus->setEnable(dmodbus->isEnable());

        QList <WLRemElement> remList=dmodbus->getRemElementList();

        dmodbus->clearAllRemoteElement();

        qDebug()<<"WLModuleDModbus::backup()"<<dmodbus->getAdrModbus()<<remList.size();

        foreach(WLRemElement remElement,remList) {
            dmodbus->addRemoteElement(remElement);
        }
    }
}

void  WLModuleDModbus::readCommand(QByteArray Data)
{
    quint8 index,ui1;

    QDataStream Stream(&Data,QIODevice::ReadOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream>>ui1;

    QByteArray BA;

    switch(ui1)
    {
    case  sendDModbus_reciveCallDataM:  while(!Stream.atEnd()){
            Stream>>ui1;
            BA+=ui1;
        }

        emit reciveCallData(BA);
        break;

    case  sendDModbus_timeoutCallDataM: while(!Stream.atEnd()){
            Stream>>ui1;
            BA+=ui1;
        }

        emit timeoutCallData(BA);
        break;


    case  sendModule_prop: Stream>>ui1;

        Init(ui1);

        setReady(true);
        break;

    case  sendModule_error:
        Stream>>ui1;//index8
        Stream>>index;  //Error

        if(ui1>200)
        {
            emit sendMessage("ModuleDModbus "+getErrorStr(errorModule,ui1),QString::number(index),(int)(ui1));
        }
        else
            if(index<getSizeDModbus())
            {
                DModbus[index]->setError(ui1);
                emit sendMessage("DModbus "+getErrorStr(errorDModbus,ui1),QString::number(index),(int)(ui1));
            }
        break;

    }

}

bool WLModuleDModbus::setInterfaceUART(quint8 index)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    m_interface=typeInterfaceUART;
    indexUART=index;

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDModbus_setInterfaceM<<(quint8)m_interface<<(quint8)indexUART;

    sendCommand(data);
    return true;
}
/*
void WLModuleDModbus::setEnable(bool _enable)
{
QByteArray data;
QDataStream Stream(&data,QIODevice::WriteOnly);

enable=_enable;

Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
Stream.setByteOrder(QDataStream::LittleEndian);

Stream<<(quint8)comDModbus_setEnableM<<(quint8)enable;

sendCommand(data);
return true;
}*/

void WLModuleDModbus::readXMLData(QXmlStreamReader &stream)
{	
    quint8 index;
    quint8 size=1;

    if(!stream.attributes().value("UART").isEmpty()){
        setInterfaceUART(stream.attributes().value("UART").toString().toInt());
    }

    if(!stream.attributes().value("size").isEmpty()) size=stream.attributes().value("size").toString().toInt();

    Init(size);

    while(!stream.atEnd())
    {

        stream.readNextStartElement();

        if(stream.name()==metaObject()->className()) break;
        if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

        if(stream.name()=="WLDModbus")
        {
            index=stream.attributes().value("index").toString().toInt();

            if(index<getSizeDModbus())
                DModbus[index]->readXMLData(stream);
        }

    }



}

void WLModuleDModbus::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("size",QString::number(getSizeDModbus()));

    switch(m_interface)
    {
    case typeInterfaceUART: stream.writeAttribute("UART",QString::number(indexUART));break;
    default: stream.writeAttribute("interface","???");break;
    }

    for(int i=0;i<getSizeDModbus();i++)
    {
        stream.writeStartElement("WLDModbus");
        stream.writeAttribute("index",QString::number(i));
        DModbus[i]->writeXMLData(stream);
        stream.writeEndElement();
    }
}



