FROM         antergos/archlinux-base-devel 
MAINTAINER   Arnau Prat <arnua.prat@gmail.com>
CMD          bash

RUN mkdir -p /home/user
WORKDIR /home/user 

ADD src ./src 
ADD CMakeLists.txt ./
ADD cmake ./cmake 

RUN mkdir build 

# Required system packages
RUN pacman -S --noconfirm gcc cmake gtest numactl make clang compiler-rt
