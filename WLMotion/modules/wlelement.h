#ifndef WLELEMENT_H
#define WLELEMENT_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QDataStream>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QStringList>
#include "wlflags.h"

#define idAxis 0
#define idCut  10

#define idLine  100
#define idCirc  101
#define idBuff  102
#define idWhell 103

const QString namesTypeElement("typeEEmpty"
                               ",typeEInput"
                               ",typeEOutput"
                               ",typeEOutPWM"
                               ",typeEEncoder"
                               ",typeEAInput"
                               ",typeEAxis"
                               ",typeEDCan"
                               ",typeEMPG"
                               ",typeEAOutput"
                               ",typeEDModbus");


class WLModule;

class WLElement : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int hideMethodCount READ getHideMethodCount)

public:
    enum typeElement{typeEEmpty=0
                     ,typeEInput
                     ,typeEOutput
                     ,typeEOutPWM
                     ,typeEEncoder
                     ,typeEAInput
                     ,typeEAxis
                     ,typeEDCan
                     ,typeEFreq
                     ,typeEMPG
                     ,typeEAOutput
                     ,typeEDModbus
                     ,typeESpindle
                     ,typeEPlanner
                    };

    Q_ENUM(typeElement)

public:

    WLElement(WLModule *_Module);
    ~WLElement();

    typeElement getTypeElement() {return m_typeE;}
    QString getTypeElementStr();
    void setTypeElement(typeElement _type) {m_typeE=_type;}

    quint8 getIndex() {return m_index;}
    void setIndex(quint8 _index) {m_index=_index;}


    void setComment(QString _comment) {m_comment=_comment;}
    void addComment(QString _comment);
    void removeComment(QString _comment);

    QString getComment() {return m_comment;}

    QString getBasicComment() {return (m_comment.split(".")).last();}

    WLModule *getModule() {return m_Module;}
private:
    int  m_hideMethodCount=17;

    typeElement m_typeE;
    quint8  m_index;

    QString m_comment;
    WLModule *m_Module=nullptr;

signals:  
    void sendMessage(QString,QString,int);

public slots:
    virtual void reset() {}
    virtual void callStatus() {}

    virtual void sendCommand(QByteArray);

    virtual void update() {} //get current data
    virtual void backup() {} //backup current data

public:
    virtual void writeXMLData(QXmlStreamWriter &) {}
    virtual void  readXMLData(QXmlStreamReader &) {}

public:
    int getHideMethodCount() const;
};

#endif // WLELEMENT_H
