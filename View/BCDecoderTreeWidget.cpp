#include "BCDecoderTreeWidget.h"
#include <QFile>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QMenu>
#include <QTextStream>
#include <QDebug>
#include "BCSettingAddIPVedioTreeWidgetItemDlg.h"

BCDecoderTreeWidgetItem::BCDecoderTreeWidgetItem(const QString &name, const QString &ip, const QString &decoder, const QString &resolution, int nLevel, QTreeWidget *parent)
    :QTreeWidgetItem( parent )
{
    m_nLevel = nLevel;
    SetAttribute(name, ip, decoder, resolution);
}

BCDecoderTreeWidgetItem::BCDecoderTreeWidgetItem(const QString &name, const QString &ip, const QString &decoder, const QString &resolution, int nLevel, QTreeWidgetItem *parent)
    :QTreeWidgetItem( parent )
{
    m_nLevel = nLevel;
    SetAttribute(name, ip, decoder, resolution);
}

BCDecoderTreeWidgetItem::~BCDecoderTreeWidgetItem()
{
    // 析构子节点
    while (0 != this->childCount())
        delete this->takeChild(0);
}

void BCDecoderTreeWidgetItem::SetAttribute(const QString &name, const QString &ip, const QString &decoder, const QString &resolution)
{
    m_qsName = name;
    m_qsIP = ip;
    m_qsDecoder = decoder;
    m_qsResolution = resolution;

    this->setText(0, m_qsName);
    QString qsNameHeader = QObject::tr("名称");
    QString qsIPHeader = QObject::tr("视频地址");
    QString qsDecoderHeader = QObject::tr("编码类型");
    QString qsResolutionHeader = QObject::tr("分辨率");
    this->setToolTip(0, QString("%1: %2 \n%3: %4 \n%5: %6 \n%7: %8")
                     .arg(qsNameHeader).arg(m_qsName)
                     .arg(qsIPHeader).arg(m_qsIP)
                     .arg(qsDecoderHeader).arg(m_qsDecoder)
                     .arg(qsResolutionHeader).arg(m_qsResolution));
}

// ---------------------------------------------------------------------------------------------------------------------------------

BCDecoderTreeWidget::BCDecoderTreeWidget(QWidget *parent)
    :QTreeWidget( parent )
{
    setHeaderHidden(true);

    Refresh();
}

void BCDecoderTreeWidget::Refresh()
{
    // 清空内部数据
    m_lstCameraIP.clear();
    this->clear();

    QFile file( QString("../xml/BCIPVideoConfig.xml") );
    if(!file.open(QIODevice::ReadOnly)){
        return;
    }

    // 将文件内容读到QDomDocument中
    QDomDocument doc;
    bool bLoadFile = doc.setContent(&file);
    file.close();

    if ( !bLoadFile )
        return;

    // 二级链表
    QDomElement docElem = doc.documentElement();

    // 添加数据
    for (int i = 0; i < docElem.childNodes().count(); i++) {
        QDomNode node = docElem.childNodes().at(i);
        if ( !node.isElement() )
            continue;

        QDomElement ele = node.toElement();
        AddNode(ele, NULL);
    }

    this->expandAll();
}

void BCDecoderTreeWidget::AddNode(QDomElement &ele, QTreeWidgetItem *pItem)
{
    // 获得属性
    int nLevel = ele.attribute("level").toInt();
    QString qsName = ele.attribute("name");
    QString qsIP = ele.attribute("ip");
    QString qsDecoder = ele.attribute("decoder");
    QString qsResolution = ele.attribute("resolution");

    // 构造结构体
    sIPInfo sip;
    sip.name = qsName;
    sip.url = qsIP;
    sip.resolution = qsResolution;
    sip.decoder = qsDecoder;

    // 构造treeWidgetItem
    BCDecoderTreeWidgetItem *pCurrentItem = NULL;
    if (0 == nLevel) {
        pCurrentItem = new BCDecoderTreeWidgetItem(qsName, qsIP, qsDecoder, qsResolution, nLevel, this);
        this->addTopLevelItem( pCurrentItem );
    } else {
        if (NULL == pItem)
            return;

        pCurrentItem = new BCDecoderTreeWidgetItem(qsName, qsIP, qsDecoder, qsResolution, nLevel, pItem);
        pItem->addChild( pCurrentItem );
    }

    if (NULL == pCurrentItem)
        return;

    // 添加结构体到链表
    m_lstCameraIP.append( sip );

    // 递归添加子节点
    for (int i = 0; i < ele.childNodes().count(); i++) {
        QDomNode node = ele.childNodes().at(i);
        if ( !node.isElement() )
            continue;

        QDomElement eleChild = node.toElement();
        AddNode(eleChild, pCurrentItem);
    }
}

QString BCDecoderTreeWidget::GetName(QString url)
{
    for (int i = 0; i < this->topLevelItemCount(); i++) {
        BCDecoderTreeWidgetItem *pItem = dynamic_cast<BCDecoderTreeWidgetItem *>( this->topLevelItem( i ) );
        if (NULL == pItem)
            continue;

        if (url == pItem->m_qsIP)
            return pItem->m_qsName;
        else {
            for (int j = 0; j < pItem->childCount(); j++) {
                BCDecoderTreeWidgetItem *pChild = dynamic_cast<BCDecoderTreeWidgetItem *>( pItem->child( j ) );
                if (NULL == pChild)
                    continue;

                if (url == pChild->m_qsIP)
                    return pChild->m_qsName;
            }
        }
    }

    return QString::null;
}

void BCDecoderTreeWidget::Filter(const QString &text)
{
    for (int i = 0; i < this->children().count(); i++) {
        QTreeWidgetItem *pItem = this->topLevelItem( i );
        Filter(pItem, text);
    }
}

void BCDecoderTreeWidget::Filter(QTreeWidgetItem *pItem, const QString &text)
{
    if (NULL == pItem)
        return;

    // 如果文字为空则显示，其他情况当包含文字时显示
    if ( text.isEmpty() )
        SetParentNoHidden( pItem );
    else if ( pItem->text(0).contains( text ) )
        SetParentNoHidden( pItem );
    else
        pItem->setHidden( true );

    for (int i = 0; i < pItem->childCount(); i++) {
        Filter(pItem->child( i ), text);
    }
}

void BCDecoderTreeWidget::SetParentNoHidden(QTreeWidgetItem *pItem)
{
    if (NULL == pItem)
        return;

    pItem->setHidden( false );
    SetParentNoHidden( pItem->parent() );
}

//void BCDecoderTreeWidget::dragEnterEvent(QDragEnterEvent *event)
//{
//    qDebug() << event->type();
//    if(event->mimeData()->hasFormat("IPVedioData")){
//        event->accept();
//    }
//    else{
//        event->ignore();
//    }
//}

void BCDecoderTreeWidget::mousePressEvent(QMouseEvent *event)
{
    qDebug() << event->type();
    // 只处理左键
    if(event->buttons() & Qt::LeftButton) {
        BCDecoderTreeWidgetItem *pCurrentItem = dynamic_cast<BCDecoderTreeWidgetItem *>( this->itemAt( event->pos() ) );
        if (NULL == pCurrentItem)
            return;

        QMimeData *mimeData = new QMimeData;    // 拖拽事件传递的数据参数
        QByteArray exData;
        QDataStream dataStream(&exData, QIODevice::WriteOnly);
        QStringList lstData;
        lstData << pCurrentItem->m_qsName << pCurrentItem->m_qsIP << pCurrentItem->m_qsDecoder << pCurrentItem->m_qsResolution;
        dataStream << lstData;
        mimeData->setData("IPVedioData", exData);

        QDrag *drag = new QDrag(this); // 将数据加载到对象中，可设置pixmap等显示
        drag->setMimeData( mimeData );
        drag->exec(); // 执行即可，参数默认为Qt::MoveAction，可将数据从src to target
    }

    QTreeWidget::mousePressEvent( event );
}

void BCDecoderTreeWidget::mouseDoubleClickEvent(QMouseEvent *)
{
}

void BCDecoderTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeWidgetItem *pItem = this->itemAt( event->pos() );

    QMenu menu;
    QAction *pAddRootAction = NULL;
    QAction *pAddAction = NULL;
    QAction *pDelAction = NULL;
    QAction *pModifyAction = NULL;
    QAction *pMoveUpAction = NULL;
    QAction *pMoveDownAction = NULL;
    QAction *pMoveTopAction = NULL;
    QAction *pMoveBottomAction = NULL;
    if (NULL == pItem) {
        pAddRootAction = menu.addAction(tr("添加节点"));
    } else {
        pAddAction = menu.addAction(tr("添加节点"));
        //pAddAction = menu.addAction(QCoreApplication::translate("QString", "添加节点"));
        pDelAction = menu.addAction(tr("删除节点"));
        pModifyAction = menu.addAction(tr("修改节点"));
        menu.addSeparator();
        pMoveUpAction = menu.addAction(tr("上移"));
        pMoveDownAction = menu.addAction(tr("下移"));
        pMoveTopAction = menu.addAction(tr("置顶"));
        pMoveBottomAction = menu.addAction(tr("置底"));
    }

    // 显示菜单
    QAction *pSelectAction = menu.exec(QCursor::pos());
    if (NULL == pSelectAction)
        return;

    if (pAddRootAction == pSelectAction) {
        AddIPVedioNode(0, pItem);
    }
    if (pAddAction == pSelectAction) {
        AddIPVedioNode(1, pItem);
    }
    if (pDelAction == pSelectAction) {
        DelIPVedioNode( pItem );
    }
    if (pModifyAction == pSelectAction) {
        ModifyIPVedioNode( pItem );
    }
    if (pMoveUpAction == pSelectAction) {
        MoveIPVedioNode(0, pItem);
    }
    if (pMoveDownAction == pSelectAction) {
        MoveIPVedioNode(1, pItem);
    }
    if (pMoveTopAction == pSelectAction) {
        MoveIPVedioNode(2, pItem);
    }
    if (pMoveBottomAction == pSelectAction) {
        MoveIPVedioNode(3, pItem);
    }
}

void BCDecoderTreeWidget::MoveIPVedioNode(int type, QTreeWidgetItem *pItem)
{
    if (NULL == pItem)
        return;

    QTreeWidgetItem *parent = pItem->parent();
    if (NULL == parent) {
        int index = this->indexOfTopLevelItem( pItem );
        int nCount = this->topLevelItemCount();
        if (0 == type) {
            if (index != 0) {
                this->takeTopLevelItem( index );
                this->insertTopLevelItem(index-1, pItem);
                this->setCurrentItem( pItem );
            }
        }
        if (1 == type) {
            if (index != nCount-1) {
                this->takeTopLevelItem( index );
                this->insertTopLevelItem(index+1, pItem);
                this->setCurrentItem( pItem );
            }
        }
        if (2 == type) {
            if (index != 0) {
                this->takeTopLevelItem( index );
                this->insertTopLevelItem(0, pItem);
                this->setCurrentItem( pItem );
            }
        }
        if (3 == type) {
            if (index != nCount-1) {
                this->takeTopLevelItem( index );
                this->insertTopLevelItem(nCount-1, pItem);
                this->setCurrentItem( pItem );
            }
        }
    } else {
        int index = parent->indexOfChild( pItem );
        int nCount = parent->childCount();
        if (0 == type) {
            if (index != 0) {
                parent->takeChild( index );
                parent->insertChild(index-1, pItem);
                this->setCurrentItem( pItem );
            }
        }
        if (1 == type) {
            if (index != nCount-1) {
                parent->takeChild( index );
                parent->insertChild(index+1, pItem);
                this->setCurrentItem( pItem );
            }
        }
        if (2 == type) {
            if (index != 0) {
                parent->takeChild( index );
                parent->insertChild(0, pItem);
                this->setCurrentItem( pItem );
            }
        }
        if (3 == type) {
            if (index != nCount-1) {
                parent->takeChild( index );
                parent->insertChild(nCount-1, pItem);
                this->setCurrentItem( pItem );
            }
        }
    }

    UpdateXmlContent();
}

void BCDecoderTreeWidget::ModifyIPVedioNode(QTreeWidgetItem *pItem)
{
    if (NULL == pItem)
        return;

    BCDecoderTreeWidgetItem *pDecoderItem = dynamic_cast<BCDecoderTreeWidgetItem *>( pItem );
    if (NULL == pDecoderItem)
        return;

    BCSettingAddIPVedioTreeWidgetItemDlg dlg(pDecoderItem->m_qsName, pDecoderItem->m_qsIP, pDecoderItem->m_qsDecoder, pDecoderItem->m_qsResolution, this);
    if (QDialog::Accepted == dlg.exec()) {
        pDecoderItem->SetAttribute(dlg.m_qsName, dlg.m_qsURL, dlg.m_qsDecoder, dlg.m_qsResolution);

        UpdateXmlContent();

        // 更新链表
        for (int i = 0; i < m_lstCameraIP.count(); i++) {
            sIPInfo sip = m_lstCameraIP.at( i );

            if (sip.url != pDecoderItem->m_qsIP)
                continue;

            sip.name = dlg.m_qsName;
            sip.resolution = dlg.m_qsResolution;
            sip.decoder = dlg.m_qsDecoder;
            m_lstCameraIP.removeAt( i );
            m_lstCameraIP.append( sip );
            break;
        }
    }
}

void BCDecoderTreeWidget::DelIPVedioNode(QTreeWidgetItem *pItem)
{
    if (NULL == pItem)
        return;

    if (NULL == pItem->parent()) {
        this->takeTopLevelItem( this->indexOfTopLevelItem( pItem ) );
    } else {
        pItem->parent()->removeChild( pItem );
    }

    // 更新链表
    BCDecoderTreeWidgetItem *pDecoderItem = dynamic_cast<BCDecoderTreeWidgetItem *>( pItem );
    if (NULL != pDecoderItem) {
        for (int i = 0; i < m_lstCameraIP.count(); i++) {
            sIPInfo sip = m_lstCameraIP.at( i );

            if (sip.url != pDecoderItem->m_qsIP)
                continue;

            m_lstCameraIP.removeAt( i );
            break;
        }
    }

    // 析构对象
    delete pItem;
    pItem = NULL;

    UpdateXmlContent();
}

void BCDecoderTreeWidget::AddIPVedioNode(int type, QTreeWidgetItem *pItem)
{
    BCSettingAddIPVedioTreeWidgetItemDlg dlg(type, this);
    if (QDialog::Accepted == dlg.exec()) {
        // 1.添加到界面
        if (0 == type) {
            // 添加根节点
            BCDecoderTreeWidgetItem *pNewItem = new BCDecoderTreeWidgetItem(dlg.m_qsName, dlg.m_qsURL, dlg.m_qsDecoder, dlg.m_qsResolution, 0, this);
            this->addTopLevelItem( pNewItem );
        } else {
            if (NULL == pItem)
                return;

            BCDecoderTreeWidgetItem *pDecoderItem = dynamic_cast<BCDecoderTreeWidgetItem *>( pItem );
            if (NULL == pDecoderItem)
                return;

            if (0 == dlg.m_nPos) {
                // 添加同级节点
                if (NULL != pItem) {
                    if (NULL != pItem->parent()) {
                        BCDecoderTreeWidgetItem *pNewItem = new BCDecoderTreeWidgetItem(dlg.m_qsName, dlg.m_qsURL, dlg.m_qsDecoder, dlg.m_qsResolution, pDecoderItem->m_nLevel, pItem->parent());
                        pItem->parent()->addChild( pNewItem );
                    } else {
                        BCDecoderTreeWidgetItem *pNewItem = new BCDecoderTreeWidgetItem(dlg.m_qsName, dlg.m_qsURL, dlg.m_qsDecoder, dlg.m_qsResolution, pDecoderItem->m_nLevel, this);
                        this->addTopLevelItem( pNewItem );
                    }
                }
            } else {
                // 添加子节点
                if (NULL != pItem) {
                    BCDecoderTreeWidgetItem *pNewItem = new BCDecoderTreeWidgetItem(dlg.m_qsName, dlg.m_qsURL, dlg.m_qsDecoder, dlg.m_qsResolution, pDecoderItem->m_nLevel+1, pItem);
                    pItem->addChild( pNewItem );
                }
            }

            sIPInfo sip;
            sip.name = dlg.m_qsName;
            sip.url = dlg.m_qsURL;
            sip.resolution = dlg.m_qsResolution;
            sip.decoder = dlg.m_qsDecoder;
            m_lstCameraIP.append( sip );
        }

        // 2.更新文件
        UpdateXmlContent();
    }
}

void BCDecoderTreeWidget::UpdateXmlContent()
{
    QDomDocument doc;

    // 添加根节点
    QDomElement eleRoot = doc.createElement(QString("Node"));
    eleRoot.setAttribute(QString("name"), "root");
    eleRoot.setAttribute(QString("ip"), "Default");
    eleRoot.setAttribute(QString("decoder"), "H264");
    eleRoot.setAttribute(QString("resolution"), "1920*1080");
    eleRoot.setAttribute(QString("level"), QString::number(-1));
    doc.appendChild( eleRoot );

    // 循环添加子节点
    for (int i = 0; i < this->topLevelItemCount(); i++) {
        QTreeWidgetItem *pItem = this->topLevelItem(i);

        AddNode(doc, eleRoot, pItem);
    }

    // 写入文件
    QFile file( QString("../xml/BCIPVideoConfig.xml") );
    if( !file.open(QIODevice::WriteOnly | QIODevice::Truncate) )
        return;
    QTextStream out(&file);
    doc.save(out,4);
    file.close();
}

void BCDecoderTreeWidget::AddNode(QDomDocument &doc, QDomElement &parent, QTreeWidgetItem *pItem)
{
    if (NULL == pItem)
        return;

    BCDecoderTreeWidgetItem *pDecoderItem = dynamic_cast<BCDecoderTreeWidgetItem *>( pItem );
    if (NULL == pDecoderItem)
        return;

    QDomElement eleCurrentNode = doc.createElement(QString("Node"));
    eleCurrentNode.setAttribute(QString("name"), pDecoderItem->m_qsName);
    eleCurrentNode.setAttribute(QString("ip"), pDecoderItem->m_qsIP);
    eleCurrentNode.setAttribute(QString("decoder"), pDecoderItem->m_qsDecoder);
    eleCurrentNode.setAttribute(QString("resolution"), pDecoderItem->m_qsResolution);
    eleCurrentNode.setAttribute(QString("level"), pDecoderItem->m_nLevel);
    //QDomNode node = parent.appendChild( eleCurrentNode );
    parent.appendChild( eleCurrentNode );

    for (int i = 0; i < pItem->childCount(); i++) {
        QTreeWidgetItem *pChild = pItem->child(i);
        AddNode(doc, eleCurrentNode, pChild);
    }
}
