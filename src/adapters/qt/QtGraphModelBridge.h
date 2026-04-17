#pragma once

#include "canvas/PatchbayModel.h"
#include "core/GraphService.h"

#include <functional>

class QtGraphModelBridge
{
public:
    explicit QtGraphModelBridge(GraphService &service);
    ~QtGraphModelBridge();

    auto model() -> PatchbayModel & { return patchbayModel; }

    std::function<void(bool)> onConnectedChanged;

private:
    void wireEventHandler();
    void handleEvent(GraphEvent event);
    void applyEvent(const NodeAdded &event);
    void applyEvent(const NodeRemoved &event);
    void applyEvent(const PortAdded &event);
    void applyEvent(const PortRemoved &event);
    void applyEvent(const LinkAdded &event);
    void applyEvent(const LinkRemoved &event);
    void applyEvent(const ConnectionChanged &event);
    void applyEvent(const ErrorOccurred &event);

    GraphService &service;
    PatchbayModel patchbayModel;
};
