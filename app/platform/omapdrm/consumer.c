
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/queue.h>

#include "test.h"
#include "omap-prod-con.h"

#define MAX_QUEUED_BUFS 10

static const bool use_plane = false;

static struct {
	int drm_fd;
	int sfd;
	volatile struct shared_data *sdata;
} global;

struct received_fb {
	TAILQ_ENTRY(received_fb) entries;
	struct framebuffer *fb;
};

struct flip_data {
	unsigned num_frames_drawn;
	struct timespec flip_time;
	struct timespec draw_start_time;

	uint64_t min_flip_time, max_flip_time;

	TAILQ_HEAD(tailhead, received_fb) fb_list_head;
	struct framebuffer *current_fb, *queued_fb;

	uint32_t plane_id;
};

static struct modeset_out *modeset_list = NULL;

static void enqueue_fb(struct flip_data *priv, struct framebuffer *fb)
{
	struct received_fb *rfb = (struct received_fb *)malloc(sizeof(*rfb));
	rfb->fb = fb;

	TAILQ_INSERT_TAIL(&priv->fb_list_head, rfb, entries);
}

static struct framebuffer *dequeue_fb(struct flip_data *priv)
{
	struct received_fb *rfb;
	struct framebuffer *fb;

	rfb = TAILQ_FIRST(&priv->fb_list_head);
	TAILQ_REMOVE(&priv->fb_list_head, priv->fb_list_head.tqh_first, entries);

	fb = rfb->fb;
	free(rfb);

	return fb;
}

static int count_queued_fbs(struct flip_data *priv)
{
	int count = 0;
	struct received_fb *rfb;

	TAILQ_FOREACH(rfb, &priv->fb_list_head, entries)
		count++;

	return count;
}

static void update_queue_counts()
{
	volatile struct shared_data *sdata = global.sdata;

	int count = 0;

	for_each_output(out, modeset_list) {
		volatile struct shared_output *sout = &sdata->outputs[count++];

		int c = count_queued_fbs((struct flip_data *)out->data);
		sout->request_count = c >= MAX_QUEUED_BUFS ? 0 : MAX_QUEUED_BUFS - c;
	}

	//printf("C %d, %d\n",
	//	sdata->outputs[0].request_count,
	//	sdata->outputs[1].request_count);

	msync((void *)global.sdata, sizeof(struct shared_data), MS_SYNC);
}

static void queue_page_flip(struct modeset_out *out, struct framebuffer *fb)
{
	struct flip_data *priv = (struct flip_data *)out->data;
	int r;

	out->pflip_pending = true;
	priv->queued_fb = fb;

	r = drmModePageFlip(out->fd, out->crtc_id, fb->fb_id, DRM_MODE_PAGE_FLIP_EVENT, out);
	ASSERT(r == 0);
}

static void queue_plane(struct modeset_out *out, struct framebuffer *fb)
{
	struct flip_data *priv = (struct flip_data *)out->data;
	int r;

	int outw = fb->width * 2 / 3;
	int outh = fb->height * 2 / 3;
	int outx = (fb->width - outw) / 2;
	int outy = (fb->height - outh) / 2;

	r = drmModeSetPlane(out->fd, priv->plane_id, out->crtc_id,
		fb->fb_id, 0,
		outx, outy, outw, outh,
		0 << 16, 0 << 16,
		fb->width << 16, fb->height << 16);
	ASSERT(r == 0);

	out->pflip_pending = true;
	priv->queued_fb = fb;

	drmVBlank vbl;

	vbl.request.type = (drmVBlankSeqType)(DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT);
	vbl.request.type = (drmVBlankSeqType)(vbl.request.type | (out->crtc_idx << DRM_VBLANK_HIGH_CRTC_SHIFT));
	vbl.request.sequence = 1;
	vbl.request.signal = (unsigned long)out;

	drmWaitVBlank(global.drm_fd, &vbl);
}

static void modeset_page_flip_event(int fd, unsigned int frame,
				    unsigned int sec, unsigned int usec,
				    void *data)
{
	struct modeset_out *out = (struct modeset_out *)data;
	struct timespec now;
	struct flip_data *priv = (struct flip_data *)out->data;

	//printf("FLIP %d\n", out->output_id);

	if (priv->current_fb) {
		struct framebuffer *fb = priv->current_fb;
		int r;

		//printf("DELETE %d\n", fb->fb_id);

		r = drmModeRmFB(fb->fd, fb->fb_id);
		ASSERT(r == 0);

		free(priv->current_fb);
	}

	priv->current_fb = priv->queued_fb;
	priv->queued_fb = NULL;

	out->pflip_pending = false;

	if (out->cleanup)
		return;

	get_time_now(&now);

	/* initialize values on first flip */
	if (priv->num_frames_drawn == 0) {
		priv->min_flip_time = UINT64_MAX;
		priv->max_flip_time = 0;
		priv->draw_start_time = now;
		priv->flip_time = now;
	}

	/* measure min/max flip time */
	if (priv->num_frames_drawn > 0) {
		uint64_t us;

		us = get_time_elapsed_us(&priv->flip_time, &now);

		priv->flip_time = now;

		if (us < priv->min_flip_time)
			priv->min_flip_time = us;

		if (us > priv->max_flip_time)
			priv->max_flip_time = us;
	}

	const int measure_interval = 100;

	if (priv->num_frames_drawn > 0 &&
		priv->num_frames_drawn % measure_interval == 0) {
		uint64_t us;
		float flip_avg;

		us = get_time_elapsed_us(&priv->draw_start_time, &now);
		flip_avg = (float)us / measure_interval / 1000;

		printf("Output %u: flip avg/min/max %f/%f/%f\n",
			out->output_id,
			flip_avg,
			priv->min_flip_time / 1000.0,
			priv->max_flip_time / 1000.0);

		priv->draw_start_time = now;

		priv->min_flip_time = UINT64_MAX;
		priv->max_flip_time = 0;
	}

	priv->num_frames_drawn += 1;

	if (TAILQ_EMPTY(&priv->fb_list_head))
		return;

	//printf("flip: queue new pflig: %d\n", out->output_id);

	struct framebuffer *fb;
	fb = dequeue_fb(priv);

	if (use_plane) {
		queue_plane(out, fb);
	} else {
		queue_page_flip(out, fb);
	}

	update_queue_counts();
}

static void init_drm()
{
	const char *card = "/dev/dri/card0";

	global.drm_fd = drm_open_dev_dumb(card);
}

static void uninit_drm()
{
	close(global.drm_fd);
}

static void receive_fb(int sfd, int *output_id, struct framebuffer *fb)
{
	char buf[16];
	int prime_fd;
	int r;

	size_t size = sock_fd_read(sfd, buf, sizeof(buf), &prime_fd);
	ASSERT(size == 1);

	*output_id = buf[0];

	struct modeset_out *out = find_output(modeset_list, *output_id);
	ASSERT(out);

	int w = out->mode.hdisplay;
	int h = out->mode.vdisplay;

	ASSERT(w != 0 && h != 0);

	r = drmPrimeFDToHandle(global.drm_fd, prime_fd, &fb->planes[0].handle);
	ASSERT(r == 0);

	fb->fd = global.drm_fd;
	fb->num_planes = 1;

	fb->width = w;
	fb->height = h;
	fb->planes[0].stride = fb->width * 32 / 8;
	fb->planes[0].size = fb->planes[0].stride * fb->height;

	r = drmModeAddFB(global.drm_fd, fb->width, fb->height, 24, 32, fb->planes[0].stride,
		   fb->planes[0].handle, &fb->fb_id);
	ASSERT(r == 0);

	//printf("received fb handle %x, prime %d, fb %d\n", fb->planes[0].handle, prime_fd, fb->fb_id);

	r = close(prime_fd);
	ASSERT(r == 0);
}

static void main_loop(int sfd)
{
	printf("reading...\n");

	drmEventContext ev = {0};
	ev.version = DRM_EVENT_CONTEXT_VERSION;
	ev.page_flip_handler = modeset_page_flip_event;
	ev.vblank_handler = modeset_page_flip_event;

	update_queue_counts();

	fd_set fds;

	FD_ZERO(&fds);

	int drm_fd = global.drm_fd;

	int count = 0;

	while (true) {
		int r;

		FD_SET(0, &fds);
		FD_SET(drm_fd, &fds);
		FD_SET(sfd, &fds);

		int max_fd = sfd > drm_fd ? sfd : drm_fd;

		r = select(max_fd + 1, &fds, NULL, NULL, NULL);

		ASSERT(r >= 0);

		if (FD_ISSET(0, &fds)) {
			fprintf(stderr, "exit due to user-input\n");
			return;
		}

		if (FD_ISSET(drm_fd, &fds)) {
			//printf("drm event\n");
			drmHandleEvent(drm_fd, &ev);
		}

		if (FD_ISSET(sfd, &fds)) {
			struct framebuffer *fb = (struct framebuffer *)malloc(sizeof(*fb));
			int output_id;

			receive_fb(sfd, &output_id, fb);

			//printf("received fb %d, for output %d, handle %x\n", count, output_id, fb->handle);
			count++;

			struct modeset_out *out = find_output(modeset_list, output_id);
			struct flip_data *priv = (struct flip_data *)out->data;
			ASSERT(out);

			if (priv->queued_fb == NULL) {
				//printf("queue pflip %d\n", out->output_id);

				if (use_plane) {
					queue_plane(out, fb);
				} else {
					queue_page_flip(out, fb);
				}
			} else {
				enqueue_fb(priv, fb);
			}

			update_queue_counts();
		}
	}

	printf("done\n");

	for_each_output(out, modeset_list) {
		out->cleanup = true;

		if (!out->pflip_pending)
			continue;

		/* if a pageflip is pending, wait for it to complete */
		fprintf(stderr,
			"wait for pending page-flip to complete for output %d...\n",
			out->output_id);

		while (out->pflip_pending) {
			int r;
			r = drmHandleEvent(drm_fd, &ev);
			ASSERT(r == 0);
		}
	}
}

static void setup_config()
{
	int count = 0;

	volatile struct shared_data *sdata = global.sdata;

	for_each_output(out, modeset_list) {
		volatile struct shared_output *sout = &sdata->outputs[count++];

		sout->output_id = out->output_id;
		sout->width = out->mode.hdisplay;
		sout->height = out->mode.vdisplay;
		sout->request_count = 0;
	}

	global.sdata->num_outputs = count;

	msync((void *)global.sdata, sizeof(struct shared_data), MS_SYNC);
}

static int connect_to_producer()
{
	struct sockaddr_un addr = { 0 };
	int sfd;

	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT(sfd != 0);

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCKNAME);

	int r = connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
	ASSERT(r == 0);

	return sfd;
}

static void open_shared_mem()
{
	int fd;

	fd = shm_open(SHARENAME, O_RDWR, S_IRUSR | S_IWUSR);
	ASSERT(fd > 0);

	struct shared_data *sdata = (struct shared_data *)mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);
	ASSERT(sdata);

	global.sdata = sdata;
}

static void find_planes(int fd, struct modeset_out *modeset_list)
{
	drmModePlaneRes *plane_resources;
	unsigned cur_plane;

	plane_resources = drmModeGetPlaneResources(fd);
	ASSERT(plane_resources);

	cur_plane = 0;

	for_each_output(out, modeset_list) {
		struct flip_data *pdata = (struct flip_data *)out->data;

		for (; cur_plane < plane_resources->count_planes; cur_plane++) {
			drmModePlane *ovr;

			ovr = drmModeGetPlane(fd, plane_resources->planes[cur_plane]);
			ASSERT(ovr);

			printf("Output %d: Plane %d\n",
				out->output_id, ovr->plane_id);

			pdata->plane_id = ovr->plane_id;

			cur_plane++;

			drmModeFreePlane(ovr);

			break;
		}
	}

	drmModeFreePlaneResources(plane_resources);
}

int main(int argc, char **argv)
{
	int r;

	init_drm();

	// Prepare all connectors and CRTCs
	modeset_prepare(global.drm_fd, &modeset_list);

	// Allocate root buffers
	modeset_alloc_fbs(modeset_list, 1);

	// Draw test pattern
	for_each_output(out, modeset_list)
		drm_draw_test_pattern(&out->bufs[0], 0);

	// Allocate private data
	for_each_output(out, modeset_list) {
		struct flip_data *priv;
		priv = (struct flip_data *)calloc(1, sizeof(struct flip_data));
		TAILQ_INIT(&priv->fb_list_head);
		out->data = priv;
	}

	if (use_plane)
		find_planes(global.drm_fd, modeset_list);

	// Set modes
	modeset_set_modes(modeset_list);

	open_shared_mem();

	setup_config();

	int sfd = connect_to_producer();

	global.sfd = sfd;

	main_loop(sfd);

	// Free private data
	for_each_output(out, modeset_list)
		free(out->data);

	r = close(sfd);
	ASSERT(r == 0);

	modeset_cleanup(modeset_list);

	uninit_drm();

	return 0;
}
