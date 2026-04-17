#pragma once

#include "SceneItemIndex.h"

class LinkItem;

namespace LinkInitializer
{
void initializeEndpoints(LinkItem &linkItem, const SceneItemIndex &itemIndex);
} // namespace LinkInitializer
