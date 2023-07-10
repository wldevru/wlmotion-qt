#include <QMetaEnum>
#include "wlelement.h"
#include "wlmodule.h"

WLElement::WLElement(WLModule *_Module)
    :QObject(_Module)
{
    m_typeE=typeEEmpty;
    m_index=0;

    m_Module=_Module;
}


WLElement::~WLElement()
{

}

QString WLElement::getTypeElementStr()
{
    return QString::fromUtf8(QMetaEnum::fromType<WLElement::typeElement>().valueToKey(getTypeElement()));
}

void WLElement::addComment(QString _comment)
{
    if(!m_comment.isEmpty()) m_comment+=",";

    m_comment+=_comment;
}

void WLElement::removeComment(QString remcomment)
{
    if(!m_comment.isEmpty())
    {
        QStringList List=m_comment.split(",");

        for(int i=0;i<List.size();i++)
        {
            if(List[i]==remcomment) List.removeAt(i--);
        }

        switch(List.size())
        {
        case 0: m_comment.clear();break;
        case 1: m_comment=List.takeFirst();break;
        default:
            m_comment.clear();
            for(int i=0;i<List.size();i++)
            {
                if(i!=0) m_comment+=",";
                m_comment+=List[i];
            }
            break;
        }
    }

}

int WLElement::getHideMethodCount() const
{
    return m_hideMethodCount;
}

void WLElement::sendCommand(QByteArray data)
{
    m_Module->sendCommand(data);
}


