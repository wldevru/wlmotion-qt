#include "wldevice.h"
#include <QDebug>

#define initStream \
    \QMutexLocker locker(&BufMutex);\
    \QByteArray data;\
    \QDataStream Stream(&data,QIODevice::WriteOnly);\
    \
    \Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);\
    \Stream.setByteOrder(QDataStream::LittleEndian);

WLVersion WLDevice::maxVersion(2,2,21);
WLVersion WLDevice::minVersion(2,2,1);

WLDevice::WLDevice(QObject *parent):WLModule(nullptr)
{
    setParent(parent);

    setTypeModule(typeDevice);
    setDevice(this);

    m_HA.clear();

    m_nameDevice.clear();

    status=DEVICE_empty;

    m_timerSend=new QTimer;

    connect(m_timerSend,SIGNAL(timeout()),SLOT(sendData()));

    connect(&m_serialPort,&QSerialPort::readyRead,this,&WLDevice::readSlot);
    connect(&m_serialPort,SIGNAL(error(QSerialPort::SerialPortError)),this,SLOT(onErrorSerialPort(QSerialPort::SerialPortError)));

    connect(&m_udpSocket,&QUdpSocket::readyRead,this,&WLDevice::readSlot);

    m_timerEth = new QTimer;
    m_timerEth->setSingleShot(true);

    connect(m_timerEth,&QTimer::timeout,this,[=](){//qDebug()<<"timeout ack";
        sendEthData();});

    m_countTxPacket=0;
    m_countRxPacket=255;
}

WLDevice::~WLDevice()
{
    qDebug()<<"~WLDevice()";
    closeConnect();

    blockSignals(true);

    delete  m_timerSend;
    delete  m_timerEth;

    removeModules();
    qDebug()<<"~WLDevice()+";
}

void WLDevice::callPropModules()
{
    for(int i=0;i<m_modules.size();i++)
        m_modules[i]->callProp();
}

void WLDevice::callProp()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<static_cast<quint8>(comDev_getProp);

    WLModule::sendCommand(data);
}

void WLDevice::reset()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<static_cast<quint8>(comDev_resetAll);

    WLModule::sendCommand(data);
}

void WLDevice::sendEthData()
{    
    QMutexLocker locker(&ethernetMutex);

    qint64 ret=m_udpSocket.write(m_bufEth);

    if(ret==m_bufEth.size()){
        if(status==DEVICE_connect) {
            Flags.set(fl_waitack);

            if(m_timeoutAck==0){
                m_timeoutAck=ETH_MINTIMEOUTACK;
            }else{
                m_timeoutAck<<=1;

                if(m_timeoutAck>ETH_MAXTIMEOUTACK) {
                    m_timeoutAck=ETH_MAXTIMEOUTACK;
                    sendMessage(getNameDevice(),"detect maximal ping Ethernet",1);
                }
            }

            m_timerEth->start(m_timeoutAck);
        }
    }else {
        qDebug()<<"WLDevice:error send udp socket = "<<ret;
    }
}

void WLDevice::updateReady()
{
    foreach(WLModule *module,m_modules)
    {
        if(!module->isReady())
        {
            if(isReady()) setReady(false);

            qDebug()<<"WLDevice::updateReady() false size"<<m_modules.size()<<module->getTypeModule();
            return;
        }
    }
    qDebug()<<"WLDevice::updateReady() true size"<<m_modules.size();
    setReady(true);

    update();
}

void WLDevice::removeModules()
{
    while(!m_modules.isEmpty())
        delete (m_modules.takeLast());

    emit changedModules(m_modules.size());

    setReady(false);
}

void WLDevice::reconnectSerialPort()
{    
    //qDebug()<<"if reconnect"<<getStatus();
    QMutexLocker locker(&connectMutex);

    if(isOpenConnect()
            &&!m_serialPort.portName().isEmpty()
            //&&getStatus()==DEVICE_connect)
            )
    {
        qDebug()<<"reconnect serial port"<<m_serialPort.portName();
        if(m_serialPort.isOpen())
        {
            m_serialPort.close();
        }

        if(!m_serialPort.open(QIODevice::ReadWrite))
        {
            QTimer::singleShot(50,this,SLOT(reconnectSerialPort()));
            emit reconnected();
        }
        else
        {
            qDebug()<<"reconnect serial port true";
            if(getModuleConnect())
            {
                getModuleConnect()->restartHeart();
                getModuleConnect()->sendHeart();
            }
        }
    }
}

void WLDevice::callStatus()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<static_cast<quint8>(comDev_getStatus);

    WLModule::sendCommand(data);
}

QList<WLDeviceInfo> WLDevice::availableDevices()
{
    QElapsedTimer Timer;
    QList<WLDeviceInfo>  retDevicesInfo;
    QList<QSerialPortInfo> portList=QSerialPortInfo::availablePorts();
    QList<WLDevice*> Devices;

    qDebug()<<"WLDevice::availableDevices()";

    foreach(QSerialPortInfo portInfo,portList)
    {
        WLDevice *Device=new WLDevice;

        Device->initSerialPort(portInfo.portName());

        if(Device->openConnect())
            Devices+=Device;
        else
            delete Device;
    }

    QList <QHostAddress> adrList = QNetworkInterface::allAddresses();
    QList <QNetworkInterface> ifaces = QNetworkInterface :: allInterfaces ();
    QUdpSocket udpSocket;
    QHostAddress HA;
    quint16 port;
    int n;
    char buf[]={0,3,0,5};
    char dataBuf[512];
    QList <QHostAddress> HADList;
    QByteArray BA(buf,4);

    udpSocket.bind();
    // Interfaces iteration

    for (int i = 0; i < ifaces.size(); i++)
    {
        // Now get all IP addresses for the current interface
        QList<QNetworkAddressEntry> addrs = ifaces[i].addressEntries();

        // And for any IP address, if it is IPv4 and the interface is active, send the packet
        for (int j = 0; j < addrs.size(); j++)
            if ((addrs[j].ip().protocol() == QAbstractSocket::IPv4Protocol) && (addrs[j].broadcast().toString() != ""))
                udpSocket.writeDatagram(BA,addrs[j].broadcast(),ETH_UDPPORT);
    }


    Timer.start();

    while(Timer.elapsed()<500)
    {
        QCoreApplication::processEvents();

        n=udpSocket.readDatagram(dataBuf,512,&HA,&port);

        bool add=false;

        if(port==ETH_UDPPORT&&n!=0)
        {

            if(HADList.indexOf(HA)==-1)
            {
                add=true;
                if(add) HADList+=HA;
            }

        }
    }

    udpSocket.close();

    foreach(QHostAddress HAD,HADList)
    {
        WLDevice  *Device = new WLDevice;

        Device->initUdpSocket(HAD);
        Device->openConnect();

        Devices+=Device;
    }

    Timer.start();
    while(Timer.elapsed()<500)
    {
        QCoreApplication::processEvents();
    }

    while(!Devices.isEmpty())
    {
        WLDevice  *Device=Devices.takeFirst();

        if(!Device->getUID96().isEmpty())  retDevicesInfo+=Device->getInfo();

        delete Device;
    }

    return retDevicesInfo;
}

void WLDevice::reboot(uint8_t type)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<static_cast<quint8>(comDev_reboot)<<type;

    WLModule::sendCommand(data);

    QTimer::singleShot(100,this,SLOT(closeConnect()));
}

void WLDevice::setInfo(WLDeviceInfo info)
{
    setNameDevice(info.name);

    if(!isOpenConnect())
    {
        initSerialPort(info.comPort);
        initUdpSocket(info.HA);
    }

    setUID96(info.UID96);
    setVersion(info.version);
}

WLDeviceInfo WLDevice::getInfo()
{
    WLDeviceInfo info;

    info.name =getNameDevice();
    info.comPort=getPortName();
    info.HA=getHA();
    info.UID96  =getUID96();
    info.version=getVersion();
    info.typeFW=getTypeFW();

    info.modules.clear();

    foreach(WLModule *module,getModules()){
        info.modules+=module->getTypeModule();
    }

    return info;
}

void WLDevice::setUID96(QString _UID96)
{
    m_UID96=_UID96;
    emit changedUID96(m_UID96);
}

void WLDevice::setStatus(enum statusDevice _status)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<static_cast<quint8>(comDev_setStatus)<<static_cast<quint8>(_status);

    WLModule::sendCommand(data);

    status=_status;

    qDebug()<<"setStatus Device"<<_status;

    callStatus();
}

void WLDevice::callModules()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<static_cast<quint8>(comDev_getModule);
    qDebug()<<"comDev_getModule";
    WLModule::sendCommand(data);
}

void WLDevice::callVersion()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<static_cast<quint8>(comDev_getVersion);

    WLModule::sendCommand(data);
}

void WLDevice::off()
{
    if(m_serialPort.isOpen())
    {
        reset();
        //wait send
        while(!outBuf.isEmpty()); //ждём отправки
    }
}

QList<WLModule *> WLDevice::getModules() const
{
    return m_modules;
}

void WLDevice::sendCommand(QByteArray data)
{
    QMutexLocker locker(&outBufMutex);

    QString str(data);

    if(!data.isEmpty())
    {
        outBuf+=data.size()+1;
        outBuf+=data;
    }
}


void WLDevice::sendData()
{
    QMutexLocker locker0(&outBufMutex);
    QMutexLocker locker1(&ethernetMutex);

    int n;

    if(!outBuf.isEmpty())
    {
        if(m_serialPort.isOpen())
        {
            //qDebug()<<"send data USB"<<
            m_serialPort.write(outBuf);
            m_serialPort.flush();

            outBuf.clear();
        }
        else
        {
            if(m_udpSocket.isOpen()
                    &&(!Flags.get(fl_waitack)))
            {
                m_bufEth.clear();
                m_bufEth+=(++m_countTxPacket);


                if(outBuf.size()>ETH_MAXETHPACK)
                {
                    n=outBuf[0];

                    while((n+outBuf[n])<ETH_MAXETHPACK){  //only full packets
                        n+=outBuf[n];
                    }

                    m_bufEth+=outBuf.mid(0,n);

                    outBuf=outBuf.mid(n);
                }
                else
                {
                    m_bufEth+=outBuf;
                    outBuf.clear();
                }

                Flags.set(fl_waitack);
                QTimer::singleShot(0,this,SLOT(sendEthData()));
            }
        }
    }

}


bool WLDevice::initSerialPort(QString portName)
{
    closeConnect();
    m_serialPort.setPortName(portName);

    qDebug()<<getNameDevice()<<"Init Serial port:"<<portName;
    return true;
}

bool WLDevice::initUdpSocket(QHostAddress HA)
{
    closeConnect();

    m_HA=HA;

    qDebug()<<getNameDevice()<<"initUdpSocket"<<HA.toString();
    return true;
}

bool WLDevice::openConnect()
{
    QMutexLocker locker(&connectMutex);

    Flags.reset(fl_waitack);

    if(!m_serialPort.portName().isEmpty())
    {
        m_serialPort.close();

        if(!m_serialPort.open(QIODevice::ReadWrite))
        {
            qDebug()<<"no init serial Port WLDevice"<<m_serialPort.portName();

            sendMessage("WLDevice:","no device ("+m_serialPort.portName()+")",0);
            return false;
        }
        else
        {
            m_timerSend->start(DEFINE_TIMERWAITUSB);
            qDebug()<<"Open QSerialPort"<<m_serialPort.isOpen()<<thread();
        }
    }
    else
    {
        if(!m_HA.isNull())
        {
            if(!m_udpSocket.open(QIODevice::ReadWrite))
            {
                qDebug()<<"no init udp WLDevice"<<m_udpSocket.open(QIODevice::ReadWrite)<<m_udpSocket.bind(ETH_UDPPORT);
                return false;
            }
            else
            {
                m_timerSend->start(DEFINE_TIMERWAITETH);
                m_udpSocket.connectToHost(m_HA,ETH_UDPPORT);
            }

            m_bufEth.clear();
        }
    }

    Flags.set(fl_openconnect);

    callStatus();
    callProp();
    callVersion();
    callUID();

    callModules();

    reset();

    return true;
}

bool WLDevice::waitForReady(int timeout)
{
    QElapsedTimer Timer;

    Timer.start();

    while((Timer.elapsed()<timeout)&&(!isReady()))
    {
        QCoreApplication::processEvents();
    }

    return isReady();
}

void WLDevice::closeConnect()
{
    if(isOpenConnect())
    {
        if(getModuleConnect()){
            getModuleConnect()->setEnableHeart(false);
        }

        if(ethernetMutex.tryLock(250)){
            ethernetMutex.unlock();
        }else {
            qDebug()<<"error close connect";
        }

        setStatus(DEVICE_init);

        Flags.reset(fl_waitack);

        QElapsedTimer Timer;

        Timer.start();

        while(Timer.elapsed()<500)
        {
            QCoreApplication::processEvents();
        }

        Flags.reset(fl_openconnect);

        connectMutex.lock();


        if(m_serialPort.isOpen())
        {
            qDebug()<<"Close Serial Port";
            m_serialPort.close();
        }
        else if(m_udpSocket.isOpen())
        {
            qDebug()<<"Close UDP Socket";
            m_udpSocket.close();
            m_bufEth.clear();
        }

        m_timerSend->stop();
        m_timerEth->stop();

        connectMutex.unlock();

        emit changedConnect(Flags.reset(fl_connect));
        emit changedReady(false);
    }
}

void WLDevice::update()
{
    qDebug()<<"WLDevice::update()";
    callStatus();

    foreach(WLModule *Module,m_modules)
    {
        qDebug()<<Module->metaObject()->className()<<"update";
        Module->update();
    }
}

void WLDevice::backup()
{
    qDebug()<<"WLDevice::backup()";
    callStatus();
    setStatus(getStatus());

    foreach(WLModule *Module,m_modules)
    {
        qDebug()<<Module->metaObject()->className()<<"backup";
        Module->backup();
    }

}


void WLDevice::addModule(WLModule *module) 
{
    if(module!=nullptr)
    {
        module->setParent(this);

        for(int i=0;i<m_modules.size();i++)
            if(module->getTypeModule()==m_modules[i]->getTypeModule()) return;

        m_modules+=module;

        connect(module,SIGNAL(sendMessage(QString,QString,int)),SIGNAL(sendMessage(QString,QString,int)));
        connect(module,SIGNAL(changedReady(bool)),SLOT(updateReady()));

        module->callProp();

        emit changedModules(m_modules.size());
    }
}


WLModule* WLDevice::getModule(typeModule type)
{
    WLModule *ret=nullptr;

    for(int i=0;i<m_modules.size();i++)
        if(m_modules.at(i)->getTypeModule()==type)
        {
            ret=m_modules.at(i);
            break;
        }

    return ret;
}


void WLDevice::readSlot()
{
    QMutexLocker locker0(&inBufMutex);
    QMutexLocker locker1(&ethernetMutex);

    if(m_serialPort.isOpen())
    {
        inBuf+=m_serialPort.readAll();
    }
    else
    {
        //if(m_udpSocket.isOpen())
        {
            QHostAddress HA;
            quint16 port;
            char bufData[512];

            int n = m_udpSocket.readDatagram(bufData,sizeof(bufData),&HA,&port);

            if(HA==m_HA&&port==ETH_UDPPORT)
            {
                QByteArray byteArray(bufData,n);
                //qDebug()<<"n"<<n;
                if(n==1)  {
                    if(Flags.get(fl_waitack)) {
                        if(m_countTxPacket==(quint8)byteArray[0]) {
                            Flags.reset(fl_waitack);
                            m_timeoutAck=0;
                            m_timerEth->stop();
                        }else{
                            // qDebug()<<"error m_countTxPacket==(quint8)byteArray[0]"<<(m_countTxPacket==(quint8)byteArray[0]);
                        }
                    }
                }
                else if(n>1) {
                    m_udpSocket.write(byteArray.data(),1);

                    if(m_countRxPacket!=(quint8)byteArray[0]){ //если новый пакет то читаем его
                        if((quint8)((quint8)byteArray[0]-(quint8)m_countRxPacket)>1)
                            qDebug()<<"error count pack RX"<<(quint8)m_countRxPacket<<(quint8)byteArray[0];

                        m_countRxPacket=(quint8)byteArray[0];

                        inBuf=byteArray.mid(1);
                        /*
          byteArray=byteArray.mid(1);

          quint16 i=0;

          while(1) {
          if((i>byteArray.size())||((quint8)byteArray[i]==0)) {
            qDebug()<<"error pack";
            break;
            }

          i+=(quint8)byteArray[i];

          if(i==byteArray.size()){
            inBuf+=byteArray;
            decodeInputData();
            break;
            }
          }
          */


                    }
                }

            }
        }
    }

    decodeInputData();
}

void WLDevice::decodeInputData()
{
    quint8 ui1,ui2,ui3;
    quint16 ui16;

    quint8 index;

    QChar buf[256];
    QString bufStr;

    quint8 ui8;
    QString uid;

    int curi=0;
    quint8 size;

    QByteArray Data;

    while(!inBuf.isEmpty())
    {
        size=(quint8)inBuf[0];

        if(inBuf.size()<size)
        {
            break;
        }

        //qDebug()<<"size"<<size<<getPortName();
        if(size==0)
        {
            qDebug()<<"size==0 !!!";
            inBuf.clear();
            break;
        }

        curi++;

        Data=inBuf.mid(1,size-1);
        inBuf=inBuf.mid(size);

        emit readDataDevice();


        if(!Data.isEmpty())
        {
            QDataStream Stream(&Data,QIODevice::ReadOnly);

            Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            Stream.setByteOrder(QDataStream::LittleEndian);

            Stream>>ui1;

            switch(ui1)
            {
            case typeDevice:Stream>>ui1;
                switch(ui1)
                {
                case sendDev_userData:
                {
                    Stream>>ui1;
                    qDebug()<<"offset"<<ui1;
                    Stream>>ui2;
                    qDebug()<<"size"<<ui1;

                    for(int i=0;i<ui2;i++)
                    {
                        Stream>>ui1;
                        qDebug()<<"data["<<i<<"]="<<ui1;
                    }
                }
                    break;

                case sendModule_error:
                    Stream>>ui1;
                    Stream>>index;

                    qDebug()<<"WLDevice Detect Error"<<index<<ui1;

                    emit sendMessage("WLDevice "+getErrorStr(errorDevice,ui1),QString::number(index),-(int)(ui1));
                    //emit ChangedErrorDevice(index,ui1);

                    if(ui1==errorDevice_nomodule)
                    {
                        for(int i=0;i<m_modules.size();i++)
                            if(m_modules[i]->getTypeModule()==index)
                            {
                                Q_ASSERT(m_modules[i]->getTypeModule()==index);
                                delete (m_modules.takeAt(i));
                                break;
                            }
                    }
                    break;

                case sendDev_prop:
                    for(int i=0;i<(size-4);i++)
                    {
                        Stream>>ui1;
                        buf[i]=ui1;
                    }

                    bufStr=QString(buf,size-4);

                    if(!bufStr.isEmpty())
                    {
                        m_prop=bufStr;

                        QStringList List=m_prop.split(".");

                        setNameDevice(List.takeFirst());
                        m_typeFW=List.takeFirst();

                        if(getNameDevice()=="WLFW"){
                            setNameDevice(List.takeFirst());
                        }

                        emit changedProp(m_prop);
                        emit changedConnect(Flags.set(fl_connect));

                        setStatus(DEVICE_connect);

                        //if(getModuleConnect())
                        //           getModuleConnect()->setEnableHeart(true); //on/off user
                    }
                    break;

                case sendDev_Module:
                    for(int i=0;i<(size-3);i++)
                    {
                        Stream>>ui1;//f
                        if(getModule(static_cast<typeModule>(ui1))==nullptr)
                        {
                            createModule((typeModule)ui1);
                        }
                        buf[i]=ui1;
                    }

                    break;

                case sendDev_UID: for(quint8 i=0;i<(96/8);i++)
                    {
                        Stream>>ui8;
                        uid+=QString::number(ui8>>4,16);
                        uid+=QString::number(ui8&0xF,16);
                    }
                    setUID96(uid);
                    break;

                case sendDev_version:Stream>>ui1;
                    Stream>>ui2;
                    Stream>>ui3;

                    setVersion(WLVersion(ui1,ui2,ui3));
                    break;

                case sendDev_status: Stream>>ui1;

                    if(status==DEVICE_connect
                            &&ui1!=DEVICE_connect) {
                        emit sendMessage(getNameDevice(),"detect reset controller!!!",-1);
                        backup();
                    }

                    if(ui1!=status)//забивает статус
                        emit changedStatus(status=static_cast<statusDevice>(ui1));

                    break;

                case typeMFW:  if(getModule((typeModule)ui1)==nullptr) //oldFW
                    {
                        createModule((typeModule)ui1);
                    };

                    emit changedProp(m_prop);

                    setReady(true);

                    break;
                }
                break;
            default: for(int i=0;i<m_modules.size();i++)
                    if(m_modules[i]->getTypeModule()==ui1)
                    {
                        m_modules[i]->readCommand(Data.mid(1));
                        break;
                    }
                break;

            }
        }

        if(inBuf.isEmpty()) break;
    }
}

void WLDevice::onErrorSerialPort(QSerialPort::SerialPortError serialPortError)
{
    qDebug()<<"onErrorSerialPort"<<serialPortError;

    //QMutexLocker locker(&connectMutex);

    switch(serialPortError)
    {
    case QSerialPort::DeviceNotFoundError:if(getModuleConnect())
        {
            if(!getModuleConnect()->isEnableHeart())
            {
                sendMessage(getNameDevice(),"QSerialPort::DeviceNotFoundError",0);
                closeConnect();
            }
        }
        break;


    default: break;
    }

}



void WLDevice::callUID()
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDev_getUID;

    qDebug()<<"Call getUID";

    WLModule::sendCommand(data);
}

WLModule *WLDevice::createModule(typeModule type)
{
    WLModule *Module=getModule(type);

    if(Module==nullptr)
    {
        if(type==typeMConnect)
        {
            WLModuleConnect *MConnect = new WLModuleConnect(this);
            connect(this,&WLDevice::readDataDevice,MConnect,&WLModuleConnect::restartHeart);

            connect(MConnect,&WLModuleConnect::timeoutHeart,this,&WLDevice::reconnectSerialPort,Qt::DirectConnection);
            connect(MConnect,&WLModuleConnect::backupConnect,this,&WLDevice::update);

            Module=MConnect;
        }

        addModule(Module);
    }

    return Module;
}

WLModule *WLDevice::createModule(QString name)
{
    typeModule type;

    if(name=="WLModuleConnect") return createModule(typeMConnect);

    return nullptr;
}


void WLDevice::readXMLData(QXmlStreamReader &stream)
{
    QString port;
    int i;

    WLModule *Module;
    bool add=getUID96().isEmpty();

    qDebug()<<"WLDevice::readXMLData<<<";

    if(getNameDevice()==stream.name()
            ||getNameDevice().isEmpty())
    {
        setNameDevice(stream.name().toString());

        if(!stream.attributes().value("UID96").isEmpty())
            setUID96(stream.attributes().value("UID96").toString());

        while(!stream.atEnd())
        {
            stream.readNextStartElement();
            qDebug()<<stream.name();

            if(stream.name()==getNameDevice()) break;
            if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

            if(stream.name()=="Modules")
            {
                while(!stream.atEnd())
                {
                    stream.readNextStartElement();
                    qDebug()<<stream.name();
                    if(stream.name()=="Modules") break;
                    if(stream.tokenType()!=QXmlStreamReader::StartElement) {
                        continue;
                    }

                    for(i=0;i<m_modules.size();i++)
                        if(stream.name()==m_modules[i]->metaObject()->className())
                        {
                            m_modules[i]->readXMLData(stream);
                            break;
                        }

                    Module=nullptr;

                    if(i==m_modules.size()&&add)
                    {
                        Module=createModule(stream.name().toString());

                        if(Module)
                            Module->readXMLData(stream);
                    }
                }
            }
        }
    }

    qDebug()<<"WLDevice::readXMLData>>>"<<stream.atEnd();
}


void WLDevice::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeStartElement(getNameDevice());

    if(!m_serialPort.portName().isEmpty())  {
        stream.writeAttribute("VCP",m_serialPort.portName());
    }
    else
        if(!m_HA.isNull())  stream.writeAttribute("IP",m_HA.toString());

    stream.writeAttribute("UID96",getUID96());

    stream.writeStartElement("Modules");
    for(int i=0;i<m_modules.size();i++)
    {
        stream.writeStartElement(m_modules[i]->metaObject()->className());
        m_modules[i]->writeXMLData(stream);
        stream.writeEndElement();
    }
    stream.writeEndElement();

    stream.writeEndElement();
}


bool WLDevice::writeToFile(QString nameFile)
{
    qDebug()<<"WLDevice::writeToFile"<<getNameDevice()<<nameFile;

    QFile FileXML(nameFile);

    QByteArray Data;

    QXmlStreamWriter stream(&Data);

    stream.setAutoFormatting(true);

    stream.setCodec(QTextCodec::codecForName("Windows-1251"));
    stream.writeStartDocument("1.0");

    stream.writeStartElement("WLDevice");
    writeXMLData(stream);
    stream.writeEndElement();

    stream.writeEndDocument();

    if(FileXML.open(QIODevice::WriteOnly))
    {
        FileXML.write(Data.constData());
        FileXML.close();
        return true;
    }
    return false;
}

void WLDevice::setVersion(WLVersion _ver)
{
    m_version=_ver;

    emit changedVersion(m_version);

    if(!isValidVersion())
        sendMessage(getNameDevice(),QString(tr("no valid version current=%1 need=%2-%3")
                                            .arg(m_version.toString())
                                            .arg(minVersion.toString())
                                            .arg(maxVersion.toString())),0);
}

void WLDevice::clearBuf()
{
    QMutexLocker locker0(&inBufMutex);
    QMutexLocker locker1(&outBufMutex);

    outBuf.clear();
    inBuf.clear();
}

bool WLDevice::writeDataUser(quint8 offset,quint8 size,quint8 *buf)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDev_writeDataUser<<offset<<size;

    for(int i=0;i<size;i++)
        Stream<<buf[i];

    WLModule::sendCommand(data);

    return true;
}

bool WLDevice::readDataUser(quint8 offset, quint8 size)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comDev_readDataUser<<offset<<size;

    WLModule::sendCommand(data);

    return true;
}

bool WLDevice::initFromFile(QString nameFile)
{
    QFile FileXML(nameFile);
    QXmlStreamReader stream;

    qDebug()<<"WLDevice::initFromFile"<<nameFile;

    if(FileXML.open(QIODevice::ReadOnly))
    {
        stream.setDevice(&FileXML);

        while(!stream.atEnd())
        {
            stream.readNextStartElement();

            if(stream.name()=="WLDevice"
                    &&stream.tokenType()==QXmlStreamReader::StartElement)
            {
                stream.readNextStartElement();
                readXMLData(stream);
            }
        }

        FileXML.close();

        if(!getModuleConnect()) createModule(typeMConnect);

        return true;
    }
    return false;
}

