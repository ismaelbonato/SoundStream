// src/backend/pipewire/PwClient.cpp
//
// Pure C++ PipeWire client.
// This is the ONLY file in the project that includes PipeWire headers.
// No Qt headers are included here.

#include "PwClient.h"

#include "backend/pipewire/PwMetadata.h"

#include <cstdint>
#include <pipewire/pipewire.h>
#include <spa/utils/hook.h>

#include <cstring>
#include <string>
#include <utility>

namespace {

void reportErrorNoThrow(const PwClient *self, std::string message) noexcept
{
    if (self == nullptr) {
        return;
    }

    const auto &handler = self->currentEventHandler();
    if (!handler) {
        return;
    }

#if defined(__cpp_exceptions)
    try {
        handler(ErrorOccurred{std::move(message)});
    } catch (...) {
        // Never allow exceptions to cross PipeWire/C callback boundaries.
    }
#else
    handler(ErrorOccurred{std::move(message)});
#endif
}

void emitEventNoThrow(const PwClient *self,
                      const char *eventName,
                      GraphEvent event) noexcept
{
    if (self == nullptr) {
        return;
    }

    const auto &handler = self->currentEventHandler();
    if (!handler) {
        return;
    }

#if defined(__cpp_exceptions)
    try {
        handler(std::move(event));
    } catch (...) {
        reportErrorNoThrow(self,
                           std::string(eventName)
                               + " handler threw unknown exception");
    }
#else
    (void) eventName;
    handler(std::move(event));
#endif
}

} // namespace

// ── Custom deleters ───────────────────────────────────────────────────────

void PwClient::destroyThreadLoop(pw_thread_loop *pwThread) noexcept
{
    pw_thread_loop_destroy(pwThread);
}

void PwClient::destroyContext(pw_context *pwContext) noexcept
{
    pw_context_destroy(pwContext);
}

void PwClient::destroyCore(pw_core *pwCore) noexcept
{
    pw_core_disconnect(pwCore);
}

void PwClient::destroyRegistry(pw_registry *pwRegistry) noexcept
{
    pw_proxy_destroy(reinterpret_cast<pw_proxy *>(pwRegistry));
}

// ── Construction / destruction ────────────────────────────────────────────

PwClient::PwClient()
{
    pw_init(nullptr, nullptr);
}

PwClient::~PwClient()
{
    stopImpl();
    pw_deinit();
}

// ── Lifecycle ─────────────────────────────────────────────────────────────

auto PwClient::start() -> bool
{
    loop.reset(pw_thread_loop_new("soundstream-pw", nullptr));
    if (!loop) {
        reportErrorNoThrow(this, "failed to create pw_thread_loop");
        return false;
    }

    context.reset(
        pw_context_new(pw_thread_loop_get_loop(loop.get()), nullptr, 0));
    if (!context) {
        reportErrorNoThrow(this, "failed to create pw_context");
        loop.reset();
        return false;
    }

    pw_thread_loop_start(loop.get());
    pw_thread_loop_lock(loop.get());

    core.reset(pw_context_connect(context.get(), nullptr, 0));
    if (!core) {
        reportErrorNoThrow(this, "failed to connect to PipeWire daemon");
        pw_thread_loop_unlock(loop.get());
        return false;
    }

    static const pw_core_events s_coreEvents = []() {
        pw_core_events evt{};
        evt.version = PW_VERSION_CORE_EVENTS;
        evt.error = onCoreError;
        return evt;
    }();

    static const pw_registry_events s_registryEvents = []() {
        pw_registry_events evt{};
        evt.version = PW_VERSION_REGISTRY_EVENTS;
        evt.global = onRegistryGlobal;
        evt.global_remove = onRegistryGlobalRemove;
        return evt;
    }();

    pw_core_add_listener(core.get(), &coreHook, &s_coreEvents, this);

    registry.reset(pw_core_get_registry(core.get(), PW_VERSION_REGISTRY, 0));
    pw_registry_add_listener(registry.get(),
                             &registryHook,
                             &s_registryEvents,
                             this);

    pw_thread_loop_unlock(loop.get());

    connected = true;
    emitEventNoThrow(this, "ConnectionChanged", ConnectionChanged{true});

    return true;
}

void PwClient::stop()
{
    stopImpl();
}

void PwClient::stopImpl()
{
    if (!loop) {
        return;
    }

    pw_thread_loop_stop(loop.get());

    // Reset in reverse-construction order
    registry.reset();
    core.reset();
    context.reset();
    loop.reset();

    if (connected) {
        connected = false;
        emitEventNoThrow(this,
                         "ConnectionChanged",
                         ConnectionChanged{false});
    }
}

// ── Commands ──────────────────────────────────────────────────────────────

void PwClient::createLink(uint32_t outputPortId, uint32_t inputPortId)
{
    if (!core) {
        return;
    }

    pw_thread_loop_lock(loop.get());

    auto *props = pw_properties_new(nullptr, nullptr);
    pw_properties_setf(props, PW_KEY_LINK_OUTPUT_PORT, "%u", outputPortId);
    pw_properties_setf(props, PW_KEY_LINK_INPUT_PORT, "%u", inputPortId);
    pw_properties_set(props, PW_KEY_OBJECT_LINGER, "true");

    pw_core_create_object(core.get(),
                          "link-factory",
                          PW_TYPE_INTERFACE_Link,
                          PW_VERSION_LINK,
                          &props->dict,
                          0);

    pw_properties_free(props);
    pw_thread_loop_unlock(loop.get());
}

void PwClient::destroyLink(uint32_t linkId)
{
    if (!registry) {
        return;
    }

    pw_thread_loop_lock(loop.get());
    const int res = pw_registry_destroy(registry.get(), linkId);
    if (res < 0) {
        reportErrorNoThrow(this,
                           "failed to destroy link " + std::to_string(linkId)
                               + " (res=" + std::to_string(res) + ")");
    }
    pw_thread_loop_unlock(loop.get());
}

// ── Registry: global added ────────────────────────────────────────────────

void PwClient::onRegistryGlobal(void *data,
                                uint32_t id,
                                uint32_t /*permissions*/,
                                const char *type,
                                uint32_t /*version*/,
                                const spa_dict *props)
{
    const auto *self = static_cast<PwClient *>(data);

    if (std::strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
        if (!PwMetadata::hasNodeIdentity(props)) {
            return;
        }

        emitEventNoThrow(self,
                         "NodeAdded",
                         NodeAdded{PwMetadata::nodeFromProperties(id, props)});
    } else if (std::strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
        emitEventNoThrow(self,
                         "PortAdded",
                         PortAdded{PwMetadata::portFromProperties(id, props)});
    } else if (std::strcmp(type, PW_TYPE_INTERFACE_Link) == 0) {
        emitEventNoThrow(self,
                         "LinkAdded",
                         LinkAdded{PwMetadata::linkFromProperties(id, props)});
    }
}

// ── Registry: global removed ──────────────────────────────────────────────

void PwClient::onRegistryGlobalRemove(void *data, uint32_t id)
{
    const auto *self = static_cast<PwClient *>(data);

    emitEventNoThrow(self, "NodeRemoved", NodeRemoved{id});
    emitEventNoThrow(self, "PortRemoved", PortRemoved{id});
    emitEventNoThrow(self, "LinkRemoved", LinkRemoved{id});
}

// ── Core callbacks ────────────────────────────────────────────────────────

void PwClient::onCoreError(
    void *data, uint32_t /*id*/, int /*seq*/, int res, const char *message)
{
    const auto *self = static_cast<PwClient *>(data);
    const std::string coreMessage = std::string((message != nullptr)
                                                    ? message
                                                    : "PipeWire core error")
                                    + " (res=" + std::to_string(res) + ")";
    reportErrorNoThrow(self, coreMessage);
}
