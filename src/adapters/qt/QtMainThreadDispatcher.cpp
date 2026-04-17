#include "adapters/qt/QtMainThreadDispatcher.h"

#include <QMetaObject>
#include <QObject>

QtMainThreadDispatcher::QtMainThreadDispatcher(QObject *target)
    : target(target)
{}

void QtMainThreadDispatcher::dispatch(std::function<void()> task)
{
    if (!target)
        return;

    QMetaObject::invokeMethod(
        target.data(),
        [task = std::move(task)]() mutable { task(); },
        Qt::QueuedConnection);
}
