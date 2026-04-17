# Tests are split by layer:
# - core: Catch2 (pure C++ logic)
# - qt: Qt Test (Qt event-loop/signal behavior)

add_subdirectory(core)
add_subdirectory(qt)
