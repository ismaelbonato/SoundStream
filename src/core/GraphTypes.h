#pragma once
// core/GraphTypes.h
//
// Plain graph data shared by core, PipeWire, and Qt/canvas code.
//
#include <cstdint>
#include <string>

struct NodeData
{
    uint32_t id = 0;
    std::string name;       // human-readable node name
    std::string mediaClass; // e.g. "Audio/Sink", "Audio/Source"
    std::string iconName;   // freedesktop icon name
};

struct PortData
{
    uint32_t id = 0;
    uint32_t nodeId = 0;
    std::string name;      // port label
    std::string direction; // "out" | "in"
    std::string mediaType; // "audio" | "midi" | "video"
};

struct LinkData
{
    uint32_t id = 0;
    uint32_t outputPortId = 0;
    uint32_t inputPortId = 0;
    bool active = false;
};

