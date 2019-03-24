FROM ubuntu:latest

RUN apt update && \
    apt install -y build-essential mtools nasm vim && \
    apt upgrade -y && \
    apt clean && \
    rm -rf /var/cache/apt/archives/* /var/lib/apt/lists/*

# docker build -t <name> .
# docker run --rm --name <name> -t -i /bin/bash
