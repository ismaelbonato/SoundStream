FROM ubuntu:25.10

ENV DEBIAN_FRONTEND=noninteractive
# Ensure a UTF-8 locale so Qt does not warn about "C" encoding
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# Install all development tools and dependencies in a single layer
RUN apt-get update && apt-get install -y \
    # ── Build tools ──────────────────────────────────────────────────────────
    build-essential \
    wget \
    software-properties-common \
    gnupg \
    ca-certificates \
    git \
    cmake \
    ninja-build \
    catch2 \
    cppcheck \
    clazy \
    sudo \
    pkg-config \
    gcc-14 \
    g++-14 \
    clang-20 \
    clang++-20 \
    clangd-20 \
    clang-format-20 \
    clang-tidy-20 \
    libc++-20-dev \
    libc++abi-20-dev \
    gdb \
    lldb-20 \
    valgrind \
    vim \
    nano \
    lcov \
    gcovr \
    # ── Qt6 ──────────────────────────────────────────────────────────────────
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-wayland \
    qt6-qpa-plugins \
    qt6-svg-plugins \
    libqt6quickcontrols2-6 \
    # ── KDE Frameworks 6 ─────────────────────────────────────────────────────
    libkirigami-dev \
    libkf6coreaddons-dev \
    libkf6i18n-dev \
    extra-cmake-modules \
    # ── PipeWire ─────────────────────────────────────────────────────────────
    libpipewire-0.3-dev \
    pipewire \
    # ── OpenGL / display ─────────────────────────────────────────────────────
    libgl-dev \
    libglvnd-dev \
    libgl1-mesa-dri \
    mesa-vulkan-drivers \
    libwayland-dev \
    wayland-protocols \
    # ── Kirigami QML runtime plugin ───────────────────────────────────────────
    qml6-module-org-kde-kirigami \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-20 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-20 100 \
    && update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-20 100 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-20 100 \
    && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-20 100 \
    && update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-14 100 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Download and install Google Test from source
RUN cd /tmp \
    && wget https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz \
    && tar -xzf v1.15.2.tar.gz \
    && cd googletest-1.15.2 \
    && mkdir build && cd build \
    && cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local \
    && make -j$(nproc) \
    && make install \
    && cd /tmp \
    && rm -rf v1.15.2.tar.gz googletest-1.15.2

# Verify installations
RUN gcc --version && g++ --version && clang --version && clang++ --version && cmake --version

# Declare build args for dynamic UID/GID
ARG USERNAME
ARG UID
ARG GID

# Remove default 'ubuntu' user to avoid UID/GID conflicts when creating a new user matching the host
RUN userdel -r ubuntu || true

# Create a user matching host UID/GID
RUN groupadd --gid ${GID} ${USERNAME} || true && \
    useradd \
      --create-home \
      --gid ${GID} \
      --groups sudo \
      --no-log-init \
      --shell /bin/bash \
      --uid ${UID} \
      ${USERNAME} && \
    echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers && \
    mkdir -p /home/${USERNAME}/workspace && \
    chown -R ${UID}:${GID} /home/${USERNAME} && \
    sed -i 's/#force_color_prompt=yes/force_color_prompt=yes/' /home/${USERNAME}/.bashrc

USER ${USERNAME}
WORKDIR /home/${USERNAME}/workspace

# Default command
CMD ["/bin/bash"]
