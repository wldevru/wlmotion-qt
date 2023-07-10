#include "wlmodulempg.h"

WLModuleMPG::WLModuleMPG(WLDevice *_Device)
    : WLModule(_Device)
{
    setTypeModule(typeMMPG);
    Init(1);
}

WLModuleMPG::~WLModuleMPG()
{
    while(MPG.isEmpty())
        delete MPG.takeLast();
}

bool WLModuleMPG::Init(int sizeMPG)
{
    if(sizeMPG<1
            ||MPG.size()== sizeMPG
            ||isReady()) return false;

    WLMPG *mpg;

    if(sizeMPG>MPG.size())
        for(int i=MPG.size();i<sizeMPG;i++ )
        {
            mpg = new WLMPG(this);
            mpg->setIndex(i);
            mpg->setParent(this);

            MPG+=mpg;
        }
    else
        while(MPG.size()!= sizeMPG)
        {
            mpg=MPG.takeLast();
            delete mpg;
        }


    return true;
}

WLMPG *WLModuleMPG::getMPG(int index)
{
    Q_ASSERT((index<getSize())&&(index<255));

    return index<getSize() ? MPG[index] : nullptr;
}

void WLModuleMPG::update()
{
    foreach(WLMPG *mpg,MPG)
        mpg->update();
}

void WLModuleMPG::backup()
{
    foreach(WLMPG *mpg,MPG)
        mpg->backup();
}

void WLModuleMPG::readCommand(QByteArray Data)
{
    quint8 index,ui1,ui2,ui3;

    QDataStream Stream(&Data,QIODevice::ReadOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream>>ui1;

    switch(ui1)
    {
    case sendMMPG_data:   Stream>>index;//index8
        Stream>>ui1;  //Flag8
        Stream>>ui2;  //indexA
        Stream>>ui3;  //Vmode(0x80) indexX(0x7F)

        MPG.at(index)->setData(ui1,ui2,ui3&0x7F,ui3&0x80);
        break;

    case  sendModule_prop: Stream>>ui1;

        Init(ui1);

        setReady(true);
        break;

    case  sendModule_error:Stream>>ui1;
        if(ui1>200){
            if(ui1==0xFF) {
                char buf[64];
                buf[Stream.readRawData(buf,sizeof(buf))]='\0';

                emit sendMessage("ModuleAxis ",buf,(int)(-ui1));
            }
            else{
                Stream>>index;
                emit sendMessage("WLModuleMPG "+getErrorStr(errorModule,ui1),QString::number(index),(int)(ui1));
            }
        }
        else{
            Stream>>index;
            if(index<getSize())
            {
                MPG[index]->setError(ui1);
                emit sendMessage("WLMPG "+getErrorStr(errorMPG,ui1),QString::number(index),(int)(ui1));
            }
        }

        break;
    }

}


void WLModuleMPG::writeXMLData(QXmlStreamWriter &stream)
{

    stream.writeAttribute("size",QString::number(getSize()));

    for(int i=0;i<getSize();i++)
    {
        stream.writeStartElement("MPG");
        stream.writeAttribute("index",QString::number(i));
        MPG[i]->writeXMLData(stream);
        stream.writeEndElement();
    }
}


void WLModuleMPG::readXMLData(QXmlStreamReader &stream)
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

        if(stream.name()=="Whell" //old style
                ||stream.name()=="MPG")
        {
            index=stream.attributes().value("index").toString().toInt();
            if(index<getSize())
                MPG[index]->readXMLData(stream);
        }
    }
}
