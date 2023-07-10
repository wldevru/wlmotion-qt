#include "wlmoduleuart.h"

WLModuleUART::WLModuleUART(WLDevice *_Device)
    : WLModule(_Device)
{
    setTypeModule(typeMUART);
    Init(1);
}

WLModuleUART::~WLModuleUART()
{
    while(UART.isEmpty())
        delete UART.takeLast();
}

bool WLModuleUART::Init(int sizeUART)
{
    if(sizeUART<1
            ||UART.size()== sizeUART
            ||isReady()) return false;

    WLUART *uart;

    if(sizeUART>UART.size())
        for(int i=UART.size();i<sizeUART;i++ )
        {
            uart = new WLUART(this);
            uart->setIndex(i);
            uart->setParent(this);
            UART+=uart;
        }
    else
        while(UART.size()!= sizeUART)
        {
            uart=UART.takeLast();
            delete uart;
        }

    return true;
}

WLUART *WLModuleUART::getUART(int index)
{
    Q_ASSERT((index<getSizeUART())&&(index<255));

    return index<getSizeUART()? UART[index]:nullptr;
}

void WLModuleUART::setEnable(int index, bool enable)
{
    if(index>=UART.size())
    {return;}

    UART[index]->setEnable(enable);
}

void WLModuleUART::transmitData(int index, QString data)
{
    if(index>=UART.size()) return;

    qDebug()<<"SIZE="<<data.toLocal8Bit().length();

    UART[index]->transmitData(data.toLocal8Bit());
}

void WLModuleUART::clearRecive(int index)
{
    if(index>=UART.size()) return;

    UART[index]->clearReciveData();
}

QString WLModuleUART::getReciveStr(int index, int len)
{
    if(index>=UART.size()) return QString();

    return UART[index]->getReciveStr(len);
}

double WLModuleUART::getReciveByte(int index,bool unsig)
{
    if(index>=UART.size()) return 0;

    if(unsig)
        return UART[index]->getReciveUBytes(1);
    else
        return UART[index]->getReciveBytes(1);
}

double WLModuleUART::getRecive2Byte(int index,bool unsig)
{
    if(index>=UART.size()) return 0;

    if(unsig)
        return UART[index]->getReciveUBytes(2);
    else
        return UART[index]->getReciveBytes(2);
}

double WLModuleUART::getRecive4Byte(int index,bool unsig)
{
    if(index>=UART.size()) return 0;

    if(unsig)
        return UART[index]->getReciveUBytes(4);
    else
        return UART[index]->getReciveBytes(4);
}


double WLModuleUART::getReciveFloat(int index)
{
    if(index>=UART.size()) return 0;

    return UART[index]->getReciveFloat() ;
}

double WLModuleUART::getReciveDouble(int index)
{
    if(index>=UART.size()) return 0;

    return UART[index]->getReciveDouble() ;
}


bool WLModuleUART::isEmptyRecive(int index)
{
    if(index>=UART.size()) return true;

    return UART[index]->isEmptyReciveData();
}

void WLModuleUART::update()
{
    foreach(WLUART *uart,UART)
        uart->sendGetData();
}

void WLModuleUART::backup()
{
    foreach(WLUART *uart,UART)
    {
        uart->setBaudrate(uart->getBaudrate());
    }
}

void  WLModuleUART::readCommand(QByteArray Data)
{
    quint8 index,ui1;

    QDataStream Stream(&Data,QIODevice::ReadOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream>>ui1;

    switch(ui1)
    {

    case sendUART_recive:  Stream>>index;

        if(index<getSizeUART())
        {
            QByteArray BA;

            while(!Stream.atEnd()){
                Stream>>ui1;
                BA+=ui1;
            }

            qDebug()<<"reciveUART"<<BA.size()<<BA;
            getUART(index)->setReciveData(BA);
            emit changedRecive(index);
        }

        break;
    case  sendModule_prop: Stream>>ui1;

        Init(ui1);

        setReady(true);
        break;

    case  sendModule_error:
        Stream>>ui1;//index8

        Stream>>index;  //Error

        if(index<getSizeUART())
        {
            UART[index]->setError(ui1);
            emit sendMessage("UART "+getErrorStr(errorUART,ui1),QString::number(index),(int)(ui1));
        }
        break;

    }

}

void WLModuleUART::readXMLData(QXmlStreamReader &stream)
{	
    quint8 index;
    quint8 size=1;
    if(!stream.attributes().value("size").isEmpty()) size=stream.attributes().value("size").toString().toInt();

    Init(size);

    while(!stream.atEnd())
    {

        stream.readNextStartElement();

        if(stream.name()==metaObject()->className()) break;
        if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

        if(stream.name()=="WLUART")
        {
            index=stream.attributes().value("index").toString().toInt();

            if(index<getSizeUART())
                UART[index]->readXMLData(stream);
        }
    }

}

void WLModuleUART::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("size",QString::number(getSizeUART()));

    for(int i=0;i<getSizeUART();i++)
    {
        stream.writeStartElement("WLUART");
        stream.writeAttribute("index",QString::number(i));
        UART[i]->writeXMLData(stream);
        stream.writeEndElement();
    }
}

