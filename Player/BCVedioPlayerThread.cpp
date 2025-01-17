#include "BCVedioPlayerThread.h"
#include "BCVedioManager.h"
#include <QTime>
#include "../Common/BCCommon.h"

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavcodec/avfft.h"
    #include "libavformat/avformat.h"
    #include "libavformat/avio.h"
    #include "libswscale/swscale.h"
    #include "libavutil/common.h"
    #include "libavutil/avstring.h"
    #include "libavutil/imgutils.h"
    #include "libavutil/time.h"
    #include "libavdevice/avdevice.h"
    #include "libswresample/swresample.h"

    #include "SDL2/SDL.h"
    #include "SDL2/SDL_events.h"
    #include "SDL2/SDL_thread.h"
}

BCVedioPlayerThread::BCVedioPlayerThread(BCVedioManager *parent)
    :QThread( parent )
{
    m_parent = parent;

    av_image_alloc(SrcData, SrcLinesize,1920, 1080, AV_PIX_FMT_YUV420P, 1);
    //av_image_alloc(DesData, DesLinesize,1920, 1080, AV_PIX_FMT_RGB24, 1);
    //av_image_alloc(DesData, DesLinesize,3840, 2160, AV_PIX_FMT_RGB24, 1);
    av_image_alloc(DesData, DesLinesize,3840, 2160, AV_PIX_FMT_YUV420P, 1);

    m_bQuit = false;
}

BCVedioPlayerThread::~BCVedioPlayerThread()
{
    // 这里的srcData可能是野指针了，这里需要重新初始化并释放
    memset(SrcData, 0, sizeof(SrcData));
    memset(SrcLinesize, 0, sizeof(SrcLinesize));
    memset(DesData, 0, sizeof(DesData));
    memset(DesLinesize, 0, sizeof(DesLinesize));
    av_freep(&SrcData[0]);
    av_freep(&DesData[0]);

    qDebug() << "BCVedioPlayerThread::~BCVedioPlayerThread()~~";
}

void BCVedioPlayerThread::Quit()
{
    m_bQuit = true;
    SDL_Delay(50);

    this->requestInterruption();
    this->quit();
    this->wait(50);
    this->deleteLater();
}

void BCVedioPlayerThread::run()
{
    while ( !this->isInterruptionRequested() ) {
        if ( m_bQuit )
            return;

        QListIterator<BCVedioWindow *> it( m_parent->m_lstWindow );
        while ( it.hasNext() ) {
            if ( m_bQuit )
                return;

            //qDebug() << m_parent->m_lstWindow.count() << "~~~~~~~~~~~";
            //QTime beginTime = QTime::currentTime();
            QMutexLocker locker(&m_parent->m_mutexWindow);

            BCVedioWindow *pWindow = it.next();
            if (-1 == m_parent->m_lstWindow.indexOf( pWindow ))
                continue;

            // ??? 这里根据chid等信息找图片
            // m_id代表tab，从0-3
            // pWindow->m_chid代表通道ID
            // --------------------------------------------------------------------------------------------------------------- BRPlayer.cpp - GetYUVFrameData() begin
            unsigned char use_target_type;
            if((pWindow->m_rect.width()>=1260)||(pWindow->m_rect.height()>=640)) {
                use_target_type = 3;
            } else	if((pWindow->m_rect.width()>=720)||(pWindow->m_rect.height()>=400)) {
                use_target_type = 3;
            } else	if((pWindow->m_rect.width()>=380)||(pWindow->m_rect.height()>=300)) {
                use_target_type = 3;
            } else {
                use_target_type = 0;
            }

            // liuwl 直接找四个线程
            if (m_parent->m_lstFrameOfDecodeThread1.isEmpty()
                    || m_parent->m_lstFrameOfDecodeThread2.isEmpty()
                    || m_parent->m_lstFrameOfDecodeThread3.isEmpty()
                    || m_parent->m_lstFrameOfDecodeThread4.isEmpty()) {
                QThread::msleep( 100 );
                qDebug() << "decode thread is empty." << m_parent->m_lstFrameOfDecodeThread1.count()
                         << m_parent->m_lstFrameOfDecodeThread2.count()
                         << m_parent->m_lstFrameOfDecodeThread3.count()
                         << m_parent->m_lstFrameOfDecodeThread4.count();
                continue;
            }

            int nFrameMinCount = 2;    // 防止数据帧太少不能解码
            m_parent->m_mutexFrame.lock();
            BCVedioFrame frame1 = (m_parent->m_lstFrameOfDecodeThread1.count() <= nFrameMinCount) ? m_parent->m_lstFrameOfDecodeThread1.first() : m_parent->m_lstFrameOfDecodeThread1.takeFirst();
            BCVedioFrame frame2 = (m_parent->m_lstFrameOfDecodeThread2.count() <= nFrameMinCount) ? m_parent->m_lstFrameOfDecodeThread2.first() : m_parent->m_lstFrameOfDecodeThread2.takeFirst();
            BCVedioFrame frame3 = (m_parent->m_lstFrameOfDecodeThread3.count() <= nFrameMinCount) ? m_parent->m_lstFrameOfDecodeThread3.first() : m_parent->m_lstFrameOfDecodeThread3.takeFirst();
            BCVedioFrame frame4 = (m_parent->m_lstFrameOfDecodeThread4.count() <= nFrameMinCount) ? m_parent->m_lstFrameOfDecodeThread4.first() : m_parent->m_lstFrameOfDecodeThread4.takeFirst();
            m_parent->m_mutexFrame.unlock();

            unsigned char	tmp_prew_pos_tab[65*5]; // updata_exp_a
            unsigned int target_pos_tab_len = 0;
            unsigned char b0,b1,b2,b3;
            unsigned char target_pos_tab[256][5];
            int i,j,c0,c1;
            for(j=0;j<4;j++)
            {
                // liuwl
                char *frame_tmp_prew_pos_tab = NULL;
                switch ( j ) {
                case 0:
                    frame_tmp_prew_pos_tab = frame1.tmp_prew_pos_tab.data();
                    break;
                case 1:
                    frame_tmp_prew_pos_tab = frame2.tmp_prew_pos_tab.data();
                    break;
                case 2:
                    frame_tmp_prew_pos_tab = frame3.tmp_prew_pos_tab.data();
                    break;
                default:
                    frame_tmp_prew_pos_tab = frame4.tmp_prew_pos_tab.data();
                    break;
                }

                if (NULL == frame_tmp_prew_pos_tab)
                    continue;

                memcpy(tmp_prew_pos_tab, (void *)frame_tmp_prew_pos_tab, 65*5);

                b0 = tmp_prew_pos_tab[(0*5)+0];
                b1 = tmp_prew_pos_tab[(0*5)+1];
                b2 = tmp_prew_pos_tab[(0*5)+2];
                b3 = tmp_prew_pos_tab[(0*5)+3];
                if((b0==0xec)&&(b1==0x98)&&(b3==0xaa))
                {
                    b0 = 0;
                    int k = b2;
                    for(i=1;i<=k;i++)
                    {
                        target_pos_tab[target_pos_tab_len][0] = tmp_prew_pos_tab[(i*5)+0];
                        target_pos_tab[target_pos_tab_len][1] = tmp_prew_pos_tab[(i*5)+1];
                        target_pos_tab[target_pos_tab_len][2] = tmp_prew_pos_tab[(i*5)+2];
                        target_pos_tab[target_pos_tab_len][3] = tmp_prew_pos_tab[(i*5)+3];
                        target_pos_tab[target_pos_tab_len][4] = tmp_prew_pos_tab[(i*5)+4];
                        target_pos_tab_len++;
                    }
                }
            }

//            for(i=0;i<target_pos_tab_len;i++)
//            {
//                if((target_pos_tab[i][0]==pWindow->m_chid)&&(target_pos_tab[i][1]==use_target_type))
//                {
//                    break;
//                }
//            }
//            c0 = target_pos_tab[i][2];
//            c1 = target_pos_tab[i][3];

//            if((c0==0)&&(c1>=1)&&(c1<=2))
//            {

//            }
//            else
//            {
//                c0 += 0;
//            }

//            if((c0<=7)&&(c1<=7))
//            {
//                j = 0;
//            }
//            else	if((c0>=11)&&(c1<=7))
//            {
//                c0-=11;
//                j = 3;
//            }
//            else	if((c0<=7)&&(c1>=19))
//            {
//                c1-=19;
//                j = 1;
//            }
//            else	if((c0>=11)&&(c1<=19))
//            {
//                c0-=11;
//                c1-=19;
//                j = 2;
//            }

            b0 = 0;
            if((target_pos_tab_len==0)||(target_pos_tab_len>=0xf0))
            {
                c0 = 0;
                c1 = 0;
                use_target_type = 0;
                j = 0;
            }
            else
            {
                if(use_target_type>3)
                {
                    use_target_type = 0;
                }
                int tmp_use_target_type = use_target_type;
                for(;tmp_use_target_type>=0;tmp_use_target_type--)
                {
                    for(i=0;i<target_pos_tab_len;i++)
                    {
                        if((target_pos_tab[i][0]==pWindow->m_chid)&&(target_pos_tab[i][1]==tmp_use_target_type))
                        {
                            break;
                        }
                    }
                    if(i<target_pos_tab_len)
                    {
                        c0 = target_pos_tab[i][2];
                        c1 = target_pos_tab[i][3];
                        use_target_type = tmp_use_target_type;

                        int k = tmp_use_target_type * 128 + pWindow->m_chid;
                        m_parent->tmp_bak_pos_tab[k][0] = c0;
                        m_parent->tmp_bak_pos_tab[k][1] = c1;
                        use_target_type = tmp_use_target_type;

                        break;
                    }
                    else
                    {
                        int k = tmp_use_target_type * 128 + pWindow->m_chid;
                        c0 = m_parent->tmp_bak_pos_tab[k][0];
                        c1 = m_parent->tmp_bak_pos_tab[k][1];

                        j = 0;
                    }
                }
            }

            if((c0>=0)&&(c0<=7)&&(c1>=0)&&(c1<=7))//updata_exp_b
            {
                j = 0;
            }
            else	if((c0>=11)&&(c0<=19)&&(c1>=0)&&(c1<=7))
            {
                c0-=11;
                j = 3;
            }
            else	if((c0>=0)&&(c0<=7)&&(c1>=19)&&(c1<=26))
            {
                c1-=19;
                j = 1;
            }
            else	if((c0>=11)&&(c0<=19)&&(c1>=19)&&(c1<=26))
            {
                c0-=11;
                c1-=19;
                j = 2;
            }

            int xstart = c0 * 240;
            int ystart = c1 * 136;
            int width = 240;
            int height = 136;
            if(use_target_type==0)
            {
                width = 240;
                height = 136;
            }
            else	if(use_target_type==1)
            {
                width = 480;
                height = 270;
            }
            else	if(use_target_type==2)
            {
                width = 960;
                height = 540;
            }
            else	if(use_target_type==3)
            {
                width = 1920;
                height = 1080;
            }
            // --------------------------------------------------------------------------------------------------------------- BRPlayer.cpp - GetYUVFrameData() end

            // j为端口ID，跟DLL中有偏差
            BYTE *pY = NULL;
            BYTE *pU = NULL;
            BYTE *pV = NULL;

            // 连接设备和连接服务器时对应的顺序不同
            int nCase1 = 0;
            int nCase2 = 1;
            int nCase3 = 2;
            if (j == nCase1) {
                pY = (BYTE *)frame2.pY.data();
                pU = (BYTE *)frame2.pU.data();
                pV = (BYTE *)frame2.pV.data();
            } else if (j == nCase2) {
                pY = (BYTE *)frame4.pY.data();
                pU = (BYTE *)frame4.pU.data();
                pV = (BYTE *)frame4.pV.data();
            } else if (j == nCase3) {
                pY = (BYTE *)frame3.pY.data();
                pU = (BYTE *)frame3.pU.data();
                pV = (BYTE *)frame3.pV.data();
            } else {
                pY = (BYTE *)frame1.pY.data();
                pU = (BYTE *)frame1.pU.data();
                pV = (BYTE *)frame1.pV.data();
            }

            int srcLeft = xstart;
            int scrTop = ystart;
            int srcWidth = width;
            int srcHeight = height;

            // 图片最大支持1920*1080，如果尺寸超出则重新计算尺寸
            srcWidth = (xstart+width) > 1920 ? (1920-xstart) : width;
            srcHeight = (ystart+height) > 1080 ? (1080-ystart) : height;

            // 窗口尺寸可能大于1920*1080
//            int destWidth = (pWindow->m_rect.width() > 1920) ? 1920 : pWindow->m_rect.width();
//            int destHeight = (pWindow->m_rect.height() > 1080) ? 1080 : pWindow->m_rect.height();
            int destWidth = pWindow->m_rect.width();
            int destHeight = pWindow->m_rect.height();

            // 当尺寸不合法时跳过
            if ((destWidth < 4) || (destHeight < 4))
                continue;

            SrcData[0] = (uint8_t *)pY + 1920*scrTop + srcLeft;
            SrcData[1] = (uint8_t *)pU + 480*scrTop + (srcLeft/2);
            SrcData[2] = (uint8_t *)pV + 480*scrTop + (srcLeft/2);

            // 这里直接用SDL渲染带句柄的窗口
            SDL_Rect WinRect;
            WinRect.x = 0;
            WinRect.y = 0;
            WinRect.w = destWidth;
            WinRect.h = destHeight;

            // 如果图片原尺寸和转换后的尺寸一样则直接渲染YUV
            //struct SwsContext *img_convert_ctx = sws_getContext(srcWidth, srcHeight,AV_PIX_FMT_YUV420P,destWidth, destHeight, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
            struct SwsContext *img_convert_ctx = sws_getContext(srcWidth, srcHeight,AV_PIX_FMT_YUV420P,destWidth, destHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
            if (NULL != img_convert_ctx) {
                sws_scale(img_convert_ctx, (const uint8_t* const*)SrcData, SrcLinesize, 0, srcHeight, DesData, DesLinesize);
                sws_freeContext(img_convert_ctx);
            }

            // 解码前有窗口，解码后可能窗口刚好被析构
            if ( m_bQuit ) {
                return;
            }

            // 在外面添加线程锁，如果开窗特别多可能会造成特别慢的问题
            if (-1 != m_parent->m_lstWindow.indexOf(pWindow)) {
                SDL_RenderClear(pWindow->pRenderer);

                // YUV render
                SDL_UpdateYUVTexture(pWindow->pYUVTexture, &WinRect,
                                DesData[0], DesLinesize[0],
                                DesData[1], DesLinesize[1],
                                DesData[2], DesLinesize[2]);
                SDL_RenderCopy(pWindow->pRenderer, pWindow->pYUVTexture, &WinRect, NULL);

                // RGB render
//                SDL_UpdateTexture(pWindow->pTexture, NULL, DesData[0], DesLinesize[0]);
//                SDL_RenderCopy(pWindow->pRenderer, pWindow->pTexture, &WinRect, NULL);
                SDL_RenderPresent(pWindow->pRenderer);
            }
//            qDebug() << QString("scale cost %1 ms, from size(%2, %3) to size(%4, %5), and C0: %6, C1: %7, j: %8").arg(beginTime.msecsTo( QTime::currentTime() ))
//                        .arg(srcWidth).arg(srcHeight).arg(destWidth).arg(destHeight)
//                        .arg(c0).arg(c1).arg(j);
        }

        SDL_Delay(10);
    }
}
