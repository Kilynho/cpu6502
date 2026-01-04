
# GCC support can be specified at major, minor, or micro version
# (e.g. 8, 8.2 or 8.2.0).
# See https://hub.docker.com/r/library/gcc/ for all supported GCC
# tags from Docker Hub.
# See https://docs.docker.com/samples/library/gcc/ for more on how to use this image
FROM debian:stable

# Instala herramientas de desarrollo y dependencias
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    libsdl2-dev \
    python3 \
    git \
    wget \
    ca-certificates

# Instala cc65 (ensamblador 6502) desde el repositorio oficial
RUN git clone https://github.com/cc65/cc65.git /opt/cc65 \
    && cd /opt/cc65 \
    && make

# Añade la carpeta bin de cc65 al PATH del contenedor
ENV PATH="/opt/cc65/bin:${PATH}"

# Copia el código fuente
WORKDIR /workspace
COPY . /workspace

# Clona msbasic (rama develop) y genera rom.bin
RUN rm -rf /workspace/msbasic && git clone --branch develop --single-branch https://github.com/Kilynho/msbasic.git /workspace/msbasic \
    && cd /workspace/msbasic \
    && chmod +x make.sh \
    && ./make.sh

# Prepara el entorno para VS Code Dev Containers
RUN useradd -ms /bin/bash devuser
USER devuser
ENV HOME /home/devuser
WORKDIR /workspace

# Comando por defecto: bash interactivo
CMD ["bash"]
