#ifndef _COMMON_DRM_H_
#define _COMMON_DRM_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

struct omap_bo;

struct framebuffer_plane {
	uint32_t handle;
	uint32_t size;
	uint32_t stride;
	uint8_t *map;
	struct omap_bo *omap_bo;
};

struct framebuffer {
	int fd;

	uint32_t width;
	uint32_t height;
	uint32_t format;

	int num_planes;
	struct framebuffer_plane planes[4];

	uint32_t fb_id;
};

int drm_open_dev_dumb(const char *node);
void drm_create_dumb_fb(int fd, uint32_t width, uint32_t height, struct framebuffer *buf);
void drm_create_dumb_fb2(int fd, uint32_t width, uint32_t height, uint32_t format,
	struct framebuffer *buf);
void drm_destroy_dumb_fb(struct framebuffer *buf);
void drm_set_dpms(int fd, uint32_t conn_id, int dpms);

#define for_each_output(pos, head) \
	for (struct modeset_out *(pos) = (head); (pos); (pos) = (pos)->next)

uint32_t drm_reserve_plane(int fd);
void drm_release_plane(uint32_t plane_id);

#endif
