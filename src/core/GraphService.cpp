#include "core/GraphService.h"

#include <cstdio>
#include <type_traits>
#include <utility>
#include <variant>

GraphService::GraphService(IMainThreadDispatcher &dispatcher,
                           IGraphBackend &backend,
                           bool autoStart)
    : dispatcher(dispatcher)
    , backend(backend)
{
    wireBackendEvents();
    if (autoStart) {
        this->backend.start();
    }
}

GraphService::~GraphService()
{
    backend.stop();
}

void GraphService::setEventHandler(GraphEventHandler handler)
{
    eventHandler = std::move(handler);
}

void GraphService::wireBackendEvents()
{
    backend.setEventHandler([this](GraphEvent event) {
        handleBackendEvent(std::move(event));
    });
}

void GraphService::postToMainThread(std::function<void()> task)
{
    dispatcher.dispatch(std::move(task));
}

void GraphService::handleBackendEvent(GraphEvent event)
{
    postToMainThread([this, event = std::move(event)]() mutable {
        applyEvent(std::move(event));
    });
}

void GraphService::applyEvent(GraphEvent event)
{
    std::visit(
        [this](const auto &specificEvent) {
            using Event = std::decay_t<decltype(specificEvent)>;

            if constexpr (std::is_same_v<Event, LinkAdded>) {
                knownLinks.insert_or_assign(specificEvent.link.id,
                                            specificEvent.link);
            } else if constexpr (std::is_same_v<Event, LinkRemoved>) {
                knownLinks.erase(specificEvent.id);
            } else if constexpr (std::is_same_v<Event, ConnectionChanged>) {
                isConnected = specificEvent.connected;
            }
        },
        event);
    emitEvent(std::move(event));
}

void GraphService::emitEvent(GraphEvent event)
{
    if (eventHandler) {
        eventHandler(std::move(event));
        return;
    }

    if (const auto *error = std::get_if<ErrorOccurred>(&event)) {
        std::fprintf(stderr,
                     "GraphService backend error: %s\n",
                     error->message.c_str());
    }
}

void GraphService::createLink(uint32_t outputPortId, uint32_t inputPortId)
{
    for (const auto &entry : knownLinks) {
        const LinkData &link = entry.second;
        if (link.outputPortId == outputPortId
            && link.inputPortId == inputPortId) {
            return;
        }
    }

    backend.createLink(outputPortId, inputPortId);
}

void GraphService::destroyLink(uint32_t linkId)
{
    backend.destroyLink(linkId);
}
