#include "wlmoduledcan.h"

WLModuleDCan::WLModuleDCan(WLDevice *_Device)
    : WLModule(_Device)
{
    setTypeModule(typeMDCan);
    Init(1);
}

WLModuleDCan::~WLModuleDCan()
{
    while(DCan.isEmpty())
        delete DCan.takeLast();
}

bool WLModuleDCan::Init(int sizeDCan)
{
    if(sizeDCan<1
            ||DCan.size()== sizeDCan
            ||isReady()) return false;

    WLDCan *dcan;

    if(sizeDCan>DCan.size())
        for(int i=DCan.size();i<sizeDCan;i++ )
        {
            dcan = new WLDCan(this);
            dcan->setIndex(i);
            dcan->setParent(this);
            DCan+=dcan;
        }
    else
        while(DCan.size()!= sizeDCan)
        {
            dcan=DCan.takeLast();
            delete dcan;
        }

    return true;
}

WLDCan *WLModuleDCan::getDCan(int index)
{
    Q_ASSERT((index<getSizeDCan())&&(index<255));

    return index<getSizeDCan()? DCan[index]:nullptr;
}

void WLModuleDCan::update()
{
    foreach(WLDCan *dcan,DCan)
        dcan->sendGetData();
}

void WLModuleDCan::backup()
{
    foreach(WLDCan *dcan,DCan)
    {
        dcan->setAdrCan(dcan->getAdrCan());
        dcan->setEnable(dcan->isEnable());

        QList <WLRemElement> remList=dcan->getRemElementList();

        dcan->clearAllRemoteElement();

        qDebug()<<"WLModuleDCan::backup()"<<dcan->getAdrCan()<<remList.size();

        foreach(WLRemElement remElement,remList) {
            dcan->addRemoteElement(remElement);
        }
    }
}

void  WLModuleDCan::readCommand(QByteArray Data)
{
    quint8 index,ui1;

    QDataStream Stream(&Data,QIODevice::ReadOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream>>ui1;

    switch(ui1)
    {
    case  sendModule_prop: Stream>>ui1;

        Init(ui1);

        setReady(true);
        break;

    case  sendModule_error:
        Stream>>ui1;//index8
        Stream>>index;  //Error

        if(ui1>200)
        {
            emit sendMessage("ModuleDCAN "+getErrorStr(errorModule,ui1),QString::number(index),(int)(ui1));
        }
        else
            if(index<getSizeDCan())
            {
                DCan[index]->setError(ui1);
                emit sendMessage("DCan "+getErrorStr(errorDCan,ui1),QString::number(index),(int)(ui1));
            }
        break;

    }

}

void WLModuleDCan::readXMLData(QXmlStreamReader &stream)
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

        if(stream.name()=="WLDCan")
        {
            index=stream.attributes().value("index").toString().toInt();

            if(index<getSizeDCan())
                DCan[index]->readXMLData(stream);
        }

    }



}

void WLModuleDCan::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("size",QString::number(getSizeDCan()));

    for(int i=0;i<getSizeDCan();i++)
    {
        stream.writeStartElement("WLDCan");
        stream.writeAttribute("index",QString::number(i));
        DCan[i]->writeXMLData(stream);
        stream.writeEndElement();
    }
}

