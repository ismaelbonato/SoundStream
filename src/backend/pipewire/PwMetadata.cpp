#include "backend/pipewire/PwMetadata.h"

#include <pipewire/keys.h>
#include <spa/utils/dict.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

namespace {

auto dictValue(const spa_dict *props, const char *key) -> std::string
{
    if (props == nullptr) {
        return {};
    }

    const char *value = spa_dict_lookup(props, key);
    return (value != nullptr && *value != 0) ? std::string(value)
                                            : std::string{};
}

auto dictUInt(const spa_dict *props,
              const char *key,
              uint32_t fallback = 0) -> uint32_t
{
    if (props == nullptr) {
        return fallback;
    }

    const char *value = spa_dict_lookup(props, key);
    if (value == nullptr || *value == 0) {
        return fallback;
    }

    char *end = nullptr;
    const unsigned long result = std::strtoull(value, &end, 10);
    return (end != value) ? static_cast<uint32_t>(result) : fallback;
}

auto classifyMediaType(std::string value) -> std::string
{
    std::ranges::transform(value, value.begin(), [](unsigned char letter) {
        return static_cast<char>(std::tolower(letter));
    });

    if (value.find("video") != std::string::npos
        || value.find("image") != std::string::npos
        || value.find("v4l2") != std::string::npos
        || value.find("video4linux") != std::string::npos
        || value.find("camera") != std::string::npos) {
        return "video";
    }
    if (value.find("midi") != std::string::npos
        || value.find("control") != std::string::npos) {
        return "midi";
    }
    if (value.find("audio") != std::string::npos) {
        return "audio";
    }
    return {};
}

struct PortLabels
{
    std::string name;
    std::string alias;
};

auto detectPortMediaType(const spa_dict *props, const PortLabels &labels) -> std::string
{
    if (auto media = classifyMediaType(dictValue(props, "media.type"));
        !media.empty()) {
        return media;
    }

    if (auto media = classifyMediaType(dictValue(props, PW_KEY_FORMAT_DSP));
        !media.empty()) {
        return media;
    }

    if (auto media = classifyMediaType(dictValue(props, "format.dsp"));
        !media.empty()) {
        return media;
    }

    if (auto media = classifyMediaType(labels.alias); !media.empty()) {
        return media;
    }

    if (auto media = classifyMediaType(labels.name); !media.empty()) {
        return media;
    }

    return {};
}

} // namespace

namespace PwMetadata {

auto hasNodeIdentity(const spa_dict *props) -> bool
{
    return !dictValue(props, PW_KEY_NODE_NAME).empty()
           || !dictValue(props, PW_KEY_APP_NAME).empty();
}

auto nodeFromProperties(uint32_t objectId, const spa_dict *props) -> NodeData
{
    const std::string technicalName = dictValue(props, PW_KEY_NODE_NAME);
    const std::string appName = dictValue(props, PW_KEY_APP_NAME);

    NodeData node;
    node.id = objectId;
    node.name = dictValue(props, PW_KEY_NODE_DESCRIPTION);
    if (node.name.empty()) {
        node.name = dictValue(props, PW_KEY_NODE_NICK);
    }
    if (node.name.empty()) {
        node.name = appName;
    }
    if (node.name.empty()) {
        node.name = technicalName;
    }
    node.mediaClass = dictValue(props, PW_KEY_MEDIA_CLASS);
    node.iconName = dictValue(props, PW_KEY_APP_ICON_NAME);
    return node;
}

auto portFromProperties(uint32_t objectId, const spa_dict *props) -> PortData
{
    const std::string portName = dictValue(props, PW_KEY_PORT_NAME);
    const std::string portAlias = dictValue(props, PW_KEY_PORT_ALIAS);

    PortData port;
    port.id = objectId;
    port.nodeId = dictUInt(props, PW_KEY_NODE_ID);
    port.name = portAlias.empty() ? portName : portAlias;
    port.mediaType = detectPortMediaType(props, PortLabels{.name = portName, .alias = portAlias});
    const std::string direction = dictValue(props, PW_KEY_PORT_DIRECTION);
    port.direction = (direction == "out") ? std::string("out")
                                          : std::string("in");
    return port;
}

auto linkFromProperties(uint32_t objectId, const spa_dict *props) -> LinkData
{
    LinkData link;
    link.id = objectId;
    link.outputPortId = dictUInt(props, PW_KEY_LINK_OUTPUT_PORT);
    link.inputPortId = dictUInt(props, PW_KEY_LINK_INPUT_PORT);
    link.active = true;
    return link;
}

} // namespace PwMetadata
