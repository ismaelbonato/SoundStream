#include "LinkInitializer.h"

namespace LinkInitializer
{

void initializeEndpoints(LinkItem &linkItem, const SceneItemIndex &itemIndex)
{
    auto srcPortOpt = itemIndex.port(linkItem.outputPortId());
    auto dstPortOpt = itemIndex.port(linkItem.inputPortId());

    if (srcPortOpt && dstPortOpt) {
        const PortItem &srcRef = srcPortOpt->get();
        const PortItem &dstRef = dstPortOpt->get();
        linkItem.setMediaType(srcRef.mediaType());
        linkItem.setEndpoints(srcRef.sceneCenter(), dstRef.sceneCenter());
    }
}

} // namespace LinkInitializer
