#ifndef WLSpindle_H
#define WLSpindle_H

#include <QObject>
#include <QDataStream>
#include <QTimer>
#include <QDebug>

#include "wlflags.h"
#include "wlmoduleioput.h"

//Spindle
#define comSpindle_setEnable   1 //

#define comSpindle_addData     2 //
#define comSpindle_clearData   3 //

#define comSpindle_setOutElement  4 //
#define comSpindle_resetOutElement  5 //

#define comSpindle_setAcc  6 //
#define comSpindle_setDec  7 //

#define comSpindle_setOutput 8 //
#define comSpindle_setFastChange 9 //

#define comSpindle_setInput 10 //

#define comSpindle_setDisableInReady 11 //

#define comSpindle_setData 128 //
#define comSpindle_getData 129 //

#define sendMSpindle_data 1 //

#define SF_enable  (1<<0)
#define SF_useadec (1<<1)
#define SF_fastch  (1<<2)
#define SF_run     (1<<3)

enum typeDataSpindle{
    dataSpindle_Scur
    ,dataSpindle_Star
    ,dataSpindle_OutCur
    ,dataSpindle_flags
};

typedef struct
{
    float  inValue=0;
    float outValue=0;

}WLSpindleData;


class WLModuleSpindle;

class WLSpindle : public WLElement
{
    Q_OBJECT

public:
    enum typeInputSpindle{SPINDLE_inReady};
    enum typeOutputSpindle{SPINDLE_outENB,SPINDLE_outRUN,SPINDLE_outFW,SPINDLE_outRE};

public:

    explicit WLSpindle(WLModuleSpindle *_ModuleSpindle);
    ~WLSpindle();

private:

    WLFlags Flags;

    float m_acc=0;
    float m_dec=0;

    typeElement m_typeSOut;

    quint8 m_iOut;

    bool m_fastSOut=true;

    WLSpindleData m_curSpindleData;
    QList<WLSpindleData> spindleDataList;

    WLIOPut *outENB;
    WLIOPut *outRUN;
    WLIOPut *outFW;
    WLIOPut *outRE;

    WLIOPut *inReady;

    bool m_disableInReady=false;

private:    
    bool setInput(typeInputSpindle type,quint8 num);
    bool setOutput(typeOutputSpindle type,quint8 num);

public:

    QList<WLSpindleData> getDataList() {return spindleDataList;}

    void setDataList(QList<WLSpindleData> dataList);

    WLSpindleData getCurData(){return m_curSpindleData;}

    bool addDataSpindle(WLSpindleData data);

    void setOutENB(int index);
    void setOutRUN(int index);
    void setOutFW(int index);
    void setOutRE(int index);

    void setInReady(int index);
public:

    WLIOPut*  getInput(typeInputSpindle type);
    WLIOPut*  getOutput(typeOutputSpindle type);

    bool setElementSOut(typeElement telement,quint8 i);
    bool resetElementSpindle();
    bool clearDataSpindle();

    quint8 getISOut() {return m_iOut;}
    typeElement getTypeSOut() {return m_typeSOut;}

    bool isFastChangeSOut() {return m_fastSOut;}
    void setFastSOut(bool enable=true) ;

    float getAcc() const {return m_acc;}
    float getDec() const {return m_dec;}

    bool setAcc(float acc);
    bool setDec(float dec);

    bool setDisableInReady(bool disable=true);
    bool isDisableInReady();

    void setData(QDataStream&);

    bool sendGetData(typeDataSpindle type);
    bool sendGetData();

public slots:
    virtual void update();
    virtual void backup();

signals:
    void changedInValue(float);
    void changedOutValue(float);
    void changedRun(bool);

public:

    virtual void writeXMLData(QXmlStreamWriter &stream);
    virtual void  readXMLData(QXmlStreamReader &stream);
};



#endif // WLSpindle_H
