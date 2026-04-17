#pragma once

#include "core/GraphTypes.h"

#include <cstdint>
#include <functional>
#include <string>
#include <variant>

struct NodeAdded
{
    NodeData node;
};

struct NodeRemoved
{
    uint32_t id = 0;
};

struct PortAdded
{
    PortData port;
};

struct PortRemoved
{
    uint32_t id = 0;
};

struct LinkAdded
{
    LinkData link;
};

struct LinkRemoved
{
    uint32_t id = 0;
};

struct ConnectionChanged
{
    bool connected = false;
};

struct ErrorOccurred
{
    std::string message;
};

using GraphEvent = std::variant<NodeAdded,
                                NodeRemoved,
                                PortAdded,
                                PortRemoved,
                                LinkAdded,
                                LinkRemoved,
                                ConnectionChanged,
                                ErrorOccurred>;

using GraphEventHandler = std::function<void(GraphEvent)>;

class IGraphBackend
{
public:
    virtual ~IGraphBackend() = default;

    virtual void setEventHandler(GraphEventHandler handler) = 0;

    virtual auto start() -> bool = 0;
    virtual void stop() = 0;

    virtual void createLink(uint32_t outputPortId, uint32_t inputPortId) = 0;
    virtual void destroyLink(uint32_t linkId) = 0;
};
