#include "wlaioput.h"
#include "wlmoduleaioput.h"

WLAIOPut::WLAIOPut(WLModuleAIOPut *_ModuleAIOPut,QString _comment, bool input):
    WLElement(_ModuleAIOPut)
{
    m_value=0.0;
    m_histValue=0.0;
    setTypeElement(input ? typeEAInput : typeEAOutput);
    setComment(_comment);
    Flags.reset();
    Flags.set(AIOPF_input,input);
    setObjectName("AIO");


    connect(this,&WLAIOPut::changedValue,this,[=](){emit changed(getIndex());});
    connect(this,&WLAIOPut::changedEnable,this,[=](){emit changed(getIndex());});
    connect(this,&WLAIOPut::changedInv,this,[=](){emit changed(getIndex());});
}

void WLAIOPut::setInv(bool _inv)
{
    Flags.set(AIOPF_inv,_inv);

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)(isInput()? comAIOPut_setDataInput:comAIOPut_setDataOutput)
         <<getIndex()<<(quint8)dataAIOPut_inv<<(quint8)_inv;

    sendCommand(data);

    emit changedInv(_inv);
}

void WLAIOPut::sendGetData()
{
    sendGetData(typeDataAIOPut::dataAIOPut_flag);
    sendGetData(typeDataAIOPut::dataAIOPut_value);
}

void WLAIOPut::sendGetData(enum  typeDataAIOPut type)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)(isInput()? comAIOPut_getDataInput:comAIOPut_getDataOutput)
         <<getIndex()<<(quint8)type;

    sendCommand(data);
}


void WLAIOPut::setData(QDataStream &Stream)
{
    quint8 ui1;

    Stream>>ui1;


    switch(ui1)
    {
    case dataAIOPut_value:  Stream>>m_value;
        emit changedValue(m_value);
        break;

    case dataAIOPut_flag:   Stream>>ui1;
        qDebug()<<"dataAIOPut_flag";

        if((ui1^Flags.m_Data)&AIOPF_enable) {Flags.tog(AIOPF_enable); emit changedEnable(Flags.get(AIOPF_enable));}

        break ;
    default: break;
    }

    return;
}

void WLAIOPut::setValue(float value)
{
    if(isInput())return;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAIOPut_setDataOutput
         <<getIndex()<<(quint8)dataAIOPut_value<<value;

    sendCommand(data);
}

void WLAIOPut::update()
{
    sendGetData();
}

void WLAIOPut::backup()
{
    setInv(isInv());

    if(!isInput())
        setValue(getValue());
}


void WLAIOPut::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("hist",QString::number(getHist()));

}

void WLAIOPut::readXMLData(QXmlStreamReader &stream)
{
    if(!stream.attributes().value("hist").isEmpty()) setHist(stream.attributes().value("Hist").toString().toFloat());
}
