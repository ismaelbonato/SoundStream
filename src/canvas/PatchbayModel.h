#pragma once
// canvas/PatchbayModel.h
//
// QObject-based model. Owns the canvas graph snapshot and
// notifies observers via Qt signals.
//
// Has NO knowledge of PipeWire commands — it only stores data and emits
// change notifications. GraphService forwards graph events here after
// they reach the main thread.

#include "core/GraphTypes.h"
#include <QObject>
#include <unordered_map>

class PatchbayModel : public QObject
{
    Q_OBJECT

public:
    explicit PatchbayModel(QObject *parent = nullptr)
        : QObject(parent)
    {}
    ~PatchbayModel() override = default;

    std::unordered_map<uint32_t, NodeData> nodes;
    std::unordered_map<uint32_t, PortData> ports;
    std::unordered_map<uint32_t, LinkData> links;

Q_SIGNALS:
    void nodeAdded(const NodeData &node);
    void nodeRemoved(quint32 nodeId);
    void portAdded(const PortData &port);
    void portRemoved(quint32 portId);
    void linkAdded(const LinkData &link);
    void linkRemoved(quint32 linkId);

public:
    // Each helper writes into the snapshot first, then emits so the view
    // always sees matching stored data.

    void notifyNodeAdded(const NodeData &nodeData)
    {
        nodes.insert_or_assign(nodeData.id, nodeData);
        Q_EMIT this->nodeAdded(nodeData);
    }
    void notifyNodeRemoved(quint32 nodeId)
    {
        nodes.erase(nodeId);
        Q_EMIT this->nodeRemoved(nodeId);
    }

    void notifyPortAdded(const PortData &portData)
    {
        ports.insert_or_assign(portData.id, portData);
        Q_EMIT this->portAdded(portData);
    }
    void notifyPortRemoved(quint32 portId)
    {
        ports.erase(portId);
        Q_EMIT this->portRemoved(portId);
    }

    void notifyLinkAdded(const LinkData &linkData)
    {
        links.insert_or_assign(linkData.id, linkData);
        Q_EMIT this->linkAdded(linkData);
    }
    void notifyLinkRemoved(quint32 linkId)
    {
        links.erase(linkId);
        Q_EMIT this->linkRemoved(linkId);
    }
};
