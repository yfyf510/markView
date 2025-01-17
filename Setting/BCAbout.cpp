#include "BCAbout.h"
#include "ui_BCAbout.h"
#include "../Common/BCCommon.h"
BCAbout::BCAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BCAbout)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 标题和图标
    setWindowTitle(tr("关于"));
    setWindowIcon( QIcon(BCCommon::g_qsImageFilePrefix + BCCommon::g_qsApplicationIcon) );

    // 设置内容
    ui->widget_2->setStyleSheet(QString("QWidget#widget_2{border-image: url(%1);}").arg(BCCommon::g_qsApplicationDefaultStylePath+"/aboutBackground.png"));

//    ui->m_pVersionLabel->setText( QString("Ver: %1").arg(BCCommon::g_qsApplicationVersion) );
//    ui->m_pWebLabel->setText( QString("Web: %1").arg("www.oriondisplay.net") );
//    ui->m_pWebLabel->setText( QString("Web: <a href = www.oriondisplay.net>www.oriondisplay.net</a>") );
//    connect(ui->m_pWebLabel,SIGNAL(linkActivated(QString)),this,SLOT(openUrl(QString)));
}

BCAbout::~BCAbout()
{
    delete ui;
}

//void BCAbout::openUrl(QString url)
//{
//    QDesktopServices::openUrl(QUrl(url));
//}

