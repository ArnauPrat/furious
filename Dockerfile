FROM         archlinux/base:latest
MAINTAINER   Arnau Prat <arnau.prat@gmail.com>
CMD          bash

RUN mkdir -p /home/user
WORKDIR /home/user 

ADD src ./src 
ADD CMakeLists.txt ./
ADD cmake ./cmake 

RUN mkdir build 

RUN pacman -Syy --noconfirm 
RUN pacman -S --noconfirm gcc cmake gtest numactl make clang llvm-libs llvm
