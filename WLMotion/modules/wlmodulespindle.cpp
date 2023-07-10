#include "wlmodulespindle.h"

WLModuleSpindle::WLModuleSpindle(WLDevice *_Device)
    : WLModule(_Device)
{
    setTypeModule(typeMSpindle);

    Init(1);

    QTimer::singleShot(1000,this,SLOT(update()));
}

WLModuleSpindle::~WLModuleSpindle()
{
    while(Spindle.isEmpty())
        delete Spindle.takeLast();
}

bool WLModuleSpindle::Init(int _size)
{
    if(InitSpindle(_size))
    {
        update();
        return true;
    }
    else
        return false;
}

bool WLModuleSpindle::InitSpindle(int size)
{
    if(size<1
            ||Spindle.size()== size
            ||isReady()) return false;

    WLSpindle *nSpindle;

    if(size>Spindle.size())
        for(int i=Spindle.size();i<size;i++ )
        {
            nSpindle = new WLSpindle(this);
            nSpindle->setIndex(i);
            nSpindle->setParent(this);

            Spindle+=nSpindle;
        }
    else
        while(Spindle.size()!= size)
        {
            nSpindle=Spindle.takeLast();
            disconnect(nSpindle);

            delete nSpindle;
        }

    return true;
}

WLSpindle *WLModuleSpindle::getSpindle(int index)
{
    Q_ASSERT(((index<Spindle.size()))&&(index<255));

    return index<Spindle.size() ? Spindle.at(index): nullptr;
}

void WLModuleSpindle::update()
{
    foreach(WLSpindle *nSpindle,Spindle)
        nSpindle->update();

    QTimer::singleShot(100,this,SLOT(update()));
}

void WLModuleSpindle::backup()
{
    foreach(WLSpindle *nSpindle,Spindle)
        nSpindle->backup();

    QTimer::singleShot(1000,this,SLOT(update()));
}

void  WLModuleSpindle::readCommand(QByteArray Data)
{
    quint8 index,ui1;

    QDataStream Stream(&Data,QIODevice::ReadOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream>>ui1;

    switch(ui1)
    {
    case comSpindle_setData: Stream>>index;//index8
        if(index<getSizeSpindle()){
            Spindle[index]->setData(Stream);
        }
        break;

    case  sendModule_prop: Stream>>ui1;

        Init(ui1);

        setReady(true);
        break;

    case  sendModule_error:
        Stream>>ui1;//index8
        Stream>>index;  //Error

        emit sendMessage("WLModuleSpindle "+getErrorStr(errorModule,ui1),QString::number(index),(int)(ui1));
        break;
    }

}

void WLModuleSpindle::readXMLData(QXmlStreamReader &stream)
{
    quint8 index;

    int size=1;

    if(!stream.attributes().value("size").isEmpty()) size=stream.attributes().value("size").toString().toInt();

    Init(size);

    while(!stream.atEnd())
    {
        stream.readNextStartElement();

        if(stream.name()==metaObject()->className()) break;
        if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

        if(stream.name()=="WLSpindle" )
        {
            index=stream.attributes().value("index").toString().toInt();
            if(index<getSizeSpindle())
                Spindle[index]->readXMLData(stream);
        }
    }
}

void WLModuleSpindle::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("size",QString::number(getSizeSpindle()));

    for(int i=0;i<getSizeSpindle();i++)
    {
        stream.writeStartElement("WLSpindle");
        stream.writeAttribute("index",QString::number(i));
        Spindle[i]->writeXMLData(stream);
        stream.writeEndElement();
    }
}

