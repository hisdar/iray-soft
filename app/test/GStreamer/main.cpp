#include <stdio.h>
#include <gst/gst.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

void print_element_factory_info(GstElementFactoryListType type);
void print_elements_info();

int main(int argc, char *argv[])
{
	/* Initialize GStreamer */  
	gst_init (&argc, &argv);

	GstElement *pipe_line;
	GstElement *app_src;
	GstElement *file_sink;

	/* Create the elements */
	app_src = gst_element_factory_make("appsrc", "source");
	file_sink = gst_element_factory_make("filesink", "sink");

	/* Create the empty pipeline */  
	pipe_line = gst_pipeline_new("test-pipeline");

	if (!app_src || !file_sink || !pipe_line) {
		printf("[ERR][%d]make src or sink or pipe line fail\n", __LINE__);
		return 0;
	}

	GstCaps *src_caps = gst_caps_new_simple("video/x-raw",
							"format", G_TYPE_STRING, "RGB",
							"width",  G_TYPE_INT, 640,
							"height", G_TYPE_INT, 576,
							"framerate", GST_TYPE_FRACTION, 25, 1,
							 NULL);

	g_object_set(G_OBJECT(app_src), "caps", src_caps, NULL);


	/* Build the pipeline. and link source and sinl. */  
	gst_bin_add_many(GST_BIN(pipe_line), app_src, file_sink, NULL);
	if (!gst_element_link (app_src, file_sink)) {
		printf("Elements could not be linked.\n");
		gst_object_unref (pipe_line);
		return -1;
	}

	return 0;
}

void print_elements_info()
{
	GstElementFactoryListType types[] = {
		GST_ELEMENT_FACTORY_TYPE_SRC,
		GST_ELEMENT_FACTORY_TYPE_ENCODER,
		GST_ELEMENT_FACTORY_TYPE_SINK,
		GST_ELEMENT_FACTORY_TYPE_PARSER,
	};

	for (unsigned i = 0; i < ARRAY_SIZE(types); i++) {
		printf("*********************************\n");
		print_element_factory_info(types[i]);
	}
}

void print_element_factory_info(GstElementFactoryListType type)
{
	GstRank minrank = GST_RANK_NONE;

	GList *g_list = gst_element_factory_list_get_elements(type, minrank);
	while (g_list != NULL) {
		GstElementFactory *factory = (GstElementFactory *)(g_list->data);
		printf("icon name  :%s\n", gst_element_factory_get_icon_name(factory));
		printf("long name  :%s\n", gst_element_factory_get_longname(factory));
		printf("author     :%s\n", gst_element_factory_get_author(factory));
		printf("klass      :%s\n", gst_element_factory_get_klass(factory));
		printf("description:%s\n", gst_element_factory_get_description(factory));
		printf("dec uri    :%s\n", gst_element_factory_get_documentation_uri(factory));
		printf("===========================================\n");
		g_list = g_list->next;
	}

	gst_plugin_feature_list_free(g_list);
}
