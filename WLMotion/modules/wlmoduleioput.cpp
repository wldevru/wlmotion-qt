#include "wlmoduleioput.h"
#include "wldevice.h"

WLModuleIOPut::WLModuleIOPut(WLDevice *_Device)
    : WLModule(_Device)
{ 
    setTypeModule(typeMIOPut);

    Init(2,1);
}

WLModuleIOPut::~WLModuleIOPut()
{
    while(Inputs.isEmpty())
        delete Inputs.takeLast();

    while(Outputs.isEmpty())
        delete Outputs.takeLast();
}

bool WLModuleIOPut::Init(int _sizeInputs, int _sizeOutputs)
{
    if(InitInputs(_sizeInputs)&&InitOutputs(_sizeOutputs)){
        return true;
    }
    else {
        return false;
    }
}

bool WLModuleIOPut::InitInputs(int sizeInputs)
{
    if(sizeInputs<2
            ||Inputs.size()==sizeInputs
            ||isReady()) return false;

    WLIOPut *input;

    if(Inputs.size()<sizeInputs)
        for(int i=Inputs.size();i<sizeInputs;i++)
        {
            input = new WLIOPut(this,"",true);
            input->setIndex(i);
            input->setParent(this);
            connect(input,SIGNAL(changed(int)),SIGNAL(changedInput(int)));
            Inputs+=input;
        }
    else
        while(Inputs.size()!=sizeInputs)
        {
            input = Inputs.takeLast();
            disconnect(input,SIGNAL(changed(int)),this,SIGNAL(changedInput(int)));
            delete input;
        }

    Inputs[0]->setData(0);
    Inputs[1]->setData(IOPF_now|IOPF_old);

    Inputs[0]->setComment("inLock0");
    Inputs[1]->setComment("inLock1");

    return true;
}

bool WLModuleIOPut::InitOutputs(int sizeOutputs)
{
    if(sizeOutputs<1
            ||Outputs.size()==sizeOutputs
            ||isReady()) return false;

    WLIOPut *output;

    if(Outputs.size()<sizeOutputs)
        for(quint8 i=Outputs.size();i<sizeOutputs;i++)
        {
            output = new WLIOPut(this);
            output->setIndex(i);
            output->setParent(this);
            Outputs+=output;
            connect(output,SIGNAL(changed(int)),SIGNAL(changedOutput(int)));
        }
    else
        while(Outputs.size()!=sizeOutputs)
        {
            output = Outputs.takeLast();
            disconnect(output,SIGNAL(changed(int)),this,SIGNAL(changedOutput(int)));
            delete output;
        }

    //QTimer *timer=new QTimer;
    //connect(timer,SIGNAL(timeout()),SLOT(sendDataOutput()));
    //timer->start(3);

    return true;
}

WLIOPut *WLModuleIOPut::getInput(int index)
{
    Q_ASSERT((index<Inputs.size())&&(index<255));

    return index<Inputs.size() ?  Inputs[index] :nullptr;
}

WLIOPut *WLModuleIOPut::getOutput(int index)
{
    Q_ASSERT((index<Outputs.size())&&(index<255));

    return index<Outputs.size()?  Outputs[index]:nullptr;
}

void WLModuleIOPut::readCommand(QByteArray Data)
{
    quint8 index,ui1,ui2;

    QDataStream Stream(&Data,QIODevice::ReadOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream>>ui1;

    //qDebug()<<"read MIOPut";

    switch(ui1)
    {
    case sendIOPut_inputBData:
        Stream>>ui1;

        while(!Stream.atEnd()){
            Stream>>ui2;

            for(int i=0;i<8;i++)
            {
                int index=(i+(ui1)*8);

                if(index<getSizeInputs())
                    Inputs[index]->setNow((ui2>>i)&0x1);
            }

            ui1++;
        }

        break;

    case sendIOPut_ioputData:

        Stream>>index;
        Stream>>ui1;

        if(ui1&IOPF_input)
        {
            if(index<getSizeInputs())  Inputs[index]->setData(ui1);
        }
        else
        {
            if(index<getSizeOutputs()) Outputs[index]->setData(ui1);
        }
        break;

    case  sendModule_prop: Stream>>ui1;
        Stream>>ui2;

        Init(ui1,ui2);

        setReady(true);
        break;

    case sendModule_error: Stream>>ui1;  //Error
        Stream>>index;

        emit sendMessage(metaObject()->className()+getErrorStr(errorIOPut,ui1),QString::number(index),-(ui1));
        break;
    }
}



void WLModuleIOPut::setInputInvStr(QString data)
{
    qDebug()<<"WLModuleIOPut::setInputInvStr"<<data;
    QStringList List=data.split(",");

    for(int i=2;i<getSizeInputs();i++)
        getInput(i)->setInv(List.indexOf(QString::number(i))!=-1);
}

QString WLModuleIOPut::getInputInvStr()
{
    QString ret;

    for(int i=2;i<Inputs.size();i++)
    {
        if(Inputs[i]->isInv())
        {
            if(!ret.isEmpty())  ret+=",";
            ret+=QString::number(i);
        }
    }

    return  ret;
}

void WLModuleIOPut::setOutputInvStr(QString data)
{
    qDebug()<<"WLModuleIOPut::setOutputInvStr"<<data;
    QStringList List=data.split(",");

    for(int i=1;i<getSizeOutputs();i++)
        getOutput(i)->setInv(List.indexOf(QString::number(i))!=-1);
}

QString WLModuleIOPut::getOutputInvStr()
{
    QString ret;

    for(int i=1;i<Outputs.size();i++)
    {
        if(Outputs[i]->isInv())
        {
            if(!ret.isEmpty())  ret+=",";
            ret+=QString::number(i);
        }
    }

    return  ret;
}

void WLModuleIOPut::setDefInvOutputs()
{
    for(int i=1;i<getSizeOutputs();i++)
        getOutput(i)->setDefInvOut();
}

void WLModuleIOPut::callInputData(quint8 index)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comIOPut_getInputData<<index;

    sendCommand(data);
}

void WLModuleIOPut::callOutputData(quint8 index)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    Stream<<(quint8)comIOPut_getOutputData<<index;

    sendCommand(data);
}

void WLModuleIOPut::sendSetOData(bool all)
{
    QByteArray data;
    QDataStream Stream(&data,QIODevice::WriteOnly);

    Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Stream.setByteOrder(QDataStream::LittleEndian);

    quint8 out,mask;
    int index;

    for(quint8 i=0;i<Outputs.size();i++)
    {
        if(Outputs[i]->getOld()!=Outputs[i]->getNow()||all)
        {
            i/=8;
            out=0;
            mask=0;

            for(quint8 j=0,b=1;j<8;j++,b<<=1)
            {
                index=i*8+j;
                // qDebug()<<index<<output[index];
                mask |= (Outputs[index]->getOld()!=Outputs[index]->getNow()||all )? b:0;
                out  |= (Outputs[index]->getNow()!=0)? b:0;

                //outputLast[index]=outputNow[index];
            }
            //qDebug()<<"setOutput"<<i<<out<<mask<<"BA";
            Stream<<(quint8)comIOPut_setOutputByte<<i<<out<<mask;
            sendCommand(data);

            Stream.device()->reset();
            data.clear();

            i=(i+1)*8;
        }
    }
}

void WLModuleIOPut::update()
{
    for(int i=2;i<getSizeInputs();i++) {
        callInputData(i);
    }

    for(int i=1;i<getSizeOutputs();i++) {
        callOutputData(i);
    }
}

void WLModuleIOPut::backup()
{
    for(int i=2;i<getSizeInputs();i++)
    {
        Inputs[i]->setInv(Inputs[i]->isInv());
        callInputData(i);
    }

    for(int i=1;i<getSizeOutputs();i++)
    {
        Outputs[i]->setInv(Outputs[i]->isInv());
        Outputs[i]->setOut(Outputs[i]->getNow());
    }
}

void WLModuleIOPut::readXMLData(QXmlStreamReader &stream)
{
    int  size=16;
    //if(!stream.attributes().value("invOutput").isEmpty()) setOutputInvStr(stream.attributes().value("invOutput").toString());

    while(!stream.atEnd())
    {
        stream.readNextStartElement();

        if(stream.name()==metaObject()->className()) break;
        if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

        if(stream.name()=="inputs")
        {
            size=16;

            if(!stream.attributes().value("size").isEmpty())    size=stream.attributes().value("size").toString().toInt();

            InitInputs(size);

            if(!stream.attributes().value("inv").isEmpty())   setInputInvStr(stream.attributes().value("inv").toString());

            while(!stream.atEnd())
            {
                stream.readNextStartElement();

                if(stream.name()=="inputs") break;
                if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

            }
        }

        if(stream.name()=="outputs")
        {
            size=16;

            if(!stream.attributes().value("size").isEmpty())    size=stream.attributes().value("size").toString().toInt();

            InitOutputs(size);

            if(!stream.attributes().value("inv").isEmpty())   setOutputInvStr(stream.attributes().value("inv").toString());

            while(!stream.atEnd())
            {
                stream.readNextStartElement();

                if(stream.name()=="outputs") break;
                if(stream.tokenType()!=QXmlStreamReader::StartElement) continue;

            }
        }
    }

}

void WLModuleIOPut::writeXMLData(QXmlStreamWriter &stream)
{
    stream.writeStartElement("inputs");
    stream.writeAttribute("size", QString::number(getSizeInputs()));
    stream.writeAttribute("inv",getInputInvStr());

    stream.writeEndElement();

    stream.writeStartElement("outputs");
    stream.writeAttribute("size", QString::number(getSizeOutputs()));
    stream.writeAttribute("inv",getOutputInvStr());

    stream.writeEndElement();
}
