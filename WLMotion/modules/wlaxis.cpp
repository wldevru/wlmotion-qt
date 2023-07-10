#include "wlaxis.h"
#include "wlmoduleaxis.h"
#include "wldevice.h"
#include <limits>

WLAxis::WLAxis(WLModuleAxis *_ModuleAxis)
    :WLElement(_ModuleAxis)
{
    setTypeElement(typeEAxis);

    nowPosition=0;
    maxPosition=std::numeric_limits<qint32>::max();
    minPosition=std::numeric_limits<qint32>::min();
    homePosition=0;
    errPosition=0;

    status=AXIS_stop;
    mode=AXIS_alone;

    orgSize=1;

    kSTCurve=0;
    delaySCurve=0;
    kGear=1;

    Freq=0;
    m_latchPos2=0;
    m_latchPos3=0;

    error=0;

    typePulse=AXIS_pulse_SD;
    outSDinv=0;

    homePosition=0;
    orgSize=100;

    inORG=&WLIOPut::In0;
    inPEL=&WLIOPut::In0;
    inMEL=&WLIOPut::In0;
    inALM=&WLIOPut::In0;
    outRALM=&WLIOPut::Out;
    outENB=&WLIOPut::Out;

    actIn[AXIS_inALM]=WLIOPut::INPUT_actNo;
    actIn[AXIS_inPEL]=WLIOPut::INPUT_actNo;
    actIn[AXIS_inMEL]=WLIOPut::INPUT_actNo;
    actIn[AXIS_inORG]=WLIOPut::INPUT_actNo;

    resetLatch();
}

WLAxis::~WLAxis()
{
}


WLIOPut*  WLAxis::getInput(typeInputAxis type)
{
    WLIOPut *ret=nullptr;
    switch(type)
    {
    case AXIS_inORG: ret=inORG;break;
    case AXIS_inALM: ret=inALM;break;
    case AXIS_inPEL: ret=inPEL;break;
    case AXIS_inMEL: ret=inMEL;break;

    default: ret=&WLIOPut::In0;
    }

    return ret;
}

WLIOPut*  WLAxis::getOutput(typeOutputAxis type)
{
    WLIOPut *ret=nullptr;
    switch(type)
    {
    case AXIS_outRALM: ret=outRALM;break;
    case AXIS_outENB:  ret=outENB; break;

    default: ret=&WLIOPut::Out;
    }
    return ret;
}


void  WLAxis::setInALM(int index)
{
    inALM->removeComment("inALM"+QString::number(getIndex()));
    disconnect(inALM,&WLIOPut::changed,this,&WLAxis::changedInALM);

    WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getModule()->getDevice()->getModule(WLDevice::typeMIOPut));

    if(index>=ModuleIOPut->getSizeInputs()) index=0;

    inALM=ModuleIOPut->getInput(index);
    inALM->addComment("inALM"+QString::number(getIndex()));

    setInput(AXIS_inALM,index);

    connect(inALM,&WLIOPut::changed,this,&WLAxis::changedInALM,Qt::QueuedConnection);
}

void  WLAxis::setInORG(int index)
{
    inORG->removeComment("inORG"+QString::number(getIndex()));

    WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getModule()->getDevice()->getModule(WLDevice::typeMIOPut));

    if(index>=ModuleIOPut->getSizeInputs()) index=0;

    inORG=ModuleIOPut->getInput(index);
    inORG->addComment("inORG"+QString::number(getIndex()));

    setInput(AXIS_inORG,index);
}


void  WLAxis::setInPEL(int index)
{
    inPEL->removeComment("inPEL"+QString::number(getIndex()));

    WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getModule()->getDevice()->getModule(WLDevice::typeMIOPut));

    if(index>=ModuleIOPut->getSizeInputs()) index=0;

    inPEL=ModuleIOPut->getInput(index);
    inPEL->addComment("inPEL"+QString::number(getIndex()));

    setInput(AXIS_inPEL,index);
}

void  WLAxis::setInMEL(int index)
{
    inMEL->removeComment("inMEL"+QString::number(getIndex()));

    WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getModule()->getDevice()->getModule(WLDevice::typeMIOPut));

    if(index>=ModuleIOPut->getSizeInputs()) index=0;

    inMEL=ModuleIOPut->getInput(index);
    inMEL->addComment("inMEL"+QString::number(getIndex()));

    setInput(AXIS_inMEL,index);
}


void  WLAxis::setOutRALM(int index)
{
    outRALM->removeComment("outRALM"+QString::number(getIndex()));

    WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getModule()->getDevice()->getModule(WLDevice::typeMIOPut));

    if(index>=ModuleIOPut->getSizeOutputs()) index=0;

    outRALM=ModuleIOPut->getOutput(index);
    outRALM->addComment("outRALM"+QString::number(getIndex()));
}


void  WLAxis::setOutENB(int index)
{
    outENB->removeComment("outENB"+QString::number(getIndex()));

    WLModuleIOPut *ModuleIOPut=static_cast<WLModuleIOPut*>(getModule()->getDevice()->getModule(WLDevice::typeMIOPut));

    if(index>=ModuleIOPut->getSizeOutputs()) index=0;

    outENB=ModuleIOPut->getOutput(index);
    outENB->addComment("outENB"+QString::number(getIndex()));

    setOutput(AXIS_outENB,index);
}


bool WLAxis::setTypePulse(typePulseAxis type,quint8 _SDinv)
{
    typePulse=type;
    outSDinv=_SDinv&(MAF_invStep|MAF_invDir);

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setTypePulse<<getIndex()<<((((quint8)typePulse)<<4)|outSDinv);

    sendCommand(data);
    return true;
}

bool WLAxis::setKSTCurve(float _kSTCurve)
{
    if(0<=_kSTCurve&&_kSTCurve<=1)
    {
        kSTCurve=_kSTCurve;

        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream<<(quint8)comAxis_setKSTCurve<<getIndex()<<kSTCurve;

        sendCommand(data);
        return true;
    }
    else
        return false;
}

bool WLAxis::setDelaySCurve(float _delaySCurve,bool save)
{
    if(_delaySCurve>=0)
    {
        if(save)
            delaySCurve=_delaySCurve;

        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream<<(quint8)comAxis_setDelaySCurve<<getIndex()<<_delaySCurve;

        sendCommand(data);
        return true;
    }
    else
        return false;
}

bool WLAxis::setKGear(float _kGear)
{
    if(_kGear>0)
    {
        kGear=_kGear;

        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream<<(quint8)comAxis_setKGear<<getIndex()<<kGear;

        sendCommand(data);
        return true;
    }
    else
        return false;
}

bool WLAxis::setEnable(bool enable)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setEnable<<getIndex()<<(quint8)enable;

    sendCommand(data);
    return true;
}

bool WLAxis::setModeSub(quint8 imasterAxis)
{
    m_iMasterAxis=imasterAxis;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setModeSub<<getIndex()<<imasterAxis;

    sendCommand(data);
    return true;
}

bool WLAxis::setEncoder(quint8 _iEncoder)
{
    iEncoder=_iEncoder;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setEncoder<<getIndex()<<iEncoder;

    sendCommand(data);
    return true;
}

bool WLAxis::setPidData(WLPidData _pidData)
{
    if(!_pidData.isValid()) return false;

    m_pidData=_pidData;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setPid<<getIndex()
         <<m_pidData.p
        <<m_pidData.i
       <<m_pidData.d
      <<m_pidData.ffp
     <<m_pidData.ffd;

    sendCommand(data);
    return true;
}


bool WLAxis::setTypeMotor(typeMotorAxis _typeMotor)
{
    typeMotor=_typeMotor;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setTypeMotor<<getIndex()<<(quint8)typeMotor;

    sendCommand(data);
    return true;
}

bool WLAxis::setErrorPidData(WLErrorPidData _errPidData)
{
    if(!_errPidData.isValid())
        return false;


    m_errorPidData=_errPidData;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setErrPid<<getIndex()<<(float)m_errorPidData.Fmin
         <<(float)m_errorPidData.Emin
        <<(float)m_errorPidData.Fmax
       <<(float)m_errorPidData.Emax;

    sendCommand(data);
    return true;
}


void WLAxis::update()
{
    sendGetDataAxis();
}

void WLAxis::backup()
{
    setEnable(isEnable());
    setDisableLimit(isDisableLimit());

    setTypePulse((typePulseAxis)getTypePulse(),getOutSDInv());

    setInORG(getInput(AXIS_inORG)->getIndex());
    setInPEL(getInput(AXIS_inPEL)->getIndex());
    setInMEL(getInput(AXIS_inMEL)->getIndex());
    setInALM(getInput(AXIS_inALM)->getIndex());

    setActIn(AXIS_inPEL,getActIn(AXIS_inPEL));
    setActIn(AXIS_inMEL,getActIn(AXIS_inMEL));
    setActIn(AXIS_inALM,getActIn(AXIS_inALM));

    setOutENB(getOutput(AXIS_outENB)->getIndex());
    setOutRALM(getOutput(AXIS_outRALM)->getIndex());

    setMinMaxPos(getMinPos(),getMaxPos());
    setKGear(getKGear());
    setDelaySCurve(getDelaySCurve());

    setPidData(m_pidData);
    setErrorPidData(m_errorPidData);

    if(getMode()==AXIS_sub)
        setModeSub(m_iMasterAxis);
}



bool WLAxis::setInput(typeInputAxis type,quint8 num)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setInput<<getIndex()<<(quint8)type<<num;

    sendCommand(data);
    return true;
}

bool WLAxis::setOutput(typeOutputAxis type,quint8 num)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setOutput<<getIndex()<<(quint8)type<<num;

    sendCommand(data);
    return true;
}

bool WLAxis::setDisableLimit(bool dis)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setDisableLimit<<getIndex()<<(quint8)dis;

    Flags.set(AF_disableLimit);

    sendCommand(data);
    return true;
}

bool WLAxis::setDisableManual(bool dis)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setDisableManual<<getIndex()<<(quint8)dis;

    Flags.set(AF_disableManual);

    sendCommand(data);
    return true;
}

bool WLAxis::setInLatch(quint8 index)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setLatchSrc<<getIndex()<<index;

    sendCommand(data);
    return true;
}

bool WLAxis::setActIn(typeInputAxis type,WLIOPut::typeActionInput typeAct)
{
    actIn[type]=typeAct;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setActIn<<getIndex()<<(quint8)type<<(quint8)typeAct;

    sendCommand(data);
    return true;
}

bool WLAxis::sendGetDataAxis()
{
    qDebug()<<"WLAxis::sendGetDataAxis"<<getIndex();

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_getDataAxis<<getIndex();

    sendCommand(data);
    return true;
}

bool WLAxis::setMPar(float Aac,float Ade,float Fst,float Fma,typeMParAxis type)
{
    WLMParAxis MPar(Aac,Ade,Fst,Fma);

    return setMPar(MPar,type);
}


bool WLAxis::setMPar(WLMParAxis MPar,typeMParAxis type)
{
    if(MPar.isValid()){
        switch(type)
        {
        case AXIS_StepMotorMParPlus:  stepMotorMParPlus=MPar;  break;
        case AXIS_StepMotorMParMinus: stepMotorMParMinus=MPar; break;
        case AXIS_StepMotorMParAll:   stepMotorMParPlus=
                    stepMotorMParMinus=MPar; break;
        default: break;
        }
    }
    else {
        return false;
    }

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setParMov<<getIndex()
         <<MPar.Aacc
        <<MPar.Adec
       <<MPar.Fst
      <<MPar.Fmax<<(static_cast<quint8>(type));

    sendCommand(data);
    return true;
}

bool WLAxis::addMParList(float Aac, float Ade, float Fst, float Fma, QString name)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_addMParList<<getIndex()
         <<Aac
        <<Ade
       <<Fst
      <<Fma;

    data.append(name);
    data.append('\0');


    qDebug()<<"WLAxis::addMParList"<<name.data()<<data.size()<<name.toUtf8().size();

    sendCommand(data);
    return true;
}

bool WLAxis::clearMParList()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_clearMParList<<getIndex();

    sendCommand(data);
    return true;
}

bool WLAxis::movPos(quint8 mask,qint32 Dist,float Fmov)
{
    qDebug()<<"WLAxis::movPos index="<<getIndex()<<" Dist="<<Dist<<" Fmov="<<Fmov;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_movPos<<mask<<getIndex()<<Dist<<Fmov;

    status=AXIS_wait;

    sendCommand(data);
    return true;
}

bool WLAxis::movVel(quint8 mask,float Fmov)
{
    qDebug()<<"WLAxis::movVel index="<<getIndex()<<" Fmov="<<Fmov;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_movVel<<mask<<getIndex()<<Fmov;

    status=AXIS_wait;

    sendCommand(data);
    return true;
}

bool WLAxis::trackPos(quint8 mask, qint32 Dist, float Fmov)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_trackPos<<mask<<getIndex()<<Dist<<Fmov;

    status=AXIS_wait;

    sendCommand(data);
    return true;
}

bool WLAxis::acc()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_acc<<(quint8)getIndex();

    sendCommand(data);
    return true;
}

bool WLAxis::pause()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_pause<<(quint8)getIndex();

    sendCommand(data);
    return true;
}

bool WLAxis::sdStop()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_sdStop<<(quint8)getIndex();

    qDebug()<<"sdStop axis"<<getIndex();

    sendCommand(data);
    return true;
}

bool WLAxis::setNewF(float newF)
{
    qDebug()<<"WLAxis;setNewF;"<<getIndex()<<"setNewF"<<newF;

    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_newF<<(quint8)getIndex()<<newF;

    sendCommand(data);
    return true;
}

bool WLAxis::emgStop()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    qDebug()<<data.size();

    Stream<<(quint8)comAxis_emgStop<<(quint8)getIndex();

    sendCommand(data);
    return true;
}

bool WLAxis::addSyhData(qint32 pos)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_addSyhData<<(quint8)getIndex()<<pos;

    sendCommand(data);
    return true;
}

bool WLAxis::setPos(qint32 pos)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_setPos<<(quint8)getIndex()<<pos;

    sendCommand(data);
    return true;
}

bool WLAxis::setMinMaxPos(qint32 minpos,qint32 maxpos)
{
    if(minpos<maxpos)
    {
        minPosition=minpos;
        maxPosition=maxpos;

        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream<<(quint8)comAxis_setMinMaxPos<<(quint8)getIndex()<<minPosition<<maxPosition;

        sendCommand(data);
        return true;
    }
    else
        return false;
}

bool WLAxis::setKF(float _KFA)
{
    if(_KFA>0)
    {
        QByteArray data;
        QDataStream Stream(&data,QIODevice::WriteOnly);

        Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        Stream.setByteOrder(QDataStream::LittleEndian);

        Stream<<(quint8)comAxis_setKF<<(quint8)getIndex()<<_KFA;

        sendCommand(data);
        return true;
    }
    return false;
}

void WLAxis::reset()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_reset<<(quint8)getIndex();

    sendCommand(data);
}

void WLAxis::setDataAxis(quint8 statusMode,quint8 _flag,qint32 Pos,float F)
{
    Flags.m_Data=_flag;

    statusAxis lstatus=status;

    statusAxis nstatus=static_cast<statusAxis>(statusMode>>4);
    modeAxis nmode  =static_cast<modeAxis>(statusMode&0x0F);


    if(status!=nstatus)
    {
        qDebug()<<"WLAxis"<<getIndex()<<"status"<<nstatus;

        if(nowPosition!=Pos)
            emit changedPosition(nowPosition=Pos);

        emit changedStatus(status=nstatus);

        if((lstatus!=statusAxis::AXIS_stop)
                &&(nstatus==statusAxis::AXIS_stop)
                &&(mode!=modeAxis::AXIS_slave))  emit finished();

        if((lstatus==statusAxis::AXIS_stop)
                &&(nstatus!=statusAxis::AXIS_stop)
                &&(mode!=modeAxis::AXIS_slave)) emit started();

    }

    if(mode!=nmode)    emit changedMode(mode=nmode);
    if(Freq!=F)        emit changedFreq(Freq=F);
}

void WLAxis::setData(QDataStream &data)
{
    quint8 type;

    //qDebug()<<"Axis::setData";
    data>>type;

    switch((typeDataAxis)type)
    {
    case dataAxis_pos:    data>>nowPosition;
        emit changedPosition(nowPosition);
        break;

    case dataAxis_posError:data>>errPosition;
        emit changedErrPosition(errPosition);
        break;

    case dataAxis_Fsum:      data>>Freq;
        emit changedFreq(Freq);
        break;

    case dataAxis_latchR: data>>m_latchPos2;
        m_validLatchR=true;
        emit changedLatchR(m_latchPos2);
        break;

    case dataAxis_latchF: data>>m_latchPos3;
        m_validLatchF=true;
        emit changedLatchF(m_latchPos3);
        break;

    }
}

void WLAxis::getData(typeDataAxis type)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comAxis_getData<<(quint8)getIndex()<<(quint8)type;

    sendCommand(data);
}


void WLAxis::readXMLData(QXmlStreamReader &stream)
{
    setEnable(false);

    if(!stream.attributes().value("kSTCurve").isEmpty())  	  setKSTCurve(stream.attributes().value("kSTCurve").toString().toFloat());

    if(!stream.attributes().value("delaySCurve").isEmpty())   setDelaySCurve(stream.attributes().value("delaySCurve").toString().toFloat());

    if(!stream.attributes().value("typePulse").isEmpty()
            &&!stream.attributes().value("outSDinv").isEmpty())  setTypePulse((typePulseAxis)stream.attributes().value("typePulse").toString().toInt()
                                                                              ,stream.attributes().value("outSDinv").toString().toInt());

    if(!stream.attributes().value("inORG").isEmpty()) setInORG(stream.attributes().value("inORG").toString().toInt());

    if(!stream.attributes().value("inALM").isEmpty()) setInALM(stream.attributes().value("inALM").toString().toInt());
    if(!stream.attributes().value("inPEL").isEmpty()) setInPEL(stream.attributes().value("inPEL").toString().toInt());
    if(!stream.attributes().value("inMEL").isEmpty()) setInMEL(stream.attributes().value("inMEL").toString().toInt());

    if(!stream.attributes().value("actInORG").isEmpty()) setActIn(AXIS_inORG,static_cast<WLIOPut::typeActionInput>(stream.attributes().value("actInORG").toString().toInt()));

    if(!stream.attributes().value("actInALM").isEmpty()) setActIn(AXIS_inALM,static_cast<WLIOPut::typeActionInput>(stream.attributes().value("actInALM").toString().toInt()));
    if(!stream.attributes().value("actInPEL").isEmpty()) setActIn(AXIS_inPEL,static_cast<WLIOPut::typeActionInput>(stream.attributes().value("actInPEL").toString().toInt()));
    if(!stream.attributes().value("actInMEL").isEmpty()) setActIn(AXIS_inMEL,static_cast<WLIOPut::typeActionInput>(stream.attributes().value("actInMEL").toString().toInt()));

    if(!stream.attributes().value("outRALM").isEmpty())	setOutRALM(stream.attributes().value("outRALM").toString().toInt());
    if(!stream.attributes().value("outENB").isEmpty())	setOutENB(stream.attributes().value("outENB").toString().toInt());

    if(!stream.attributes().value("encoder").isEmpty()) setEncoder(stream.attributes().value("encoder").toInt());

    if(!stream.attributes().value("pid").isEmpty())
    {
        WLPidData pid;
        pid.fromString(stream.attributes().value("pid").toString());
        setPidData(pid);
    }

    WLMParAxis MPar;

    if(!stream.attributes().value("stepMotorMParPlus").isEmpty()) {
        MPar.fromString(stream.attributes().value("stepMotorMParPlus").toString());
        setMPar(MPar,AXIS_StepMotorMParPlus);
    }

    if(!stream.attributes().value("stepMotorMParMinus").isEmpty()) {
        MPar.fromString(stream.attributes().value("stepMotorMParMinus").toString());
        setMPar(MPar,AXIS_StepMotorMParMinus);
    }

    if(!stream.attributes().value("typeMotor").isEmpty()){
        setTypeMotor((typeMotorAxis)stream.attributes().value("typeMotor").toInt());
    }
    else {
        setTypeMotor(AXIS_stepMotor);
    }

    if(!stream.attributes().value("errPid").isEmpty())
    {
        WLErrorPidData errorPid;
        errorPid.fromString(stream.attributes().value("errPid").toString());
        setErrorPidData(errorPid);
    }
}

void WLAxis::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeAttribute("kSTCurve",   QString::number(getKSTCurve()));
    stream.writeAttribute("delaySCurve",QString::number(getDelaySCurve()));

    stream.writeAttribute("typeMotor",QString::number(getTypeMotor()));
    stream.writeAttribute("typePulse",QString::number(typePulse));
    stream.writeAttribute("outSDinv", QString::number(outSDinv));

    stream.writeAttribute("inORG",QString::number(inORG->getIndex()));
    stream.writeAttribute("actInORG",QString::number(actIn[AXIS_inORG]));

    stream.writeAttribute("inPEL",QString::number(inPEL->getIndex()));
    stream.writeAttribute("actInPEL",QString::number(actIn[AXIS_inPEL]));

    stream.writeAttribute("inMEL",QString::number(inMEL->getIndex()));
    stream.writeAttribute("actInMEL",QString::number(actIn[AXIS_inMEL]));

    stream.writeAttribute("inALM",QString::number(inALM->getIndex()));
    stream.writeAttribute("actInALM",QString::number(actIn[AXIS_inALM]));

    stream.writeAttribute("outRALM",QString::number(outRALM->getIndex()));

    stream.writeAttribute("outENB",QString::number(outENB->getIndex()));

    stream.writeAttribute("encoder",QString::number(getEncoder()));
    stream.writeAttribute("pid",m_pidData.toString());

    stream.writeAttribute("errPid",getErrorPidData().toString());

    stream.writeAttribute("stepMotorMParPlus",stepMotorMParPlus.toString());
    stream.writeAttribute("stepMotorMParMinus",stepMotorMParMinus.toString());
}
