# gcc \
#     src/* \
#     -lwayland-client -lxkbcommon -lwayland-egl -lEGL \
#     && WAYLAND_DEBUG=1 ./a.out && rm a.out

gcc \
    engine/src/* \
    basic_rendering.c \
    -lwayland-client -lxkbcommon -lwayland-egl -lEGL \
    && ./a.out && rm a.out
