#include "backend/pipewire/PwMetadata.h"

#include <catch2/catch_test_macros.hpp>
#include <pipewire/keys.h>
#include <spa/utils/dict.h>

#include <array>

namespace {

template<size_t Size>
auto makeDict(const std::array<spa_dict_item, Size> &items) -> spa_dict
{
    return spa_dict{.flags = 0,
                    .n_items = static_cast<uint32_t>(items.size()),
                    .items = items.data()};
}

} // namespace

TEST_CASE("PwMetadata maps node display name priority", "[backend][pipewire]")
{
    const std::array items{spa_dict_item{.key = PW_KEY_NODE_NAME, .value = "technical"},
                           spa_dict_item{.key = PW_KEY_NODE_NICK, .value = "Nick"},
                           spa_dict_item{.key = PW_KEY_NODE_DESCRIPTION, .value = "Friendly"},
                           spa_dict_item{.key = PW_KEY_APP_NAME, .value = "App"},
                           spa_dict_item{.key = PW_KEY_MEDIA_CLASS, .value = "Audio/Sink"},
                           spa_dict_item{.key = PW_KEY_APP_ICON_NAME, .value = "audio-card"}};
    const spa_dict props = makeDict(items);

    REQUIRE(PwMetadata::hasNodeIdentity(&props));

    const NodeData node = PwMetadata::nodeFromProperties(7, &props);
    REQUIRE(node.id == 7U);
    REQUIRE(node.name == "Friendly");
    REQUIRE(node.mediaClass == "Audio/Sink");
    REQUIRE(node.iconName == "audio-card");
}

TEST_CASE("PwMetadata maps node name fallbacks", "[backend][pipewire]")
{
    {
        const std::array items{spa_dict_item{.key = PW_KEY_NODE_NAME, .value = "technical"},
                               spa_dict_item{.key = PW_KEY_NODE_NICK, .value = "Nick"}};
        const spa_dict props = makeDict(items);
        REQUIRE(PwMetadata::nodeFromProperties(1, &props).name == "Nick");
    }

    {
        const std::array items{spa_dict_item{.key = PW_KEY_NODE_NAME, .value = "technical"},
                               spa_dict_item{.key = PW_KEY_APP_NAME, .value = "App"}};
        const spa_dict props = makeDict(items);
        REQUIRE(PwMetadata::nodeFromProperties(1, &props).name == "App");
    }

    {
        const std::array items{spa_dict_item{.key = PW_KEY_NODE_NAME, .value = "technical"}};
        const spa_dict props = makeDict(items);
        REQUIRE(PwMetadata::nodeFromProperties(1, &props).name == "technical");
    }

    REQUIRE_FALSE(PwMetadata::hasNodeIdentity(nullptr));
}

TEST_CASE("PwMetadata maps ports from PipeWire properties", "[backend][pipewire]")
{
    const std::array items{spa_dict_item{.key = PW_KEY_NODE_ID, .value = "42"},
                           spa_dict_item{.key = PW_KEY_PORT_NAME, .value = "capture_FL"},
                           spa_dict_item{.key = PW_KEY_PORT_ALIAS, .value = "Front Left Audio"},
                           spa_dict_item{.key = PW_KEY_PORT_DIRECTION, .value = "out"}};
    const spa_dict props = makeDict(items);

    const PortData port = PwMetadata::portFromProperties(99, &props);
    REQUIRE(port.id == 99U);
    REQUIRE(port.nodeId == 42U);
    REQUIRE(port.name == "Front Left Audio");
    REQUIRE(port.direction == "out");
    REQUIRE(port.mediaType == "audio");
}

TEST_CASE("PwMetadata maps links from PipeWire properties", "[backend][pipewire]")
{
    const std::array items{spa_dict_item{.key = PW_KEY_LINK_OUTPUT_PORT, .value = "11"},
                           spa_dict_item{.key = PW_KEY_LINK_INPUT_PORT, .value = "12"}};
    const spa_dict props = makeDict(items);

    const LinkData link = PwMetadata::linkFromProperties(55, &props);
    REQUIRE(link.id == 55U);
    REQUIRE(link.outputPortId == 11U);
    REQUIRE(link.inputPortId == 12U);
    REQUIRE(link.active);
}
