#pragma once

#include "core/GraphTypes.h"

#include <cstdint>

struct spa_dict;

namespace PwMetadata {

[[nodiscard]] auto hasNodeIdentity(const spa_dict *props) -> bool;
[[nodiscard]] auto nodeFromProperties(uint32_t objectId,
                                      const spa_dict *props) -> NodeData;
[[nodiscard]] auto portFromProperties(uint32_t objectId,
                                      const spa_dict *props) -> PortData;
[[nodiscard]] auto linkFromProperties(uint32_t objectId,
                                      const spa_dict *props) -> LinkData;

} // namespace PwMetadata
