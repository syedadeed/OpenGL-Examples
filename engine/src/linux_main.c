#include "../include/linux_main.h"

void init_master(Window* w){
    w->WIDTH = 1280;
    w->HEIGHT = 720;
    w->PIXEL_SIZE = 4;
    w->window_exist = -1;

    w->registry_listner.global = global_object_handler;
    w->xdg_shell_listener.ping = xdg_ping;
    w->xdg_base_surface_listener.configure = xdg_surface_configure;
    w->frame_cb_listener.done = new_frame;
    w->toplevel_listener.configure = toplevel_configure;
    w->toplevel_listener.close = toplevel_closed;
    w->toplevel_listener.wm_capabilities = toplevel_capabs;
    w->seat_listener.capabilities = seat_capabs;
    w->seat_listener.name = seat_name;
    w->keyboard_listener.keymap = kb_get_keymap;
    w->keyboard_listener.key = kb_get_key;
    w->keyboard_listener.modifiers = kb_get_modifiers;
    w->keyboard_listener.enter = kb_surface_enter;
    w->keyboard_listener.leave = kb_surface_leave;
    w->keyboard_listener.repeat_info = kb_repeat_info;

    w->kb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    w->display = wl_display_connect(NULL);

    w->egl_display = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_KHR, w->display, NULL);
    eglInitialize(w->egl_display, NULL, NULL);
    eglBindAPI(EGL_OPENGL_API);
    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };//possible additions can be EGL_SAMPLE_BUFFERS/EGL_SAMPLES for multisampling support
    EGLint num_config = 0;
    eglChooseConfig(w->egl_display, config_attribs, &(w->config), 1, &num_config);
    assert(num_config == 1);
    EGLint context_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,  //specifying opengl major version being 3
        EGL_CONTEXT_MINOR_VERSION, 3,  //specifying opengl minor version being 3(net version 3.3)
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };//possible additions can be EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE for opengl debugging
      //on macos you also need to set forward compatiblity bit to true
    w->egl_context = eglCreateContext(w->egl_display, w->config, EGL_NO_CONTEXT, context_attribs);

    w->registry = wl_display_get_registry(w->display);
    wl_registry_add_listener(w->registry, &(w->registry_listner), (void*)w);
    wl_display_roundtrip(w->display);

    w->surface = wl_compositor_create_surface(w->compositor);

    w->egl_window = wl_egl_window_create(w->surface, w->WIDTH, w->HEIGHT);
    w->egl_surface = eglCreatePlatformWindowSurface(w->egl_display, w->config, w->egl_window, NULL);
    eglMakeCurrent(w->egl_display, w->egl_surface, w->egl_surface, w->egl_context);
    gladLoadGLLoader((GLADloadproc)eglGetProcAddress);

    w->xdg_base_surface = xdg_wm_base_get_xdg_surface(w->xdg_shell, w->surface);
    xdg_surface_add_listener(w->xdg_base_surface, &(w->xdg_base_surface_listener), (void*)w);
    w->toplevel = xdg_surface_get_toplevel(w->xdg_base_surface);
    xdg_toplevel_add_listener(w->toplevel, &(w->toplevel_listener), (void*)w);

    xdg_toplevel_set_app_id(w->toplevel, "Fuck");
    xdg_toplevel_set_fullscreen(w->toplevel, w->output_screen);

    wl_surface_commit(w->surface);
}

void global_object_handler(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version){
    Window* w = (Window*)data;
    if(!strcmp(interface, wl_compositor_interface.name)){
        w->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version);
    }else if(!strcmp(interface, wl_output_interface.name)){
        w->output_screen = wl_registry_bind(registry, name, &wl_output_interface, version);
    }else if(!strcmp(interface, xdg_wm_base_interface.name)){
        w->xdg_shell = wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(w->xdg_shell, &(w->xdg_shell_listener), (void*)w);
    }else if(!strcmp(interface, wl_seat_interface.name)){
        w->seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
        wl_seat_add_listener(w->seat, &(w->seat_listener), (void*)w);
    }
}

void seat_capabs(void *data, struct wl_seat *wl_seat, uint32_t capabilities){
    Window* w = (Window*)data;
    if((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) > 0){
        w->keyboard = wl_seat_get_keyboard(w->seat);
        wl_keyboard_add_listener(w->keyboard, &(w->keyboard_listener), (void*)w);
    }
}

void toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states){
    Window* w = (Window*)data;
    if(width == 0 || height == 0) return;

    if(w->WIDTH != width || w->HEIGHT != height){
        w->WIDTH = width;
        w->HEIGHT = height;
        wl_egl_window_resize(w->egl_window, w->WIDTH, w->HEIGHT, 0, 0);
        //glViewport is technically not necessary here if the client isnt high af 
        //and it calls this func before doing any rendering, but ofc we cant assume things
        glViewport(0, 0, w->WIDTH, w->HEIGHT);
    }
}

void xdg_surface_configure(void *data, struct xdg_surface *x_surface, uint32_t serial){
    xdg_surface_ack_configure(x_surface, serial);
    Window* w = (Window*)data;
    if(w->window_exist == -1){
        w->window_exist = 1;
        wl_callback_add_listener(wl_surface_frame(w->surface), &(w->frame_cb_listener), (void*)w);
        Init(w->WIDTH, w->HEIGHT);
        eglSwapBuffers(w->egl_display, w->egl_surface);
    }
}

void new_frame(void *data, struct wl_callback *wl_frame_callback, uint32_t callback_data){
    Window* w = (Window*)data;
    wl_callback_destroy(wl_frame_callback);
    wl_callback_add_listener(wl_surface_frame(w->surface), &(w->frame_cb_listener), (void*)w);
    Render(w->WIDTH, w->HEIGHT);
    eglSwapBuffers(w->egl_display, w->egl_surface);
}

void kb_get_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size){
    Window* w = (Window*)data;
    assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);
    char* map_shm = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    assert(map_shm != MAP_FAILED);
    w->kb_keymap = xkb_keymap_new_from_string(w->kb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    w->kb_state = xkb_state_new(w->kb_keymap);
    munmap(map_shm, size);
    close(fd);
}

void kb_get_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state){
    if(state == WL_KEYBOARD_KEY_STATE_RELEASED) return; //unset the net keyboard key state always
    Window* w = (Window*)data;
    xkb_keysym_t symbol = xkb_state_key_get_one_sym(w->kb_state, key + 8); //Must add 8
    switch(symbol){
        case XKB_KEY_A:
            printf("A\n");break;
        case XKB_KEY_a:
            printf("a\n");break;
    }
}

void kb_get_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group){
    Window* w = (Window*)data;
    xkb_state_update_mask(w->kb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

void destroy_master(Window* w){
    wl_seat_release(w->seat);
    wl_display_disconnect(w->display);
}

void xdg_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial){
    xdg_wm_base_pong(xdg_wm_base, serial);
}

void toplevel_closed(void *data, struct xdg_toplevel *xdg_toplevel){
    Window* w = (Window*)data;
    w->window_exist = 0;
}

void seat_name(void *data, struct wl_seat *wl_seat, const char *name){}
void toplevel_capabs(void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities){}
void kb_surface_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys){}
void kb_surface_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface){}
void kb_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay){}

int main(){
    Window w = {};memset(&w, 0, sizeof(Window));
    init_master(&w);
    while (w.window_exist != 0){
        wl_display_dispatch(w.display);
    }
    destroy_master(&w);
    return 0;
}
