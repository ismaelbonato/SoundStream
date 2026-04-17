#pragma once

#include "SceneItemIndex.h"

#include <QList>

struct NodeRemovalCleanup
{
    QList<quint32> stalePortIds;
    bool clearsDragSource = false;
};

namespace NodeRemovalHelper
{
NodeRemovalCleanup analyzePortCleanup(const SceneItemIndex &itemIndex,
                                      quint32 nodeId,
                                      const PortItem *dragSourcePort);
} // namespace NodeRemovalHelper
