#include "BCVedioDecodeThread.h"
#include <QPixmap>
#include <QThread>
#include <QBuffer>
#include <QTime>
#include <QLabel>
#include "BCVedioManager.h"
#include "../Common/BCCommon.h"

#define DelayCount 4000
#define DelayCountByServer 0

BCVeioDecodeThread::BCVeioDecodeThread(const QString &ip, int port, int id, BCVedioManager *parent)
    :QThread( NULL )
{
    m_parent = parent;
    m_ip = ip;
    m_port = port;
    m_id = id;
    m_bQuit = false;
    m_nDelayCount = DelayCount+1;

    // init ffmpeg image
    main_buffer_len = 0;
    //main_buffer_ptr = (unsigned char *)malloc(16*1*1024*1024);

    // init 3531
    g_dec_hi3531_app.m_hi3531_end=0;
    g_dec_hi3531_app.m_hi3531_handle = NULL;
    g_dec_hi3531_app.m_hi3531_imageenhanceenable = 0;
    g_dec_hi3531_app.m_hi3531_multithreadenable = 0;
    g_dec_hi3531_app.m_hi3531_strenthcoeff = 40;
    g_dec_hi3531_app.m_hi3531_deinterlaceenable = 0;
    g_dec_hi3531_app.m_hi3531_directoutputenable = 0;
    g_dec_hi3531_app.m_hi3531_showframeinfo = 0;
    g_dec_hi3531_app.m_hi3531_flags = 0;
    g_dec_hi3531_app.m_hi3531_result = -1;
    g_dec_hi3531_app.m_hi3531_pic_cnt = 0;
    //g_dec_hi3531_app.m_hi3531_main_ptr = (unsigned char *)malloc(16*2*2048*2048);
    //tmp_local_time_id = 0;

    init_hi3531_h264_decode();

//    if (g_dec_hi3531_app.m_hi3531_main_ptr==NULL) {
//        tmp_local_time_id = 0;
//    }
//    tmp_local_time_id = m_id * 0x1234;

    // init tcp socket
    m_pSocket = new QTcpSocket(this);
    connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(onRecvTcpMessage()));
    ConnectSocket();

    // frame signal slot
    connect(this, SIGNAL(sigAppendVedioFrame(int,QByteArray,QByteArray,QByteArray,HI_U32,HI_U32,HI_U32,HI_U32,QByteArray)),
            m_parent, SLOT(onAppendVedioFrame(int,QByteArray,QByteArray,QByteArray,HI_U32,HI_U32,HI_U32,HI_U32,QByteArray)), Qt::DirectConnection);
}

BCVeioDecodeThread::~BCVeioDecodeThread()
{
    Hi264DecDestroy(g_dec_hi3531_app.m_hi3531_handle);
    //free(g_dec_hi3531_app.m_hi3531_main_info_aux_ptr);

    //free(g_dec_hi3531_app.m_hi3531_main_ptr);
    //free(main_buffer_ptr);

    qDebug() << "BCVeioDecodeThread::~BCVeioDecodeThread() ~~" << m_id;
}

void BCVeioDecodeThread::SetConnectInfo(const QString &ip, int port)
{
    if ((m_ip == ip) && (m_port == port))
        return;

    m_ip = ip;
    m_port = port;

    QAbstractSocket::SocketState state = m_pSocket->state();
    if ((state == QAbstractSocket::ConnectedState) || (state == QAbstractSocket::ConnectingState)) {
        m_pSocket->disconnectFromHost();
        m_pSocket->waitForDisconnected( 500 );
        m_pSocket->connectToHost(QHostAddress(m_ip), m_port+m_id);
    }
}

void BCVeioDecodeThread::ReadyQuit()
{
    // socket
    disconnect(m_pSocket, SIGNAL(readyRead()), this, SLOT(onRecvTcpMessage()));
    disconnect(this, SIGNAL(sigAppendVedioFrame(int,QByteArray,QByteArray,QByteArray,HI_U32,HI_U32,HI_U32,HI_U32,QByteArray)),
            m_parent, SLOT(onAppendVedioFrame(int,QByteArray,QByteArray,QByteArray,HI_U32,HI_U32,HI_U32,HI_U32,QByteArray)));

    // clear socket recv list
    m_mutex.lock();
    m_lstRecvByteArr.clear();
    m_mutex.unlock();

    m_bQuit = true;
}

void BCVeioDecodeThread::Quit()
{
    if ( m_pSocket->state() == QAbstractSocket::ConnectedState ) {
        m_pSocket->disconnectFromHost();
        m_pSocket->waitForDisconnected( 50 );
    }
    m_pSocket->deleteLater();

    // thread
    this->requestInterruption();
    this->quit();
    this->wait(50);
    this->deleteLater();
}

void BCVeioDecodeThread::onRecvTcpMessage()
{
    if ( m_bQuit )
        return;

    QByteArray byteTcpRecv = m_pSocket->readAll();

    //qDebug() << m_ip << m_port << m_id << "~~~~~~~~~~~~" << m_lstRecvByteArr.count();

    m_nDelayCount++;

    int nDelayCount = DelayCount;
    if (m_nDelayCount < nDelayCount) {
        //
        qDebug() << "delay frame " << m_nDelayCount;
    } else {
        m_mutex.lock();
        m_lstRecvByteArr.append( byteTcpRecv );
        m_mutex.unlock();

        m_nDelayCount = DelayCount+1;
    }
}

void BCVeioDecodeThread::DisConnectSocket()
{
    if ( m_pSocket->state() == QAbstractSocket::ConnectedState ) {
        m_pSocket->disconnectFromHost();
    }
    m_nDelayCount = 0;
}

void BCVeioDecodeThread::ConnectSocket()
{
    if ( m_ip.isEmpty() )
        return;

    m_pSocket->connectToHost(QHostAddress(m_ip), m_port+m_id);
    m_pSocket->waitForConnected( 500 );
}

bool BCVeioDecodeThread::IsConnected()
{
    return (m_pSocket->state() == QAbstractSocket::ConnectedState) ? true : false;
}

void BCVeioDecodeThread::run()
{
    while ( !this->isInterruptionRequested() ) {
        if ( m_bQuit )
            return;

        QByteArray recvByte;
        m_mutex.lock();
        if ( !m_lstRecvByteArr.isEmpty() )
            recvByte.append( m_lstRecvByteArr.takeFirst() );
        m_mutex.unlock();

        if ( !recvByte.isEmpty() ) {
            int nRevLen = recvByte.size();

            if (nRevLen < (8*1024*1024)) {
                g_dec_hi3531_app.m_hi3531_result = Hi264DecFrame(g_dec_hi3531_app.m_hi3531_handle, (HI_U8 *)recvByte.data(), nRevLen, 0, &(g_dec_hi3531_app.m_hi3531_dec_frame),g_dec_hi3531_app.m_hi3531_flags);
            } else {
                g_dec_hi3531_app.m_hi3531_result = HI_H264DEC_NEED_MORE_BITS;
            }

            while (HI_H264DEC_NEED_MORE_BITS != g_dec_hi3531_app.m_hi3531_result) {
                if (HI_H264DEC_OK == g_dec_hi3531_app.m_hi3531_result) {   //get a picture
                    if ( g_dec_hi3531_app.m_hi3531_imageenhanceenable ) {    //image enhance
                        Hi264DecImageEnhance(g_dec_hi3531_app.m_hi3531_handle, &(g_dec_hi3531_app.m_hi3531_dec_frame),g_dec_hi3531_app.m_hi3531_strenthcoeff);
                    }

                    HI_U8 *pY = (g_dec_hi3531_app.m_hi3531_dec_frame).pY;
                    HI_U8 *pU = (g_dec_hi3531_app.m_hi3531_dec_frame).pU;
                    HI_U8 *pV = (g_dec_hi3531_app.m_hi3531_dec_frame).pV;
                    HI_U32 width    = (g_dec_hi3531_app.m_hi3531_dec_frame).uWidth;
                    HI_U32 height   = (g_dec_hi3531_app.m_hi3531_dec_frame).uHeight - (g_dec_hi3531_app.m_hi3531_dec_frame).uCroppingBottomOffset;
                    HI_U32 yStride  = (g_dec_hi3531_app.m_hi3531_dec_frame).uYStride;
                    HI_U32 uvStride = (g_dec_hi3531_app.m_hi3531_dec_frame).uUVStride;
                    HI_U32 frame_yuv_size = height* yStride;

                    unsigned char tmp_prew_pos_tab[65*5];
                    // --------------------------------------------------------------------------------------------------------------- user_hi3531_dec.cpp - hi_decode_h264_to_buffer() begin
                    unsigned char b0,b1,b2,b3,b4,b5,b6;
                    memcpy(tmp_line_mask_id_ptr,(HI_U8 *)(pY + width*1087 + 0),1920);
                    int i = 0;
                    for(i=0;i<1920;i++)
                    {
                        if(tmp_line_mask_id_ptr[i]<0x40)
                        {
                            tmp_line_mask_id_ptr_a[i] = 0;
                        }
                        else
                        {
                            tmp_line_mask_id_ptr_a[i] = 1;
                        }
                    }
                    for(i=0;i<1920;i++)
                    {
                        if(		(tmp_line_mask_id_ptr_a[i+0]==1)&&(tmp_line_mask_id_ptr_a[i+1]==0)&&(tmp_line_mask_id_ptr_a[i+2]==1)&&(tmp_line_mask_id_ptr_a[i+3]==0)
                            &&  (tmp_line_mask_id_ptr_a[i+4]==1)&&(tmp_line_mask_id_ptr_a[i+5]==0)&&(tmp_line_mask_id_ptr_a[i+6]==1)&&(tmp_line_mask_id_ptr_a[i+7]==0)
                            &&  (tmp_line_mask_id_ptr_a[i+8]==1)&&(tmp_line_mask_id_ptr_a[i+9]==0)&&(tmp_line_mask_id_ptr_a[i+10]==1)&&(tmp_line_mask_id_ptr_a[i+11]==0)
                            &&  (tmp_line_mask_id_ptr_a[i+12]==1)&&(tmp_line_mask_id_ptr_a[i+13]==0)&&(tmp_line_mask_id_ptr_a[i+14]==1)&&(tmp_line_mask_id_ptr_a[i+15]==1)
                            )
                        {
                            i += 16;
                            break;
                        }
                    }
                    if(i<0x30)
                    {
                        b0 = 0;
                        HI_U8 bits_index = 0;
                        HI_U8 byte_index = 4;
                        for(;i<1920;i++)
                        {
                            b0 >>= 1;
                            b0 += (tmp_line_mask_id_ptr_a[i]==1)?0x80:0;
                            bits_index++;
                            if(bits_index>=8)
                            {
                                bits_index = 0;
                                tmp_line_mask_id_ptr[byte_index] = b0;
                                byte_index++;
                            }
                        }
                        b0 = 0;
                        {
                            unsigned int target_pos_tab_len = 0;
                            for(i=0;i<256*5;i++)
                            {
                                ((unsigned char *)&target_pos_tab[0][0])[i] = 0;
                            }
                            unsigned int j = 0;
                            {
                                unsigned char limit_cnt = 0;
                                memcpy(tmp_prew_pos_tab,&tmp_line_mask_id_ptr[4],240);
                                b0 = tmp_prew_pos_tab[(0<<2)+0];
                                b1 = tmp_prew_pos_tab[(0<<2)+1];
                                b2 = tmp_prew_pos_tab[(0<<2)+2];
                                b3 = tmp_prew_pos_tab[(0<<2)+3];
                                if((b0==0xec)&&(b1==0x98)&&(b3==0xaa))
                                {
                                    limit_cnt = b2;
                                }
                                target_pos_tab[target_pos_tab_len][0] = 0xec;
                                target_pos_tab[target_pos_tab_len][1] = 0x98;
                                target_pos_tab[target_pos_tab_len][2] = limit_cnt;
                                target_pos_tab[target_pos_tab_len][3] = 0xaa;
                                target_pos_tab[target_pos_tab_len][4] = 0x55;
                                target_pos_tab_len++;
                                for(i=0;i<limit_cnt;i++)
                                {
//                                    b0 = tmp_prew_pos_tab[(i<<2)+0+4];
//                                    b1 = tmp_prew_pos_tab[(i<<2)+1+4];
//                                    b2 = tmp_prew_pos_tab[(i<<2)+2+4];
//                                    b3 = tmp_prew_pos_tab[(i<<2)+3+4];
//                                    if((b2&0xc0)==0x80)
//                                    {
//                                        b5 = b0>>6;
//                                        b5 &= 0x01;
//                                        b6 = (b1<<1)+b5;
//                                        b4 = (b3>>6);
//                                        b2 &= 0x1f;

//                                        //lock_target_pos = ((b3&0x1f)<<8)+b2;
//                                        target_pos_tab[target_pos_tab_len][0] = b6;
//                                        target_pos_tab[target_pos_tab_len][1] = b4;
//                                        target_pos_tab[target_pos_tab_len][2] = b2;
//                                        target_pos_tab[target_pos_tab_len][3] = (b3&0x1f);
//                                        target_pos_tab[target_pos_tab_len][4] = (j+2)&0x03;
//                                        target_pos_tab_len++;
//                                    }

                                    // updata_exp_a
                                    //b0 = tmp_prew_pos_tab[(i<<2)+0+4];
                                    b1 = tmp_prew_pos_tab[(i*3)+0+4];
                                    b2 = tmp_prew_pos_tab[(i*3)+1+4];
                                    b3 = tmp_prew_pos_tab[(i*3)+2+4];
                                    if((b2&0xc0)==0x80)
                                    {
                                        b5 = b0>>6;
                                        b5 &= 0x01;
                                        b6 = b1;//(b1<<1)+b5;//index
                                        b4 = (b3>>6);//src_type
                                        b2 &= 0x1f;//position_in_page

                                        //lock_target_pos = ((b3&0x1f)<<8)+b2;
                                        target_pos_tab[target_pos_tab_len][0] = b6;//index
                                        target_pos_tab[target_pos_tab_len][1] = b4;//src_type
                                        target_pos_tab[target_pos_tab_len][2] = b2;//position_in_page
                                        target_pos_tab[target_pos_tab_len][3] = (b3&0x1f);//page index
                                        target_pos_tab[target_pos_tab_len][4] = (j+2)&0x03;
                                        target_pos_tab_len++;
                                    }
                                }
                            }
                            b0 = 0;
                        }
//                        int n=0;
//                        for(i=0;i<48;i++)
//                        {
//                            for(int j=0;j<5;j++)
//                            {
//                                tmp_prew_pos_tab[n++] = target_pos_tab[i][j];
//                            }
//                        }
                        int n = 0;//updata_exp_a
                        for(i=0;i<(64+1);i++)
                        {
                            for(int j=0;j<5;j++)
                            {
                                tmp_prew_pos_tab[n++] = target_pos_tab[i][j];
                            }
                        }
                    }
                    else {
//                        int n=0;
//                        for(i=0;i<48;i++)
//                        {
//                            for(int j=0;j<5;j++)
//                            {
//                                tmp_prew_pos_tab[n++] = 0xff;
//                            }
//                        }
                        int n=0;
                        for(i=0;i<(64+1);i++)
                        {
                            for(int j=0;j<5;j++)
                            {
                                tmp_prew_pos_tab[n++] = 0xff;
                            }
                        }
                    }
                    // --------------------------------------------------------------------------------------------------------------- user_hi3531_dec.cpp - hi_decode_h264_to_buffer() end

                    // 发送帧数据信号
                    QByteArray byteY((const char *)pY, frame_yuv_size);
                    QByteArray byteU((const char *)pU, height* uvStride/2);
                    QByteArray byteV((const char *)pV, height* uvStride/2);
                    QByteArray byte_tmp_prew_pos_tab((const char *)tmp_prew_pos_tab, 65*5);

                    emit sigAppendVedioFrame(m_id, byteY, byteU, byteV, width, height, yStride, uvStride, byte_tmp_prew_pos_tab);

                    // 数据接收不够可能会造成死循环
                    if ( m_bQuit )
                        return;
                }

                g_dec_hi3531_app.m_hi3531_result = Hi264DecFrame(g_dec_hi3531_app.m_hi3531_handle, NULL,  0, 0, &(g_dec_hi3531_app.m_hi3531_dec_frame),  g_dec_hi3531_app.m_hi3531_flags);
            }
        } else {
            QThread::msleep( 20 );
        }
    }
}

bool BCVeioDecodeThread::init_hi3531_h264_decode()
{
    Hi264DecGetInfo(&g_dec_hi3531_app.m_hi3531_lib_info);

    g_dec_hi3531_app.m_hi3531_directoutputenable = 0;
    g_dec_hi3531_app.m_hi3531_deinterlaceenable = 0;
    g_dec_hi3531_app.m_hi3531_multithreadenable = 1;
    g_dec_hi3531_app.m_hi3531_imageenhanceenable = 1;
    g_dec_hi3531_app.m_hi3531_showframeinfo = 1;

    /*init the config info for docoder*/
    g_dec_hi3531_app.m_hi3531_dec_attrbute.uBufNum        = 2;//16;     // reference frames number: 16
    g_dec_hi3531_app.m_hi3531_dec_attrbute.uPicHeightInMB = 70;     // D1(720x576)
    g_dec_hi3531_app.m_hi3531_dec_attrbute.uPicWidthInMB  = 128;
    g_dec_hi3531_app.m_hi3531_dec_attrbute.uStreamInType  = 0x00;   // bitstream begin with "00 00 01" or "00 00 00 01"
    g_dec_hi3531_app.m_hi3531_tcp_ch = m_id;

    if(g_dec_hi3531_app.m_hi3531_deinterlaceenable)
    {
        /* bit0 = 1: H.264 normal output mode; bit0 = 0: direct output mode */
        g_dec_hi3531_app.m_hi3531_dec_attrbute.uWorkMode = 0x00;
    }
    else
    {
        g_dec_hi3531_app.m_hi3531_dec_attrbute.uWorkMode = 0x01;
    }

    if(g_dec_hi3531_app.m_hi3531_deinterlaceenable)
    {
        /* bit4 = 1:  enable deinteralce;    bit4 = 0: disable deinterlace */
        g_dec_hi3531_app.m_hi3531_dec_attrbute.uWorkMode |= 0x10;
    }

    // 多线程
    if (g_dec_hi3531_app.m_hi3531_multithreadenable)
    {
        g_dec_hi3531_app.m_hi3531_dec_attrbute.uWorkMode |= 0x20;
    }

    //return true;
    /*create a decoder*/
    g_dec_hi3531_app.m_hi3531_handle = Hi264DecCreate(&g_dec_hi3531_app.m_hi3531_dec_attrbute);
    if(g_dec_hi3531_app.m_hi3531_handle==NULL)
    {
        return	false;
    }

    return true;
}
