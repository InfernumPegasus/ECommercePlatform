FROM debian:trixie

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=1000

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        gcc-14 \
        g++-14 \
        gdb \
        git \
        ninja-build \
        pkg-config \
        python3 \
        python3-pip \
        python3-venv \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/pytools \
    && /opt/pytools/bin/pip install --no-cache-dir --upgrade pip \
    && /opt/pytools/bin/pip install --no-cache-dir "cmake>=3.28,<4" conan

RUN groupadd --gid ${USER_GID} ${USERNAME} \
    && useradd --uid ${USER_UID} --gid ${USER_GID} -m ${USERNAME}

ENV CONAN_HOME=/home/${USERNAME}/.conan2
ENV CC=/usr/bin/gcc-14
ENV CXX=/usr/bin/g++-14
ENV PATH="/opt/pytools/bin:/home/${USERNAME}/.local/bin:${PATH}"

USER ${USERNAME}
WORKDIR /workspaces/ECommercePlatform
