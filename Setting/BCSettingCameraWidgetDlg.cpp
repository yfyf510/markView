#include "BCSettingCameraWidgetDlg.h"
#include "ui_BCSettingCameraWidgetDlg.h"


#include <QDebug>
#include <QPainter>
#include <QToolButton>
#include <QMessageBox>
#include <QDrag>
#include <QMimeData>
#include <QDesktopWidget>
#include <QDomDocument>
#include <QMenu>

#include "../Camera/HCNetSDK.h"
#include "../Camera/DecodeCardSdk.h"
#include "../Camera/plaympeg4.h"
#include "../Camera/DataType.h"

// =====================================================================
// 信号源Item
class BCCameraTreeWidgetItem : public QTreeWidgetItem{
public:
    BCCameraTreeWidgetItem(QString Channel, QString VideoHandle = nullptr, QTreeWidgetItem *parent = 0 );
    QString mChannel;
    QString mVideoHandle; //录像文件句柄
};
BCCameraTreeWidgetItem::BCCameraTreeWidgetItem(QString Channel , QString VideoHandle, QTreeWidgetItem *parent): QTreeWidgetItem(parent)
{
    this->mChannel = Channel;
    VideoHandle.isEmpty()? this->mVideoHandle = Channel:this->mVideoHandle = VideoHandle;
    this->setText(0, mVideoHandle );

}

// ==============================================================
// 视频显示窗口
BCCameraDisplayUnitWidget::BCCameraDisplayUnitWidget(BCSettingCameraWidgetDlg *settingCameraWidgetDlg)
    :QWidget( settingCameraWidgetDlg )
{
    this->setAcceptDrops( true );

    this->m_bSelected = false;    //当前窗口选中状态
    this->m_isVideo = false;      //当前是否在录像
    this->m_ChannelID = -1;       //通道Id
    this->m_iPlayhandle = -1;     //直播句柄
    this->m_iPlayBackhandle = -1; //录播句柄
    this->m_pSettingCameraWidgetDlg = settingCameraWidgetDlg; //父窗体指针

    this->m_pChildWidget = NULL;        //菜单窗口
    this->m_pChildWIdgetVideo = NULL;   //视频播放窗口

    InitChildWidget();            //加载子窗口
}

BCCameraDisplayUnitWidget::~BCCameraDisplayUnitWidget()
{
    //停止录播
    NET_DVR_StopPlayBack(m_iPlayBackhandle);
    //停止直播
    NET_DVR_StopRealPlay(m_iPlayhandle);
    //停止录制视频
    this->m_pSettingCameraWidgetDlg->StopTranscribeVideo( m_ChannelID );
    this->m_isVideo = false;
}

//初始化子窗口
void BCCameraDisplayUnitWidget::InitChildWidget()
{
    //视频窗===================
    m_pChildWIdgetVideo = new QWidget(this);

    //菜单窗===================
    m_pChildWidget = new QWidget( this );
    QHBoxLayout * m_pChildWidgetLayout = new QHBoxLayout();
    m_pChildWidgetLayout->setMargin( 2 );
    m_pChildWidget->setLayout( m_pChildWidgetLayout );

    m_pStartVideobtn = new QToolButton(); //录像/停止
    m_pStartVideobtn->setText(tr("开始录制"));
    m_pStartVideobtn->setMinimumSize(25, 25);


    m_pTipLabel = new QLabel("信号源名称");
    m_pTipLabel->setMinimumSize(25, 25);
    QFont font ( "Microsoft YaHei", 9 );
    m_pTipLabel->setFont( font );
    m_pStartVideobtn->setFont( font );

    m_pChildWidgetLayout->addSpacing( 2 );
    m_pChildWidgetLayout->addWidget( m_pTipLabel );
    m_pChildWidgetLayout->addStretch();
    m_pChildWidgetLayout->addWidget( m_pStartVideobtn );
    m_pChildWidgetLayout->addSpacing( 2 );

    m_pChildWidget->setMaximumHeight( 30 );
    m_pChildWidget->setStyleSheet("background-color:LightSteelBlue;");  //默认颜色

    m_pChildWIdgetVideo->installEventFilter( this );
    m_pChildWidget->installEventFilter( this );

    QVBoxLayout *pMainlayout = new QVBoxLayout();
    this->setLayout( pMainlayout );
    pMainlayout->addWidget( m_pChildWIdgetVideo );
    pMainlayout->addWidget( m_pChildWidget );
    pMainlayout->setMargin( 0 );
    pMainlayout->setSpacing( 0 );

    m_pStartVideobtn->setVisible( false );

    //录制
    connect(m_pStartVideobtn,SIGNAL(clicked(bool)),this,SLOT(onStartVideobtnClicked(bool)));
}

void BCCameraDisplayUnitWidget::SetRealPlay(int chid, QString cChannelName)
{
    NET_DVR_PREVIEWINFO struPreview = {0};
    struPreview.lChannel = chid;        //
    struPreview.dwStreamType = 0;
    struPreview.dwLinkMode = 0;
    struPreview.hPlayWnd = (HWND)m_pChildWIdgetVideo->winId(); //
    //struPreview.hPlayWnd = (HWND)winId(); //
    struPreview.bBlocked = 1;
    //停止录播
    NET_DVR_StopPlayBack(m_iPlayBackhandle);
    //停止直播
    NET_DVR_StopRealPlay(m_iPlayhandle);

    int iRealHandle = NET_DVR_RealPlay_V40(m_pSettingCameraWidgetDlg->IUserID, &struPreview, NULL, NULL);
    m_iPlayBackhandle = -1;  //当前窗口在直播,所以录播id为-1  -> 在小窗口中需要用于判断,当前窗口如果是录播则不显示录像按钮, 需要实时更新
    m_iPlayhandle = iRealHandle;
    m_ChannelID = chid;
    m_pTipLabel->setText( cChannelName );
}

void BCCameraDisplayUnitWidget::SetRealPlayBack(QString sFileHandle, QString sFileName)
{
    //停止录播
    NET_DVR_StopPlayBack(this->m_iPlayBackhandle);
    //停止直播
    NET_DVR_StopRealPlay(this->m_iPlayhandle);

    int nPlaybackHandld = NET_DVR_PlayBackByName(this->m_pSettingCameraWidgetDlg->IUserID, sFileHandle.toLatin1().data(), (HWND)this->m_pChildWIdgetVideo->winId());
    if(NET_DVR_PlayBackControl_V40 (nPlaybackHandld,NET_DVR_PLAYSTART,0,0,0,0) == -1) {
        qDebug() << "录像文件播放失败";
        return;
    }
    this->m_iPlayhandle = -1;
    this->m_iPlayBackhandle = nPlaybackHandld;  //记录回放句柄,用于控制回放视频
    this->m_pTipLabel->setText( sFileName );
}

//拖放 -> 进入
void BCCameraDisplayUnitWidget::dragEnterEvent( QDragEnterEvent *dragEv )
{
    if (dragEv->mimeData()->hasFormat("cameraInputChannel") || dragEv->mimeData()->hasFormat("videoFile")) {
        dragEv->acceptProposedAction(); //用这个结束可以告诉源窗口已接收到信号
    } else {
        dragEv->ignore();
    }
}

//拖放 -> 松开 播放视频
void BCCameraDisplayUnitWidget::dropEvent(QDropEvent *dropEv)
{
    if (dropEv->mimeData()->hasFormat("cameraInputChannel")) {
        QByteArray data = dropEv->mimeData()->data("cameraInputChannel");
        if( !data.isEmpty() ) {
            QString ItemName = m_pSettingCameraWidgetDlg->GetCameraTreeWidgetItemText( data.toInt());
            this->m_pSettingCameraWidgetDlg->StartPlayVideo(data.toInt(), this, ItemName);
        }
        dropEv->accept();
    } else if(dropEv->mimeData()->hasFormat("videoFile")) {
        QByteArray data = dropEv->mimeData()->data("videoFile");
        if( !data.isEmpty() ) {
            QString ItemName = m_pSettingCameraWidgetDlg->GetCameraVideoFileTreeWidgetItemText( QString(data) );
            this->m_pSettingCameraWidgetDlg->StartPlatBackVideo(QString(data), ItemName, this);
        }
        dropEv->accept();
    } else {
        dropEv->ignore();
    }
}

void BCCameraDisplayUnitWidget::leaveEvent(QEvent *ev)
{
    m_pStartVideobtn->setVisible( false );
}

bool BCCameraDisplayUnitWidget::eventFilter(QObject *obj, QEvent *ev)
{
    if(obj == m_pChildWIdgetVideo){
        if(ev->type() == QEvent::Enter) {
            if(m_iPlayhandle == -1 || m_iPlayBackhandle != -1) { //窗口为空/录播时  不显示录像按钮
                m_pStartVideobtn->setVisible( false );
            } else {
                m_pStartVideobtn->setVisible( true );
            }
        }
        if(ev->type() == QEvent::MouseButtonPress) {  //选中一个窗口
            m_pSettingCameraWidgetDlg->CancelALLWidgetSelected();   //取消其他窗口选中状态
            m_pChildWidget->setStyleSheet("background-color:LightGreen;");  //选中颜色
            this->m_bSelected = true;

        }
    }
    if(obj == m_pChildWidget ) {
        if(ev->type() == QEvent::Enter) {
            if(m_iPlayhandle == -1 || m_iPlayBackhandle != -1) { //窗口为空/录播时  不显示录像按钮
                m_pStartVideobtn->setVisible( false );
            } else {
                m_pStartVideobtn->setVisible( true );
            }
        }
    }
}

void BCCameraDisplayUnitWidget::onStartVideobtnClicked(bool)
{
    if(m_ChannelID == -1 )
        return;

    if( m_isVideo ) { //停止录像
        m_pSettingCameraWidgetDlg->StopTranscribeVideo( m_ChannelID );
    } else { //开始录像
        m_pSettingCameraWidgetDlg->StartTranscribeVideo( m_ChannelID );
    }

    m_isVideo = !m_isVideo;
    m_pStartVideobtn->setText(m_isVideo ? tr("停止录像") : tr("开始录像"));
}

//==========================================================================================================================
//====== 主类

BCSettingCameraWidgetDlg::BCSettingCameraWidgetDlg(QStringList lstDivInfo, QWidget *parent): QDialog(parent),ui(new Ui::BCSettingCameraWidgetDlg)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->IUserID = -1;

    ui->treeWidgetChannelsrc->viewport()->installEventFilter( this );
    ui->treeWidgetVideoFile->viewport()->installEventFilter( this );
    ui->comboBox_CameraNames->addItem(lstDivInfo.at(0));
    //去表头
    ui->treeWidget->setHeaderHidden( true );
    ui->treeWidgetVideoFile->setHeaderHidden( true );
    ui->treeWidgetChannelsrc->setHeaderHidden( true );
    //日期控件格式 及 默认显示时间
    ui->dateTimeEdit_start->setCalendarPopup(true);
    ui->dateTimeEdit_stop->setCalendarPopup(true);
    ui->dateTimeEdit_start->setDateTime(QDateTime::currentDateTime().addDays(-30));
    ui->dateTimeEdit_stop->setDateTime(QDateTime::currentDateTime().addDays(1));

    LoginCamera(lstDivInfo);      //默认直接注册

    InitChildWidgetDlg(2, 2);     //加载默认分割

    //双击切换画面
    connect(ui->treeWidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(onTreeWidgetDoubleClicked(QTreeWidgetItem*,int)));
}

BCSettingCameraWidgetDlg::~BCSettingCameraWidgetDlg()
{
    NET_DVR_Logout(this->IUserID);
    NET_DVR_Cleanup();
    UpLocadRenameFile();
    delete ui;
}

//初始化主界面子窗口
void BCSettingCameraWidgetDlg::InitChildWidgetDlg(int h, int w)
{
    while (0 != ui->widget_rightChild->layout()->count())
        delete ui->widget_rightChild->layout()->itemAt(0)->widget();

    delete ui->widget_rightChild->layout();
    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->setMargin( 0 );
    pGridLayout->setSpacing( 2 );

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            pGridLayout->addWidget(new BCCameraDisplayUnitWidget( this ), i, j);
        }
    }
    ui->widget_rightChild->setLayout( pGridLayout );

    //默认选中第一个Widget
    SelectCameraWidget();

    //开启默认播放
    DefaultPlayVideo();
}

//加载输入源数据
void BCSettingCameraWidgetDlg::InitInputChannel(QList<int> lstChannels)
{
    //添加头结点
    ui->treeWidgetChannelsrc->setHeaderLabel(ui->comboBox_CameraNames->currentText());

    //添加添加
    QMap<int, QString> mapChIDName = LoadRenameFile();
    for(int i = 0; i < lstChannels.count(); i++) {
        QTreeWidgetItem * pDivItem = new BCCameraTreeWidgetItem( QString::number(lstChannels.at(i)), mapChIDName.value(lstChannels.at(i)) );
        ui->treeWidgetChannelsrc->addTopLevelItem( pDivItem );
    }
}

//切换分割模式槽函数
void BCSettingCameraWidgetDlg::onTreeWidgetDoubleClicked(QTreeWidgetItem*,int)
{
    int num = ui->treeWidget->currentIndex().row();
    switch( num ){
        case 0:
            InitChildWidgetDlg(1, 1);
        break;
        case 1:
            InitChildWidgetDlg(2, 2);
        break;
        case 2:
            InitChildWidgetDlg(3, 3);
        break;
        case 3:
            InitChildWidgetDlg(4, 4);
        break;
    }
}

//登录摄像机
void BCSettingCameraWidgetDlg::on_pushButton_CameraLink_clicked()
{
//    LoginCamera(this->lstDivInfo);
}

//查找录像文件按钮
void BCSettingCameraWidgetDlg::on_pushButton_VideoSelect_clicked()
{
    // 查找文件 -> 插入到列表中
    ui->treeWidgetVideoFile->clear();

    QMap<QString, QString> mapFilenames;  //存储系统名称与自己定义的名称
    for(int i = 0; i < ui->treeWidgetChannelsrc->topLevelItemCount(); i++) {
        BCCameraTreeWidgetItem *pItem = dynamic_cast<BCCameraTreeWidgetItem*>(ui->treeWidgetChannelsrc->topLevelItem(i));
        if( NULL == pItem )
            continue;

        NET_DVR_FILECOND_V40 struFileCond={0};
        struFileCond.dwFileType = 0xFF;  //所有类型,详情信息请在头文件查看
        struFileCond.lChannel = pItem->mChannel.toInt();  //通道号
        struFileCond.dwIsLocked = 0xFF;
        struFileCond.dwUseCardNo = 0;
        struFileCond.struStartTime.dwYear    = ui->dateTimeEdit_start->date().year();    //开始时间
        struFileCond.struStartTime.dwMonth   = ui->dateTimeEdit_start->date().month();
        struFileCond.struStartTime.dwDay     = ui->dateTimeEdit_start->date().day();
        struFileCond.struStartTime.dwHour    = ui->dateTimeEdit_start->time().hour();
        struFileCond.struStartTime.dwMinute  = ui->dateTimeEdit_start->time().minute();
        struFileCond.struStartTime.dwSecond  = ui->dateTimeEdit_start->time().second();
        struFileCond.struStopTime.dwYear     = ui->dateTimeEdit_stop->date().year();  //结束时间
        struFileCond.struStopTime.dwMonth    = ui->dateTimeEdit_stop->date().month();
        struFileCond.struStopTime.dwDay      = ui->dateTimeEdit_stop->date().day();
        struFileCond.struStopTime.dwHour     = ui->dateTimeEdit_stop->time().hour();
        struFileCond.struStopTime.dwMinute   = ui->dateTimeEdit_stop->time().minute();
        struFileCond.struStopTime.dwSecond   = ui->dateTimeEdit_stop->time().second();

        int lFindHandle = NET_DVR_FindFile_V40(IUserID, &struFileCond);
        if( lFindHandle < 0 ) {
            int errNo = NET_DVR_GetLastError();
            qDebug() << errNo;
            //QMessageBox::critical(this,tr("错误"),tr("文件搜索异常,错误码:%1").arg(QString::number(errNo)));
            return;
        }

        NET_DVR_FINDDATA_V40 struFileData;
        while ( true ) {
            int result = NET_DVR_FindNextFile_V40(lFindHandle, &struFileData); //搜索文件
            if(result == NET_DVR_ISFINDING) {
                continue;
            } else if(result == NET_DVR_FILE_SUCCESS) { //获取文件成功
                QString m_FileName = QString("%1-%2-%3 %4:%5:%6").arg(struFileData.struStartTime.dwYear).arg(struFileData.struStartTime.dwMonth).arg(struFileData.struStartTime.dwDay).arg(struFileData.struStartTime.dwHour).arg(struFileData.struStartTime.dwMinute).arg(struFileData.struStartTime.dwSecond);
                qDebug() << "录像文件名称:" << struFileData.sFileName << m_FileName; //
                mapFilenames.insert(struFileData.sFileName, m_FileName); //key:文件句柄, val:前台显示名称(录像日期)
            } else if(result == NET_DVR_FILE_NOFIND || result == NET_DVR_NOMOREFILE)  { //未找到文件/查找结束
                break;
            } else {
                qDebug() << QString("获取文件失败,非法的文件状态...");
                break;
            }
        }
    }

    //加载到treeWidget
    LoadVideoFile( mapFilenames );
}

//加载录像文件
void BCSettingCameraWidgetDlg::LoadVideoFile(QMap<QString, QString> mapFilenames)
{
    for(int i =0; i < mapFilenames.keys().count(); i++) {
        QString mKey = mapFilenames.keys().at(i);
        QString mVal = mapFilenames.value(mKey);
        QTreeWidgetItem * mItem = new BCCameraTreeWidgetItem( mKey, mVal );
        ui->treeWidgetVideoFile->addTopLevelItem( mItem );
    }
}

BCCameraDisplayUnitWidget *BCSettingCameraWidgetDlg::SelectCameraWidget()
{
    BCCameraDisplayUnitWidget *pDisplayWidget = NULL;
    for(int i = 0; i < ui->widget_rightChild->layout()->count(); i++) {
        BCCameraDisplayUnitWidget *pTempDisplayWidget = qobject_cast<BCCameraDisplayUnitWidget*>(ui->widget_rightChild->layout()->itemAt(i)->widget());
        if (NULL == pTempDisplayWidget)
            continue;

        if ( !pTempDisplayWidget->m_bSelected )
            continue;

        pDisplayWidget = pTempDisplayWidget;
        break;
    }

    if (NULL == pDisplayWidget)
        pDisplayWidget = qobject_cast<BCCameraDisplayUnitWidget*>(ui->widget_rightChild->layout()->itemAt(0)->widget());

    if (NULL != pDisplayWidget) {
        pDisplayWidget->m_bSelected = true;
        pDisplayWidget->m_pChildWidget->setStyleSheet("background-color:LightGreen;");  //选中颜色
    }

    return pDisplayWidget;
}

void BCSettingCameraWidgetDlg::CancelALLWidgetSelected()
{
    for(int i = 0; i < ui->widget_rightChild->layout()->count(); i++) {
        BCCameraDisplayUnitWidget * mDisplayWidget = qobject_cast<BCCameraDisplayUnitWidget*>(ui->widget_rightChild->layout()->itemAt(i)->widget());
        mDisplayWidget->m_bSelected = false;
        mDisplayWidget->m_pChildWidget->setStyleSheet("background-color:LightSteelBlue;");  //默认颜色
    }
}

//登录设备
void BCSettingCameraWidgetDlg::LoginCamera(QStringList lstDivInfo)
{
    // 已经注册直接返回
    if( -1 != this->IUserID || lstDivInfo.isEmpty() )
        return;

    // 加载海康摄像机信息
    NET_DVR_Init();

    // 连接指定摄像机
    QString ip = lstDivInfo.at( 1 );
    QString user = lstDivInfo.at( 3 );
    QString passwd = lstDivInfo.at( 4 );
    int port = lstDivInfo.at(2).toInt();

    // 以下代码参考海康Demo
    NET_DVR_DEVICEINFO_V30 struDeviceInfo;
    QList<int> lstChannels;

    IUserID = NET_DVR_Login_V30(const_cast<char *>( qPrintable(ip) ), port, const_cast<char *>( qPrintable(user) ), const_cast<char *>( qPrintable(passwd) ) , &struDeviceInfo);
    if(IUserID < 0) {
        int errNo = NET_DVR_GetLastError();
        QString str = tr("设备注册失败,错误号:%3").arg(errNo);
        QMessageBox::critical(this,"错误",str);
        return;
    }

    int startChannel = struDeviceInfo.byStartDChan;
    NET_DVR_IPPARACFG_V40 IPAccessCfgV40;
    memset(&IPAccessCfgV40,0,sizeof(NET_DVR_IPPARACFG));
    int iGroupNO = 0;
    DWORD dwReturned = 0;
    if ( !NET_DVR_GetDVRConfig(IUserID, NET_DVR_GET_IPPARACFG_V40, iGroupNO, &IPAccessCfgV40,sizeof(NET_DVR_IPPARACFG_V40), &dwReturned) ) {
        int errNo = NET_DVR_GetLastError();
        QMessageBox::critical(this,tr("错误"),tr("获取录像机信息失败,错误码:%1").arg(QString::number(errNo)));
        return;
    }

    for (int i=0; i<IPAccessCfgV40.dwDChanNum; i++) {
        if(IPAccessCfgV40.struStreamMode[i].byGetStreamType == 0) {
            BYTE byIPID = IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byIPID;
            BYTE byIPIDHigh=IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byIPIDHigh;
            int iDevInfoIndex=byIPIDHigh*256 + byIPID-1-iGroupNO*64;
            QString tIP = QString(IPAccessCfgV40.struIPDevInfo[iDevInfoIndex].struIP.sIpV4);  //ip
            if( tIP.count() < 9) {
                continue;
            }
            lstChannels.append(startChannel + i);
        }
    }

    //加载输入源 treeWidget
    InitInputChannel( lstChannels ); // 修改
}
void BCSettingCameraWidgetDlg::StartPlatBackVideo(QString sFileHandle, QString sFileName, BCCameraDisplayUnitWidget *pDisplayWidget)
{
    pDisplayWidget->SetRealPlayBack(sFileHandle, sFileName);
}

//读取重命名文件
QMap<int, QString> BCSettingCameraWidgetDlg::LoadRenameFile()
{
    QMap<int, QString> mapChIDName;

    //读取文件
    QFile mFile( QString("../xml/BCCameraChannelItmeRename.xml"));
    if ( !mFile.open(QIODevice::ReadOnly) )
        return mapChIDName;

    QDomDocument rdoc;
    bool bLoadFile = rdoc.setContent(&mFile,true);
    if ( !bLoadFile )
        return mapChIDName;

    QDomElement rdocElem = rdoc.documentElement();
    QDomNodeList rNode = rdocElem.childNodes();
    for (int i = 0; !rNode.isEmpty() && i < rNode.count(); i++) {
        QDomElement ele = rNode.at(i).toElement();
        mapChIDName.insert(ele.attribute("channel").toInt(), ele.attribute("name"));
    }
    mFile.close();

    return mapChIDName;
}
//写入重命名文件
void BCSettingCameraWidgetDlg::UpLocadRenameFile()
{
    //组织Map
    QMap<QString, QString> mapChIDName;
    for (int i = 0; i < ui->treeWidgetChannelsrc->topLevelItemCount(); i++) {
        BCCameraTreeWidgetItem *pItem = dynamic_cast<BCCameraTreeWidgetItem*>(ui->treeWidgetChannelsrc->topLevelItem(i));
        mapChIDName.insert(pItem->mChannel, pItem->text(0));
    }

    //Map写入文件
    QDomDocument wdoc;
    QDomElement weleRoot = wdoc.createElement(QString("BR"));
    for (int i = 0; i < mapChIDName.keys().count(); i++) {
        QDomElement eleChild = wdoc.createElement("INFO");
        eleChild.setAttribute(QString("channel"), mapChIDName.keys().at(i));
        eleChild.setAttribute(QString("name"), mapChIDName.value(mapChIDName.keys().at(i)));
        weleRoot.appendChild( eleChild );
    }

    wdoc.appendChild( weleRoot);
    QFile wfile( QString("../xml/BCCameraChannelItmeRename.xml"));
    if( !wfile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "文件写入失败";
        return;
    }
    QTextStream out(&wfile);
    wdoc.save(out, 4);
    wfile.close();
}

//按指定通道播放视频
void BCSettingCameraWidgetDlg::StartPlayVideo(int chid, BCCameraDisplayUnitWidget *pDisplayWidget, QString ChammelName)
{
    if (chid < 0 || NULL == pDisplayWidget)
        return;

    pDisplayWidget->SetRealPlay( chid, ChammelName );\
}

//默认播放
void BCSettingCameraWidgetDlg::DefaultPlayVideo()
{
    for (int i = 0; i < ui->widget_rightChild->layout()->count(); i++) {
        BCCameraDisplayUnitWidget *pDisplayWidget = qobject_cast<BCCameraDisplayUnitWidget*>(ui->widget_rightChild->layout()->itemAt(i)->widget());
        BCCameraTreeWidgetItem *pItem = dynamic_cast<BCCameraTreeWidgetItem*>(ui->treeWidgetChannelsrc->topLevelItem(i));
        if (NULL == pDisplayWidget || NULL == pItem)
            return;

        StartPlayVideo(pItem->mChannel.toInt(), pDisplayWidget, pItem->text(0));
    }
}

//开始录制
void BCSettingCameraWidgetDlg::StartTranscribeVideo(int nChannel)
{
    if( nChannel < 0 )
        return;

    if ( !NET_DVR_StartDVRRecord(IUserID, nChannel, 0) ) {
        int errNo = NET_DVR_GetLastError();
        QMessageBox::critical(this,tr("错误"),tr("录制失败,错误码:%1").arg(QString::number(errNo)));
    }
}

//结束录制
void BCSettingCameraWidgetDlg::StopTranscribeVideo( int nChannel )
{
    if (nChannel < 0)
        return;

    NET_DVR_StopDVRRecord(IUserID, nChannel);
}

bool BCSettingCameraWidgetDlg::eventFilter(QObject *mObj, QEvent *mEv)
{
    if (mObj == ui->treeWidgetChannelsrc->viewport()) {
        //鼠标移动 ==================================
        if(mEv->type() == QEvent::MouseMove) {
            BCCameraTreeWidgetItem *pItem = dynamic_cast<BCCameraTreeWidgetItem *>(ui->treeWidgetChannelsrc->currentItem());
            if (NULL != pItem) {
                QMimeData * pMimeData = new QMimeData;
                QString exDataChannel = pItem->mChannel;
                pMimeData->setData("cameraInputChannel",exDataChannel.toLatin1());
                QDrag * pDrag = new QDrag( this );
                pDrag->setMimeData( pMimeData );
                pDrag->exec();
            }
        }
        //鼠标双击 ==================================
        if(mEv->type() == QEvent::MouseButtonDblClick) {
            BCCameraTreeWidgetItem *pItem = dynamic_cast<BCCameraTreeWidgetItem *>(ui->treeWidgetChannelsrc->currentItem());
            BCCameraDisplayUnitWidget *pDisplayWidget = SelectCameraWidget();
            if (NULL != pItem && NULL != pDisplayWidget)
                StartPlayVideo(pItem->mChannel.toInt(), pDisplayWidget, pItem->text(0));
        }
        //右键菜单 ==================================
        if(mEv->type() == QEvent::ContextMenu) {
            QMenu menu;
            QAction *actionRename = menu.addAction(tr("重命名"));
            QAction *pSelectAction = menu.exec(QCursor::pos());
            if (actionRename == pSelectAction) {
                onChannelItemRename();
            }
        }
    }

    // 录像文件
    if (mObj == ui->treeWidgetVideoFile->viewport()) {
        if(mEv->type() == QEvent::MouseButtonDblClick) { //双击
            BCCameraTreeWidgetItem *pItem = dynamic_cast<BCCameraTreeWidgetItem *>(ui->treeWidgetVideoFile->currentItem());
            BCCameraDisplayUnitWidget *pDisplayWidget = SelectCameraWidget();
            if (NULL != pItem && NULL != pDisplayWidget)
                StartPlatBackVideo( pItem->mChannel, pItem->text(0), pDisplayWidget );
        }
        if (mEv->type() == QEvent::MouseMove) {
            BCCameraTreeWidgetItem *pItem = dynamic_cast<BCCameraTreeWidgetItem *>(ui->treeWidgetVideoFile->currentItem());
            if (NULL != pItem) {
                QMimeData * pMimeData = new QMimeData;
                QString exDataChannel = pItem->mChannel;
                pMimeData->setData("videoFile",exDataChannel.toLatin1());
                QDrag * pDrag = new QDrag( this );
                pDrag->setMimeData( pMimeData );
                pDrag->exec();
            }
        }
    }
}

QString BCSettingCameraWidgetDlg::GetCameraTreeWidgetItemText(int ItemID)
{
    for(int i = 0; i < ui->treeWidgetChannelsrc->topLevelItemCount(); i++) {
        BCCameraTreeWidgetItem * pItem = dynamic_cast<BCCameraTreeWidgetItem*>(ui->treeWidgetChannelsrc->topLevelItem( i ));
        if( NULL == pItem )
            return QString();
        if( pItem->mChannel.toInt() == ItemID )
            return pItem->text(0);
    }
}

QString BCSettingCameraWidgetDlg::GetCameraVideoFileTreeWidgetItemText(QString Itemhandle)
{
    for(int i = 0; i < ui->treeWidgetVideoFile->topLevelItemCount(); i++) {
        BCCameraTreeWidgetItem * pItem = dynamic_cast<BCCameraTreeWidgetItem*>(ui->treeWidgetVideoFile->topLevelItem( i ));
        if( NULL == pItem )
            return QString();
        if( pItem->mChannel == Itemhandle )
            return pItem->text(0);
    }
}

//Item 编辑模式
void BCSettingCameraWidgetDlg::onChannelItemRename()
{
    BCCameraTreeWidgetItem *pItem = dynamic_cast<BCCameraTreeWidgetItem*>(ui->treeWidgetChannelsrc->currentItem());
    pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
    ui->treeWidgetChannelsrc->editItem( pItem );
}

//云台控制
void BCSettingCameraWidgetDlg::SendCameraCenterCtrl(int dir, int release)
{
    BCCameraDisplayUnitWidget *pDisplayWidget = SelectCameraWidget();
    NET_DVR_PTZControl(pDisplayWidget->m_iPlayhandle, dir, release);
}

//左上
void BCSettingCameraWidgetDlg::on_toolButton_leftUP_pressed()
{
    SendCameraCenterCtrl(UP_LEFT, 0);
}

void BCSettingCameraWidgetDlg::on_toolButton_leftUP_released()
{
    SendCameraCenterCtrl(UP_LEFT, 1);
}

//上
void BCSettingCameraWidgetDlg::on_toolButton_up_pressed()
{
    SendCameraCenterCtrl(TILT_UP, 0);
}

void BCSettingCameraWidgetDlg::on_toolButton_up_released()
{
    SendCameraCenterCtrl(TILT_UP, 1);
}

//右上
void BCSettingCameraWidgetDlg::on_toolButton_rightUp_pressed()
{
    SendCameraCenterCtrl(UP_RIGHT, 0);
}

void BCSettingCameraWidgetDlg::on_toolButton_rightUp_released()
{
    SendCameraCenterCtrl(UP_RIGHT, 1);
}

//左
void BCSettingCameraWidgetDlg::on_toolButton_left_pressed()
{
    SendCameraCenterCtrl(PAN_LEFT, 0);
}

void BCSettingCameraWidgetDlg::on_toolButton_left_released()
{
    SendCameraCenterCtrl(PAN_LEFT, 1);
}

//右
void BCSettingCameraWidgetDlg::on_toolButton_right_pressed()
{
    SendCameraCenterCtrl(PAN_RIGHT, 0);
}

void BCSettingCameraWidgetDlg::on_toolButton_right_released()
{
    SendCameraCenterCtrl(PAN_RIGHT, 1);
}

//左下
void BCSettingCameraWidgetDlg::on_toolButton_leftDown_pressed()
{
    SendCameraCenterCtrl(DOWN_LEFT, 0);
}

void BCSettingCameraWidgetDlg::on_toolButton_leftDown_released()
{
    SendCameraCenterCtrl(DOWN_LEFT, 1);
}

//下
void BCSettingCameraWidgetDlg::on_toolButton_Down_pressed()
{
    SendCameraCenterCtrl(TILT_DOWN, 0);
}

void BCSettingCameraWidgetDlg::on_toolButton_Down_released()
{
    SendCameraCenterCtrl(TILT_DOWN, 1);
}

//右下
void BCSettingCameraWidgetDlg::on_toolButton_rightDown_pressed()
{
    SendCameraCenterCtrl(DOWN_RIGHT, 0);
}

void BCSettingCameraWidgetDlg::on_toolButton_rightDown_released()
{
    SendCameraCenterCtrl(DOWN_RIGHT, 1);
}

//拉近
void BCSettingCameraWidgetDlg::on_toolButton_ZoomIn_pressed()
{
    SendCameraCenterCtrl(ZOOM_IN, 0);
}

void BCSettingCameraWidgetDlg::on_toolButton_ZoomIn_released()
{
    SendCameraCenterCtrl(ZOOM_IN, 1);
}

//推远
void BCSettingCameraWidgetDlg::on_toolButton_ZoomOut_pressed()
{
    SendCameraCenterCtrl(ZOOM_OUT, 0);
}

void BCSettingCameraWidgetDlg::on_toolButton_ZoomOut_released()
{
    SendCameraCenterCtrl(ZOOM_OUT, 1);
}
