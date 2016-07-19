/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include <gc_vdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dlfcn.h>
#include <poll.h>

#include <sys/time.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>

#include <linux/input.h>

#include <pthread.h>

/******************************************************************************/
/* inline xdg-shell-client-protocol.h */

extern int xdg_shell_version;

struct wl_client;
struct wl_resource;

struct xdg_shell;
struct xdg_surface;
struct xdg_popup;

extern const struct wl_interface xdg_shell_interface_v4;
extern const struct wl_interface xdg_shell_interface_v5;
extern const struct wl_interface xdg_surface_interface_v4;
extern const struct wl_interface xdg_surface_interface_v5;
extern const struct wl_interface xdg_popup_interface_v4;
extern const struct wl_interface xdg_popup_interface_v5;

#ifndef XDG_SHELL_VERSION_ENUM
#define XDG_SHELL_VERSION_ENUM
/**
 * xdg_shell_version - latest protocol version
 * @XDG_SHELL_VERSION_CURRENT: Always the latest version
 *
 * The 'current' member of this enum gives the version of the protocol.
 * Implementations can compare this to the version they implement using
 * static_assert to ensure the protocol and implementation versions match.
 */
enum xdg_shell_version {
    XDG_SHELL_VERSION_CURRENT_v4 = 4,
    XDG_SHELL_VERSION_CURRENT_v5 = 5,
};
#endif /* XDG_SHELL_VERSION_ENUM */

#ifndef XDG_SHELL_ERROR_ENUM
#define XDG_SHELL_ERROR_ENUM
enum xdg_shell_error {
    XDG_SHELL_ERROR_ROLE = 0,
    XDG_SHELL_ERROR_DEFUNCT_SURFACES = 1,
    XDG_SHELL_ERROR_NOT_THE_TOPMOST_POPUP = 2,
    XDG_SHELL_ERROR_INVALID_POPUP_PARENT = 3,
};
#endif /* XDG_SHELL_ERROR_ENUM */

/**
 * xdg_shell - create desktop-style surfaces
 * @ping: check if the client is alive
 *
 * xdg_shell allows clients to turn a wl_surface into a "real window"
 * which can be dragged, resized, stacked, and moved around by the user.
 * Everything about this interface is suited towards traditional desktop
 * environments.
 */
struct xdg_shell_listener {
    /**
     * ping - check if the client is alive
     * @serial: pass this to the pong request
     *
     * The ping event asks the client if it's still alive. Pass the
     * serial specified in the event back to the compositor by sending
     * a "pong" request back with the specified serial.
     *
     * Compositors can use this to determine if the client is still
     * alive. It's unspecified what will happen if the client doesn't
     * respond to the ping request, or in what timeframe. Clients
     * should try to respond in a reasonable amount of time.
     *
     * A compositor is free to ping in any way it wants, but a client
     * must always respond to any xdg_shell object it created.
     */
    void (*ping)(void *data,
             struct xdg_shell *xdg_shell,
             uint32_t serial);
};

static inline int
xdg_shell_add_listener(struct xdg_shell *xdg_shell,
               const struct xdg_shell_listener *listener, void *data)
{
    return wl_proxy_add_listener((struct wl_proxy *) xdg_shell,
                     (void (**)(void)) listener, data);
}

#define XDG_SHELL_USE_UNSTABLE_VERSION_v4    0
#define XDG_SHELL_GET_XDG_SURFACE_v4    1
#define XDG_SHELL_GET_XDG_POPUP_v4    2
#define XDG_SHELL_PONG_v4    3


#define XDG_SHELL_DESTROY_v5    0
#define XDG_SHELL_USE_UNSTABLE_VERSION_v5    1
#define XDG_SHELL_GET_XDG_SURFACE_v5    2
#define XDG_SHELL_GET_XDG_POPUP_v5    3
#define XDG_SHELL_PONG_v5    4

static inline void
xdg_shell_set_user_data(struct xdg_shell *xdg_shell, void *user_data)
{
    wl_proxy_set_user_data((struct wl_proxy *) xdg_shell, user_data);
}

static inline void *
xdg_shell_get_user_data(struct xdg_shell *xdg_shell)
{
    return wl_proxy_get_user_data((struct wl_proxy *) xdg_shell);
}

static inline void
xdg_shell_destroy(struct xdg_shell *xdg_shell)
{
    if (xdg_shell_version == XDG_SHELL_VERSION_CURRENT_v4) {
        wl_proxy_destroy((struct wl_proxy *) xdg_shell);
    } else if (xdg_shell_version == XDG_SHELL_VERSION_CURRENT_v5) {
        wl_proxy_marshal((struct wl_proxy *) xdg_shell,
                XDG_SHELL_DESTROY_v5);

        wl_proxy_destroy((struct wl_proxy *) xdg_shell);
    }
}

static inline void
xdg_shell_use_unstable_version(struct xdg_shell *xdg_shell, int32_t version)
{
    if (xdg_shell_version == XDG_SHELL_VERSION_CURRENT_v4) {
        wl_proxy_marshal((struct wl_proxy *) xdg_shell,
                XDG_SHELL_USE_UNSTABLE_VERSION_v4, version);
    } else if (xdg_shell_version == XDG_SHELL_VERSION_CURRENT_v5) {
        wl_proxy_marshal((struct wl_proxy *) xdg_shell,
                XDG_SHELL_USE_UNSTABLE_VERSION_v5, version);
    }
}

static inline struct xdg_surface *
xdg_shell_get_xdg_surface(struct xdg_shell *xdg_shell, struct wl_surface *surface)
{
    struct wl_proxy *id;

    if (xdg_shell_version == XDG_SHELL_VERSION_CURRENT_v4) {
        id = wl_proxy_marshal_constructor((struct wl_proxy *) xdg_shell,
                XDG_SHELL_GET_XDG_SURFACE_v4, &xdg_surface_interface_v4, NULL, surface);
    } else if (xdg_shell_version == XDG_SHELL_VERSION_CURRENT_v5) {
        id = wl_proxy_marshal_constructor((struct wl_proxy *) xdg_shell,
                XDG_SHELL_GET_XDG_SURFACE_v5, &xdg_surface_interface_v5, NULL, surface);
    } else {
        id = NULL;
    }

    return (struct xdg_surface *) id;
}

static inline struct xdg_popup *
xdg_shell_get_xdg_popup_v4(struct xdg_shell *xdg_shell, struct wl_surface *surface, struct wl_surface *parent, struct wl_seat *seat, uint32_t serial, int32_t x, int32_t y, uint32_t flags)
{
    struct wl_proxy *id;

    id = wl_proxy_marshal_constructor((struct wl_proxy *) xdg_shell,
            XDG_SHELL_GET_XDG_POPUP_v4, &xdg_popup_interface_v4, NULL, surface, parent, seat, serial, x, y, flags);

    return (struct xdg_popup *) id;
}

static inline struct xdg_popup *
xdg_shell_get_xdg_popup_v5(struct xdg_shell *xdg_shell, struct wl_surface *surface, struct wl_surface *parent, struct wl_seat *seat, uint32_t serial, int32_t x, int32_t y)
{
    struct wl_proxy *id;

    id = wl_proxy_marshal_constructor((struct wl_proxy *) xdg_shell,
             XDG_SHELL_GET_XDG_POPUP_v5, &xdg_popup_interface_v5, NULL, surface, parent, seat, serial, x, y);

    return (struct xdg_popup *) id;
}

static inline void
xdg_shell_pong(struct xdg_shell *xdg_shell, uint32_t serial)
{
    if (xdg_shell_version == XDG_SHELL_VERSION_CURRENT_v4) {
        wl_proxy_marshal((struct wl_proxy *) xdg_shell,
                 XDG_SHELL_PONG_v4, serial);
    } else if (xdg_shell_version == XDG_SHELL_VERSION_CURRENT_v5) {
        wl_proxy_marshal((struct wl_proxy *) xdg_shell,
                 XDG_SHELL_PONG_v5, serial);
    }
}

#ifndef XDG_SURFACE_RESIZE_EDGE_ENUM
#define XDG_SURFACE_RESIZE_EDGE_ENUM
/**
 * xdg_surface_resize_edge - edge values for resizing
 * @XDG_SURFACE_RESIZE_EDGE_NONE: (none)
 * @XDG_SURFACE_RESIZE_EDGE_TOP: (none)
 * @XDG_SURFACE_RESIZE_EDGE_BOTTOM: (none)
 * @XDG_SURFACE_RESIZE_EDGE_LEFT: (none)
 * @XDG_SURFACE_RESIZE_EDGE_TOP_LEFT: (none)
 * @XDG_SURFACE_RESIZE_EDGE_BOTTOM_LEFT: (none)
 * @XDG_SURFACE_RESIZE_EDGE_RIGHT: (none)
 * @XDG_SURFACE_RESIZE_EDGE_TOP_RIGHT: (none)
 * @XDG_SURFACE_RESIZE_EDGE_BOTTOM_RIGHT: (none)
 *
 * These values are used to indicate which edge of a surface is being
 * dragged in a resize operation.
 */
enum xdg_surface_resize_edge {
    XDG_SURFACE_RESIZE_EDGE_NONE = 0,
    XDG_SURFACE_RESIZE_EDGE_TOP = 1,
    XDG_SURFACE_RESIZE_EDGE_BOTTOM = 2,
    XDG_SURFACE_RESIZE_EDGE_LEFT = 4,
    XDG_SURFACE_RESIZE_EDGE_TOP_LEFT = 5,
    XDG_SURFACE_RESIZE_EDGE_BOTTOM_LEFT = 6,
    XDG_SURFACE_RESIZE_EDGE_RIGHT = 8,
    XDG_SURFACE_RESIZE_EDGE_TOP_RIGHT = 9,
    XDG_SURFACE_RESIZE_EDGE_BOTTOM_RIGHT = 10,
};
#endif /* XDG_SURFACE_RESIZE_EDGE_ENUM */

#ifndef XDG_SURFACE_STATE_ENUM
#define XDG_SURFACE_STATE_ENUM
/**
 * xdg_surface_state - types of state on the surface
 * @XDG_SURFACE_STATE_MAXIMIZED: the surface is maximized
 * @XDG_SURFACE_STATE_FULLSCREEN: the surface is fullscreen
 * @XDG_SURFACE_STATE_RESIZING: (none)
 * @XDG_SURFACE_STATE_ACTIVATED: (none)
 *
 * The different state values used on the surface. This is designed for
 * state values like maximized, fullscreen. It is paired with the configure
 * event to ensure that both the client and the compositor setting the
 * state can be synchronized.
 *
 * States set in this way are double-buffered. They will get applied on the
 * next commit.
 *
 * Desktop environments may extend this enum by taking up a range of values
 * and documenting the range they chose in this description. They are not
 * required to document the values for the range that they chose. Ideally,
 * any good extensions from a desktop environment should make its way into
 * standardization into this enum.
 *
 * The current reserved ranges are:
 *
 * 0x0000 - 0x0FFF: xdg-shell core values, documented below. 0x1000 -
 * 0x1FFF: GNOME
 */
enum xdg_surface_state {
    XDG_SURFACE_STATE_MAXIMIZED = 1,
    XDG_SURFACE_STATE_FULLSCREEN = 2,
    XDG_SURFACE_STATE_RESIZING = 3,
    XDG_SURFACE_STATE_ACTIVATED = 4,
};
#endif /* XDG_SURFACE_STATE_ENUM */

/**
 * xdg_surface - A desktop window
 * @configure: suggest a surface change
 * @close: surface wants to be closed
 *
 * An interface that may be implemented by a wl_surface, for
 * implementations that provide a desktop-style user interface.
 *
 * It provides requests to treat surfaces like windows, allowing to set
 * properties like maximized, fullscreen, minimized, and to move and resize
 * them, and associate metadata like title and app id.
 *
 * The client must call wl_surface.commit on the corresponding wl_surface
 * for the xdg_surface state to take effect. Prior to committing the new
 * state, it can set up initial configuration, such as maximizing or
 * setting a window geometry.
 *
 * Even without attaching a buffer the compositor must respond to initial
 * committed configuration, for instance sending a configure event with
 * expected window geometry if the client maximized its surface during
 * initialization.
 *
 * For a surface to be mapped by the compositor the client must have
 * committed both an xdg_surface state and a buffer.
 */
struct xdg_surface_listener {
    /**
     * configure - suggest a surface change
     * @width: (none)
     * @height: (none)
     * @states: (none)
     * @serial: (none)
     *
     * The configure event asks the client to resize its surface or
     * to change its state.
     *
     * The width and height arguments specify a hint to the window
     * about how its surface should be resized in window geometry
     * coordinates. See set_window_geometry.
     *
     * If the width or height arguments are zero, it means the client
     * should decide its own window dimension. This may happen when the
     * compositor need to configure the state of the surface but
     * doesn't have any information about any previous or expected
     * dimension.
     *
     * The states listed in the event specify how the width/height
     * arguments should be interpreted, and possibly how it should be
     * drawn.
     *
     * Clients should arrange their surface for the new size and
     * states, and then send a ack_configure request with the serial
     * sent in this configure event at some point before committing the
     * new surface.
     *
     * If the client receives multiple configure events before it can
     * respond to one, it is free to discard all but the last event it
     * received.
     */
    void (*configure)(void *data,
              struct xdg_surface *xdg_surface,
              int32_t width,
              int32_t height,
              struct wl_array *states,
              uint32_t serial);
    /**
     * close - surface wants to be closed
     *
     * The close event is sent by the compositor when the user wants
     * the surface to be closed. This should be equivalent to the user
     * clicking the close button in client-side decorations, if your
     * application has any...
     *
     * This is only a request that the user intends to close your
     * window. The client may choose to ignore this request, or show a
     * dialog to ask the user to save their data...
     */
    void (*close)(void *data,
              struct xdg_surface *xdg_surface);
};

static inline int
xdg_surface_add_listener(struct xdg_surface *xdg_surface,
             const struct xdg_surface_listener *listener, void *data)
{
    return wl_proxy_add_listener((struct wl_proxy *) xdg_surface,
                     (void (**)(void)) listener, data);
}

#define XDG_SURFACE_DESTROY    0
#define XDG_SURFACE_SET_PARENT    1
#define XDG_SURFACE_SET_TITLE    2
#define XDG_SURFACE_SET_APP_ID    3
#define XDG_SURFACE_SHOW_WINDOW_MENU    4
#define XDG_SURFACE_MOVE    5
#define XDG_SURFACE_RESIZE    6
#define XDG_SURFACE_ACK_CONFIGURE    7
#define XDG_SURFACE_SET_WINDOW_GEOMETRY    8
#define XDG_SURFACE_SET_MAXIMIZED    9
#define XDG_SURFACE_UNSET_MAXIMIZED    10
#define XDG_SURFACE_SET_FULLSCREEN    11
#define XDG_SURFACE_UNSET_FULLSCREEN    12
#define XDG_SURFACE_SET_MINIMIZED    13

static inline void
xdg_surface_set_user_data(struct xdg_surface *xdg_surface, void *user_data)
{
    wl_proxy_set_user_data((struct wl_proxy *) xdg_surface, user_data);
}

static inline void *
xdg_surface_get_user_data(struct xdg_surface *xdg_surface)
{
    return wl_proxy_get_user_data((struct wl_proxy *) xdg_surface);
}

static inline void
xdg_surface_destroy(struct xdg_surface *xdg_surface)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_DESTROY);

    wl_proxy_destroy((struct wl_proxy *) xdg_surface);
}

static inline void
xdg_surface_set_parent(struct xdg_surface *xdg_surface, struct xdg_surface *parent)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_SET_PARENT, parent);
}

static inline void
xdg_surface_set_title(struct xdg_surface *xdg_surface, const char *title)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_SET_TITLE, title);
}

static inline void
xdg_surface_set_app_id(struct xdg_surface *xdg_surface, const char *app_id)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_SET_APP_ID, app_id);
}

static inline void
xdg_surface_show_window_menu(struct xdg_surface *xdg_surface, struct wl_seat *seat, uint32_t serial, int32_t x, int32_t y)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_SHOW_WINDOW_MENU, seat, serial, x, y);
}

static inline void
xdg_surface_move(struct xdg_surface *xdg_surface, struct wl_seat *seat, uint32_t serial)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_MOVE, seat, serial);
}

static inline void
xdg_surface_resize(struct xdg_surface *xdg_surface, struct wl_seat *seat, uint32_t serial, uint32_t edges)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_RESIZE, seat, serial, edges);
}

static inline void
xdg_surface_ack_configure(struct xdg_surface *xdg_surface, uint32_t serial)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_ACK_CONFIGURE, serial);
}

static inline void
xdg_surface_set_window_geometry(struct xdg_surface *xdg_surface, int32_t x, int32_t y, int32_t width, int32_t height)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_SET_WINDOW_GEOMETRY, x, y, width, height);
}

static inline void
xdg_surface_set_maximized(struct xdg_surface *xdg_surface)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_SET_MAXIMIZED);
}

static inline void
xdg_surface_unset_maximized(struct xdg_surface *xdg_surface)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_UNSET_MAXIMIZED);
}

static inline void
xdg_surface_set_fullscreen(struct xdg_surface *xdg_surface, struct wl_output *output)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_SET_FULLSCREEN, output);
}

static inline void
xdg_surface_unset_fullscreen(struct xdg_surface *xdg_surface)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_UNSET_FULLSCREEN);
}

static inline void
xdg_surface_set_minimized(struct xdg_surface *xdg_surface)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_surface,
             XDG_SURFACE_SET_MINIMIZED);
}

/**
 * xdg_popup - short-lived, popup surfaces for menus
 * @popup_done: popup interaction is done
 *
 * A popup surface is a short-lived, temporary surface that can be used
 * to implement menus. It takes an explicit grab on the surface that will
 * be dismissed when the user dismisses the popup. This can be done by the
 * user clicking outside the surface, using the keyboard, or even locking
 * the screen through closing the lid or a timeout.
 *
 * When the popup is dismissed, a popup_done event will be sent out, and at
 * the same time the surface will be unmapped. The xdg_popup object is now
 * inert and cannot be reactivated, so clients should destroy it.
 * Explicitly destroying the xdg_popup object will also dismiss the popup
 * and unmap the surface.
 *
 * Clients will receive events for all their surfaces during this grab
 * (which is an "owner-events" grab in X11 parlance). This is done so that
 * users can navigate through submenus and other "nested" popup windows
 * without having to dismiss the topmost popup.
 *
 * Clients that want to dismiss the popup when another surface of their own
 * is clicked should dismiss the popup using the destroy request.
 *
 * The parent surface must have either an xdg_surface or xdg_popup role.
 *
 * Specifying an xdg_popup for the parent means that the popups are nested,
 * with this popup now being the topmost popup. Nested popups must be
 * destroyed in the reverse order they were created in, e.g. the only popup
 * you are allowed to destroy at all times is the topmost one.
 *
 * If there is an existing popup when creating a new popup, the parent must
 * be the current topmost popup.
 *
 * A parent surface must be mapped before the new popup is mapped.
 *
 * When compositors choose to dismiss a popup, they will likely dismiss
 * every nested popup as well. When a compositor dismisses popups, it will
 * follow the same dismissing order as required from the client.
 *
 * The x and y arguments passed when creating the popup object specify
 * where the top left of the popup should be placed, relative to the local
 * surface coordinates of the parent surface. See xdg_shell.get_xdg_popup.
 *
 * The client must call wl_surface.commit on the corresponding wl_surface
 * for the xdg_popup state to take effect.
 *
 * For a surface to be mapped by the compositor the client must have
 * committed both the xdg_popup state and a buffer.
 */
struct xdg_popup_listener_v4 {
    /**
     * popup_done - popup interaction is done
     *
     * The popup_done event is sent out when a popup is dismissed by
     * the compositor. The client should destroy the xdg_popup object
     * at this point.
     */
    void (*popup_done)(void *data,
               struct xdg_popup *xdg_popup,
               uint32_t serial);
};

static inline int
xdg_popup_add_listener_v4(struct xdg_popup *xdg_popup,
               const struct xdg_popup_listener_v4 *listener, void *data)
{
    return wl_proxy_add_listener((struct wl_proxy *) xdg_popup,
                     (void (**)(void)) listener, data);
}

struct xdg_popup_listener_v5 {
    /**
     * popup_done - popup interaction is done
     *
     * The popup_done event is sent out when a popup is dismissed by
     * the compositor. The client should destroy the xdg_popup object
     * at this point.
     */
    void (*popup_done)(void *data,
               struct xdg_popup *xdg_popup);
};

static inline int
xdg_popup_add_listener_v5(struct xdg_popup *xdg_popup,
               const struct xdg_popup_listener_v5 *listener, void *data)
{
    return wl_proxy_add_listener((struct wl_proxy *) xdg_popup,
                     (void (**)(void)) listener, data);
}

#define XDG_POPUP_DESTROY    0

static inline void
xdg_popup_set_user_data(struct xdg_popup *xdg_popup, void *user_data)
{
    wl_proxy_set_user_data((struct wl_proxy *) xdg_popup, user_data);
}

static inline void *
xdg_popup_get_user_data(struct xdg_popup *xdg_popup)
{
    return wl_proxy_get_user_data((struct wl_proxy *) xdg_popup);
}

static inline void
xdg_popup_destroy(struct xdg_popup *xdg_popup)
{
    wl_proxy_marshal((struct wl_proxy *) xdg_popup,
             XDG_POPUP_DESTROY);

    wl_proxy_destroy((struct wl_proxy *) xdg_popup);
}

/******************************************************************************/
/* inline xdg-shell-protocol.c */
#include "wayland-util.h"

extern const struct wl_interface wl_output_interface;
extern const struct wl_interface wl_seat_interface;
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface xdg_popup_interface_v4;
extern const struct wl_interface xdg_surface_interface_v4;

extern const struct wl_interface xdg_surface_interface_v5;
extern const struct wl_interface xdg_popup_interface_v5;

static const struct wl_interface *types_v4[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    &xdg_surface_interface_v4,
    &wl_surface_interface,
    &xdg_popup_interface_v4,
    &wl_surface_interface,
    &wl_surface_interface,
    &wl_seat_interface,
    NULL,
    NULL,
    NULL,
    NULL,
    &wl_surface_interface,
    &wl_seat_interface,
    NULL,
    NULL,
    NULL,
    &wl_seat_interface,
    NULL,
    &wl_seat_interface,
    NULL,
    NULL,
    &wl_output_interface,
};

static const struct wl_message xdg_shell_requests_v4[] = {
    { "use_unstable_version", "i", types_v4 + 0 },
    { "get_xdg_surface", "no", types_v4 + 4 },
    { "get_xdg_popup", "nooouiiu", types_v4 + 6 },
    { "pong", "u", types_v4 + 0 },
};

static const struct wl_message xdg_shell_events_v4[] = {
    { "ping", "u", types_v4 + 0 },
};

WL_EXPORT const struct wl_interface xdg_shell_interface_v4 = {
    "xdg_shell", 1,
    4, xdg_shell_requests_v4,
    1, xdg_shell_events_v4,
};

static const struct wl_message xdg_surface_requests_v4[] = {
    { "destroy", "", types_v4 + 0 },
    { "set_parent", "?o", types_v4 + 14 },
    { "set_title", "s", types_v4 + 0 },
    { "set_app_id", "s", types_v4 + 0 },
    { "show_window_menu", "ouii", types_v4 + 15 },
    { "move", "ou", types_v4 + 19 },
    { "resize", "ouu", types_v4 + 21 },
    { "ack_configure", "u", types_v4 + 0 },
    { "set_window_geometry", "iiii", types_v4 + 0 },
    { "set_maximized", "", types_v4 + 0 },
    { "unset_maximized", "", types_v4 + 0 },
    { "set_fullscreen", "?o", types_v4 + 24 },
    { "unset_fullscreen", "", types_v4 + 0 },
    { "set_minimized", "", types_v4 + 0 },
};

static const struct wl_message xdg_surface_events_v4[] = {
    { "configure", "iiau", types_v4 + 0 },
    { "close", "", types_v4 + 0 },
};

WL_EXPORT const struct wl_interface xdg_surface_interface_v4 = {
    "xdg_surface", 1,
    14, xdg_surface_requests_v4,
    2, xdg_surface_events_v4,
};

static const struct wl_message xdg_popup_requests_v4[] = {
    { "destroy", "", types_v4 + 0 },
};

static const struct wl_message xdg_popup_events_v4[] = {
    { "popup_done", "u", types_v4 + 0 },
};

WL_EXPORT const struct wl_interface xdg_popup_interface_v4 = {
    "xdg_popup", 1,
    1, xdg_popup_requests_v4,
    1, xdg_popup_events_v4,
};


static const struct wl_interface *types_v5[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    &xdg_surface_interface_v5,
    &wl_surface_interface,
    &xdg_popup_interface_v5,
    &wl_surface_interface,
    &wl_surface_interface,
    &wl_seat_interface,
    NULL,
    NULL,
    NULL,
    &xdg_surface_interface_v5,
    &wl_seat_interface,
    NULL,
    NULL,
    NULL,
    &wl_seat_interface,
    NULL,
    &wl_seat_interface,
    NULL,
    NULL,
    &wl_output_interface,
};

static const struct wl_message xdg_shell_requests_v5[] = {
    { "destroy", "", types_v5 + 0 },
    { "use_unstable_version", "i", types_v5 + 0 },
    { "get_xdg_surface", "no", types_v5 + 4 },
    { "get_xdg_popup", "nooouii", types_v5 + 6 },
    { "pong", "u", types_v5 + 0 },
};

static const struct wl_message xdg_shell_events_v5[] = {
    { "ping", "u", types_v5 + 0 },
};

WL_EXPORT const struct wl_interface xdg_shell_interface_v5 = {
    "xdg_shell", 1,
    5, xdg_shell_requests_v5,
    1, xdg_shell_events_v5,
};

static const struct wl_message xdg_surface_requests_v5[] = {
    { "destroy", "", types_v5 + 0 },
    { "set_parent", "?o", types_v5 + 13 },
    { "set_title", "s", types_v5 + 0 },
    { "set_app_id", "s", types_v5 + 0 },
    { "show_window_menu", "ouii", types_v5 + 14 },
    { "move", "ou", types_v5 + 18 },
    { "resize", "ouu", types_v5 + 20 },
    { "ack_configure", "u", types_v5 + 0 },
    { "set_window_geometry", "iiii", types_v5 + 0 },
    { "set_maximized", "", types_v5 + 0 },
    { "unset_maximized", "", types_v5 + 0 },
    { "set_fullscreen", "?o", types_v5 + 23 },
    { "unset_fullscreen", "", types_v5 + 0 },
    { "set_minimized", "", types_v5 + 0 },
};

static const struct wl_message xdg_surface_events_v5[] = {
    { "configure", "iiau", types_v5 + 0 },
    { "close", "", types_v5 + 0 },
};

WL_EXPORT const struct wl_interface xdg_surface_interface_v5 = {
    "xdg_surface", 1,
    14, xdg_surface_requests_v5,
    2, xdg_surface_events_v5,
};

static const struct wl_message xdg_popup_requests_v5[] = {
    { "destroy", "", types_v5 + 0 },
};

static const struct wl_message xdg_popup_events_v5[] = {
    { "popup_done", "", types_v5 + 0 },
};

WL_EXPORT const struct wl_interface xdg_popup_interface_v5 = {
    "xdg_popup", 1,
    1, xdg_popup_requests_v5,
    1, xdg_popup_events_v5,
};


/*******************************************************************************
***** Version Signature *******************************************************/

const char * _VDK_PLATFORM = "\n\0$PLATFORM$WAYLAND\n";


#define EVENT_QUEUE_SIZE 128

struct vdk_display;

struct vdk_window
{
    struct vdk_display *display;

    struct wl_surface *surface;
    struct wl_egl_window *wl_win;
    int x;
    int y;
    int width;
    int height;

    struct xdg_surface *xdg_surface;

    /* Event queue. */
    vdkEvent event_queue[EVENT_QUEUE_SIZE];
    volatile int event_rpos;
    volatile int event_wpos;
    pthread_mutex_t event_mutex;

    int motion_x;
    int motion_y;

    struct wl_callback *callback;

    /* Link in _vdkPrivate::win_list. */
    struct wl_list link;

    /* Link in vdk_display::win_list. */
    struct wl_list link_in_dpy;
};

struct vdk_pixmap
{
    struct vdk_display *display;
    struct wl_egl_pixmap *wl_pix;

    int width;
    int height;
    int bpp;

    /* Link in _vdkPrivate::pix_list. */
    struct wl_list link;

    /* Link in vdk_display::pix_list. */
    struct wl_list link_in_dpy;
};

struct vdk_display
{
    struct wl_display *wl_dpy;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_seat *seat;
    struct wl_pointer *pointer;
    struct wl_touch *touch;
    struct wl_keyboard *keyboard;
    struct wl_shm *shm;
    struct wl_cursor_theme *cursor_theme;
    struct wl_cursor *default_cursor;
    struct wl_surface *cursor_surface;
    struct wl_output *output;

    struct xdg_shell *shell;

    /* compositor information. */
    int width;
    int height;
    int refresh;

    /* check focused window. */
    struct vdk_window *pointer_win;
    struct vdk_window *keyboard_win;

    /* Created windows in list. */
    struct wl_list win_list;

    /* Created pixmaps in list. */
    struct wl_list pix_list;

    /* Link in _vdkPrivate::dpy_list. */
    struct wl_list link;
};

struct _vdkPrivate
{
    struct wl_list dpy_list;
    struct wl_list win_list;
    struct wl_list pix_list;
    void *         egl;
};

static vdkPrivate _vdk = NULL;

static struct vdk_window * vdk_find_window(vdkPrivate priv, struct wl_egl_window * wl_win)
{
    struct vdk_window *win;

    wl_list_for_each(win, &priv->win_list, link) {
        if (win->wl_win == wl_win) {
            return win;
        }
    }

    return NULL;
}

static struct vdk_window * vdk_find_window_by_surface(vdkPrivate priv, struct wl_surface *sur)
{
    struct vdk_window *win;

    wl_list_for_each(win, &priv->win_list, link) {
        if (win->surface == sur) {
            return win;
        }
    }

    return NULL;
}

/* currently supported: 4, 5. */
int xdg_shell_version = 5;

static void detect_xdg_shell(void)
{
    int major = 0;
    int minor = 0;
    FILE *fp;

    /*
     * TODO: find a better way to detect xdg_shell version.
     * xdg_shell, xdg_surface, xdg_pop are interfaces exported by weston
     * desktop shell. We must use xdg_surface otherwise it may just get
     * stuck.
     * What's more, there's no way get those interface versions, though
     * wayland website claimed that every interface is versioned.
     * Here we just run 'weston --version' to obtain weston version.
     */
    fp = popen("weston --version", "r");
    if (!fp) {
        fprintf(stderr, "can not execute 'weston'\n");
        goto error;
    }

    fscanf(fp, "weston %d.%d", &major, &minor);

    if (major != 1 || minor < 6 || minor > 9) {
        goto error;
    }

    if (minor >= 6 && minor <= 7) {
        xdg_shell_version = 4;
    } else if (minor <= 9) {
        xdg_shell_version = 5;
    }

    return;

error:
    fprintf(stderr, "xdg_shell interface exported\n");
    fprintf(stderr, "Unknown xdg_shell version\n");
}

static void
handle_surface_configure(void *data, struct xdg_surface *surface,
             int32_t width, int32_t height,
             struct wl_array *states, uint32_t serial)
{
    struct vdk_window *win = data;
    uint32_t *p;

    wl_array_for_each(p, states) {
        uint32_t state = *p;
        switch (state) {
        case XDG_SURFACE_STATE_FULLSCREEN:
            break;
        }
    }

    if (width > 0 && height > 0) {
        win->width = width;
        win->height = height;
    }

    if (win->wl_win) {
        wl_egl_window_resize(win->wl_win, win->width, win->height, 0, 0);
    }

    xdg_surface_ack_configure(surface, serial);
}

static void
handle_surface_delete(void *data, struct xdg_surface *xdg_surface)
{
}

static const struct xdg_surface_listener xdg_surface_listener = {
    handle_surface_configure,
    handle_surface_delete,
};

static void
create_xdg_surface(struct vdk_window *win, struct vdk_display *dpy)
{
    win->xdg_surface = xdg_shell_get_xdg_surface(dpy->shell,
                            win->surface);

    xdg_surface_add_listener(win->xdg_surface,
                  &xdg_surface_listener, win);

    xdg_surface_set_title(win->xdg_surface, "vdk-window");
}

static struct vdk_window * vdk_create_window(vdkPrivate priv,
        struct vdk_display *dpy, int x, int y, int width, int height)
{
    struct wl_egl_window *wl_win = NULL;
    struct wl_surface *surface = NULL;
    struct vdk_window *win;

    surface = wl_compositor_create_surface(dpy->compositor);

    if (!surface) {
        fprintf(stderr, "%s(%d): wl_compositor_create_surface failed\n",
                __func__, __LINE__);
        goto error;
    }

    wl_win = wl_egl_window_create(surface, width, height);

    if (!wl_win) {
        fprintf(stderr, "%s(%d): wl_egl_window_create failed\n",
                __func__, __LINE__);
        goto error;
    }

    win = (struct vdk_window *) malloc(sizeof (struct vdk_window));
    if (!win) {
        fprintf(stderr, "%s(%d): out of memory\n",
                __func__, __LINE__);
        goto error;
    }

    memset(win, 0, sizeof (struct vdk_window));

    win->display  = dpy;
    win->surface  = surface;
    win->wl_win   = wl_win;
    win->x        = x;
    win->y        = y;
    win->width    = width;
    win->height   = height;
    win->callback = NULL;

    win->event_wpos = 0;
    win->event_rpos = 0;
    pthread_mutex_init(&win->event_mutex, NULL);

    if (dpy->shell) {
        create_xdg_surface(win, dpy);
    }

    return win;

error:
    if (wl_win) {
        wl_egl_window_destroy(wl_win);
    }

    if (surface) {
        wl_surface_destroy(surface);
    }

    return NULL;
}

static void vdk_destroy_window(vdkPrivate priv, struct vdk_window *win)
{
    /* Do destroy window. */
    wl_egl_window_destroy(win->wl_win);
    wl_surface_destroy(win->surface);

    if (win->callback) {
        wl_callback_destroy(win->callback);
    }

    free(win);
}


static struct vdk_pixmap * vdk_find_pixmap(vdkPrivate priv, struct wl_egl_pixmap * wl_pix)
{
    struct vdk_pixmap *pix;

    wl_list_for_each(pix, &priv->pix_list, link) {
        if (pix->wl_pix == wl_pix) {
            return pix;
        }
    }

    return NULL;
}

static struct vdk_pixmap * vdk_create_pixmap(vdkPrivate priv,
        struct vdk_display *dpy, int width, int height, int bpp)
{
    struct vdk_pixmap *pix;
    struct wl_egl_pixmap *wl_pix;

    wl_pix = wl_egl_pixmap_create(width, height, bpp);

    if (!wl_pix) {
        fprintf(stderr, "%s(%d): wl_egl_pixmap_create failed\n",
                __func__, __LINE__);
        return NULL;
    }

    pix = (struct vdk_pixmap *) malloc(sizeof (struct vdk_pixmap));
    if (!pix) {
        fprintf(stderr, "%s(%d): out of memory\n",
                __func__, __LINE__);
        wl_egl_pixmap_destroy(wl_pix);
        return NULL;
    }

    pix->display = dpy;
    pix->wl_pix  = wl_pix;
    pix->width   = width;
    pix->height  = height;
    pix->bpp     = bpp;

    return pix;
}

static void vdk_destroy_pixmap(vdkPrivate priv, struct vdk_pixmap *pix)
{
    /* Do destroy pixmap. */
    wl_egl_pixmap_destroy(pix->wl_pix);
    free(pix);
}

static struct vdk_display * vdk_find_display(vdkPrivate priv, struct wl_display *wl_dpy)
{
    struct vdk_display *dpy;

    wl_list_for_each(dpy, &priv->dpy_list, link) {
        if (dpy->wl_dpy == wl_dpy) {
            return dpy;
        }
    }

    return NULL;
}

static void
pointer_handle_enter(void *data, struct wl_pointer *pointer,
             uint32_t serial, struct wl_surface *surface,
             wl_fixed_t sx, wl_fixed_t sy)
{
    struct vdk_display *dpy = data;
    struct vdk_window *win;
    struct wl_buffer *buffer;
    struct wl_cursor *cursor = dpy->default_cursor;
    struct wl_cursor_image *image;

    if (cursor) {
        image = dpy->default_cursor->images[0];
        buffer = wl_cursor_image_get_buffer(image);
        if (!buffer)
            return;
        wl_pointer_set_cursor(pointer, serial,
                      dpy->cursor_surface,
                      image->hotspot_x,
                      image->hotspot_y);
        wl_surface_attach(dpy->cursor_surface, buffer, 0, 0);
        wl_surface_damage(dpy->cursor_surface, 0, 0,
                  image->width, image->height);
        wl_surface_commit(dpy->cursor_surface);
    }

    win = vdk_find_window_by_surface(_vdk, surface);

    if (win) {
        dpy->pointer_win = win;
    }
}

static void
pointer_handle_leave(void *data, struct wl_pointer *pointer,
             uint32_t serial, struct wl_surface *surface)
{
    struct vdk_display *dpy = data;
    struct vdk_window *win;

    win = vdk_find_window_by_surface(_vdk, surface);

    if (dpy->pointer_win == win) {
        dpy->pointer_win = NULL;
    }
}

static void
pointer_handle_motion(void *data, struct wl_pointer *pointer,
              uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
    struct vdk_display *dpy = data;
    struct vdk_window *win = dpy->pointer_win;
    vdkEvent *evt;

    if (!win) {
        return;
    }

    pthread_mutex_lock(&win->event_mutex);
    evt  = &win->event_queue[win->event_wpos];

    evt->type = VDK_POINTER;
    /* wl_fixed_t is 24.8 fixed. */
    evt->data.pointer.x = sx >> 8;
    evt->data.pointer.y = sy >> 8;

    win->motion_x = sx >> 8;
    win->motion_y = sy >> 8;
    win->event_wpos = (win->event_wpos + 1) % EVENT_QUEUE_SIZE;

    if (win->event_rpos == win->event_wpos) {
        /* remove oldest event. */
        win->event_rpos = (win->event_rpos + 1) % EVENT_QUEUE_SIZE;
    }
    pthread_mutex_unlock(&win->event_mutex);
}

static void
pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
              uint32_t serial, uint32_t time, uint32_t button,
              uint32_t state)
{
    struct vdk_display *dpy = data;
    struct vdk_window *win = dpy->pointer_win;
    vdkEvent *evt;

    if (!win) {
        return;
    }

    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
        pthread_mutex_lock(&win->event_mutex);
        evt  = &win->event_queue[win->event_wpos];

        evt->type = VDK_BUTTON;
        evt->data.button.left   = (button == BTN_LEFT);
        evt->data.button.middle = (button == BTN_MIDDLE);
        evt->data.button.right  = (button == BTN_RIGHT);
        evt->data.button.x      = win->motion_x;
        evt->data.button.y      = win->motion_y;

        win->event_wpos = (win->event_wpos + 1) % EVENT_QUEUE_SIZE;

        if (win->event_rpos == win->event_wpos) {
            /* remove oldest event. */
            win->event_rpos = (win->event_rpos + 1) % EVENT_QUEUE_SIZE;
        }
        pthread_mutex_unlock(&win->event_mutex);
    }

    if (win->xdg_surface &&
        button == BTN_LEFT &&
        state == WL_POINTER_BUTTON_STATE_PRESSED) {
        xdg_surface_move(win->xdg_surface,
                         dpy->seat, serial);
    }
}

static void
pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
            uint32_t time, uint32_t axis, wl_fixed_t value)
{
    struct vdk_display *dpy = data;
    struct vdk_window *win = dpy->pointer_win;

    if (!win) {
        return;
    }

    /* TODO: axis events? */
}

static const struct wl_pointer_listener pointer_listener = {
    pointer_handle_enter,
    pointer_handle_leave,
    pointer_handle_motion,
    pointer_handle_button,
    pointer_handle_axis,
};

static void
keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard,
               uint32_t format, int fd, uint32_t size)
{
}

static void
keyboard_handle_enter(void *data, struct wl_keyboard *keyboard,
              uint32_t serial, struct wl_surface *surface,
              struct wl_array *keys)
{
    struct vdk_display *dpy = data;
    struct vdk_window *win;

    win = vdk_find_window_by_surface(_vdk, surface);

    if (win) {
        dpy->keyboard_win = win;
    }
}

static void
keyboard_handle_leave(void *data, struct wl_keyboard *keyboard,
              uint32_t serial, struct wl_surface *surface)
{
    struct vdk_display *dpy = data;
    struct vdk_window *win;

    win = vdk_find_window_by_surface(_vdk, surface);

    if (dpy->keyboard_win == win) {
        dpy->keyboard_win = NULL;
    }
}

/* Structure that defined keyboard mapping. */
typedef struct _keyMap
{
    /* Normal key. */
    vdkKeys normal;

    /* Extended key. */
    vdkKeys extended;
}
keyMap;

static keyMap keys[] =
{
    /* 00 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 01 */ { VDK_ESCAPE,          VDK_UNKNOWN     },
    /* 02 */ { VDK_1,               VDK_UNKNOWN     },
    /* 03 */ { VDK_2,               VDK_UNKNOWN     },
    /* 04 */ { VDK_3,               VDK_UNKNOWN     },
    /* 05 */ { VDK_4,               VDK_UNKNOWN     },
    /* 06 */ { VDK_5,               VDK_UNKNOWN     },
    /* 07 */ { VDK_6,               VDK_UNKNOWN     },
    /* 08 */ { VDK_7,               VDK_UNKNOWN     },
    /* 09 */ { VDK_8,               VDK_UNKNOWN     },
    /* 0A */ { VDK_9,               VDK_UNKNOWN     },
    /* 0B */ { VDK_0,               VDK_UNKNOWN     },
    /* 0C */ { VDK_HYPHEN,          VDK_UNKNOWN     },
    /* 0D */ { VDK_EQUAL,           VDK_UNKNOWN     },
    /* 0E */ { VDK_BACKSPACE,       VDK_UNKNOWN     },
    /* 0F */ { VDK_TAB,             VDK_UNKNOWN     },
    /* 10 */ { VDK_Q,               VDK_UNKNOWN     },
    /* 11 */ { VDK_W,               VDK_UNKNOWN     },
    /* 12 */ { VDK_E,               VDK_UNKNOWN     },
    /* 13 */ { VDK_R,               VDK_UNKNOWN     },
    /* 14 */ { VDK_T,               VDK_UNKNOWN     },
    /* 15 */ { VDK_Y,               VDK_UNKNOWN     },
    /* 16 */ { VDK_U,               VDK_UNKNOWN     },
    /* 17 */ { VDK_I,               VDK_UNKNOWN     },
    /* 18 */ { VDK_O,               VDK_UNKNOWN     },
    /* 19 */ { VDK_P,               VDK_UNKNOWN     },
    /* 1A */ { VDK_LBRACKET,        VDK_UNKNOWN     },
    /* 1B */ { VDK_RBRACKET,        VDK_UNKNOWN     },
    /* 1C */ { VDK_ENTER,           VDK_PAD_ENTER   },
    /* 1D */ { VDK_LCTRL,           VDK_RCTRL       },
    /* 1E */ { VDK_A,               VDK_UNKNOWN     },
    /* 1F */ { VDK_S,               VDK_UNKNOWN     },
    /* 20 */ { VDK_D,               VDK_UNKNOWN     },
    /* 21 */ { VDK_F,               VDK_UNKNOWN     },
    /* 22 */ { VDK_G,               VDK_UNKNOWN     },
    /* 23 */ { VDK_H,               VDK_UNKNOWN     },
    /* 24 */ { VDK_J,               VDK_UNKNOWN     },
    /* 25 */ { VDK_K,               VDK_UNKNOWN     },
    /* 26 */ { VDK_L,               VDK_UNKNOWN     },
    /* 27 */ { VDK_SEMICOLON,       VDK_UNKNOWN     },
    /* 28 */ { VDK_SINGLEQUOTE,     VDK_UNKNOWN     },
    /* 29 */ { VDK_BACKQUOTE,       VDK_UNKNOWN     },
    /* 2A */ { VDK_LSHIFT,          VDK_UNKNOWN     },
    /* 2B */ { VDK_BACKSLASH,       VDK_UNKNOWN     },
    /* 2C */ { VDK_Z,               VDK_UNKNOWN     },
    /* 2D */ { VDK_X,               VDK_UNKNOWN     },
    /* 2E */ { VDK_C,               VDK_UNKNOWN     },
    /* 2F */ { VDK_V,               VDK_UNKNOWN     },
    /* 30 */ { VDK_B,               VDK_UNKNOWN     },
    /* 31 */ { VDK_N,               VDK_UNKNOWN     },
    /* 32 */ { VDK_M,               VDK_UNKNOWN     },
    /* 33 */ { VDK_COMMA,           VDK_UNKNOWN     },
    /* 34 */ { VDK_PERIOD,          VDK_UNKNOWN     },
    /* 35 */ { VDK_SLASH,           VDK_PAD_SLASH   },
    /* 36 */ { VDK_RSHIFT,          VDK_UNKNOWN     },
    /* 37 */ { VDK_PAD_ASTERISK,    VDK_PRNTSCRN    },
    /* 38 */ { VDK_LALT,            VDK_RALT        },
    /* 39 */ { VDK_SPACE,           VDK_UNKNOWN     },
    /* 3A */ { VDK_CAPSLOCK,        VDK_UNKNOWN     },
    /* 3B */ { VDK_F1,              VDK_UNKNOWN     },
    /* 3C */ { VDK_F2,              VDK_UNKNOWN     },
    /* 3D */ { VDK_F3,              VDK_UNKNOWN     },
    /* 3E */ { VDK_F4,              VDK_UNKNOWN     },
    /* 3F */ { VDK_F5,              VDK_UNKNOWN     },
    /* 40 */ { VDK_F6,              VDK_UNKNOWN     },
    /* 41 */ { VDK_F7,              VDK_UNKNOWN     },
    /* 42 */ { VDK_F8,              VDK_UNKNOWN     },
    /* 43 */ { VDK_F9,              VDK_UNKNOWN     },
    /* 44 */ { VDK_F10,             VDK_UNKNOWN     },
    /* 45 */ { VDK_NUMLOCK,         VDK_UNKNOWN     },
    /* 46 */ { VDK_SCROLLLOCK,      VDK_BREAK       },
    /* 47 */ { VDK_PAD_7,           VDK_HOME        },
    /* 48 */ { VDK_PAD_8,           VDK_UP          },
    /* 49 */ { VDK_PAD_9,           VDK_PGUP        },
    /* 4A */ { VDK_PAD_HYPHEN,      VDK_UNKNOWN     },
    /* 4B */ { VDK_PAD_4,           VDK_LEFT        },
    /* 4C */ { VDK_PAD_5,           VDK_UNKNOWN     },
    /* 4D */ { VDK_PAD_6,           VDK_RIGHT       },
    /* 4E */ { VDK_PAD_PLUS,        VDK_UNKNOWN     },
    /* 4F */ { VDK_PAD_1,           VDK_END         },
    /* 50 */ { VDK_PAD_2,           VDK_DOWN        },
    /* 51 */ { VDK_PAD_3,           VDK_PGDN        },
    /* 52 */ { VDK_PAD_0,           VDK_INSERT      },
    /* 53 */ { VDK_PAD_PERIOD,      VDK_DELETE      },
    /* 54 */ { VDK_SYSRQ,           VDK_UNKNOWN     },
    /* 55 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 56 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 57 */ { VDK_F11,             VDK_UNKNOWN     },
    /* 58 */ { VDK_F12,             VDK_UNKNOWN     },
    /* 59 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 5A */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 5B */ { VDK_UNKNOWN,         VDK_LWINDOW     },
    /* 5C */ { VDK_UNKNOWN,         VDK_RWINDOW     },
    /* 5D */ { VDK_UNKNOWN,         VDK_MENU        },
    /* 5E */ { VDK_UNKNOWN,         VDK_POWER       },
    /* 5F */ { VDK_UNKNOWN,         VDK_SLEEP       },
    /* 60 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 61 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 62 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 63 */ { VDK_UNKNOWN,         VDK_WAKE        },
    /* 64 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 65 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* For TTC Board only */
    /* 66 */ { VDK_HOME,            VDK_UNKNOWN     },
    /* 67 */ { VDK_UP,              VDK_UNKNOWN     },
    /* 68 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 69 */ { VDK_LEFT,            VDK_UNKNOWN     },
    /* 6A */ { VDK_RIGHT,           VDK_UNKNOWN     },
    /* 6B */ { VDK_ESCAPE,          VDK_UNKNOWN     },
    /* 6C */ { VDK_DOWN,            VDK_UNKNOWN     },
    /* 6D */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 6E */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 6F */ { VDK_BACKSPACE,       VDK_UNKNOWN     },
    /* End */
    /* 70 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 71 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 72 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 73 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 74 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 75 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 76 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 77 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 78 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 79 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7A */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7B */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7C */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7D */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7E */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7F */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
};

static void
keyboard_handle_key(void *data, struct wl_keyboard *keyboard,
            uint32_t serial, uint32_t time, uint32_t key,
            uint32_t state)
{
    struct vdk_display *dpy = data;
    struct vdk_window *win = dpy->keyboard_win;
    vdkEvent *evt;
    vdkKeys scancode;

    if (!win) {
        return;
    }

    pthread_mutex_lock(&win->event_mutex);
    evt  = &win->event_queue[win->event_wpos];

    /* TODO: Fix keyboard. */
    scancode = keys[key & 0x7F].normal;

    evt->type = VDK_KEYBOARD;
    evt->data.keyboard.scancode = scancode;
    evt->data.keyboard.key      = ((scancode < VDK_SPACE)
                                   || (scancode >= VDK_F1))
                                ? 0 : (char) scancode;
    evt->data.keyboard.pressed  = state;

    win->event_wpos = (win->event_wpos + 1) % EVENT_QUEUE_SIZE;

    if (win->event_rpos == win->event_wpos) {
        /* remove oldest event. */
        win->event_rpos = (win->event_rpos + 1) % EVENT_QUEUE_SIZE;
    }
    pthread_mutex_unlock(&win->event_mutex);
}

static void
keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard,
              uint32_t serial, uint32_t mods_depressed,
              uint32_t mods_latched, uint32_t mods_locked,
              uint32_t group)
{
    struct vdk_display *dpy = data;
    struct vdk_window *win = dpy->keyboard_win;

    if (!win) {
        return;
    }

    /* TODO: mofifiers. */
}

static const struct wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap,
    keyboard_handle_enter,
    keyboard_handle_leave,
    keyboard_handle_key,
    keyboard_handle_modifiers,
};

static void
seat_handle_capabilities(void *data, struct wl_seat *seat,
             enum wl_seat_capability caps)
{
    struct vdk_display *d = data;

    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !d->pointer) {
        d->pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(d->pointer, &pointer_listener, d);
    } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && d->pointer) {
        wl_pointer_destroy(d->pointer);
        d->pointer = NULL;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !d->keyboard) {
        d->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(d->keyboard, &keyboard_listener, d);
    } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && d->keyboard) {
        wl_keyboard_destroy(d->keyboard);
        d->keyboard = NULL;
    }
}

static const struct wl_seat_listener seat_listener = {
    seat_handle_capabilities,
};

static void
xdg_shell_ping(void *data, struct xdg_shell *shell, uint32_t serial)
{
    xdg_shell_pong(shell, serial);
}

static const struct xdg_shell_listener xdg_shell_listener = {
    xdg_shell_ping,
};

static void
output_geometry(void *data, struct wl_output *wl_output,
        int32_t x, int32_t y, int32_t physical_width, int32_t physical_height,
        int32_t subpixel, const char *make, const char *model, int32_t transform)
{
}

static void
output_mode(void *data, struct wl_output *wl_output,
        uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
    struct vdk_display *d = data;

    d->width   = width;
    d->height  = height;
    d->refresh = refresh;
}

static void output_done(void *data, struct wl_output *wl_output)
{
}

static void
output_scale(void *data, struct wl_output *wl_output, int32_t factor)
{
}

static const struct wl_output_listener output_listener = {
    output_geometry,
    output_mode,
    output_done,
    output_scale,
};

static void
registry_handle_global(void *data, struct wl_registry *registry,
               uint32_t name, const char *interface, uint32_t version)
{
    struct vdk_display *d = data;

    if (strcmp(interface, "wl_compositor") == 0) {
        d->compositor =
            wl_registry_bind(registry, name,
                     &wl_compositor_interface, 1);
    } else if (strcmp(interface, "xdg_shell") == 0) {
        detect_xdg_shell();

        if (xdg_shell_version == XDG_SHELL_VERSION_CURRENT_v4) {
            d->shell = wl_registry_bind(registry, name,
                            &xdg_shell_interface_v4, 1);
        } else if (xdg_shell_version == XDG_SHELL_VERSION_CURRENT_v5) {
            d->shell = wl_registry_bind(registry, name,
                            &xdg_shell_interface_v5, 1);
        }

        xdg_shell_add_listener(d->shell, &xdg_shell_listener, d);
        xdg_shell_use_unstable_version(d->shell, xdg_shell_version);
    } else if (strcmp(interface, "wl_seat") == 0) {
        d->seat = wl_registry_bind(registry, name,
                       &wl_seat_interface, 1);
        wl_seat_add_listener(d->seat, &seat_listener, d);
    } else if (strcmp(interface, "wl_shm") == 0) {
        d->shm = wl_registry_bind(registry, name,
                      &wl_shm_interface, 1);
        d->cursor_theme = wl_cursor_theme_load(NULL, 32, d->shm);
        if (!d->cursor_theme) {
            fprintf(stderr, "unable to load default theme\n");
            return;
        }
        d->default_cursor =
            wl_cursor_theme_get_cursor(d->cursor_theme, "left_ptr");
        if (!d->default_cursor) {
            fprintf(stderr, "unable to load default left pointer\n");
        }
    } else if (strcmp(interface, "wl_output") == 0) {
        d->output = wl_registry_bind(registry, name,
                       &wl_output_interface, 1);
        wl_output_add_listener(d->output, &output_listener, d);
    }
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
                  uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
    registry_handle_global,
    registry_handle_global_remove
};

static struct vdk_display * vdk_create_display(vdkPrivate priv, const char *name)
{
    struct wl_display *wl_dpy;
    struct vdk_display *dpy;

    wl_dpy = wl_display_connect(name);

    if (!wl_dpy) {
        fprintf(stderr, "%s(%d): wl_display_connect failed\n",
                __func__, __LINE__);
        return NULL;
    }

    dpy = (struct vdk_display *) malloc(sizeof (struct vdk_display));
    if (!dpy) {
        wl_display_disconnect(wl_dpy);
        fprintf(stderr, "%s(%d): out of memory\n",
                __func__, __LINE__);
        return NULL;
    }

    memset(dpy, 0, sizeof (struct vdk_display));
    wl_list_init(&dpy->win_list);
    wl_list_init(&dpy->pix_list);

    dpy->wl_dpy = wl_dpy;

    dpy->registry = wl_display_get_registry(wl_dpy);
    wl_registry_add_listener(dpy->registry,
                 &registry_listener, dpy);

    wl_display_roundtrip(wl_dpy);
    wl_display_dispatch(wl_dpy);

    return dpy;
}

static void vdk_destroy_display(vdkPrivate priv, struct vdk_display *dpy)
{
    struct vdk_window *w, *nextw;
    struct vdk_pixmap *p, *nextp;

    wl_list_for_each_safe(w, nextw, &dpy->win_list, link_in_dpy) {
        /* remove window in lists. */
        wl_list_remove(&w->link_in_dpy);
        wl_list_remove(&w->link);

        vdk_destroy_window(priv, w);
    }

    wl_list_for_each_safe(p, nextp, &dpy->pix_list, link_in_dpy) {
        /* remove pixmap in lists. */
        wl_list_remove(&p->link_in_dpy);
        wl_list_remove(&p->link);

        vdk_destroy_pixmap(priv, p);
    }

    /* do destroy display. */
    if (dpy->output) {
        wl_output_destroy(dpy->output);
    }

    if (dpy->cursor_surface) {
        wl_surface_destroy(dpy->cursor_surface);
    }

    if (dpy->cursor_theme)
        wl_cursor_theme_destroy(dpy->cursor_theme);

    if (dpy->shm) {
        wl_shm_destroy(dpy->shm);
    }

    if (dpy->compositor)
        wl_compositor_destroy(dpy->compositor);

    wl_registry_destroy(dpy->registry);
    wl_display_flush(dpy->wl_dpy);
    wl_display_disconnect(dpy->wl_dpy);

    free(dpy);
}

/*******************************************************************************
** Initialization.
*/

VDKAPI vdkPrivate VDKLANG
vdkInitialize(
    void
    )
{
    if (_vdk != NULL)
    {
        return _vdk;
    }

    _vdk = (vdkPrivate) malloc(sizeof(struct _vdkPrivate));

    if (_vdk == NULL)
    {
        return NULL;
    }

    wl_list_init(&_vdk->dpy_list);
    wl_list_init(&_vdk->win_list);
    wl_list_init(&_vdk->pix_list);

#if !gcdSTATIC_LINK
    _vdk->egl = dlopen("libEGL.so", RTLD_LAZY);
#endif

    return _vdk;
}

VDKAPI void VDKLANG
vdkExit(
    vdkPrivate Private
    )
{
    struct vdk_display *d, *next;

    if ((Private != _vdk) || (_vdk == NULL))
    {
        return;
    }

    wl_list_for_each_safe(d, next, &Private->dpy_list, link) {
        wl_list_remove(&d->link);
        vdk_destroy_display(_vdk, d);
    }

    if (_vdk->egl != NULL)
    {
        dlclose(_vdk->egl);
        _vdk->egl = NULL;
    }

    free(Private);
    _vdk = NULL;
}

/*******************************************************************************
** Display.
*/

VDKAPI vdkDisplay VDKLANG
vdkGetDisplayByIndex(
    vdkPrivate Private,
    int DisplayIndex
    )
{
    char name[64];
    struct vdk_display *dpy;

    snprintf(name, sizeof (name), "wayland-%d", DisplayIndex);

    /* create a new display. */
    dpy = vdk_create_display(Private, name);

    if (dpy != NULL) {
        wl_list_insert(&Private->dpy_list, &dpy->link);
        return dpy->wl_dpy;
    }

    return NULL;
}

VDKAPI vdkDisplay VDKLANG
vdkGetDisplay(
    vdkPrivate Private
    )
{
    struct vdk_display *dpy;

    if (Private != _vdk) {
        Private = _vdk;
    }

    dpy = vdk_create_display(Private, NULL);

    if (dpy != NULL) {
        wl_list_insert(&Private->dpy_list, &dpy->link);
        return dpy->wl_dpy;
    }

    return NULL;
}

VDKAPI int VDKLANG
vdkGetDisplayInfo(
    vdkDisplay Display,
    int * Width,
    int * Height,
    unsigned long * Physical,
    int * Stride,
    int * BitsPerPixel
    )
{
    struct vdk_display *dpy;

    if (!Display) {
        return 0;
    }

    dpy = vdk_find_display(_vdk, (struct wl_display *) Display);

    if (!dpy) {
        return 0;
    }

    if (Width) {
        *Width = dpy->width;
    }

    if (Height) {
        *Height = dpy->height;
    }

    if (Physical) {
        *Physical = ~0UL;
    }

    if (Stride) {
        *Stride = 0;
    }

    if (BitsPerPixel) {
        *BitsPerPixel = 0;
    }

    return 1;
}

VDKAPI void VDKLANG
vdkDestroyDisplay(
    vdkDisplay Display
    )
{
    struct vdk_display *dpy;

    if (!Display) {
        return;
    }

    dpy = vdk_find_display(_vdk, (struct wl_display *) Display);
    if (dpy) {
        wl_list_remove(&dpy->link);
        vdk_destroy_display(_vdk, dpy);
    }
}

/*******************************************************************************
** Windows
*/

vdkWindow
vdkCreateWindow(
    vdkDisplay Display,
    int X,
    int Y,
    int Width,
    int Height
    )
{
    struct vdk_display *dpy;
    struct vdk_window *win;

    dpy = vdk_find_display(_vdk, Display);

    if (!dpy) {
        return NULL;
    }

    if (Width == 0) {
        Width = dpy->width;
    }

    if (Height == 0) {
        Height = dpy->height;
    }

    if (X < 0) {
        X = (dpy->width - Width) / 2;
    }

    if (Y < 0) {
        Y = (dpy->height - Height) / 2;
    }

    win = vdk_create_window(_vdk, dpy, X, Y, Width, Height);

    if (!win) {
        return NULL;
    }

    if (win->xdg_surface) {
        xdg_surface_set_window_geometry(win->xdg_surface, X, Y, Width, Height);
    }

    if (!dpy->cursor_surface) {
        dpy->cursor_surface = wl_compositor_create_surface(dpy->compositor);
    }

    /* insert to lists. */
    wl_list_insert(&_vdk->win_list, &win->link);
    wl_list_insert(&dpy->win_list,  &win->link_in_dpy);

    return win->wl_win;
}

VDKAPI int VDKLANG
vdkGetWindowInfo(
    vdkWindow Window,
    int * X,
    int * Y,
    int * Width,
    int * Height,
    int * BitsPerPixel,
    unsigned int * Offset
    )
{
    struct vdk_window *win;

    if (BitsPerPixel) {
        return 0;
    }

    win = vdk_find_window(_vdk, Window);

    if (!win) {
        return 0;
    }

    if (X) {
        *X = win->x;
    }

    if (Y) {
        *Y = win->y;
    }

    if (Width) {
        *Width = win->width;
    }

    if (Height) {
        *Height = win->height;
    }

    if (Offset) {
        *Offset = 0;
    }

    return 0;
}

VDKAPI void VDKLANG
vdkDestroyWindow(
    vdkWindow Window
    )
{
    struct vdk_window *win;

    win = vdk_find_window(_vdk, Window);
    if (win) {
        /* remove window in lists. */
        wl_list_remove(&win->link_in_dpy);
        wl_list_remove(&win->link);

        if (win->display->pointer_win == win) {
            win->display->pointer_win = NULL;
        }

        if (win->display->keyboard_win == win) {
            win->display->keyboard_win = NULL;
        }

        vdk_destroy_window(_vdk, win);
    }
}

VDKAPI int VDKLANG
vdkShowWindow(
    vdkWindow Window
    )
{
    return 0;
}

VDKAPI int VDKLANG
vdkHideWindow(
    vdkWindow Window
    )
{
    return 0;
}

VDKAPI void VDKLANG
vdkSetWindowTitle(
    vdkWindow Window,
    const char * Title
    )
{
    struct vdk_window *win;

    win = vdk_find_window(_vdk, Window);

    if (win && win->xdg_surface) {
        xdg_surface_set_title(win->xdg_surface, Title);
    }
}

VDKAPI void VDKLANG
vdkCapturePointer(
    vdkWindow Window
    )
{
}

/*******************************************************************************
** Events.
*/
VDKAPI int VDKLANG
vdkGetEvent(
    vdkWindow Window,
    vdkEvent * Event
    )
{
    struct vdk_window *win;
    struct vdk_display * dpy;
    struct pollfd fds;
    int ret;

    win = vdk_find_window(_vdk, Window);
    if (!win) {
        return 0;
    }

    dpy = win->display;

    /* read and dispatch queued events if any. */
    /* wl_display_prepare_read: 0 on success and -1 if queue not empty. */
    while (wl_display_prepare_read(dpy->wl_dpy) != 0) {
        wl_display_dispatch_pending(dpy->wl_dpy);
    }

    fds.fd     = wl_display_get_fd(dpy->wl_dpy);
    fds.events = POLLIN | POLLPRI;

    /* poll with zero timeout. */
    ret = poll(&fds, 1, 0);

    if (ret > 0) {
        /* data comes. */
        wl_display_read_events(dpy->wl_dpy);
    } else {
        /* no data. */
        wl_display_cancel_read(dpy->wl_dpy);
    }

    wl_display_dispatch_pending(win->display->wl_dpy);

    pthread_mutex_lock(&win->event_mutex);

    if (win->event_rpos != win->event_wpos) {
        /* Peek event. */
        *Event = win->event_queue[win->event_rpos];
        win->event_rpos = (win->event_rpos + 1) % EVENT_QUEUE_SIZE;
        pthread_mutex_unlock(&win->event_mutex);
        return 1;
    }

    pthread_mutex_unlock(&win->event_mutex);
    return 0;
}

/*******************************************************************************
** EGL Support. ****************************************************************
*/

EGL_ADDRESS
vdkGetAddress(
    vdkPrivate Private,
    const char * Function
    )
{
#if gcdSTATIC_LINK
    return (EGL_ADDRESS) eglGetProcAddress(Function);
#else
    return (EGL_ADDRESS) dlsym(Private->egl, Function);
 #endif
}

/*******************************************************************************
** Time. ***********************************************************************
*/

/*
    vdkGetTicks

    Get the number of milliseconds since the system started.

    PARAMETERS:

        None.

    RETURNS:

        unsigned int
            The number of milliseconds the system has been running.
*/
VDKAPI unsigned int VDKLANG
vdkGetTicks(
    void
    )
{
    struct timeval tv;

    /* Return the time of day in milliseconds. */
    gettimeofday(&tv, 0);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/*******************************************************************************
** Pixmaps. ********************************************************************
*/

VDKAPI vdkPixmap VDKLANG
vdkCreatePixmap(
    vdkDisplay Display,
    int Width,
    int Height,
    int BitsPerPixel
    )
{
    struct vdk_display *dpy;
    struct vdk_pixmap *pix;

    dpy = vdk_find_display(_vdk, Display);

    if (!dpy) {
        return NULL;
    }

    pix = vdk_create_pixmap(_vdk, dpy, Width, Height, BitsPerPixel);

    if (!pix) {
        return NULL;
    }

    /* insert to lists. */
    wl_list_insert(&_vdk->pix_list, &pix->link);
    wl_list_insert(&dpy->pix_list, &pix->link_in_dpy);

    return pix->wl_pix;
}

VDKAPI int VDKLANG
vdkGetPixmapInfo(
    vdkPixmap Pixmap,
    int * Width,
    int * Height,
    int * BitsPerPixel,
    int * Stride,
    void ** Bits
    )
{
    struct vdk_pixmap *pix;

    pix = vdk_find_pixmap(_vdk, Pixmap);

    if (!pix) {
        return 0;
    }

    if (Width) {
        *Width = pix->width;
    }

    if (Height) {
        *Height = pix->height;
    }

    if (BitsPerPixel) {
        *BitsPerPixel = pix->bpp;
    }

    if (Stride || Bits) {
        int stride;
        void *bits;

        if (wl_egl_pixmap_get_pixels(pix->wl_pix, &bits, &stride) != 0) {
            return 0;
        }

        if (Stride) {
            *Stride = stride;
        }

        if (Bits) {
            *Bits = bits;
        }
    }

    return 1;
}

VDKAPI void VDKLANG
vdkDestroyPixmap(
    vdkPixmap Pixmap
    )
{
    struct vdk_pixmap *pix;

    pix = vdk_find_pixmap(_vdk, Pixmap);

    if (pix) {
        /* remove pixmap in lists. */
        wl_list_remove(&pix->link_in_dpy);
        wl_list_remove(&pix->link);

        vdk_destroy_pixmap(_vdk, pix);
    }
}

/*******************************************************************************
** ClientBuffers. **************************************************************
*/

VDKAPI vdkClientBuffer VDKLANG
vdkCreateClientBuffer(
    int Width,
    int Height,
    int Format,
    int Type
    )
{
    return NULL;
}

VDKAPI int VDKLANG
vdkGetClientBufferInfo(
    vdkClientBuffer ClientBuffer,
    int * Width,
    int * Height,
    int * Stride,
    void ** Bits
    )
{
    return 0;
}

VDKAPI int VDKLANG
vdkDestroyClientBuffer(
    vdkClientBuffer ClientBuffer
    )
{
    return 0;
}
