#pragma once
// src/backend/pipewire/PwClient.h
//
// Low-level PipeWire graph backend. Owns the pw_thread_loop / pw_context /
// pw_core / pw_registry lifecycle and emits plain GraphEvent values.
//
// Events are emitted on the PipeWire thread; GraphService is responsible for
// hopping them to the Qt/main thread before UI state changes.

#include "core/GraphBackend.h"

#include <spa/utils/hook.h>

#include <cstdint>
#include <memory>
#include <utility>

struct pw_thread_loop;
struct pw_context;
struct pw_core;
struct pw_registry;
struct spa_dict;

class PwClient final : public IGraphBackend
{
public:
    PwClient();
    ~PwClient() override;

    PwClient(const PwClient &) = delete;
    auto operator=(const PwClient &) -> PwClient & = delete;
    PwClient(PwClient &&) = delete;
    auto operator=(PwClient &&) -> PwClient & = delete;

    void setEventHandler(GraphEventHandler handler) override
    {
        eventHandler = std::move(handler);
    }

    [[nodiscard]] auto currentEventHandler() const -> const GraphEventHandler &
    {
        return eventHandler;
    }

    auto start() -> bool override;
    void stop() override;

    void createLink(uint32_t outputPortId, uint32_t inputPortId) override;
    void destroyLink(uint32_t linkId) override;

private:
    void stopImpl();

    static void onRegistryGlobal(void *data,
                                 uint32_t id,
                                 uint32_t permissions,
                                 const char *type,
                                 uint32_t version,
                                 const spa_dict *props);
    static void onRegistryGlobalRemove(void *data, uint32_t id);
    static void onCoreError(
        void *data, uint32_t id, int seq, int res, const char *message);

    static void destroyThreadLoop(pw_thread_loop *pwThread) noexcept;
    static void destroyContext(pw_context *pwContext) noexcept;
    static void destroyCore(pw_core *pwCore) noexcept;
    static void destroyRegistry(pw_registry *pwRegistry) noexcept;

    template<typename T, auto DestroyFn>
    struct FnDeleter
    {
        void operator()(T *ptr) const noexcept
        {
            if (ptr != nullptr) {
                DestroyFn(ptr);
            }
        }
    };

    using LoopPtr = std::unique_ptr<
        pw_thread_loop,
        FnDeleter<pw_thread_loop, &PwClient::destroyThreadLoop>>;
    using ContextPtr
        = std::unique_ptr<pw_context,
                          FnDeleter<pw_context, &PwClient::destroyContext>>;
    using CorePtr
        = std::unique_ptr<pw_core, FnDeleter<pw_core, &PwClient::destroyCore>>;
    using RegistryPtr
        = std::unique_ptr<pw_registry,
                          FnDeleter<pw_registry, &PwClient::destroyRegistry>>;

    LoopPtr loop;
    ContextPtr context;
    CorePtr core;
    RegistryPtr registry;

    spa_hook registryHook{};
    spa_hook coreHook{};

    bool connected = false;
    GraphEventHandler eventHandler;
};
