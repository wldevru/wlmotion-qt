#include "wlmpg.h"
#include "wlmodulempg.h"
#include "wldevice.h"

WLMPG::WLMPG(WLModuleMPG *_ModuleMPG)
    : WLElement(_ModuleMPG)
{    
    setTypeElement(typeEMPG);

    error=0;
    iEncoder=255;

    m_pulses=200;

    iOutENB=0;

    for(int i=0;i<sizeInAxis;i++)
        iInAxis[i]=0;

    for(int i=0;i<sizeInX;i++)
        iInX[i]=0;

    iInVmode=0;
}

WLMPG::~WLMPG()
{

}

quint16 WLMPG::getPulses() const
{
    return m_pulses;
}

void WLMPG::update()
{
    sendGetData();
}

void WLMPG::backup()
{
    setInX(getIndexInX(),sizeInX);
    setInAxis(getIndexInAxis(),sizeInX);
    setInVmode(getInVmode());

    setEncoder(getEncoder());
}

void WLMPG::setPulses(const quint16 &pulses)
{
    m_pulses = pulses;
}

bool WLMPG::setEnable(bool enable)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_setEnable<<(quint8)getIndex()<<(quint8)enable;

    sendCommand(data);
    return true;
}

bool WLMPG::setDataAxis(quint8 index, quint8 iAxis, float kTrack,QVariant userData)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_setDataAxis<<getIndex()<<index<<iAxis<<kTrack;

    m_data[index].indexAxis=iAxis;
    m_data[index].kTrack=kTrack;
    m_data[index].userData=userData;

    sendCommand(data);
    return true;
}

bool WLMPG::clearDataAxis()
{
    if(getModule()->getDevice()->getVersion().ver <= WLVersion(2,2,18).ver)
    {
        for (quint8 i=0;i<sizeMPGData;i++) {
            setDataAxis(i);
        }
    }
    else {

        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream<<(quint8)comMPG_clearDataAxis<<getIndex();

        for (int i=0;i<sizeMPGData;i++) {
            m_data[i]=WLMPGData();
        }

        sendCommand(data);
        return true;
    }
}


bool WLMPG::setEncoder(quint8 _iEncoder)
{
    iEncoder=_iEncoder;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_setEncoder<<getIndex()<<iEncoder;

    sendCommand(data);

    return true;
}

bool WLMPG::setVmode(quint8 index)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_setManualVmode<<getIndex()<<index;

    sendCommand(data);
    return true;
}

bool WLMPG::setIndexAxis(quint8 index)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_setManualIA<<getIndex()<<index;

    qDebug()<<"setManualIA"<<index;

    sendCommand(data);
    return true;
}

bool WLMPG::setIndexX(quint8 index)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_setManualIX<<getIndex()<<index;

    sendCommand(data);
    return true;
}

bool WLMPG::setOutENB(quint8 index)
{
    iOutENB=index;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_setOutputENB<<getIndex()<<iOutENB;

    sendCommand(data);
    return true;
}

bool WLMPG::setFlag(quint8 flag)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_setFlag<<getIndex()<<flag;

    sendCommand(data);

    Flags.m_Data=flag|(Flags.m_Data&MPGF_enable);
    return true;
}

bool WLMPG::setInAxis(quint8 *iInput, quint8 size)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_setInputAxis<<getIndex();

    if(size<sizeInAxis)
        for(int i=size;i<sizeInAxis;i++) iInAxis[i]=0;

    for(int i=0;i<size;i++)
    {
        Stream<<iInput[i];
        iInAxis[i]=iInput[i];
    }


    sendCommand(data);
    return true;
}

bool WLMPG::setInX(quint8 *iInput, quint8 size)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_setInputX<<getIndex();

    if(size<sizeInX)
        for(int i=size;i<sizeInX;i++) iInX[i]=0;

    for(int i=0;i<size;i++)
    {
        Stream<<iInput[i];
        iInX[i]=iInput[i];
    }


    sendCommand(data);
    return true;
}

bool WLMPG::setInVmode(quint8 iInput)
{
    iInVmode=iInput;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_setInputVmode<<getIndex()<<iInVmode;

    sendCommand(data);
    return true;
}

bool WLMPG::sendGetData()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comMPG_getData<<getIndex();

    sendCommand(data);
    return true;
}

void WLMPG::setData(quint8 Flag, quint8 indexA, quint8 indexX,bool Vmode)
{
    bool update=((Flags.m_Data&MPGF_enable)^(Flag&MPGF_enable))!=0;

    Flags.m_Data=Flag;

    if(update||curIndexAxis!=indexA)   {qDebug()<<"Changed curMPG iA"<<curIndexAxis; emit changedCurIndexAxis(curIndexAxis=indexA);}
    if(update||curIndexX!=indexX)      {qDebug()<<"Changed curMPG iX"<<curIndexX;    emit changedCurIndexX(curIndexX=indexX);      }
    if(update||curVmode!=Vmode)        {qDebug()<<"Changed curMPG Vmode"<<Vmode;     emit changedCurVmode(curVmode=Vmode);         }
}

void WLMPG::writeXMLData(QXmlStreamWriter &stream)
{
    QString buf;

    buf.clear();
    for(quint8 i=0;i<sizeInAxis;i++)
    {
        if(i!=0) buf+=",";
        buf+=QString::number(iInAxis[i]);
    }

    stream.writeAttribute("inAxis",buf);

    buf.clear();
    for(quint8 i=0;i<sizeInX;i++)
    {
        if(i!=0) buf+=",";
        buf+=QString::number(iInX[i]);
    }

    stream.writeAttribute("inX",buf);

    stream.writeAttribute("inVmode",QString::number(getInVmode()));
    stream.writeAttribute("outENB",QString::number(getOutENB()));

    stream.writeAttribute("flag",QString::number(getFlag()));

    stream.writeAttribute("pulses",QString::number(getPulses()));
    stream.writeAttribute("encoder",QString::number(getEncoder()));
}

void WLMPG::readXMLData(QXmlStreamReader &stream)
{
    QStringList List;
    QVector <quint8> iInputs;

    if(!stream.attributes().value("inAxis").isEmpty())
    {
        List=stream.attributes().value("inAxis").toString().split(",");

        iInputs.clear();

        foreach(QString str,List)
        {
            iInputs+=str.toInt();
        }

        setInAxis(iInputs.data(),iInputs.size());
    }

    if(!stream.attributes().value("inX").isEmpty())
    {
        List=stream.attributes().value("inX").toString().split(",");

        iInputs.clear();

        foreach(QString str,List)
        {
            iInputs+=str.toInt();
        }

        setInX(iInputs.data(),iInputs.size());
    }

    if(!stream.attributes().value("inVmode").isEmpty())
        setInVmode(stream.attributes().value("inVmode").toInt());

    if(!stream.attributes().value("outENB").isEmpty())
        setOutENB(stream.attributes().value("outENB").toInt());

    if(!stream.attributes().value("flag").isEmpty())
        setFlag(stream.attributes().value("flag").toInt());

    if(!stream.attributes().value("encoder").isEmpty())
        setEncoder(stream.attributes().value("encoder").toUInt());

    if(!stream.attributes().value("pulses").isEmpty())
        setPulses(stream.attributes().value("pulses").toUInt());
}
