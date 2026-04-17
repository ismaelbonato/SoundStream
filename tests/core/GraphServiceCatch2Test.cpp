#include "core/GraphService.h"

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace
{
class FakeDispatcher final : public IMainThreadDispatcher
{
public:
    enum class Mode {
        Immediate,
        Queued,
    };

    explicit FakeDispatcher(Mode mode)
        : mode(mode)
    {}

    void dispatch(std::function<void()> task) override
    {
        ++dispatchCount;
        if (mode == Mode::Immediate) {
            task();
            return;
        }

        queuedTasks.push_back(std::move(task));
    }

    void runQueued()
    {
        for (auto &task : queuedTasks) {
            task();
        }
        queuedTasks.clear();
    }

    int dispatchCount = 0;

private:
    Mode mode;
    std::vector<std::function<void()>> queuedTasks;
};

class FakeGraphBackend final : public IGraphBackend
{
public:
    void setEventHandler(GraphEventHandler handler) override
    {
        eventHandler = std::move(handler);
    }

    auto start() -> bool override
    {
        ++startCalls;
        return true;
    }

    void stop() override
    {
        ++stopCalls;
    }

    void createLink(uint32_t outputPortId, uint32_t inputPortId) override
    {
        createLinkCalls.push_back({outputPortId, inputPortId});
    }

    void destroyLink(uint32_t linkId) override
    {
        destroyLinkCalls.push_back(linkId);
    }

    GraphEventHandler eventHandler;
    int startCalls = 0;
    int stopCalls = 0;
    std::vector<std::pair<uint32_t, uint32_t>> createLinkCalls;
    std::vector<uint32_t> destroyLinkCalls;
};
} // namespace

TEST_CASE("GraphService forwards link commands to backend", "[core][graph-service]")
{
    FakeDispatcher dispatcher(FakeDispatcher::Mode::Immediate);
    FakeGraphBackend backend;

    {
        GraphService service(dispatcher, backend, false);
        service.createLink(10, 11);
        service.destroyLink(99);

        REQUIRE(backend.startCalls == 0);
        REQUIRE(backend.createLinkCalls.size() == 1);
        REQUIRE(backend.createLinkCalls.front().first == 10u);
        REQUIRE(backend.createLinkCalls.front().second == 11u);
        REQUIRE(backend.destroyLinkCalls.size() == 1);
        REQUIRE(backend.destroyLinkCalls.front() == 99u);
    }

    REQUIRE(backend.stopCalls == 1);
}

TEST_CASE("GraphService forwards events on dispatcher", "[core][graph-service]")
{
    FakeDispatcher dispatcher(FakeDispatcher::Mode::Queued);
    FakeGraphBackend backend;
    GraphService service(dispatcher, backend, false);

    std::vector<uint32_t> removedNodeIds;
    std::vector<uint32_t> removedPortIds;
    std::vector<uint32_t> removedLinkIds;
    std::optional<NodeData> observedNode;
    std::optional<PortData> observedPort;
    std::optional<LinkData> observedLink;

    service.setEventHandler([&](GraphEvent event) {
        std::visit(
            [&](const auto &specificEvent) {
                using Event = std::decay_t<decltype(specificEvent)>;

                if constexpr (std::is_same_v<Event, NodeAdded>) {
                    observedNode = specificEvent.node;
                } else if constexpr (std::is_same_v<Event, NodeRemoved>) {
                    removedNodeIds.push_back(specificEvent.id);
                } else if constexpr (std::is_same_v<Event, PortAdded>) {
                    observedPort = specificEvent.port;
                } else if constexpr (std::is_same_v<Event, PortRemoved>) {
                    removedPortIds.push_back(specificEvent.id);
                } else if constexpr (std::is_same_v<Event, LinkAdded>) {
                    observedLink = specificEvent.link;
                } else if constexpr (std::is_same_v<Event, LinkRemoved>) {
                    removedLinkIds.push_back(specificEvent.id);
                }
            },
            event);
    });

    REQUIRE(backend.eventHandler);

    backend.eventHandler(NodeAdded{NodeData{.id = 7,
                                            .name = "Speakers",
                                            .mediaClass = "Audio/Sink",
                                            .iconName = "audio-card"}});
    backend.eventHandler(PortAdded{PortData{.id = 8,
                                            .nodeId = 7,
                                            .name = "Front Left",
                                            .direction = "out",
                                            .mediaType = "audio"}});
    backend.eventHandler(LinkAdded{LinkData{.id = 9,
                                            .outputPortId = 8,
                                            .inputPortId = 10,
                                            .active = true}});

    REQUIRE(dispatcher.dispatchCount == 3);
    dispatcher.runQueued();
    REQUIRE(observedNode.has_value());
    REQUIRE(observedPort.has_value());
    REQUIRE(observedLink.has_value());

    backend.eventHandler(LinkRemoved{9});
    backend.eventHandler(PortRemoved{8});
    backend.eventHandler(NodeRemoved{7});
    dispatcher.runQueued();
    REQUIRE(removedLinkIds == std::vector<uint32_t>{9});
    REQUIRE(removedPortIds == std::vector<uint32_t>{8});
    REQUIRE(removedNodeIds == std::vector<uint32_t>{7});
}

TEST_CASE("GraphService tracks connection state and forwards errors", "[core][graph-service]")
{
    FakeDispatcher dispatcher(FakeDispatcher::Mode::Queued);
    FakeGraphBackend backend;
    GraphService service(dispatcher, backend, false);

    std::optional<bool> observedConnection;
    std::optional<std::string> observedError;
    service.setEventHandler([&](GraphEvent event) {
        std::visit(
            [&](const auto &specificEvent) {
                using Event = std::decay_t<decltype(specificEvent)>;

                if constexpr (std::is_same_v<Event, ConnectionChanged>) {
                    observedConnection = specificEvent.connected;
                } else if constexpr (std::is_same_v<Event, ErrorOccurred>) {
                    observedError = specificEvent.message;
                }
            },
            event);
    });

    backend.eventHandler(ConnectionChanged{true});
    backend.eventHandler(ErrorOccurred{"boom"});

    REQUIRE_FALSE(service.connected());
    dispatcher.runQueued();
    REQUIRE(service.connected());
    REQUIRE(observedConnection == true);
    REQUIRE(observedError == std::optional<std::string>{"boom"});

    backend.eventHandler(ConnectionChanged{false});
    dispatcher.runQueued();
    REQUIRE_FALSE(service.connected());
    REQUIRE(observedConnection == false);
}

TEST_CASE("GraphService ignores duplicate link create requests", "[core][graph-service]")
{
    FakeDispatcher dispatcher(FakeDispatcher::Mode::Immediate);
    FakeGraphBackend backend;
    GraphService service(dispatcher, backend, false);

    backend.eventHandler(LinkAdded{LinkData{.id = 42,
                                            .outputPortId = 132,
                                            .inputPortId = 59,
                                            .active = true}});

    service.createLink(132, 59);

    REQUIRE(backend.createLinkCalls.empty());
}
