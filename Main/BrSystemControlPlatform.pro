#-------------------------------------------------
#
# Project created by QtCreator 2016-05-06T13:49:55
#
#-------------------------------------------------

QT       += core gui network xml axcontainer
QT += sql serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#DEFINES += QT_NO_WARNING_OUTPUT\ QT_NO_DEBUG_OUTPUT
#屏蔽debug输�??#DEFINES += QT_NO_WARNING_OUTPUT\ QT_NO_DEBUG_OUTPUT
TARGET = markView
TEMPLATE = app
INCLUDEPATH += ../Setting \
               ../View

SOURCES +=\
        MainWindow.cpp \
    ../Common/BCCommon.cpp \
    ../Model/BCMChannel.cpp \
    ../Model/BCMDisplay.cpp \
    ../Model/BCMGroupDisplay.cpp \
    ../Model/BCMGroupScene.cpp \
    ../Model/BCMSubTitlesScene.cpp \
    ../Model/BCMWindowScene.cpp \
    ../View/BCToolBar.cpp \
    ../View/BCScreenName.cpp \
    ../Model/BCMRoom.cpp \
    ../View/BCScreenDlg.cpp \
    ../Setting/AlarmManagement.cpp \
    ../Setting/BCAbout.cpp \
    ../Setting/BCExProcessDialog.cpp \
    ../Setting/BCSettingDisplyModelStyle.cpp \
    ../Setting/BCSettingMainPanelStyle.cpp \
    ../Setting/BCSettingPasswordStyle.cpp \
    ../Setting/BCWarningLogWindow.cpp \
    ../Setting/LogQueryDialog.cpp \
    ../View/BCCutScreen.cpp \
    ../View/BCVideoControl.cpp \
    ../View/BCSystemplan.cpp \
    ../View/BCSignal.cpp \
    ../View/BCScene.cpp \
    ../View/BCMatrixInput.cpp \
    ../View/BCMatrixCut.cpp \
    ../View/BCMatrix.cpp \
    ../View/BCDecoder.cpp \
    ../View/BCControl.cpp \
    ../View/BCWidgetBtn.cpp \
    ../View/BCFaceWidget.cpp \
    ../View/BCSetSignalDlg.cpp \
    ../View/BCSignalTreeWidget.cpp \
    ../View/BCRibbonMainToolBar.cpp \
    ../View/BCRibbonMainToolBarAction.cpp \
    ../View/BCRibbonBar.cpp \
    ../Setting/BCPlanMapButtonGraphicsItem.cpp \
    ../Setting/BCPlanMapDlg.cpp \
    ../Setting/BCPlanMapGraphicsRectItem.cpp \
    ../Setting/BCPlanMapTreeWidget.cpp \
    ../Setting/BCPlanMapGraphicsView.cpp \
    ../View/BCMonitorWidgetData.cpp \
    ../View/BCSignalListWidgetData.cpp \
    ../View/BCAutoDateDlg.cpp \
    ../View/BCSignalName.cpp \
    ../Setting/BCLoginDlg.cpp \
    ../Common/BCXMLManager.cpp \
    ../Common/BCCommunication.cpp \
    ../Common/BCLocalServer.cpp \
    ../Common/BCLocalDatabase.cpp \
    main.cpp \
    ../Setting/BCOtherDeviceControlDlg.cpp \
    ../Model/BCMGroupChannel.cpp \
    ../Setting/BCSettingCommunicationDlg.cpp \
    ../Setting/BCSettingDisplayInfoDlg.cpp \
    ../Setting/BCSettingDisplaySetNetSetDlg.cpp \
    ../View/BCLocationDlg.cpp \
    ../View/BCLocationSegmentationWidget.cpp \
    ../Setting/BCSettingLanguageDlg.cpp \
    ../Setting/BCSettingOutOfDateDlg.cpp \
    ../View/BCDecoderTreeWidget.cpp \
    ../Setting/BCSettingSignalWindowPropertyDlg.cpp \
    ../Setting/BCSettingSubTitlesDlg.cpp \
    ../Setting/BCSettingBatchSubTitlesDlg.cpp \
    ../Setting/BCSettingIPVedioIPDlg.cpp \
    ../Setting/BCSceneListWidgetData.cpp \
    ../Setting/BCSettingDeviceFormatDlg.cpp \
    ../Setting/BCSettingWaitingDlg.cpp \
    ../Common/BCLocalTcpSocket.cpp \
    ../Setting/BCSettingBoardCardDlg.cpp \
    ../Setting/BCSettingCutInputChannelDlg.cpp \
    ../Setting/BCSettingPerviewDlg.cpp \
    ../Setting/BCSettingOutSideCommandDlg.cpp \
    ../Setting/BCSettingAddIPVedioTreeWidgetItemDlg.cpp \
    ../Common/BCLocalSerialPort.cpp \
    ../Setting/BCSettingOtherDeviceControlDlg.cpp \
    ../Setting/BCSettingDeviceCustomResolutionDlg.cpp \
    ../Setting/BCSettingDisplaySwitchConfigDlg.cpp \
    ../Setting/BCSettingDisplaySwitchRoomWidget.cpp \
    ../View/BCMatrixRoomWidget.cpp \
    ../View/BCMatrixInputNodeWidget.cpp \
    ../View/BCMatrixOutputNodeWidget.cpp \
    ../Setting/BCSettingTaskPlanningDlg.cpp \
    ../Setting/BCSettingTaskDetailDlg.cpp \
    ../View/BCSignalWindowDisplayWidget.cpp \
    ../View/BCRoomWidget.cpp \
    ../View/BCSingleDisplayWidget.cpp \
    ../View/BCGroupDisplayWidget.cpp \
    ../View/BCSingleDisplayVirtualWidget.cpp \
    ../Player/BCVedioManager.cpp \
    ../Player/BCVedioDecodeThread.cpp \
    ../Player/BCVedioPlayerThread.cpp \
    ../Setting/BCSettingModifyEDIDDialog.cpp \
    ../Setting/BCSettingAutoReadInputChannelConfigDlg.cpp \
    ../Setting/BCSettingDebugDlg.cpp \
    ../Setting/BCSettingWindowCopyConfigDlg.cpp \
    ../View/BCControlTreeWidget.cpp \
    ../Setting/BCSettingSingleDisplaySwitchConfigDlg.cpp \
    ../Setting/BCSettingIPVSignalWindowLoopConfigDlg.cpp \
    ../View/BCSceneMatrixTreeWidget.cpp \
    ../Setting/BCSettingModifyNameDlg.cpp \
    ../Setting/BCSettingModifyVP2000IPDlg.cpp \
    ../Setting/BCSettingRemoveShadowDlg.cpp \
    ../Setting/BCSettingMatrixFormatDlg.cpp \
    ../Setting/BCSettingDisplayMakerConfigDlg.cpp \
    ../Model/BCMMatrix.cpp \
    ../Setting/BCSettingJointMatrixChannelConfigDlg.cpp \
    ../Setting/BCSettingMatrixCutInputChannelDlg.cpp \
    ../View/BCSignalSrouceSceneViewWidget.cpp \
    ../Common/BCOutsideInterfaceServer.cpp \
    ../Setting/BCSettingOutsideInterfaceDlg.cpp \
    ../Setting/BCSettingRepeatApplicationWarningDlg.cpp \
    ../Setting/BCSettingRoomVirtualResolutionDlg.cpp \
    ../Setting/BCSettingLEDResolutionDlg.cpp \
    ../Setting/BCSettingChannelRemoteIPDlg.cpp \
    ../Setting/BCSettingCameraWidgetDlg.cpp \
    ../View/BCLocationGroupWidget.cpp


HEADERS  += MainWindow.h \
    ../Common/BCCommon.h \
    ../Model/BCMChannel.h \
    ../Model/BCMDisplay.h \
    ../Model/BCMGroupDisplay.h \
    ../Model/BCMGroupScene.h \
    ../Model/BCMSubTitlesScene.h \
    ../Model/BCMWindowScene.h \
    ../View/BCToolBar.h \
    ../View/BCScreenName.h \
    ../Model/BCMRoom.h \
    ../View/BCScreenDlg.h \
    ../Setting/AlarmManagement.h \
    ../Setting/BCAbout.h \
    ../Setting/BCExProcessDialog.h \
    ../Setting/BCSettingDisplyModelStyle.h \
    ../Setting/BCSettingMainPanelStyle.h \
    ../Setting/BCSettingPasswordStyle.h \
    ../Setting/BCWarningLogWindow.h \
    ../Setting/LogQueryDialog.h \
    ../View/BCVideoControl.h \
    ../View/BCSystemplan.h \
    ../View/BCSignal.h \
    ../View/BCScene.h \
    ../View/BCMatrixInput.h \
    ../View/BCMatrixCut.h \
    ../View/BCMatrix.h \
    ../View/BCDecoder.h \
    ../View/BCCutScreen.h \
    ../View/BCControl.h \
    ../View/BCWidgetBtn.h \
    ../View/BCFaceWidget.h \
    ../View/BCSetSignalDlg.h \
    ../View/BCSignalTreeWidget.h \
    ../View/BCRibbonMainToolBar.h \
    ../View/BCRibbonMainToolBarAction.h \
    ../View/BCRibbonBar.h \
    ../Setting/BCPlanMapButtonGraphicsItem.h \
    ../Setting/BCPlanMapDlg.h \
    ../Setting/BCPlanMapGraphicsRectItem.h \
    ../Setting/BCPlanMapTreeWidget.h \
    ../Setting/BCPlanMapGraphicsView.h \
    ../View/BCMonitorWidgetData.h \
    ../View/BCSignalListWidgetData.h \
    ../View/BCAutoDateDlg.h \
    ../View/BCSignalName.h \
    ../Setting/BCLoginDlg.h \
    ../Common/BCXMLManager.h \
    ../Common/BCCommunication.h \
    ../Common/BCLocalServer.h \
    ../Common/BCLocalDatabase.h \
    ../Setting/BCOtherDeviceControlDlg.h \
    ../Model/BCMGroupChannel.h \
    ../Setting/BCSettingCommunicationDlg.h \
    ../Setting/BCSettingDisplayInfoDlg.h \
    ../Setting/BCSettingDisplaySetNetSetDlg.h \
    ../View/BCLocationDlg.h \
    ../View/BCLocationSegmentationWidget.h \
    ../Setting/BCSettingLanguageDlg.h \
    ../Setting/BCSettingOutOfDateDlg.h \
    ../View/BCDecoderTreeWidget.h \
    ../Setting/BCSettingSignalWindowPropertyDlg.h \
    ../Setting/BCSettingSubTitlesDlg.h \
    ../Setting/BCSettingBatchSubTitlesDlg.h \
    ../Setting/BCSettingIPVedioIPDlg.h \
    ../Setting/BCSceneListWidgetData.h \
    ../Setting/BCSettingDeviceFormatDlg.h \
    ../Setting/BCSettingWaitingDlg.h \
    ../Common/BCLocalTcpSocket.h \
    ../Setting/BCSettingBoardCardDlg.h \
    ../Setting/BCSettingCutInputChannelDlg.h \
    ../Setting/BCSettingPerviewDlg.h \
    ../Setting/BCSettingOutSideCommandDlg.h \
    ../Setting/BCSettingAddIPVedioTreeWidgetItemDlg.h \
    ../Common/BCLocalSerialPort.h \
    ../Setting/BCSettingOtherDeviceControlDlg.h \
    ../Setting/BCSettingDeviceCustomResolutionDlg.h \
    ../Setting/BCSettingDisplaySwitchConfigDlg.h \
    ../Setting/BCSettingDisplaySwitchRoomWidget.h \
    ../View/BCMatrixRoomWidget.h \
    ../View/BCMatrixInputNodeWidget.h \
    ../View/BCMatrixOutputNodeWidget.h \
    ../Setting/BCSettingTaskPlanningDlg.h \
    ../Setting/BCSettingTaskDetailDlg.h \
    ../View/BCSignalWindowDisplayWidget.h \
    ../View/BCRoomWidget.h \
    ../View/BCSingleDisplayWidget.h \
    ../View/BCGroupDisplayWidget.h \
    ../View/BCSingleDisplayVirtualWidget.h \
    ../Player/BCVedioManager.h \
    ../Player/BCVedioDecodeThread.h \
    ../Player/BCVedioPlayerThread.h \
    ../Setting/BCSettingModifyEDIDDialog.h \
    ../Setting/BCSettingAutoReadInputChannelConfigDlg.h \
    ../Setting/BCSettingDebugDlg.h \
    ../Setting/BCSettingWindowCopyConfigDlg.h \
    ../View/BCControlTreeWidget.h \
    ../Setting/BCSettingSingleDisplaySwitchConfigDlg.h \
    ../Setting/BCSettingIPVSignalWindowLoopConfigDlg.h \
    ../View/BCSceneMatrixTreeWidget.h \
    ../Setting/BCSettingModifyNameDlg.h \
    ../Setting/BCSettingModifyVP2000IPDlg.h \
    ../Setting/BCSettingRemoveShadowDlg.h \
    ../Setting/BCSettingMatrixFormatDlg.h \
    ../Setting/BCSettingDisplayMakerConfigDlg.h \
    ../Model/BCMMatrix.h \
    ../Setting/BCSettingJointMatrixChannelConfigDlg.h \
    ../Setting/BCSettingMatrixCutInputChannelDlg.h \
    ../View/BCSignalSrouceSceneViewWidget.h \
    ../Common/BCOutsideInterfaceServer.h \
    ../Setting/BCSettingOutsideInterfaceDlg.h \
    ../Setting/BCSettingRepeatApplicationWarningDlg.h \
    ../Setting/BCSettingRoomVirtualResolutionDlg.h \
    ../Setting/BCSettingLEDResolutionDlg.h \
    ../Setting/BCSettingChannelRemoteIPDlg.h \
    ../Setting/BCSettingCameraWidgetDlg.h \
    ../View/BCLocationGroupWidget.h

FORMS    += \
    ../View/BCScreenName.ui \
    ../View/BCScreenDlg.ui \
    ../Setting/AlarmManagement.ui \
    ../Setting/BCAbout.ui \
    ../Setting/BCExProcessDialog.ui \
    ../Setting/BCSettingDisplyModelStyle.ui \
    ../Setting/BCSettingMainPanelStyle.ui \
    ../Setting/BCSettingPasswordStyle.ui \
    ../Setting/BCWarningLogWindow.ui \
    ../Setting/LogQueryDialog.ui \
    ../View/BCFaceWidget.ui \
    ../View/BCSignal.ui \
    ../View/BCVideoControl.ui \
    ../View/BCSystemplan.ui \
    ../View/BCScene.ui \
    ../View/BCMatrixInput.ui \
    ../View/BCMatrixCut.ui \
    ../View/BCMatrix.ui \
    ../View/BCCutScreen.ui \
    ../View/BCDecoder.ui \
    ../View/BCWidgetBtn.ui \
    ../View/BCControl.ui \
    ../View/BCSetSignalDlg.ui \
    ../Setting/BCPlanMapDlg.ui \
    ../View/BCMonitorWidgetData.ui \
    ../View/BCSignalListWidgetData.ui \
    ../View/BCAutoDateDlg.ui \
    ../View/BCSignalName.ui \
    ../Setting/BCLoginDlg.ui \
    ../Setting/BCOtherDeviceControlDlg.ui \
    ../Setting/BCSettingCommunicationDlg.ui \
    ../Setting/BCSettingDisplayInfoDlg.ui \
    ../Setting/BCSettingDisplaySetNetSetDlg.ui \
    ../View/BCLocationDlg.ui \
    ../Setting/BCSettingLanguageDlg.ui \
    ../Setting/BCSettingOutOfDateDlg.ui \
    ../Setting/BCSettingSignalWindowPropertyDlg.ui \
    ../Setting/BCSettingSubTitlesDlg.ui \
    ../Setting/BCSettingBatchSubTitlesDlg.ui \
    ../Setting/BCSettingIPVedioIPDlg.ui \
    ../Setting/BCSettingDeviceFormatDlg.ui \
    ../Setting/BCSettingWaitingDlg.ui \
    ../Setting/BCSettingBoardCardDlg.ui \
    ../Setting/BCSettingCutInputChannelDlg.ui \
    ../Setting/BCSettingPerviewDlg.ui \
    ../Setting/BCSettingOutSideCommandDlg.ui \
    ../Setting/BCSettingAddIPVedioTreeWidgetItemDlg.ui \
    ../Setting/BCSettingOtherDeviceControlDlg.ui \
    ../Setting/BCSettingDeviceCustomResolutionDlg.ui \
    ../Setting/BCSettingDisplaySwitchConfigDlg.ui \
    ../Setting/BCSettingDisplaySwitchRoomWidget.ui \
    ../View/BCMatrixRoomWidget.ui \
    ../View/BCMatrixInputNodeWidget.ui \
    ../View/BCMatrixOutputNodeWidget.ui \
    ../Setting/BCSettingTaskPlanningDlg.ui \
    ../Setting/BCSettingTaskDetailDlg.ui \
    ../View/BCSignalWindowDisplayWidget.ui \
    ../View/BCSingleDisplayWidget.ui \
    ../Setting/BCSettingModifyEDIDDialog.ui \
    ../Setting/BCSettingAutoReadInputChannelConfigDlg.ui \
    ../Setting/BCSettingDebugDlg.ui \
    ../Setting/BCSettingWindowCopyConfigDlg.ui \
    ../Setting/BCSettingSingleDisplaySwitchConfigDlg.ui \
    ../Setting/BCSettingIPVSignalWindowLoopConfigDlg.ui \
    ../Setting/BCSettingModifyNameDlg.ui \
    ../Setting/BCSettingModifyVP2000IPDlg.ui \
    ../Setting/BCSettingRemoveShadowDlg.ui \
    ../Setting/BCSettingMatrixFormatDlg.ui \
    ../Setting/BCSettingDisplayMakerConfigDlg.ui \
    ../Setting/BCSettingJointMatrixChannelConfigDlg.ui \
    ../Setting/BCSettingMatrixCutInputChannelDlg.ui \
    ../Setting/BCSettingOutsideInterfaceDlg.ui \
    ../Setting/BCSettingRepeatApplicationWarningDlg.ui \
    ../Setting/BCSettingRoomVirtualResolutionDlg.ui \
    ../Setting/BCSettingLEDResolutionDlg.ui \
    ../Setting/BCSettingChannelRemoteIPDlg.ui \
    ../Setting/BCSettingCameraWidgetDlg.ui

MOC_DIR = ../temp/moc
RCC_DIR = ../temp/rcc
UI_DIR = ../temp/ui
OBJECTS_DIR = ../temp/obj

DESTDIR = ../bin

TRANSLATIONS = ../resource/language/SIMPLIFIED_CHINESE.ts ../resource/language/TRADITIONAL_CHINESE.ts ../resource/language/ENGLISH.ts ../resource/language/KOREAN.ts \
                ../resource/language/RUSSIAN.ts ../resource/language/SPANISH.ts ../resource/language/FRENCH.ts

RESOURCES += \
    ../resource/image.qrc

#exe icon
win32:RC_FILE = icon.rc

#ribbon style
#C:\Program Files (x86)\Developer Machines\QtitanRibbon
include($$(QTITANDIR)/src/shared/qtitanribbon.pri)

#3531 decode video
LIBS += -L$$PWD/../Player/lib/ -lhi_h264dec_w \
        -L$$PWD/../Player/lib/ -lavcodec \
        -L$$PWD/../Player/lib/ -lavdevice \
        -L$$PWD/../Player/lib/ -lavfilter \
        -L$$PWD/../Player/lib/ -lavformat \
        -L$$PWD/../Player/lib/ -lavutil \
        -L$$PWD/../Player/lib/ -lpostproc \
        -L$$PWD/../Player/lib/ -lSDL2 \
        -L$$PWD/../Player/lib/ -lSDL2main \
        -L$$PWD/../Player/lib/ -lswresample \
        -L$$PWD/../Player/lib/ -lswscale

INCLUDEPATH += $$PWD/../Player/include

#海康硬盘录像机
win32: LIBS += -L$$PWD/../Camera/ -lHCNetSDK
INCLUDEPATH += $$PWD/../Camera
DEPENDPATH += $$PWD/../Camera
