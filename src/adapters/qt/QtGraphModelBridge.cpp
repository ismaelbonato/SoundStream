#include "adapters/qt/QtGraphModelBridge.h"

#include <utility>
#include <variant>

QtGraphModelBridge::QtGraphModelBridge(GraphService &service)
    : service(service)
{
    wireEventHandler();
}

QtGraphModelBridge::~QtGraphModelBridge()
{
    service.setEventHandler({});
}

void QtGraphModelBridge::wireEventHandler()
{
    service.setEventHandler([this](GraphEvent event) {
        handleEvent(std::move(event));
    });
}

void QtGraphModelBridge::handleEvent(GraphEvent event)
{
    std::visit([this](const auto &specificEvent) { applyEvent(specificEvent); },
               event);
}

void QtGraphModelBridge::applyEvent(const NodeAdded &event)
{
    patchbayModel.notifyNodeAdded(event.node);
}

void QtGraphModelBridge::applyEvent(const NodeRemoved &event)
{
    patchbayModel.notifyNodeRemoved(event.id);
}

void QtGraphModelBridge::applyEvent(const PortAdded &event)
{
    patchbayModel.notifyPortAdded(event.port);
}

void QtGraphModelBridge::applyEvent(const PortRemoved &event)
{
    patchbayModel.notifyPortRemoved(event.id);
}

void QtGraphModelBridge::applyEvent(const LinkAdded &event)
{
    patchbayModel.notifyLinkAdded(event.link);
}

void QtGraphModelBridge::applyEvent(const LinkRemoved &event)
{
    patchbayModel.notifyLinkRemoved(event.id);
}

void QtGraphModelBridge::applyEvent(const ConnectionChanged &event)
{
    if (onConnectedChanged)
        onConnectedChanged(event.connected);
}

void QtGraphModelBridge::applyEvent(const ErrorOccurred &)
{
}
