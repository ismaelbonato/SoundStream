#pragma once

#include "adapters/qt/QtGraphModelBridge.h"
#include "adapters/qt/QtMainThreadDispatcher.h"
#include "backend/pipewire/PwClient.h"
#include "core/GraphService.h"


class QApplication;
class QQmlApplicationEngine;

class SoundStream
{
public:
    SoundStream(QApplication &app, QQmlApplicationEngine &engine);

private:
    void wireRootObjects();

    QApplication &app;
    QQmlApplicationEngine &engine;

    QtMainThreadDispatcher dispatcher;
    PwClient backend;
    GraphService service;
    QtGraphModelBridge graphBridge;
};
