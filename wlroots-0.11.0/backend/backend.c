#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <libinput.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wayland-server-core.h>
#include <wlr/backend/drm.h>
#include <wlr/backend/headless.h>
#include <wlr/backend/interface.h>
#include <wlr/backend/libinput.h>
#include <wlr/backend/multi.h>
#include <wlr/backend/noop.h>
#include <wlr/backend/session.h>
#include <wlr/backend/wayland.h>
#include <wlr/config.h>
#include <wlr/util/log.h>
#include "backend/multi.h"

#if WLR_HAS_FBDEV_BACKEND
#include <wlr/backend/fbdev.h>
#endif

#if WLR_HAS_DBUS_BACKEND
#include <wlr/backend/dbus.h>
#endif

#if WLR_HAS_X11_BACKEND
#include <wlr/backend/x11.h>
#endif

void wlr_backend_init(struct wlr_backend *backend,
		const struct wlr_backend_impl *impl) {
	assert(backend);
	backend->impl = impl;
	wl_signal_init(&backend->events.destroy);
	wl_signal_init(&backend->events.new_input);
	wl_signal_init(&backend->events.new_output);
}

bool wlr_backend_start(struct wlr_backend *backend) {
	if (backend->impl->start) {
		return backend->impl->start(backend);
	}
	return true;
}

void wlr_backend_destroy(struct wlr_backend *backend) {
	if (!backend) {
		return;
	}

	if (backend->impl && backend->impl->destroy) {
		backend->impl->destroy(backend);
	} else {
		free(backend);
	}
}

struct wlr_renderer *wlr_backend_get_renderer(struct wlr_backend *backend) {
	if (backend->impl->get_renderer) {
		return backend->impl->get_renderer(backend);
	}
	return NULL;
}

struct wlr_session *wlr_backend_get_session(struct wlr_backend *backend) {
	if (backend->impl->get_session) {
		return backend->impl->get_session(backend);
	}
	return NULL;
}

clockid_t wlr_backend_get_presentation_clock(struct wlr_backend *backend) {
	if (backend->impl->get_presentation_clock) {
		return backend->impl->get_presentation_clock(backend);
	}
	return CLOCK_MONOTONIC;
}

static size_t parse_outputs_env(const char *name) {
	const char *outputs_str = getenv(name);
	if (outputs_str == NULL) {
		return 1;
	}

	char *end;
	int outputs = (int)strtol(outputs_str, &end, 10);
	if (*end || outputs < 0) {
		wlr_log(WLR_ERROR, "%s specified with invalid integer, ignoring", name);
		return 1;
	}

	return outputs;
}

static struct wlr_backend *attempt_wl_backend(struct wl_display *display,
		wlr_renderer_create_func_t create_renderer_func) {
	struct wlr_backend *backend = wlr_wl_backend_create(display, NULL, create_renderer_func);
	if (backend == NULL) {
		return NULL;
	}

	size_t outputs = parse_outputs_env("WLR_WL_OUTPUTS");
	for (size_t i = 0; i < outputs; ++i) {
		wlr_wl_output_create(backend);
	}

	return backend;
}

#if WLR_HAS_X11_BACKEND
static struct wlr_backend *attempt_x11_backend(struct wl_display *display,
		const char *x11_display, wlr_renderer_create_func_t create_renderer_func) {
	struct wlr_backend *backend = wlr_x11_backend_create(display, x11_display, create_renderer_func);
	if (backend == NULL) {
		return NULL;
	}

	size_t outputs = parse_outputs_env("WLR_X11_OUTPUTS");
	for (size_t i = 0; i < outputs; ++i) {
		wlr_x11_output_create(backend);
	}

	return backend;
}
#endif

static struct wlr_backend *attempt_headless_backend(
		struct wl_display *display, wlr_renderer_create_func_t create_renderer_func) {
	struct wlr_backend *backend = wlr_headless_backend_create(display, create_renderer_func);
	if (backend == NULL) {
		return NULL;
	}

	size_t outputs = parse_outputs_env("WLR_HEADLESS_OUTPUTS");
	for (size_t i = 0; i < outputs; ++i) {
		wlr_headless_add_output(backend, 1280, 720);
	}

	return backend;
}

static struct wlr_backend *attempt_noop_backend(struct wl_display *display) {
	struct wlr_backend *backend = wlr_noop_backend_create(display);
	if (backend == NULL) {
		return NULL;
	}

	size_t outputs = parse_outputs_env("WLR_NOOP_OUTPUTS");
	for (size_t i = 0; i < outputs; ++i) {
		wlr_noop_add_output(backend);
	}

	return backend;
}

static struct wlr_backend *attempt_drm_backend(struct wl_display *display,
		struct wlr_backend *backend, struct wlr_session *session,
		wlr_renderer_create_func_t create_renderer_func) {
	int gpus[8];
	size_t num_gpus = wlr_session_find_gpus(session, 8, gpus);
	struct wlr_backend *primary_drm = NULL;
	wlr_log(WLR_INFO, "Found %zu GPUs", num_gpus);

	for (size_t i = 0; i < num_gpus; ++i) {
		struct wlr_backend *drm = wlr_drm_backend_create(display, session,
			gpus[i], primary_drm, create_renderer_func);
		if (!drm) {
			wlr_log(WLR_ERROR, "Failed to open DRM device %d", gpus[i]);
			continue;
		}

		if (!primary_drm) {
			primary_drm = drm;
		}

		wlr_multi_backend_add(backend, drm);
	}

	return primary_drm;
}

#if WLR_HAS_FBDEV_BACKEND
#define MAX_DEVICES 32
static struct wlr_backend *attempt_fbdev_backend(struct wl_display *display,
		struct wlr_backend *backend, struct wlr_session *session,
		wlr_renderer_create_func_t create_renderer_func) {
	int fbdevs[MAX_DEVICES];
	struct wlr_backend *primary_fbdev = NULL;
	char* shmfb = getenv("WLR_SHMDEV");
	char* devs = getenv("WLR_FBDEVS");
	size_t num_fbdevs = 0;
	if (devs != NULL) {
	    char *istr;
	    istr = strtok (devs, ",");
	    while (istr != NULL)
	    {
		int tmp = open(istr, O_RDWR, S_IRWXU);
		if (tmp >= 0)
		{
		    fbdevs[num_fbdevs] = tmp;
		    num_fbdevs++;
		    wlr_log(WLR_INFO, "Found %s framebuffer devices", istr);
		}
		istr = strtok (NULL, " ");
	    }
	}
	if (shmfb != NULL) {
	    fbdevs[num_fbdevs] = shm_open(shmfb, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG);
	    	    wlr_log(WLR_INFO, "Created %s shmframebuffer device", shmfb);
	    num_fbdevs ++;
	}

	for (size_t i = 0; i < num_fbdevs; i++) {
		struct wlr_backend *fbdev = wlr_fbdev_backend_create(display,
			fbdevs[i], primary_fbdev,
			create_renderer_func);
		if (!fbdev) {
			wlr_log(WLR_ERROR, "Failed to open framebuffer device %d",
				fbdevs[i]);
			continue;
		}

		if (!primary_fbdev) {
			primary_fbdev = fbdev;
		}

		// Each framebuffer backend has one output
		wlr_fbdev_add_output(fbdev, 0, 0);
	}

	return primary_fbdev;
}
#endif

static struct wlr_backend *attempt_backend_by_name(struct wl_display *display,
		struct wlr_backend *backend, struct wlr_session **session,
		const char *name, wlr_renderer_create_func_t create_renderer_func) {
	if (strcmp(name, "wayland") == 0) {
		return attempt_wl_backend(display, create_renderer_func);
#if WLR_HAS_X11_BACKEND
	} else if (strcmp(name, "x11") == 0) {
		return attempt_x11_backend(display, NULL, create_renderer_func);
#endif
	} else if (strcmp(name, "headless") == 0) {
		return attempt_headless_backend(display, create_renderer_func);
	} else if (strcmp(name, "noop") == 0) {
		return attempt_noop_backend(display);
	} else if (strcmp(name, "drm") == 0 || strcmp(name, "libinput") == 0) {
		// DRM, libinput and fbdev need a session
		if (!*session) {
			*session = wlr_session_create(display);
			if (!*session) {
				wlr_log(WLR_ERROR, "failed to start a session");
				return NULL;
			}
		}

		if (strcmp(name, "libinput") == 0) {
			return wlr_libinput_backend_create(display, *session);
		} else if (strcmp(name, "drm") == 0) {
			return attempt_drm_backend(display, backend, *session, create_renderer_func);
		}
#if WLR_HAS_FBDEV_BACKEND
	} else if (strcmp(name, "fbdev") == 0) {
		return attempt_fbdev_backend(display, backend, NULL,
			create_renderer_func);
#endif
#if WLR_HAS_DBUS_BACKEND
	} else if (strcmp(name, "dbus") == 0) {
		return wlr_dbus_backend_create(display);
#endif
	}

	wlr_log(WLR_ERROR, "unrecognized backend '%s'", name);
	return NULL;
}

struct wlr_backend *wlr_backend_autocreate(struct wl_display *display,
		wlr_renderer_create_func_t create_renderer_func) {
	struct wlr_backend *backend = wlr_multi_backend_create(display);
	struct wlr_multi_backend *multi = (struct wlr_multi_backend *)backend;
	if (!backend) {
		wlr_log(WLR_ERROR, "could not allocate multibackend");
		return NULL;
	}

	char *names = getenv("WLR_BACKENDS");
	if (names) {
		names = strdup(names);
		if (names == NULL) {
			wlr_log(WLR_ERROR, "allocation failed");
			wlr_backend_destroy(backend);
			return NULL;
		}

		char *saveptr;
		char *name = strtok_r(names, ",", &saveptr);
		while (name != NULL) {
			struct wlr_backend *subbackend = attempt_backend_by_name(display,
				backend, &multi->session, name, create_renderer_func);
			if (subbackend == NULL) {
				wlr_log(WLR_ERROR, "failed to start backend '%s'", name);
				wlr_session_destroy(multi->session);
				wlr_backend_destroy(backend);
				free(names);
				return NULL;
			}

			if (!wlr_multi_backend_add(backend, subbackend)) {
				wlr_log(WLR_ERROR, "failed to add backend '%s'", name);
				wlr_session_destroy(multi->session);
				wlr_backend_destroy(backend);
				free(names);
				return NULL;
			}

			name = strtok_r(NULL, ",", &saveptr);
		}

		free(names);
		return backend;
	}

	if (getenv("WAYLAND_DISPLAY") || getenv("_WAYLAND_DISPLAY") ||
			getenv("WAYLAND_SOCKET")) {
		struct wlr_backend *wl_backend = attempt_wl_backend(display,
			create_renderer_func);
		if (!wl_backend) {
			goto error;
		}

		wlr_multi_backend_add(backend, wl_backend);
		return backend;
	}

#if WLR_HAS_X11_BACKEND
	const char *x11_display = getenv("DISPLAY");
	if (x11_display) {
		struct wlr_backend *x11_backend =
			attempt_x11_backend(display, x11_display, create_renderer_func);
		if (!x11_backend) {
			goto error;
		}

		wlr_multi_backend_add(backend, x11_backend);
		return backend;
	}
#endif

	// Attempt DRM+libinput
	multi->session = wlr_session_create(display);
	if (!multi->session) {
		wlr_log(WLR_ERROR, "Failed to start a DRM session");
		wlr_backend_destroy(backend);
		return NULL;
	}

	struct wlr_backend *libinput = wlr_libinput_backend_create(display,
		multi->session);
	if (!libinput) {
		wlr_log(WLR_ERROR, "Failed to start libinput backend");
		wlr_session_destroy(multi->session);
		wlr_backend_destroy(backend);
		return NULL;
	}
	wlr_multi_backend_add(backend, libinput);

	struct wlr_backend *primary_drm = attempt_drm_backend(display, backend,
		multi->session, create_renderer_func);
	if (!primary_drm) {
		wlr_log(WLR_ERROR, "Failed to open any DRM device");
		wlr_backend_destroy(libinput);
		wlr_session_destroy(multi->session);
		wlr_backend_destroy(backend);
		return NULL;
	}

	return backend;

error:
	wlr_backend_destroy(backend);
	return NULL;
}
