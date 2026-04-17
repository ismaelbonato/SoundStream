#pragma once

#include "LinkItem.h"
#include "NodeItem.h"
#include "PortItem.h"

#include <QHash>
#include <QPointer>

#include <functional>
#include <optional>

class SceneItemIndex
{
public:
    [[nodiscard]] auto hasNode(quint32 id) const -> bool
    {
        return nodeItems.contains(id);
    }
    [[nodiscard]] auto hasLink(quint32 id) const -> bool
    {
        return linkItems.contains(id);
    }

    [[nodiscard]] auto node(quint32 id) const
        -> std::optional<std::reference_wrapper<NodeItem>>
    {
        if (auto *item = nodeItems.value(id, nullptr).data()) {
            return std::ref(*item);
        }
        return std::nullopt;
    }

    [[nodiscard]] auto port(quint32 id) const
        -> std::optional<std::reference_wrapper<PortItem>>
    {
        if (auto *item = portItems.value(id, nullptr).data()) {
            return std::ref(*item);
        }
        return std::nullopt;
    }

    void insertNode(quint32 id, NodeItem *item) { nodeItems.insert(id, item); }
    void insertPort(quint32 id, PortItem *item) { portItems.insert(id, item); }
    void insertLink(quint32 id, LinkItem *item) { linkItems.insert(id, item); }

    auto takeNode(quint32 id) -> QPointer<NodeItem>
    {
        return nodeItems.take(id);
    }
    auto takeLink(quint32 id) -> QPointer<LinkItem>
    {
        return linkItems.take(id);
    }

    void removePort(quint32 id) { portItems.remove(id); }
    void removeLink(quint32 id) { linkItems.remove(id); }

    [[nodiscard]] auto nodes() const
        -> const QHash<quint32, QPointer<NodeItem>> &
    {
        return nodeItems;
    }
    [[nodiscard]] auto ports() const
        -> const QHash<quint32, QPointer<PortItem>> &
    {
        return portItems;
    }
    [[nodiscard]] auto links() const
        -> const QHash<quint32, QPointer<LinkItem>> &
    {
        return linkItems;
    }

private:
    QHash<quint32, QPointer<NodeItem>> nodeItems;
    QHash<quint32, QPointer<PortItem>> portItems;
    QHash<quint32, QPointer<LinkItem>> linkItems;
};
