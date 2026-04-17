#pragma once
// core/GraphService.h
//
// Pure core service: forwards backend graph events on the main thread and
// keeps only the link index needed for command policy.

#include "core/GraphTypes.h"
#include "core/GraphBackend.h"
#include "core/IMainThreadDispatcher.h"

#include <cstdint>
#include <functional>
#include <unordered_map>

class GraphService
{
public:
    GraphService(IMainThreadDispatcher &dispatcher,
                 IGraphBackend &backend,
                 bool autoStart = true);
    ~GraphService();

    [[nodiscard]] auto connected() const -> bool { return isConnected; }
    void setEventHandler(GraphEventHandler handler);

    void createLink(uint32_t outputPortId, uint32_t inputPortId);
    void destroyLink(uint32_t linkId);

private:
    void wireBackendEvents();
    void postToMainThread(std::function<void()> task);
    void handleBackendEvent(GraphEvent event);
    void applyEvent(GraphEvent event);
    void emitEvent(GraphEvent event);

    IMainThreadDispatcher &dispatcher;
    IGraphBackend &backend;
    GraphEventHandler eventHandler;
    std::unordered_map<uint32_t, LinkData> knownLinks;
    bool isConnected = false;
};
