FROM ubuntu:latest

LABEL maintainer "Sukesan1984"

RUN mkdir /workspace
ADD ./workspace /workspace
WORKDIR /workspace

RUN apt-get update
RUN apt-get -y install sudo && \
    sudo apt-get -y install build-essential && \
    sudo apt-get -y install git && \
    sudo apt-get -y install vim && \
    sudo apt-get -y install man && \
    sudo apt-get -y install manpages-dev && \
    sudo apt-get -y install gcc binutils libc6-dev && \
    apt-get install -y strace

