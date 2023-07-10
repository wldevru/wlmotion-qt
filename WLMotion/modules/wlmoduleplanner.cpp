#include "wlmoduleplanner.h"
#include "wlmotion.h"

WLModulePlanner::WLModulePlanner(WLDevice *_Device)
    : WLModule(_Device)
{
    setTypeModule(typeMPlanner);

    m_sizeBuf=0;
    m_free=0;
    m_status=PLANNER_stop;
    Flags.set(PLF_empty);
    m_curIdElementBuf=0;
    m_lastIdElementBuf=0;
    m_indexRingElementBuf=0;

    inProbe=&WLIOPut::In0;
    inPause=&WLIOPut::In0;
    inStop=&WLIOPut::In0;

    outDisTHC=&WLIOPut::Out;

    m_validProbeR=false;
    m_validProbeF=false;

    QTimer::singleShot(1000,this,&WLModulePlanner::callTrackPlanner);
}

WLModulePlanner::~WLModulePlanner()
{

}

int WLModulePlanner::getTimeoutAddElement() const
{
    return m_timeoutAddElement;
}

void WLModulePlanner::setTimeoutAddElement(int timeoutAddElement)
{
    if(timeoutAddElement>=0)
        m_timeoutAddElement = timeoutAddElement;
}

bool WLModulePlanner::setInput(typeInputPlanner type, quint8 num)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setInput<<(quint8)type<<num;

    sendCommand(data);
    return true;
}

bool WLModulePlanner::setOutput(typeOutputPlanner type, quint8 num)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setOutput<<(quint8)type<<num;

    sendCommand(data);
    return true;
}

bool WLModulePlanner::setIgnoreInput(typeInputPlanner type, quint8 ignore)
{
    switch(type)
    {
    case PLANNER_inPause: m_ignoreInPause=ignore; break;
    case PLANNER_inStop:  m_ignoreInStop=ignore; break;
    default: return false;
    }

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setIgnoreInput<<(quint8)type<<ignore;

    sendCommand(data);
    return true;
}


WLIOPut *WLModulePlanner::getInput(typeInputPlanner type)
{
    WLIOPut *ret=nullptr;

    switch(type)
    {
    case PLANNER_inProbe: ret=inProbe;break;
    case PLANNER_inPause: ret=inPause;break;
    case PLANNER_inStop:  ret=inStop; break;

    default: ret=&WLIOPut::In0;
    }

    return ret;
}

WLIOPut *WLModulePlanner::getOutput(typeOutputPlanner type)
{
    WLIOPut *ret=nullptr;

    switch(type)
    {
    case PLANNER_outAntidive: ret=outDisTHC;break;
    default: ret=&WLIOPut::Out;
    }

    return ret;
}

void WLModulePlanner::clear()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_clear;

    m_indexRingElementBuf=0;

    sendCommand(data);
}

void WLModulePlanner::setModeRun(modeRunPlanner modeRun)
{
    m_modeRun=modeRun;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setModeRun<<(quint8)m_modeRun;

    sendCommand(data);
}

WLSpindle *WLModulePlanner::getSpindle()
{
    WLModuleSpindle *MSpindle=static_cast<WLModuleSpindle*>(getDevice()->getModule(typeMSpindle));

    if(MSpindle){
        return MSpindle->getSpindle(iSpindle);
    }

    return nullptr;
}

bool WLModulePlanner::setISpindle(quint8 index)
{
    iSpindle=index;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setISpindle<<index;

    sendCommand(data);

    return true;
}

void WLModulePlanner::setInProbe(int index)
{
    inProbe->removeComment("inProbe");

    WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getDevice()->getModule(typeMIOPut));

    if(index>=ModuleIOPut->getSizeInputs()) index=0;

    inProbe=ModuleIOPut->getInput(index);
    inProbe->addComment("inProbe");

    setInput(PLANNER_inProbe,index);
}

void WLModulePlanner::setInPause(int index)
{
    inPause->removeComment("inPause");

    WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getDevice()->getModule(typeMIOPut));

    if(index>=ModuleIOPut->getSizeInputs()) index=0;

    inPause=ModuleIOPut->getInput(index);
    inPause->addComment("inPause");

    connect(inPause,&WLIOPut::changed,this,[=](int _index){if(inPause->getIndex()==_index
                                                              &&inPause->getCond()>1) emit changedPause(inPause->getNow());});

    setInput(PLANNER_inPause,index);
}

void WLModulePlanner::setInStop(int index)
{
    inStop->removeComment("inStop");

    WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getDevice()->getModule(typeMIOPut));

    if(index>=ModuleIOPut->getSizeInputs()) index=0;

    inStop=ModuleIOPut->getInput(index);
    inStop->addComment("inStop");

    connect(inStop,&WLIOPut::changed,this,[=](int _index){if(inStop->getIndex()==_index
                                                             &&inStop->getCond()>1) emit changedStop(inStop->getNow());});

    setInput(PLANNER_inStop,index);
}

void WLModulePlanner::setOutAntidive(int index)
{
    outDisTHC->removeComment("outAntidive");

    WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getDevice()->getModule(typeMIOPut));

    if(index>=ModuleIOPut->getSizeOutputs()) index=0;

    outDisTHC=ModuleIOPut->getOutput(index);
    outDisTHC->addComment("outAntidive");

    setOutput(PLANNER_outAntidive,index);
}

bool WLModulePlanner::setFAntidive(float f)
{
    if(f<0) return false;

    m_Fantidive=f;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setData<<(quint8)dataPlanner_Fantidive<<m_Fantidive;

    sendCommand(data);
    return true;
}

bool WLModulePlanner::setActInProbe(WLIOPut::typeActionInput typeAct)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    m_actInProbe=typeAct;

    Stream<<(quint8)comPlanner_setActInProbe<<(quint8)typeAct;

    qDebug()<<"WLModulePlanner::setActInProbe"<<(quint8)typeAct<<typeAct;

    sendCommand(data);
    return true;
}

bool WLModulePlanner::setWaitAxis(bool en)
{
    m_waitAxis=en;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setWaitAxisMode<<(quint8)m_waitAxis;

    sendCommand(data);
    return true;
}

bool WLModulePlanner::setAutoOffSOut(bool en)
{
    m_autoOffSOut=en;

    if(getDevice()->getVersion().ver<WLVersion(2,2,12).ver) {
        qDebug()<<"no support: comPlanner_setAutoOffSOut";
        return false;
    }

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setAutoOffSOut<<(quint8)m_autoOffSOut;

    sendCommand(data);
    return true;
}


bool WLModulePlanner::setWaitSFastElement(bool en)
{
    m_waitSFastElement=en;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setWaitSFastElement<<(quint8)m_waitSFastElement;

    sendCommand(data);
    return true;
}

void WLModulePlanner::setData(QDataStream &data)
{
    quint8 type;

    data>>type;

    switch((typeDataPlanner)type)
    {
    case dataPlanner_Fsum: //data>>nowPosition;
        //emit changedPosition(nowPosition);
        break;

    default: break;
    }

}

void WLModulePlanner::getData(typeDataPlanner type)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_getData<<(quint8)type;

    sendCommand(data);
}

void WLModulePlanner::callTrackPlanner()
{
    getData(typeDataPlanner::dataPlanner_curSpindleInValue);
    getData(typeDataPlanner::dataPlanner_curSpindleOutValue);
    sendGetData();

    QTimer::singleShot(100,this,&WLModulePlanner::callTrackPlanner);
}

void WLModulePlanner::sendGetData()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_getDataPlanner;

    sendCommand(data);
}

void WLModulePlanner::update()
{
    sendGetData();
}

void WLModulePlanner::backup()
{
    setKF(getKF());
    setKSOut(getKSOut());
    setSmoothAng(getSmoothAng());
    setModeRun(getModeRun());
    setWaitAxis(isWaitAxis());

    setInProbe(getInput(PLANNER_inProbe)->getIndex());
    setInPause(getInput(PLANNER_inPause)->getIndex());
    setOutAntidive(getOutput(PLANNER_outAntidive)->getIndex());

    setIAxisSlave(m_indexsAxis.data(),m_indexsAxis.size());

    setActInProbe(m_actInProbe);

    QTimer::singleShot(1000,this,&WLModulePlanner::callTrackPlanner);
}


bool WLModulePlanner::setIAxisSlave(quint8 *indexsAxis,quint8 size)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setISlaveAxis<<size;

    m_indexsAxis.clear();

    for(int i=0;i<size;i++)
    {
        m_indexsAxis<<indexsAxis[i];
        Stream<<indexsAxis[i];
    }

    sendCommand(data);

    return true;
}

bool WLModulePlanner::setHPause(quint8 enable,qint32 hPause)
{
    Flags.set(PLF_usehpause,enable);

    m_hPause=hPause;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setHPause<<enable<<m_hPause;

    sendCommand(data);

    return true;
}


bool WLModulePlanner::addULine(quint8 mask,quint8 size,quint8 indexs[],qint32 endPos[],qint32 midPos[],float S,float Fmov,quint32 _id)
{
    if(Mutex.tryLock(m_timeoutAddElement))
    {
        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream<<(quint8)comPlanner_addULine<<mask<<size;//<<endPos[0]<<endPos[1]<<endPos[2]<<S<<Fmov   ;

        for(int i=0;i<size;i++)
            Stream<<indexs[i]<<endPos[i]<<midPos[i];

        Stream<<S
             <<Fmov
            <<_id;

        QString en,mid;

        for(int i=0;i<size;i++){
            en+=QString::number(endPos[i])+" ";
            mid+=QString::number(midPos[i])+" ";
        }

        qDebug()<<"addBufULine en:"<<en<<" mid:"<<mid<<"id:"<<_id<<"indexR"<<m_indexRingElementBuf<<"S:"<<S<<"F:"<<Fmov;

        m_free--;
        //emit ChangedFreeBuf(getFreeBuf());

        m_indexRingElementBuf++;
        m_lastIdElementBuf=_id;

        Flags.reset(PLF_empty);

        sendCommand(data);
        Mutex.unlock();
        return true;
    }
    else {
        return false;
    }
}


bool WLModulePlanner::addLine(quint8 mask,quint8 size,quint8 indexs[],qint32 endPos[],float S,float Fmov,quint32 _id)
{
    if(Mutex.tryLock(m_timeoutAddElement))
    {
        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream
                <<(quint8)comPlanner_addLine
               <<mask
              <<size;

        for(int i=0;i<size;i++)
            Stream<<indexs[i]<<endPos[i];

        Stream<<S
             <<Fmov
            <<_id;

        //ebug()<<"index"<<m_lastIndexElementBuf<<m_free;
        QString en;

        for(int i=0;i<size;i++)
            en+=QString::number(endPos[i])+" ";

        qDebug()<<"addBufLine3D en:"<<en<<"id:"<<_id<<"indexR"<<m_indexRingElementBuf<<"S:"<<S<<"F:"<<Fmov<<"mask:"<<mask;
        m_free--;
        //emit ChangedFreeBuf(getFreeBuf());

        m_indexRingElementBuf++;
        m_lastIdElementBuf=_id;

        Flags.reset(PLF_empty);

        sendCommand(data);

        Mutex.unlock();
        return true;
    }
    else {
        return false;
    }
}

bool WLModulePlanner::addCirc(quint8 mask,quint8 size,quint8 indexs[],qint32 endPos[],qint32 cenPosIJ[],float S,float Fmov,quint32 _id)
{
    if(Mutex.tryLock(m_timeoutAddElement))
    {
        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);


        Stream
                <<(quint8)comPlanner_addCirc
               <<mask
              <<size;

        for(quint8 i=0;i<size;i++)
            Stream<<indexs[i]<<endPos[i];

        Stream<<(qint32)cenPosIJ[0]<<cenPosIJ[1];

        Stream<<S
             <<Fmov
            <<_id;


        QString en;

        for(int i=0;i<size;i++)
            en+=QString::number(endPos[i])+" ";

        qDebug()<<"addBufCirc3D en:"<<en<<" cp:"<<cenPosIJ[0]<<cenPosIJ[1]<<" id:"<<_id<<"indexR"<<m_indexRingElementBuf<<"S:"<<S<<"F:"<<Fmov;

        m_free--;
        //emit ChangedFreeBuf(getFreeBuf());

        m_indexRingElementBuf++;
        m_lastIdElementBuf=_id;

        Flags.reset(PLF_empty);

        sendCommand(data);

        Mutex.unlock();
        return true;
    }
    else {
        return false;
    }
}

bool WLModulePlanner::addDelay(quint32 delayms,float S, quint32 _id)
{
    if(Mutex.tryLock(m_timeoutAddElement))
    {
        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream
                <<(quint8)comPlanner_addDelay
               <<delayms
              <<S
             <<_id;

        //ebug()<<"index"<<m_lastIndexElementBuf<<m_free;
        qDebug()<<"addDelay:"<<delayms<<" S"<<S<<" id:"<<_id<<"indexR"<<m_indexRingElementBuf;
        m_free--;
        //emit ChangedFreeBuf(getFreeBuf());

        m_indexRingElementBuf++;
        m_lastIdElementBuf=_id;

        Flags.reset(PLF_empty);

        sendCommand(data);

        Mutex.unlock();
        return true;
    }
    else {
        return false;
    }

}

bool WLModulePlanner::addM6X(WLElement::typeElement type, quint8 index,float value, quint32 _id,float posTask,enum typeM6XTask typeTask)
{
    if(getDevice()->getVersion().ver<WLVersion(2,2,12).ver) {
        qDebug()<<"no support: addM6X";
        return true;
    }

    if(Mutex.tryLock(m_timeoutAddElement))
    {
        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream<<(quint8)comPlanner_addM6X
             <<(quint8)type
            <<index
           <<value
          <<posTask
         <<(quint8)typeTask
        <<_id;

        //ebug()<<"index"<<m_lastIndexElementBuf<<m_free;
        qDebug()<<"addM6X:"<<type<<" index:"<<index<<" value:"<<value<<" id:"<<_id<<"indexR"<<m_indexRingElementBuf;

        m_free--;
        //emit ChangedFreeBuf(getFreeBuf());

        m_indexRingElementBuf++;
        m_lastIdElementBuf=_id;

        Flags.reset(PLF_empty);

        sendCommand(data);

        Mutex.unlock();
        return true;
    }
    else {
        return false;
    }
}


bool WLModulePlanner::startMov()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_start;

    sendCommand(data);

    if(getStatus()==PLANNER_stop)
        m_indexRingElementBuf=0;

    qDebug()<<"WLModulePlanner::startMov()";

    return true;
}

bool WLModulePlanner::stopMov()
{
    Flags.set(PLF_empty);

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_stop;

    sendCommand(data);
    return true;
}

bool WLModulePlanner::pauseMov()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_pause;

    sendCommand(data);

    return true;
}

bool WLModulePlanner::setKF(float _KF)
{
    m_KF=_KF;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_setKF<<_KF;

    sendCommand(data);
    return true;
}

bool WLModulePlanner::setKFpause(float _F)
{
    if((0.0f<_F)&&(_F<=1.0f))
    {
        m_KFpause=_F;

        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream<<(quint8)comPlanner_setKFpause<<m_KFpause;

        sendCommand(data);
        return true;
    }
}

bool WLModulePlanner::setSmoothAng(float ang_gr)
{
    if(0<=ang_gr&&ang_gr<=30)
    {
        m_smoothAng=ang_gr;

        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream<<(quint8)comPlanner_setSmoothAng<<ang_gr;

        sendCommand(data);
        return true;
    }
    else
        return false;
}

bool WLModulePlanner::setSOut(float s)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    qDebug()<<"setSOutBuf"<<s;

    Stream<<(quint8)comPlanner_setSOut<<s;

    sendCommand(data);
    return true;
}

bool WLModulePlanner::setKSOut(float k)
{
    m_KSOut=k;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    qDebug()<<"setKSOutBuf"<<k;

    Stream<<(quint8)comPlanner_setKSOut<<k;

    sendCommand(data);
    return true;
}

bool WLModulePlanner::setEnableSOut(quint8 enable)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comPlanner_enableSOut<<enable;

    sendCommand(data);
    return true;
}


void WLModulePlanner::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("SmoothAngGr",QString::number(getSmoothAng()));
    stream.writeAttribute("waitAxis",QString::number(isWaitAxis()));
    stream.writeAttribute("waitSFastElement",QString::number(isWaitSFastElement()));

    stream.writeAttribute("KFpause",QString::number(getKFpause()));

    stream.writeAttribute("inProbe",QString::number(getInput(PLANNER_inProbe)->getIndex()));
    stream.writeAttribute("inPause",QString::number(getInput(PLANNER_inPause)->getIndex()));
    stream.writeAttribute("inStop",QString::number(getInput(PLANNER_inStop)->getIndex()));

    stream.writeAttribute("outAntidive",QString::number(getOutput(PLANNER_outAntidive)->getIndex()));
}

void WLModulePlanner::readXMLData(QXmlStreamReader &stream)
{
    while(!stream.atEnd())
    {
        if(!stream.attributes().value("SmoothAngGr").isEmpty())
            setSmoothAng(stream.attributes().value("SmoothAngGr").toString().toFloat());

        if(!stream.attributes().value("waitAxis").isEmpty())
            setWaitAxis(stream.attributes().value("waitAxis").toString().toInt());

        if(!stream.attributes().value("waitSFastElement").isEmpty())
            setWaitSFastElement(stream.attributes().value("waitSFastElement").toString().toInt());

        if(!stream.attributes().value("KFpause").isEmpty())
            setKFpause(stream.attributes().value("KFpause").toString().toFloat());

        if(!stream.attributes().value("inProbe").isEmpty())
            setInProbe(stream.attributes().value("inProbe").toString().toInt());

        if(!stream.attributes().value("inPause").isEmpty())
            setInPause(stream.attributes().value("inPause").toString().toInt());

        if(!stream.attributes().value("inStop").isEmpty())
            setInStop(stream.attributes().value("inStop").toString().toInt());

        if(!stream.attributes().value("outAntidive").isEmpty())
            setOutAntidive(stream.attributes().value("outAntidive").toString().toInt());

        if(!stream.attributes().value("iSpindle").isEmpty())
            setISpindle(stream.attributes().value("iSpindle").toString().toInt());
        else
            setISpindle(0);

        ///old style 08/09/2022
        if(!getDevice()->getModule(WLDevice::typeMSpindle))
        {
            getDevice()->createModule(WLDevice::typeMSpindle);
        }

        if(!stream.attributes().value("SOut").isEmpty())
        {
            QStringList list=stream.attributes().value("SOut").toString().split(",");

            if(list.size()==2)
            {
                getSpindle()->setElementSOut(static_cast<typeElement>(list.at(0).toUShort())
                                             ,(quint8)list.at(1).toUShort());
            }
        }

        if(!stream.attributes().value("accSOut").isEmpty())
            getSpindle()->setAcc(stream.attributes().value("accSOut").toFloat());

        if(!stream.attributes().value("decSOut").isEmpty())
            getSpindle()->setDec(stream.attributes().value("decSOut").toFloat());

        if(!stream.attributes().value("fastChangeSOut").isEmpty())
            getSpindle()->setFastSOut(stream.attributes().value("fastChangeSOut").toInt());

        if(!stream.attributes().value("outENBSpindle").isEmpty())
            getSpindle()->setOutENB(stream.attributes().value("outENBSpindle").toString().toInt());

        stream.readNextStartElement();

        if(stream.name()==metaObject()->className()) break;
        if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

        if(stream.name()=="spindleData")
        {
            WLSpindleData sdata;

            sdata.inValue=stream.attributes().value("inValue").toString().toFloat();
            sdata.outValue=stream.attributes().value("outValue").toString().toFloat();

            getSpindle()->addDataSpindle(sdata);
        }

        //------------
    }

}

void  WLModulePlanner::readCommand(QByteArray Data)
{
    quint8 index,ui1,ui2,ui3,ui4;
    quint32 ui32;
    float f1,f2;
    qint32 l;

    QDataStream Stream(&Data,QIODevice::ReadOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Mutex.lock();

    Stream>>ui1;

    switch(ui1)
    {
    case comPlanner_setData:  Stream>>index;//index8

        setData(Stream);
        break;

    case sendPlanner_data:
        Stream>>ui1;//size
        Stream>>ui2;//ring
        Stream>>ui3;//status
        Stream>>ui4;//flags
        Stream>>ui32;//curID
        Stream>>f1;//k element complete
        Stream>>f2;//Star

        emit changedSOut(f2);

        if(!Stream.atEnd())
        {
            quint32 ring32;
            quint32 lastWrite;

            Stream>>ring32;
            Stream>>lastWrite;

            //qDebug()<<"WLModulePlanner sendPlanner_data ring32"<<ring32<<" lastWrite"<<lastWrite;
        }

        if(m_indexRingElementBuf==ui2)
        {
            m_curIdElementBuf=ui32;
            emit changedCurIdElement(m_curIdElementBuf,f1);
        }

        if((m_indexRingElementBuf==ui2)
                ||((m_status!=PLANNER_stop)&&(ui3==PLANNER_stop)))
        {
            m_curIdElementBuf=ui32;


            //qDebug()<<"WLModulePlanner ring"<<m_indexRingElementBuf<<ui2;

            if(m_status!=(statusPlanner)(ui3)
                    //||MPLANNER_pause==(statusPlanner)(ui3)
                    )
            {
                if((m_status!=PLANNER_stop)&&(ui3==PLANNER_stop))
                {
                    m_indexRingElementBuf=0;
                }

                m_status=(statusPlanner)(ui3);

                qDebug()<<"WLModulePlanner changed Status Planner"<<m_status;

                emit changedStatus(m_status);
            }


            if(Flags.m_Data!=ui4)
            {
                uint8_t lastFlag=Flags.m_Data;

                Flags.m_Data=ui4;

                if((Flags.m_Data&PLF_empty)!=(lastFlag&PLF_empty))
                {
                    qDebug()<<"WLModulePlanner changedFlag PLF_EMPTY"<<(Flags.m_Data&PLF_empty);
                    emit changedEmpty(Flags.m_Data&PLF_empty);
                }
            }

            if(m_free!=ui1){
                m_free=ui1;
                emit changedFree(m_free);
            }

        }
        break;


    case sendPlanner_rInProbe:
        mutexPos.lock();

        m_posProbeR.clear();

        for(ui1=0;ui1<(Data.size()-1)/4;ui1++){
            Stream>>l;
            m_posProbeR+=l;
        }
        m_validProbeR=true;


        mutexPos.unlock();

        qDebug()<<"WLModulePlanner detect RProbePlanner";

        emit changedProbe(true);
        break;

    case sendPlanner_fInProbe:
        mutexPos.lock();

        m_posProbeF.clear();
        for(ui1=0;ui1<(Data.size()-1)/4;ui1++){
            Stream>>l;
            m_posProbeF+=l;
        }
        m_validProbeF=true;

        mutexPos.unlock();

        qDebug()<<"WLModulePlanner detect FProbePlanner";

        emit changedProbe(false);
        break;

    case sendPlanner_stopPoint:
        mutexPos.lock();

        m_posStop.clear();
        for(ui1=0;ui1<(Data.size()-1)/4;ui1++){
            Stream>>l;
            m_posStop+=l;
        }
        m_validPosStop=true;

        mutexPos.unlock();

        qDebug()<<"WLModulePlanner detect stopPoint";
        break;

    case sendPlanner_pausePoint:
        mutexPos.lock();

        m_posPause.clear();
        for(ui1=0;ui1<(Data.size()-1)/4;ui1++){
            Stream>>l;
            m_posPause+=l;
        }
        m_validPosPause=true;

        mutexPos.unlock();

        qDebug()<<"WLModulePlanner detect pausePoint";
        break;

    case sendPlanner_curPoint:
        mutexPos.lock();

        m_posCur.clear();
        for(ui1=0;ui1<(Data.size()-1)/4;ui1++){
            Stream>>l;
            m_posCur+=l;
        }

        mutexPos.unlock();

        break;

    case  sendModule_prop: Stream>>ui1;
        m_sizeBuf=ui1;

        setReady(true);

        update();
        break;

    case sendModule_error:
        Stream>>ui1;  //Error
        Stream>>index;

        if(ui1>startIndexErrorModule)
        {
            qDebug()<<"WLModulePlanner error module"<<ui1;
            emit sendMessage("WLModulePlanner module"+getErrorStr(errorModule,ui1),"",-(int)(ui1));
        }
        else{
            switch(ui1)
            {
            case errorPlanner_waxis:    qDebug()<<"WLModulePlanner error axis"<<index;
                emit sendMessage("WLMotionPlanner+Axis:"+getErrorStr(errorAxis,index),"",-(int)(ui1));break;

            case errorPlanner_welement: qDebug()<<"WLModulePlanner error element"<<index;
                emit sendMessage("WLMotionPlanner+Elementis:"+getErrorStr(errorElementPlanner,index),"",-(int)(ui1));break;

            default:                    qDebug()<<"WLModulePlanner error"<<index;
                emit sendMessage("WLMotionPlanner"+getErrorStr(errorPlanner,ui1),"",-(int)(ui1));break;
            }
        }

        emit reset();
        break;

    case sendPlanner_signal:    Stream>>ui1;
        /*
                                  switch(ui1)
                                  {
                                  case _sigChangedSOut_f32: Stream>>f1; emit  ChangedSOut(f1); break;
                                  }
                                  */

        break;
    }

    Mutex.unlock();

}
