# PIPER on Tenstorrent Greyskull - Docker Image
#
# Multi-stage build for optimized image size
#
# Build:
#   docker build -t piper-greyskull:latest .
#
# Run:
#   docker run --rm --device=/dev/tenstorrent0 \
#     --cap-add=SYS_RAWIO --cap-add=SYS_ADMIN \
#     -v /dev/hugepages:/dev/hugepages \
#     piper-greyskull:latest

FROM ubuntu:22.04 AS base

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    curl \
    jq \
    python3.10 \
    python3-pip \
    python3-venv \
    python3-dev \
    linux-headers-generic \
    dkms \
    libhugetlbfs-dev \
    pciutils \
    lshw \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -s /bin/bash ttuser && \
    echo "ttuser ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER ttuser
WORKDIR /home/ttuser

#
# Stage 2: TT-Metal SDK
#
FROM base AS tt-metal-builder

# Clone and build TT-Metal
RUN git clone https://github.com/tenstorrent/tt-metal.git && \
    cd tt-metal && \
    export ARCH_NAME=grayskull && \
    export TT_METAL_HOME=$(pwd) && \
    cmake -B build -G Ninja && \
    cmake --build build

#
# Stage 3: TT-Buda Framework
#
FROM tt-metal-builder AS tt-buda-builder

# Create Python virtual environment
RUN python3 -m venv ~/tt-env

# Clone and build TT-Buda
RUN git clone https://github.com/tenstorrent/tt-buda.git && \
    cd tt-buda && \
    . ~/tt-env/bin/activate && \
    pip install --upgrade pip setuptools wheel && \
    pip install -r requirements.txt && \
    python setup.py install

#
# Stage 4: Final PIPER image
#
FROM tt-buda-builder AS piper

# Copy PIPER source
COPY --chown=ttuser:ttuser . /home/ttuser/PIPER

WORKDIR /home/ttuser/PIPER

# Install PIPER dependencies
RUN . ~/tt-env/bin/activate && \
    pip install -r requirements.txt

# Build C++ kernels
RUN cd examples/kernels && \
    export TT_METAL_HOME=/home/ttuser/tt-metal && \
    cmake -B build && \
    cmake --build build

# Set environment variables
ENV TT_METAL_HOME=/home/ttuser/tt-metal
ENV ARCH_NAME=grayskull
ENV PYTHONPATH=${TT_METAL_HOME}:${PYTHONPATH}
ENV PATH=/home/ttuser/tt-env/bin:${PATH}

# Activate venv by default
RUN echo 'source ~/tt-env/bin/activate' >> ~/.bashrc

# Default command: run verification
CMD ["/bin/bash", "-c", "source ~/tt-env/bin/activate && ./scripts/verify_installation.sh"]

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD tt-smi -l || exit 1

# Labels
LABEL maintainer="PIPER Development Team"
LABEL description="PIPER ML preprocessing accelerator on Tenstorrent Greyskull"
LABEL version="1.0.0"
LABEL hardware="Greyskull e75/e150"
