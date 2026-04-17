# SoundStream

**PipeWire patchbay for KDE Plasma (Kirigami)** — v0.1 (Tier 1)

Visualise and edit the live PipeWire audio graph in a native Kirigami UI.
Drag from an output port (orange dot) to an input port (blue dot) to create a link.
Right-click a node to disconnect all its links.

## Dependencies

| Package | Min version | Notes |
|---------|-------------|-------|
| CMake   | 3.20        | |
| Qt6     | 6.6         | Core, Gui, Quick, QuickControls2 |
| KF6     | 6.0         | Kirigami, I18n, CoreAddons |
| ECM     | 6.0         | KDE Extra CMake Modules |
| libpipewire | 0.3.60  | `libpipewire-0.3` pkg-config name |


## Dev Container / Docker Compose workflow

This repository ships a ready-to-use development container setup.

### 1) Generate local container environment

```bash
./setEnv.sh
```

This creates `.env` with your current user/group IDs and display variables
used by `docker-compose.yml`.

### 2) Build and start the dev container

```bash
docker compose up -d --build
```

### 3) Enter the container shell

This opens a shell already positioned at the workspace folder (`/home/${USER}/workspace`).

```bash
docker compose exec soundstream-dev bash
```

### VS Code dev container (optional)

If you use VS Code, `./setEnv.sh` will run automatically when you open the repository, and you can use the "Remote Containers: Reopen in Container" command to enter the dev container with your editor.

---

## Build & run

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build -j$(nproc)

# Run directly (no install needed)
./build/bin/soundstream
```



`core/GraphTypes.h` contains the plain `NodeData`, `PortData`, and `LinkData` values shared across the backend and canvas. `backend/pipewire/PwMetadata` keeps PipeWire property parsing testable without adding a separate mapper layer.

---

## Contributing

Issues and PRs welcome. For major changes, please open an issue first to discuss the proposed change.

---

## Acknowledgements


Thanks to **Rui Nuno Capela** for **QPWGraph** and **QjackCtl**.

- Rui Nuno Capela’s work was an important technical reference during this project.

For a neutral implementation planning list, see `docs/feature-checklist.md`.
