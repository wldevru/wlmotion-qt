#include "wlioput.h"
#include "wlmoduleioput.h"

WLIOPut WLIOPut::In0(nullptr,"In_0.-1",0);
WLIOPut WLIOPut::In1(nullptr,"In_1.-1",1);
WLIOPut WLIOPut::Out(nullptr,"Out.-1",0);


WLIOPut::WLIOPut(WLModuleIOPut *_ModuleIOPut,QString _comment, bool input)
    :WLElement(_ModuleIOPut)
{
    setTypeElement(input ? typeEInput : typeEOutput);

    setComment(_comment);
    Flags.reset();
    Flags.set(IOPF_input,input);
    setObjectName("IO");
}

void WLIOPut::setData(quint8 _flags)
{
    const auto last=Flags.m_Data;

    uint8_t mask= IOPF_input
            |IOPF_asend
            |IOPF_pulse
            |(isOutput() ? IOPF_old:0);

    _flags&=~mask;

    Flags.m_Data=(Flags.m_Data&mask)|_flags;

    if((_flags&IOPF_inv)^(last&IOPF_inv))
    {
        if(isInput())
            qDebug()<<"WLIOPut::input"<<getIndex()<<"changed inv"<<((_flags&IOPF_inv)!=0);
        else
            qDebug()<<"WLIOPut::output"<<getIndex()<<"changed inv"<<((_flags&IOPF_inv)!=0);

        //emit changedInv(_flags&IOPF_inv);
    }

    if(last!=Flags.m_Data)
        emit changed(getIndex());
}


unsigned char getByteFrom (WLIOPut *B0
                           ,WLIOPut *B1
                           ,WLIOPut *B2
                           ,WLIOPut *B3
                           ,WLIOPut *B4
                           ,WLIOPut *B5
                           ,WLIOPut *B6
                           ,WLIOPut *B7)
{
    unsigned char ret=0;

    ret=B0->getNow()? bit0:0
                      |B1->getNow()? bit1:0;

    if(B2) ret|=B2->getNow()? bit2:0;
    else
        if(B3) ret|=B3->getNow()? bit3:0;
        else
            if(B4) ret|=B4->getNow()? bit4:0;
            else
                if(B5) ret|=B5->getNow()? bit5:0;
                else
                    if(B6) ret|=B6->getNow()? bit6:0;
                    else
                        if(B7) ret|=B7->getNow()? bit7:0;

    return ret;
}

void setByteTo(unsigned char byte
               ,WLIOPut *B0
               ,WLIOPut *B1
               ,WLIOPut *B2
               ,WLIOPut *B3
               ,WLIOPut *B4
               ,WLIOPut *B5
               ,WLIOPut *B6
               ,WLIOPut *B7)
{
    B0->setOut(byte&bit0);
    B1->setOut(byte&bit1);

    if(B2) B2->setOut(byte&bit2);
    else
        if(B3) B3->setOut(byte&bit3);
        else
            if(B4) B4->setOut(byte&bit4);
            else
                if(B5) B5->setOut(byte&bit5);
                else
                    if(B6) B6->setOut(byte&bit6);
                    else
                        if(B7) B7->setOut(byte&bit7);
}

void setIOPutInvStr(QString data,WLIOPut *ioputs,int size)
{
    QStringList List=data.split(",");
    int index;
    for(int i=0;i<List.size();i++)
    {
        index=List[i].toInt();
        if((index<size)
                &&(index>=0))  ioputs[index].setInv();
    }
}

QString getIOPutInvStr(WLIOPut *ioputs,int size)
{
    QString List;

    for(int i=0;i<size;i++)
        if(ioputs[i].isInv())
        {
            if(List.isEmpty())
                List+=QString::number(i);
            else
                List+=","+QString::number(i);
        }
    return  List;
}

void WLIOPut::setInv(bool _inv)
{
    //if((Flags.get(IOPF_inv))^_inv)
    {
        Flags.set(IOPF_inv,_inv);

        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        if(Flags.get(IOPF_input))
            Stream<<(quint8)comIOPut_setInputInv<<getIndex()<<(quint8)_inv;
        else
            Stream<<(quint8)comIOPut_setOutputInv<<getIndex()<<(quint8)_inv;

        sendCommand(data);

        emit changedInv(_inv);
    }
}

void WLIOPut::setOut(bool now)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comIOPut_setOutputTo<<getIndex()<<(quint8)now;

    sendCommand(data);
}

void WLIOPut::setOutPulse(bool _now,quint32 time_ms)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comIOPut_setOutputPulse<<getIndex()<<(quint8)_now<<time_ms;

    sendCommand(data);
}

