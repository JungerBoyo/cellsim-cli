FROM fedora:36

# By default, anything you run in Docker is done as superuser.
# Conan runs some install commands as superuser, and will prepend `sudo` to
# these commands, unless `CONAN_SYSREQUIRES_SUDO=0` is in your env variables.
ENV CONAN_SYSREQUIRES_SUDO 0
# Some packages request that Conan use the system package manager to install
# a few dependencies. This flag allows Conan to proceed with these installations;
# leaving this flag undefined can cause some installation failures.
ENV CONAN_SYSREQUIRES_MODE enabled
# installing tools
RUN dnf install -y cmake ccache clang llvm python3-pip python3.11 spirv-tools glslang vim git
RUN dnf install -y libXext-devel libXfixes-devel libXrender-devel libICE-devel libSM-devel libXt-devel libXmu-devel libXi-devel libXpm-devel libXaw libXaw-devel libXtst-devel libXcursor-devel libXrandr-devel libXcomposite-devel libXdamage-devel libXScrnSaver-devel libXinerama-devel libXres-devel libXv-devel libXvMC-devel libXxf86vm-devel libuuid-devel xorg-x11-xtrans-devel xkeyboard-config-devel xcb-util-wm-devel xcb-util-renderutil-devel xcb-util-keysyms-devel xcb-util-image-devel xcb-util-devel libxkbfile-devel libfontenc-devel libXdmcp-devel
RUN pip install conan
# install cellsim
RUN git clone https://github.com/JungerBoyo/CellSimRemake
RUN cmake -S . -B build/ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON && \
    cmake --build build/ -j

CMD ["/bin/bash"]