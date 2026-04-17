#pragma once

#include "../../core/IMainThreadDispatcher.h"

class QObject;

#include <QPointer>

class QtMainThreadDispatcher : public IMainThreadDispatcher
{
public:
    explicit QtMainThreadDispatcher(QObject *target);

    void dispatch(std::function<void()> task) override;

private:
    QPointer<QObject> target;
};
