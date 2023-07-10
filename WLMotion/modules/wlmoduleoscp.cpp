#include "wlmoduleoscp.h"

WLModuleOscp::WLModuleOscp(WLDevice *_Device)
    : WLModule(_Device)
{
    setTypeModule(typeMOscp);

    Init(1);
}

WLModuleOscp::~WLModuleOscp()
{
    while(chOscp.isEmpty())
        delete chOscp.takeLast();
}

bool WLModuleOscp::Init(int _size)
{
    if(InitOscp(_size))
    {
        update();
        setRun(false);
        return true;
    }
    else
        return false;
}

bool WLModuleOscp::InitOscp(int size)
{
    if(size<1
            ||chOscp.size()== size
            ||isReady()) return false;

    WLChOscp *nOscp;

    if(size>chOscp.size())
        for(int i=chOscp.size();i<size;i++ )
        {
            nOscp = new WLChOscp;
            nOscp->index=i;
            chOscp+=nOscp;
        }
    else
        while(chOscp.size()!= size)
        {
            nOscp=chOscp.takeLast();

            delete nOscp;
        }

    return true;
}

void WLModuleOscp::getAvaibleSource()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMOscp_getAvaSource;

    sendCommand(data);
}

WLChOscp *WLModuleOscp::getOscp(int index)
{
    Q_ASSERT(((index<chOscp.size()))&&(index<255));

    return index<chOscp.size() ? chOscp.at(index): nullptr;
}

bool WLModuleOscp::setSourceChannel(WLChOscp chOscp)
{
    return setSourceChannel(chOscp.index
                            ,chOscp.src.element
                            ,chOscp.indexElement
                            ,chOscp.src.typeData);
}

bool WLModuleOscp::setSourceChannel(quint8 indexch,WLElement::typeElement element, uint8_t indexElement, uint8_t typeData)
{
    WLChOscp *Oscp=getOscp(indexch);

    if(Oscp){
        Oscp->src.element=element;
        Oscp->src.typeData=typeData;

        Oscp->indexElement=indexElement;

        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream
                <<(quint8)comMOscp_setSourceChannel
               <<(quint8)indexch
              <<(quint8)Oscp->src.element
             <<(quint8)Oscp->indexElement
            <<(quint8)Oscp->src.typeData;

        sendCommand(data);
        return true;
    }

    return false;
}

bool WLModuleOscp::setRun(bool run)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMOscp_setRun<<(quint8)run;

    sendCommand(data);

    return true;
}

void WLModuleOscp::update()
{
    getAvaibleSource();
}

void WLModuleOscp::backup()
{
    foreach(WLChOscp *channel,chOscp){
        setSourceChannel(*channel);
    }
}


void  WLModuleOscp::readCommand(QByteArray Data)
{
    quint8 index,ui1;

    QDataStream Stream(&Data,QIODevice::ReadOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream>>ui1;

    switch(ui1)
    {
    case sendMOscp_avaSource:{
        source.clear();

        while(!Stream.atEnd())
        {
            quint8 telement;
            quint8 tdata;

            Stream>>telement>>tdata;

            WLSrcChOscp src;

            src.element=static_cast<WLElement::typeElement>(telement);
            src.typeData=tdata;

            source<<src;
        }

        emit changedSource(source);
    }

    case sendMOscp_dataCh : {
        quint32 time_us;
        Stream>>time_us;

        QList <double> values;

        for(int i=0;!Stream.atEnd();i++){
            quint8 type;

            Stream>>type;

            switch(type) {
            case OSC_u8: {
                quint8 u8;
                Stream>>u8;
                chOscp[i]->value=u8;
            }
                break;

            case OSC_i8: {
                qint8 i8;
                Stream>>i8;
                chOscp[i]->value=i8;
            }
                break;

            case OSC_u16: {
                quint16 ui16;
                Stream>>ui16;
                chOscp[i]->value=ui16;
            }
                break;

            case OSC_i16: {
                qint16 i16;
                Stream>>i16;
                chOscp[i]->value=i16;
            }
                break;

            case OSC_u32:{
                quint32 ui32;
                Stream>>ui32;
                chOscp[i]->value=ui32;
            }
                break;

            case OSC_i32:{
                qint32 i32;
                Stream>>i32;
                chOscp[i]->value=i32;
            }
                break;
            case OSC_f:  {
                float f32;
                Stream>>f32;
                chOscp[i]->value=f32;
            }
                break;
            case OSC_d: {
                double d;
                Stream>>d;
                chOscp[i]->value=d;
            }
                break;
            case OSC_dl: {
                qint64 dl;
                Stream>>dl;
                chOscp[i]->value=dl;
            }
                break;
            case OSC_empty: chOscp[i]->value=0;break;
            }

            values+=chOscp[i]->value;
        }

        emit changedValues((double)time_us/1000000,values);
    }
        break;

        /*
case comOscp_setData: Stream>>index;//index8
                         if(index<getSizeOscp()){
                          Oscp[index]->setData(Stream);
                          }
                         break;
*/
    case  sendModule_prop: Stream>>ui1;

        Init(ui1);

        setReady(true);
        break;

    case  sendModule_error:
        Stream>>ui1;//index8
        Stream>>index;  //Error

        emit sendMessage("WLModuleOscp "+getErrorStr(errorModule,ui1),QString::number(index),(int)(ui1));
        break;
    }

}

void WLModuleOscp::readXMLData(QXmlStreamReader &stream)
{
    quint8 index;

    int size=1;

    if(!stream.attributes().value("size").isEmpty()) size=stream.attributes().value("size").toString().toInt();

    Init(size);
    /*
while(!stream.atEnd())
{
stream.readNextStartElement();

if(stream.name()==metaObject()->className()) break;
if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

if(stream.name()=="WLOscp" )
       {
       index=stream.attributes().value("index").toString().toInt();
       if(index<getSizeOscp())
           Oscp[index]->readXMLData(stream);
      }
}
*/
}

void WLModuleOscp::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("size",QString::number(getSizeOscp()));
    /*
for(int i=0;i<getSizeOscp();i++)
 {
 stream.writeStartElement("WLOscp");
 stream.writeAttribute("index",QString::number(i));
   Oscp[i]->writeXMLData(stream);
 stream.writeEndElement();
 }*/
}

