#ifndef WLModulePLANNER_H
#define WLModulePLANNER_H

#include <QObject>
#include <QDebug>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QStringList>
#include <QMutex>
#include <QTimer>
#include <QMap>
#include <QTranslator>

#include "wlmodule.h"
#include "wlaxis.h"
#include "wlmoduleioput.h"
#include "wlmodulespindle.h"

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

//Planner
#define comPlanner_addCirc       2 //
#define comPlanner_addLine       3 //
#define comPlanner_start         4 //
#define comPlanner_stop          5 //
#define comPlanner_pause         6 //
#define comPlanner_reset         7 //
#define comPlanner_setKF         8 //
#define comPlanner_setSmoothAng  9 //установка предела сглаживания 
#define comPlanner_setSOut         10 
#define comPlanner_setKFpause       11
#define comPlanner_setMaxNView     12
#define comPlanner_clear           13
#define comPlanner_toSpindle        14//to spindle command
#define comPlanner_setKSOut        15 
#define comPlanner_addULine        16
#define comPlanner_setISlaveAxis   17
#define comPlanner_enableSOut      18

#define comPlanner_setActInProbe  19 //set action in probe
#define comPlanner_setInput       20 //setInput

#define comPlanner_setHPause      21 //set offset z detectpause;
#define comPlanner_setModeRun     22 //set mode run

#define comPlanner_setIgnoreInput  25 //set ignore input
#define comPlanner_addDelay        26 //add Delay_ms element
#define comPlanner_setISpindle     27 //set Spindle
#define comPlanner_setWaitAxisMode 28//
#define comPlanner_setWaitSFastElement 29//

#define comPlanner_setAutoOffSOut 30// when stop moving
#define comPlanner_addM6X         31//add M6X
#define comPlanner_setOutput      32//setOutput

#define comPlanner_getDataPlanner     101
#define comPlanner_getCurPointPlanner  102

#define comPlanner_setData 128
#define comPlanner_getData 129

#define comPlanner_getProp    102

#define sendPlanner_rInProbe   19 //send position inProbe (rise __/--- front)
#define sendPlanner_fInProbe   20 //send position inProbe (fall --\__ front)
#define sendPlanner_stopPoint  21 //send position stopPlanner
#define sendPlanner_pausePoint 22 //send position pausePlanner
#define sendPlanner_curPoint   23 //send current position


#define sendPlanner_signal 200
/* 
enum typeSignalBuf{_sigChgEmptyBuf_ui8   
                  ,_sigChgSOut_f32
                  ,_sigChgFreeSizeBuf_ui8};
*/ 
#define sendPlanner_data   201
//#define sendPlanner_prop   202 //запрос данных модуля

//#define sendPlanner_error  255

#define MASK_abs       1<<1 //absolute coordinate
#define MASK_circxyz   1<<2 //3 axis circ
#define MASK_ccw       1<<3 //counter clock wise
#define MASK_fast      1<<4 //fast
#define MASK_ensmooth  1<<5 //enable smooth

#define errorPlanner_emg       1
#define errorPlanner_buffull   2
#define errorPlanner_setdata   3
#define errorPlanner_waxis     4
#define errorPlanner_welement  5 


#define errorULine_pos   1
#define errorULine_mov   2
#define errorCirc_pos    3
#define errorCirc_radius 4
#define errorLine_df     5
#define errorLine_count  6

#define PLF_enable     (1<<0)
#define PLF_safeprobe  (1<<1)
#define PLF_empty      (1<<2)
#define PLF_moving     (1<<3)
#define PLF_chgdata    (1<<4)
#define PLF_usehpause  (1<<5)

#define PLF_enableSOut (1<<7)

const QString errorPlanner(QT_TR_NOOP("0,no error"
                                      ",1,executed emg stop"
                                      ",2,buf is full"
                                      ",3,wrong set data"));

const QString errorElementPlanner(QT_TR_NOOP("0,no error"
                                             ",1,wrong ULine position"
                                             ",2,wrong ULine mov distance"
                                             ",3,wrong Circ position"
                                             ",4,wrong Circ radius"
                                             ",5,error Line calc"
                                             ",6,wrong Line count"));

enum statusPlanner{PLANNER_stop
                   ,PLANNER_run
                   ,PLANNER_pause
                   ,PLANNER_paused
                   ,PLANNER_stopped
                   ,PLANNER_waitStart};

enum modeRunPlanner{PLANNER_normal
                    ,PLANNER_oneElement
                    ,PLANNER_oneBlock};

enum typeInputPlanner{PLANNER_inProbe
                      ,PLANNER_inPause
                      ,PLANNER_inStop};

enum typeOutputPlanner{PLANNER_outAntidive};

class WLModulePlanner : public WLModule
{
    Q_OBJECT

public:
    enum typeInputSpindle{SPINDLE_inEMGStop};
    enum typeOutputSpindle{SPINDLE_outENB};

    enum typeM6XTask{M6XTask_time,M6XTask_pos,M6XTask_koffset,M6XTask_immediately};

    enum typeDataPlanner{
        dataPlanner_curSpindleInValue
        ,dataPlanner_curSpindleOutValue
        ,dataPlanner_Fsum
        ,dataPlanner_modeRun
        ,dataPlanner_smootAng
        ,dataPlanner_SOut
        ,dataPlanner_waitAxisMode
        ,dataPlanner_Fantidive
    };

    Q_ENUM(typeDataPlanner)

public:
    WLModulePlanner(WLDevice *_Device);
    ~WLModulePlanner();

private:

    QMutex Mutex;
    QMutex mutexPos;

    statusPlanner  m_status=PLANNER_stop;
    modeRunPlanner m_modeRun=PLANNER_normal;

    WLFlags Flags;
    int m_sizeBuf;
    int m_free;

    float m_KFpause=0.25f;

    quint8  m_indexRingElementBuf=0;
    quint32 m_curIdElementBuf=0;
    quint32 m_lastIdElementBuf=0;

    float m_accSpindle=0;
    float m_decSpindle=0;

    float m_KF=1;
    float m_KSOut=1;

    float m_Fantidive=0;

    float m_smoothAng=15;

    qint32  m_hPause;

    WLIOPut *inProbe;
    WLIOPut *inPause;
    WLIOPut *inStop;

    WLIOPut *outDisTHC;

    int m_timeoutAddElement=20;

    bool m_ignoreInPause=false;
    bool m_ignoreInStop=false;

    QList<quint8> m_axisList;

    QList<qint32> m_posProbeR;
    QList<qint32> m_posProbeF;
    QList<qint32> m_posStop;
    QList<qint32> m_posPause;
    QList<qint32> m_posCur;

    WLIOPut::typeActionInput m_actInProbe=WLIOPut::INPUT_actNo;

    bool m_validProbeR=false;
    bool m_validProbeF=false;
    bool m_validPosStop=false;
    bool m_validPosPause=false;

    bool m_waitAxis=false;
    bool m_waitSFastElement=true; //wait changed S value in fast element
    bool m_autoOffSOut=false;

    QVector <quint8> m_indexsAxis;

    quint8 iSpindle=255;

private:
    bool setInput(typeInputPlanner getTypeModule,quint8 num);
    bool setIgnoreInput(typeInputPlanner getTypeModule,quint8 ignore);

    bool setOutput(typeOutputPlanner getTypeModule,quint8 num);

public:
    WLIOPut*  getInput(typeInputPlanner type);
    WLIOPut*  getOutput(typeOutputPlanner type);

    void clear();

    void setModeRun(modeRunPlanner modeRun);
    modeRunPlanner getModeRun(){return m_modeRun;}

    WLSpindle* getSpindle();

    bool setISpindle(quint8 index);

    void setInProbe(int index);
    void setInPause(int index);
    void setInStop(int index);

    void setOutAntidive(int index);
    bool setFAntidive(float f);

    bool isIgnorePause() {return  m_ignoreInPause;}
    bool isIgnoreStop()  {return  m_ignoreInStop;}

    int getSizeBuf() {return m_sizeBuf;}
    int getCountBuf() {return m_sizeBuf - m_free;}

    float getKFpause() {return m_KFpause;}
    float getSmoothAng() {return m_smoothAng;}

    quint32 getCurIdElement() {return m_curIdElementBuf;}
    quint32 getLastIdElement() {return m_lastIdElementBuf;}

    bool setIAxisSlave(quint8 *indexsAxis,quint8 size);
    QVector <quint8> getIAxisSlave() {return m_indexsAxis;}
    int getIAxis(uint8_t i) {return m_indexsAxis.indexOf(i);}

    bool setHPause(quint8 enable,qint32 hPause);

    bool clearDataSpindle();

    bool addULine(quint8 mask,quint8 size,quint8 indexs[],qint32 endPos[],qint32 midPos[],float S,float Fmov,quint32 _id);
    bool addLine(quint8 mask,quint8 size ,quint8 indexs[],qint32 endPos[],float S,float Fmov,quint32 _id);
    bool addCirc(quint8 mask,quint8 size ,quint8 indexs[],qint32 endPos[],qint32 cenPosIJ[],float S,float Fmov,quint32 _id);
    bool addDelay(quint32 delayms,float S,quint32 _id);
    bool addM6X(WLElement::typeElement type,quint8 index,float value,quint32 _id,float posTask=0,enum typeM6XTask=M6XTask_pos);

    bool startMov();
    bool stopMov();
    bool pauseMov();

    bool setKF(float _KF);
    float getKF()  {return m_KF;}
    bool setKFpause(float _KF);

    bool setSmoothAng(float ang_gr);

    bool setSOut(float s);
    bool setKSOut(float k);
    float getKSOut(){return m_KSOut;}

    bool setEnableSOut(quint8 enable);

    int getFree()   {return m_free;}

    bool setActInProbe(WLIOPut::typeActionInput typeAct);

    bool setIgnoreInPause(bool ignore) {return setIgnoreInput(PLANNER_inPause,ignore);}
    bool setIgnoreInStop(bool ignore)  {return setIgnoreInput(PLANNER_inStop,ignore);}

    bool setWaitAxis(bool);
    bool isWaitAxis() {return m_waitAxis;}

    bool setAutoOffSOut(bool);
    bool isAutoOffSOut() {return m_autoOffSOut;}

    bool setWaitSFastElement(bool);
    bool isWaitSFastElement() {return m_waitSFastElement;}

    bool isIgnoreInPause() {return m_ignoreInPause;}
    bool isIgnoreInStop()  {return m_ignoreInStop;}

    statusPlanner getStatus()  const {return m_status;}

    bool isEmpty() {return  Flags.get(PLF_empty);}
    bool isMoving(){return  Flags.get(PLF_moving);}
    bool isBusy()  {return !isEmpty()||isMoving();}
    bool isStop()  {return  m_status=PLANNER_stop;}

    qint32 getProbeR(int index) {QMutexLocker locker(&mutexPos); if(0<=index&&index<m_posProbeR.size()) return m_posProbeR.at(index); else return 0;}
    qint32 getProbeF(int index) {QMutexLocker locker(&mutexPos); if(0<=index&&index<m_posProbeF.size()) return m_posProbeF.at(index); else return 0;}

    void resetProbe() {m_validProbeR=m_validProbeF=false;}

    bool isProbeR() {return m_validProbeR;}
    bool isProbeF() {return m_validProbeF;}

    qint32 getPosStop(int index)  {QMutexLocker locker(&mutexPos); if(0<=index&&index<m_posStop.size())  return m_posStop.at(index);  else return 0;}
    qint32 getPosPause(int index) {QMutexLocker locker(&mutexPos); if(0<=index&&index<m_posPause.size()) return m_posPause.at(index); else return 0;}
    qint32 getPosAxis(int index)  {QMutexLocker locker(&mutexPos); if(0<=index&&index<m_posPause.size()) return m_posCur.at(index);   else return 0;}

    bool isValidPosStop()  {return m_validPosStop;}
    bool isValidPosPause() {return m_validPosPause;}

    void resetPosStop()  {m_validPosStop=false;}
    void resetPosPause() {m_validPosStop=false;}

    void setData(QDataStream &data);
    void getData(typeDataPlanner getTypeModule);

    int getTimeoutAddElement() const;
    void setTimeoutAddElement(int timeoutAddElement);
private slots:
    void callTrackPlanner();

public slots:
    void sendGetData();

public slots:
    virtual void update();
    virtual void backup();

signals:
    void changedFree(int);
    void changedStatus(int);
    void changedCurIdElement(quint32,float k=0);
    void changedSOut(float);
    void changedProbe(bool);
    void changedPause(bool);
    void changedStop(bool);
    void changedEmpty(bool);
    void reset();

public:
    virtual void getProp() {}
    virtual void writeXMLData(QXmlStreamWriter &stream);
    virtual void  readXMLData(QXmlStreamReader &stream);
    virtual void readCommand(QByteArray data);

};
#endif // WLModulePLANNER_H

