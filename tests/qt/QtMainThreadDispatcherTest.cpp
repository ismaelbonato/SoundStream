#include "QtMainThreadDispatcher.h"

#include <QCoreApplication>
#include <QObject>
#include <QtTest/QtTest>

class QtMainThreadDispatcherTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void dispatchRunsTask()
    {
        QObject target;
        QtMainThreadDispatcher dispatcher(&target);

        bool called = false;
        dispatcher.dispatch([&called]() { called = true; });

        QTRY_VERIFY(called);
    }

    void dispatchWithNullTargetIsNoOp()
    {
        QtMainThreadDispatcher dispatcher(nullptr);

        bool called = false;
        dispatcher.dispatch([&called]() { called = true; });

        QTest::qWait(5);
        QVERIFY(!called);
    }
};

QTEST_MAIN(QtMainThreadDispatcherTest)
#include "QtMainThreadDispatcherTest.moc"
