FROM emscripten/emsdk:2.0.30

RUN \
  apt update -y && \
  apt install -y \
    bash \
    build-essential \
    cmake \
    curl \
    git \
    libffi-dev \
    libgdbm-dev \
    libncurses5-dev \
    libnss3-dev \
    libreadline-dev \
    libsqlite3-dev \
    libssl-dev \
    libbz2-dev \
    npm \
    python3 \
    python3-pip \
    python3-setuptools \
    zlib1g-dev

WORKDIR /build

ENTRYPOINT [ "/bin/bash" ]
