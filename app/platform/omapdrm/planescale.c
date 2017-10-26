
#include "test.h"
#include <sys/time.h>

static struct modeset_out *modeset_list = NULL;

struct flip_data {
	uint32_t plane_id;
	struct framebuffer plane_buf;
	unsigned w, h;
};

static void find_planes(int fd, struct modeset_out *modeset_list)
{
	for_each_output(out, modeset_list) {
		struct flip_data *pdata = (struct flip_data *)out->data;

		uint32_t plane_id = drm_reserve_plane(fd);
		ASSERT(plane_id > 0);

		pdata->plane_id = plane_id;

		printf("Output %d: using plane %d\n",
			out->output_id, plane_id);
	}
}

static int color_flag = 0;
static struct timeval old_tm;
static struct timeval new_tm;
static int is_set_plane = 1;

int main(int argc, char **argv)
{
	int fd;
	int opt;
	const char *card = "/dev/dri/card0";

	while ((opt = getopt(argc, argv, "c:")) != -1) {
		switch (opt) {
		case 'c':
			card = optarg;
			break;
		}
	}

	// open the DRM device
	fd = drm_open_dev_dumb(card);

	// Prepare all connectors and CRTCs
	modeset_prepare(fd, &modeset_list);

	// Allocate buffers
	modeset_alloc_fbs(modeset_list, 2);

	// Allocate private data
	for_each_output(out, modeset_list)
		out->data = calloc(1, sizeof(struct flip_data));

	find_planes(fd, modeset_list);

	for_each_output(out, modeset_list) {
		struct flip_data *pdata = (struct flip_data *)out->data;

		drm_create_dumb_fb2(out->fd,
			//out->mode.hdisplay, out->mode.vdisplay,
			640, 576,
			DRM_FORMAT_NV12,
			&pdata->plane_buf);

		drm_draw_test_pattern(&pdata->plane_buf, 3);

		pdata->w = out->mode.hdisplay;
		pdata->h = out->mode.vdisplay;
	}

	// Set modes
	modeset_set_modes(modeset_list);

	gettimeofday(&new_tm, NULL);
	while (true) {

		for_each_output(out, modeset_list) {
			struct flip_data *pdata = (struct flip_data *)out->data;
			struct framebuffer *buf;

			buf = &pdata->plane_buf;

			if (pdata->w < buf->width / 4)
				pdata->w = buf->width;
			if (pdata->h < buf->height / 4)
				pdata->h = buf->height;

			/*printf("%d: %4dx%4d -> %4dx%4d (%1.2f x %1.2f)    ",
				out->output_id,
				buf->width, buf->height, pdata->w, pdata->h,
				(float)pdata->w / buf->width,
				(float)pdata->h / buf->height);*/
		}

		//printf("\n"); fflush(stdout);

		for_each_output(out, modeset_list) {
			struct flip_data *pdata = (struct flip_data *)out->data;
			struct framebuffer *buf;
			int r;

			buf = &pdata->plane_buf;

			if (is_set_plane) {
			r = drmModeSetPlane(out->fd, pdata->plane_id, out->crtc_id,
				buf->fb_id, 0,
				0, 0, pdata->w, pdata->h,
				0 << 16, 0 << 16,
				buf->width << 16, buf->height << 16);
			ASSERT(r == 0);
				is_set_plane = 0;
			}
		}

		for_each_output(out, modeset_list) {
			struct flip_data *pdata = (struct flip_data *)out->data;
			uint8_t *map = pdata->plane_buf.planes[0].map;
			uint32_t color = 0;
			if (color_flag) {
				color = MAKE_RGB(0, 0, 0);
			} else {
				color = MAKE_RGB(255, 255, 255);
			}
			color_flag = !color_flag;
			memset(map, color, 640 * 676 * 4);
			//pdata->w -= 1;
			//pdata->h -= 1;
		}

		//memcpy(&old_tm, &new_tm, sizeof(struct timeval));
		old_tm.tv_sec = new_tm.tv_sec;
		old_tm.tv_usec = new_tm.tv_usec;
		//usleep(1000 * 32);
		gettimeofday(&new_tm, NULL);
		long usec = new_tm.tv_usec - old_tm.tv_usec;
		long sec  = new_tm.tv_sec  - old_tm.tv_sec;
		if (usec < 0) {
			usec += 1000000;
			sec -= 1;
		}

		printf("time diff[%lu.%lu], old time[%lu.%lu], new time[%lu.%lu]\n",
			sec, usec, old_tm.tv_sec, old_tm.tv_usec, new_tm.tv_sec, new_tm.tv_usec);
	}

	// Free private data
	for_each_output(out, modeset_list)
		free(out->data);

	// Free modeset data
	modeset_cleanup(modeset_list);

	close(fd);

	fprintf(stderr, "exiting\n");

	return 0;
}
