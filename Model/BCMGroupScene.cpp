#include "BCMGroupScene.h"
#include "BCMWindowScene.h"
#include "BCMRoom.h"
#include "BCMSubTitlesScene.h"
#include "../Common/BCCommon.h"
#include "../Common/BCLocalServer.h"
#include "../Common/BCCommunication.h"

BCMGroupScene::BCMGroupScene(BCMRoom *pRoom)
    :QObject(0)
{
    m_pRoom = pRoom;

    // 创建场景组ID
    CreateGroupSceneID();
    m_qsName = QString("GROUP%1").arg(m_nID+1);
}

BCMGroupScene::~BCMGroupScene()
{
    // 清空信号窗场景
    while ( !m_lstWindowScene.isEmpty() )
        delete m_lstWindowScene.takeFirst();
    // 清空字幕场景
    while ( !m_lstSubTitlesScene.isEmpty() )
        delete m_lstSubTitlesScene.takeFirst();
}

void BCMGroupScene::CreateGroupSceneID()
{
    m_nID = 0;
    if (NULL == m_pRoom)
        return;

    QList<int> lstGroupSceneIDs;

    // 循环场景
    QList<BCMGroupScene *> lstGroupScenes = m_pRoom->GetGroupScenes();
    QListIterator<BCMGroupScene *> itGroupScene( lstGroupScenes );
    while ( itGroupScene.hasNext() ) {
        BCMGroupScene *pGroupScene = itGroupScene.next();
        if (NULL == pGroupScene)
            continue;

        // 将ID添加到临时链表
        int id = pGroupScene->GetGroupSceneID();
        lstGroupSceneIDs.append( id );
    }

    // 从0开始循环，如果中间有空缺直接返回，无空缺返回最后一个
    for (int i = 0; i < lstGroupSceneIDs.count()+1; i++) {
        if (lstGroupSceneIDs.contains(i))
            continue;

        m_nID = i;
        return;
    }
}

BCMWindowScene *BCMGroupScene::GetWindowScene(int id)
{
    QListIterator<BCMWindowScene *> it( m_lstWindowScene );
    while (it.hasNext()) {
        BCMWindowScene *pScene = it.next();
        if (NULL == pScene)
            continue;

        if (pScene->GetWindowSceneID() == id)
            return pScene;
    }

    return NULL;
}

void BCMGroupScene::AddWindowScene(BCMWindowScene *pWindowScene)
{
    QMutexLocker locker(&m_pRoom->m_mutex);

    // 如果包含参数场景则直接返回
    if ( m_lstWindowScene.contains( pWindowScene) )
        return;

    m_lstWindowScene.append( pWindowScene );

    // 本地数据库
    if ( !BCCommon::g_bConnectWithServer ) {
        BCLocalServer *pServer = BCLocalServer::Application();

        // 添加场景数据
        pServer->AddScene(pWindowScene->GetWindowSceneID(),
                          m_pRoom->GetRoomID(),
                          m_nID, 0,
                          pWindowScene->GetWindowSceneName(),
                          pWindowScene->GetWindowSceneLoopInterval());

        UpdateWindowSceneSort();
    }
}

void BCMGroupScene::RemoveWindowScene(int id)
{
    QMutexLocker locker(&m_pRoom->m_mutex);

    BCMWindowScene *pWindowScene = NULL;
    QListIterator<BCMWindowScene *> it( m_lstWindowScene );
    while (it.hasNext()) {
        BCMWindowScene *pCurrentScene = it.next();
        if (NULL == pCurrentScene)
            continue;

        if (pCurrentScene->GetWindowSceneID() != id)
            continue;

        pWindowScene = pCurrentScene;
        break;
    }

    if (NULL == pWindowScene)
        return;

    m_lstWindowScene.removeOne( pWindowScene );

    // 本地数据库
    if ( !BCCommon::g_bConnectWithServer ) {
        BCLocalServer *pServer = BCLocalServer::Application();

        // 删除场景
        pServer->RemoveScene(m_pRoom->GetRoomID(),
                             m_nID,
                             pWindowScene->GetWindowSceneID());

        UpdateWindowSceneSort();
    } else {
        // 服务器不用特殊处理
    }

    // 主动析构场景对象
    delete pWindowScene;
    pWindowScene = NULL;
}

void BCMGroupScene::DeleteWindowScene(BCMWindowScene *pWindowScene)
{
    QMutexLocker locker(&m_pRoom->m_mutex);

    if (NULL == pWindowScene)
        return;

    m_lstWindowScene.removeOne( pWindowScene );

    // 本地数据库
    if ( !BCCommon::g_bConnectWithServer ) {
        BCLocalServer *pServer = BCLocalServer::Application();

        // 删除场景
        pServer->RemoveScene(m_pRoom->GetRoomID(),
                             m_nID,
                             pWindowScene->GetWindowSceneID());

        UpdateWindowSceneSort();
    } else {
        // 服务器删除场景
    }

    // 主动析构场景对象
    delete pWindowScene;
    pWindowScene = NULL;
}

void BCMGroupScene::ClearWindowScene()
{
    // 本地数据库
    if ( !BCCommon::g_bConnectWithServer ) {
        BCLocalServer *pServer = BCLocalServer::Application();

        // 删除场景
        QListIterator<BCMWindowScene *> it( m_lstWindowScene );
        while ( it.hasNext() ) {
            BCMWindowScene *pWindowScene = it.next();

            pServer->RemoveScene(m_pRoom->GetRoomID(),
                                 m_nID,
                                 pWindowScene->GetWindowSceneID());
        }
    }

    // 析构场景对象
    while ( !m_lstWindowScene.isEmpty() )
        delete m_lstWindowScene.takeFirst();
}

void BCMGroupScene::SetGroupSceneName(const QString &qsName, bool bSave)
{
    m_qsName = qsName;

    // 本地数据库
    if ( bSave ) {
        if ( !BCCommon::g_bConnectWithServer ) {
            BCLocalServer *pServer = BCLocalServer::Application();

            pServer->UpdateGroupScene(m_nID,
                                      m_pRoom->GetRoomID(),
                                      0,
                                      0,
                                      1,
                                      m_qsName);
        }
    }
}

void BCMGroupScene::UpdateWindowSceneSort()
{
    QMap<int, int> map;
    for (int i = 0; i < m_lstWindowScene.count(); i++) {
        BCMWindowScene *pScene = m_lstWindowScene.at(i);

        map.insert(pScene->GetWindowSceneID(), i);
    }

    if ( BCCommon::g_bConnectWithServer ) {
        BCCommunication *pServer = BCCommunication::Application();
        pServer->UpdateSceneSort(m_pRoom->GetRoomID(), m_nID, map);
    } else {
        BCLocalServer *pServer = BCLocalServer::Application();
        pServer->UpdateSceneSort(m_pRoom->GetRoomID(), m_nID, map);
    }
}

void BCMGroupScene::UpdateWindowSceneSort(QMap<int, int> map)
{
    QList<int> lstSort = map.values();

    // 升序排列
    qSort(lstSort.begin(), lstSort.end());

    QList<BCMWindowScene *> lstWindowScene;
    for (int i = 0; i < lstSort.count(); i++) {
        int value = lstSort.at( i );
        int key = map.key( value );

        BCMWindowScene *pSortWindowScene = this->GetWindowScene( key );
        if (NULL != pSortWindowScene)
            lstWindowScene.append( pSortWindowScene );
    }

    // 清空原链表并赋值
    m_lstWindowScene.clear();
    m_lstWindowScene = lstWindowScene;
}

void BCMGroupScene::MoveToUp(BCMWindowScene *pWindowScene)
{
    if ((NULL == pWindowScene) || (!m_lstWindowScene.contains(pWindowScene)))
        return;

    int index = m_lstWindowScene.indexOf( pWindowScene );
    if (index > 0)
        m_lstWindowScene.swap(index, index-1);

    UpdateWindowSceneSort();
}

void BCMGroupScene::MoveToDown(BCMWindowScene *pWindowScene)
{
    if ((NULL == pWindowScene) || (!m_lstWindowScene.contains(pWindowScene)))
        return;

    int index = m_lstWindowScene.indexOf( pWindowScene );
    if (index < m_lstWindowScene.count()-1)
        m_lstWindowScene.swap(index, index+1);

    UpdateWindowSceneSort();
}

void BCMGroupScene::MoveToTop(BCMWindowScene *pWindowScene)
{
    if ((NULL == pWindowScene) || (!m_lstWindowScene.contains(pWindowScene)))
        return;

    int index = m_lstWindowScene.indexOf( pWindowScene );
    if (index > 0) {
        BCMWindowScene *pTempScene = m_lstWindowScene.at( index );

        for (int i = index; i > 0; i--) {
            m_lstWindowScene.swap(i, i-1);
        }

        m_lstWindowScene.replace(0, pTempScene);
    }

    UpdateWindowSceneSort();
}

void BCMGroupScene::MoveToBottom(BCMWindowScene *pWindowScene)
{
    if ((NULL == pWindowScene) || (!m_lstWindowScene.contains(pWindowScene)))
        return;

    int index = m_lstWindowScene.indexOf( pWindowScene );
    if (index < m_lstWindowScene.count()-1) {
        BCMWindowScene *pTempScene = m_lstWindowScene.at( index );

        for (int i = index; i < m_lstWindowScene.count()-1; i++) {
            m_lstWindowScene.swap(i, i+1);
        }

        m_lstWindowScene.replace(m_lstWindowScene.count()-1, pTempScene);
    }

    UpdateWindowSceneSort();
}
