#include "SoundStream.h"

#include "canvas/PatchbayView.h"

#include <QApplication>
#include <QDebug>
#include <QPointer>
#include <QQmlApplicationEngine>

SoundStream::SoundStream(QApplication &app, QQmlApplicationEngine &engine)
    : app(app)
    , engine(engine)
    , dispatcher(&app)
    , service(dispatcher, backend)
    , graphBridge(service)
{
    wireRootObjects();
}

void SoundStream::wireRootObjects()
{
    const auto roots = engine.rootObjects();
    if (roots.isEmpty()) {
        return;
    }

    auto *patchbay = roots.first()->findChild<PatchbayView *>(
        QStringLiteral("patchbayView"));
    if (patchbay == nullptr) {
        qWarning() << "SoundStream: PatchbayView \"patchbayView\" not found - "
                      "canvas will be empty";
        return;
    }

    patchbay->setPatchbayModel(&graphBridge.model());

    QPointer<PatchbayView> patchbayGuard(patchbay);
    graphBridge.onConnectedChanged = [patchbayGuard](bool connected) {
        if (patchbayGuard) {
            patchbayGuard->setConnected(connected);
        }
    };

    QObject::connect(patchbay,
                     &PatchbayView::linkCreateRequested,
                     &app,
                     [this](quint32 outputPortId, quint32 inputPortId) {
                         service.createLink(outputPortId, inputPortId);
                     });

    QObject::connect(patchbay,
                     &PatchbayView::linkDestroyRequested,
                     &app,
                     [this](quint32 linkId) { service.destroyLink(linkId); });
}
