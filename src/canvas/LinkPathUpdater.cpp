#include "LinkPathUpdater.h"

namespace LinkPathUpdater
{

void updateLinksForPort(quint32 portId, const SceneItemIndex &itemIndex)
{
    auto movedPortOpt = itemIndex.port(portId);
    if (!movedPortOpt) return;

    const PortItem &movedRef = movedPortOpt->get();
    const QPointF movedPos = movedRef.sceneCenter();

    for (const auto &linkObs : itemIndex.links()) {
        LinkItem *link = linkObs.data();
        if (!link) continue;

        if (link->outputPortId() == portId) {
            auto dstOpt = itemIndex.port(link->inputPortId());
            const QPointF dstPos = dstOpt ? dstOpt->get().sceneCenter() : movedPos;
            link->setEndpoints(movedPos, dstPos);
        } else if (link->inputPortId() == portId) {
            auto srcOpt = itemIndex.port(link->outputPortId());
            const QPointF srcPos = srcOpt ? srcOpt->get().sceneCenter() : movedPos;
            link->setEndpoints(srcPos, movedPos);
        }
    }
}

} // namespace LinkPathUpdater
