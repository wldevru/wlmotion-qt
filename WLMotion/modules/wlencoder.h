#ifndef WLENCODER_H
#define WLENCODER_H

#include <QObject>
#include <QDataStream>
#include <QTimer>
#include <QDebug>

#include "wlmodule.h"

//Encoder
#define comEnc_setASendData   1 //set autosend data
#define comEnc_setEnable      2 //set enable encoder
#define comEnc_setScale       3 //set scale count
#define comEnc_setCount       4 //set cur count
#define comEnc_setInv         5 //count inv (dir)

#define comEnc_setData 128
#define comEnc_getData 129

#define ENCF_enable  1<<0
#define ENCF_send    1<<1
#define ENCF_invalid 1<<2
#define ENCF_inv     1<<3

typedef enum WLTypeEncoder  {TypeEncoder_NO=0,TypeEncoder_TIMER=1,TypeEncoder_REM=3} WLTypeEncoder;

enum typeDataEncoder{
    dataEncoder_count
    ,dataEncoder_enable
    ,dataEncoder_flag
};

const QString errorEncoder(QT_TR_NOOP("0,no error"
                                      ",1,-"));

class WLModuleEncoder;

class WLEncoder : public WLElement
{
    Q_OBJECT

public:

    WLEncoder(WLModuleEncoder *_ModuleEncoder);
    ~WLEncoder();

private:

    qint32 m_count;
    quint16 scale;

    quint8 error;

    WLFlags Flags;

public:
    bool setCount(qint32 cnt);

    void setError(quint8 err)  {emit changedError(error=err);}

    long count();

    bool isEnable() {return Flags.get(ENCF_enable);}
    bool isInv()    {return Flags.get(ENCF_inv);}

    void setData(QDataStream &Stream);

signals:

    void changedError(quint8);
    void changedCount(long);
    void changed(int);

public:

    bool setCount(long cnt);
    bool setEnable(bool enable);
    bool setScale(quint16 _scale);
    bool setInv(quint16 _inv);

    bool togInv() {return setInv(!isInv());}

    bool sendGetData();
    bool sendGetData(typeDataEncoder type);

public:

    virtual void writeXMLData(QXmlStreamWriter &stream);
    virtual void  readXMLData(QXmlStreamReader &stream);

    // WLElement interface
public slots:
    void update();
    void backup();
};



#endif // WLENCODER_H
