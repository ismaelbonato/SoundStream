#include "NodeRemovalHelper.h"

namespace NodeRemovalHelper {

NodeRemovalCleanup analyzePortCleanup(const SceneItemIndex &itemIndex,
                                      quint32 nodeId,
                                      const PortItem *dragSourcePort)
{
    NodeRemovalCleanup cleanup;

    for (auto it = itemIndex.ports().constBegin();
         it != itemIndex.ports().constEnd();
         ++it) {
        const PortItem *port = it.value();
        if (port == nullptr) {
            cleanup.stalePortIds.append(it.key());
            continue;
        }

        if (port->nodeId() == nodeId) {
            if (dragSourcePort == port) {
                cleanup.clearsDragSource = true;
            }
            cleanup.stalePortIds.append(it.key());
        }
    }

    return cleanup;
}

} // namespace NodeRemovalHelper
