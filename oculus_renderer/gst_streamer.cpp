#include "gst_streamer.h"

bool GstStreamer::video_flipped_{false};
GstGLContext *GstStreamer::gl_context_{nullptr};
GstGLDisplay *GstStreamer::gl_display_{nullptr};
guint GstStreamer::video_0_buffer_{0};
guint GstStreamer::video_1_buffer_{0};
int GstStreamer::video_height_{0};
int GstStreamer::video_width_{0};

void GstStreamer::initialize(HGLRC gl_context, HDC device_context)
{
    hglrc_ = gl_context;
    hdc_ = device_context;

    // Initialize Gstreamer
    gst_init(nullptr, nullptr);
    loop_ = g_main_loop_new(nullptr, FALSE);

    // Turn off OpenGL context
    wglMakeCurrent(0, 0);
    platform_ = "wgl";
    gl_display_ = gst_gl_display_new();
    gl_context_ = gst_gl_context_new_wrapped(gl_display_, (guintptr)hglrc_,
                                             gst_gl_platform_from_string(platform_),
                                             GST_GL_API_OPENGL);
}

void GstStreamer::createPipelines()
{
    if (input_ == StreamInput::TEST_FILE)
    {
        pipeline_video_0_ = GST_PIPELINE(gst_parse_launch("filesrc location=Video/SampleMP4/video.mp4 ! "
                                                          "qtdemux ! avdec_h264 ! videoconvert ! "
                                                          "glupload ! gleffects effect=0 ! "
                                                          "fakesink sync=1 name=video1", nullptr));

        pipeline_video_1_ = GST_PIPELINE(gst_parse_launch("filesrc location=Video/SampleMP4/video.mp4 ! "
                                                          "qtdemux ! avdec_h264 ! videoconvert ! "
                                                          "glupload ! gleffects effect=0 ! "
                                                          "fakesink sync=1 name=video2", nullptr));

        video_flipped_ = false;
    }
    else if (input_ == StreamInput::CAMERAS)
    {
        std::string pipe1{"udpsrc port=1235 caps=\"application/x-rtp, media=(string)video, "
                          "clock-rate=(int)90000, encoding-name=(string)H264\" ! "
                          "rtph264depay ! h264parse ! decodebin ! videoconvert ! "
                          "videoflip method=clockwise ! glupload ! "
                          "gleffects_identity ! identity name=video1 ! glimagesink sync=0"};

        std::string pipe2{"udpsrc port=1234 caps=\"application/x-rtp, media=(string)video, "
                          "clock-rate=(int)90000, encoding-name=(string)H264\" ! "
                          "rtph264depay ! h264parse ! decodebin ! videoconvert ! "
                          "videoflip method=clockwise ! glupload ! "
                          "gleffects_identity ! identity name=video2 ! glimagesink sync=0"};

        pipeline_video_0_ = GST_PIPELINE(gst_parse_launch(pipe1.c_str(), nullptr));
        pipeline_video_1_ = GST_PIPELINE(gst_parse_launch(pipe2.c_str(), nullptr));

        video_flipped_ = true;
    }

    bus_video_0_ = gst_pipeline_get_bus(GST_PIPELINE(pipeline_video_0_));
    bus_video_1_ = gst_pipeline_get_bus(GST_PIPELINE(pipeline_video_1_));

    gst_bus_add_signal_watch(bus_video_0_);
    gst_bus_enable_sync_message_emission(bus_video_0_);
    g_signal_connect(bus_video_0_, "sync-message", G_CALLBACK(syncBusCallback), this);
    gst_object_unref(bus_video_0_);

    gst_bus_add_signal_watch(bus_video_1_);
    gst_bus_enable_sync_message_emission(bus_video_1_);
    g_signal_connect(bus_video_1_, "sync-message", G_CALLBACK(syncBusCallback), this);
    gst_object_unref(bus_video_1_);

    if (input_ == StreamInput::TEST_FILE)
    {
        gst_element_set_state(GST_ELEMENT(pipeline_video_0_), GST_STATE_PAUSED);
        if (gst_element_get_state(GST_ELEMENT(pipeline_video_0_), &state_, nullptr,
                                  GST_CLOCK_TIME_NONE) != GST_STATE_CHANGE_SUCCESS)
        {
            qDebug() << "GstStreamer::createPipelines(): Failed to pause pipeline #0.";
            throw Exception("GstStreamer::createPipelines()", "Failed to pause pipeline #0.");
        }

        gst_element_set_state(GST_ELEMENT(pipeline_video_1_), GST_STATE_PAUSED);
        if (gst_element_get_state(GST_ELEMENT(pipeline_video_1_), &state_, nullptr,
                                  GST_CLOCK_TIME_NONE) != GST_STATE_CHANGE_SUCCESS)
        {
            qDebug() << "GstStreamer::createPipelines(): Failed to pause pipeline #1.";
            throw Exception("GstStreamer::createPipelines()", "Failed to pause pipeline #1.");
        }
    }
    else if (input_ == StreamInput::CAMERAS)
    {
        gst_element_set_state(GST_ELEMENT(pipeline_video_0_), GST_STATE_PAUSED);
        if (gst_element_get_state(GST_ELEMENT(pipeline_video_0_), &state_, nullptr,
                                  GST_CLOCK_TIME_NONE) != GST_STATE_CHANGE_NO_PREROLL)
        {
            qDebug() << "GstStreamer::createPipelines(): Failed to pause pipeline #0.";
            throw Exception("GstStreamer::createPipelines()", "Failed to pause pipeline #0.");
        }

        gst_element_set_state(GST_ELEMENT(pipeline_video_1_), GST_STATE_PAUSED);
        if (gst_element_get_state(GST_ELEMENT(pipeline_video_1_), &state_, nullptr,
                                  GST_CLOCK_TIME_NONE) != GST_STATE_CHANGE_NO_PREROLL)
        {
            qDebug() << "GstStreamer::createPipelines(): Failed to pause pipeline #1.";
            throw Exception("GstStreamer::createPipelines()", "Failed to pause pipeline #1.");
        }
    }

    wglMakeCurrent(hdc_, hglrc_);

    fakesink_video_0_ = gst_bin_get_by_name(GST_BIN(pipeline_video_0_), "video1");
    fakesink_video_1_ = gst_bin_get_by_name(GST_BIN(pipeline_video_1_), "video2");

    g_object_set(G_OBJECT(fakesink_video_0_), "signal-handoffs", TRUE, NULL);
    g_object_set(G_OBJECT(fakesink_video_1_), "signal-handoffs", TRUE, NULL);

    g_signal_connect(fakesink_video_0_, "handoff", G_CALLBACK(onGstBuffer), this);
    g_signal_connect(fakesink_video_1_, "handoff", G_CALLBACK(onGstBuffer), this);

    queue_input_buf_video_0_ = g_async_queue_new();
    queue_output_buf_video_0_ = g_async_queue_new();
    queue_input_buf_video_1_ = g_async_queue_new();
    queue_output_buf_video_1_ = g_async_queue_new();

    g_object_set_data(G_OBJECT(fakesink_video_0_), "queue_input_buf", queue_input_buf_video_0_);
    g_object_set_data(G_OBJECT(fakesink_video_0_), "queue_output_buf", queue_output_buf_video_0_);
    g_object_set_data(G_OBJECT(fakesink_video_0_), "loop", loop_);

    g_object_set_data(G_OBJECT(fakesink_video_1_), "queue_input_buf", queue_input_buf_video_1_);
    g_object_set_data(G_OBJECT(fakesink_video_1_), "queue_output_buf", queue_output_buf_video_1_);
    g_object_set_data(G_OBJECT(fakesink_video_1_), "loop", loop_);

    gst_object_unref(fakesink_video_0_);
    gst_object_unref(fakesink_video_1_);

    gst_element_set_state(GST_ELEMENT(pipeline_video_0_), GST_STATE_PLAYING);
    gst_element_set_state(GST_ELEMENT(pipeline_video_1_), GST_STATE_PLAYING);
}

gboolean GstStreamer::syncBusCallback(GstBus *bus, GstMessage *msg, gpointer data)
{
    Q_UNUSED(bus);
    Q_UNUSED(data);

    switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_NEED_CONTEXT:
    {
        const gchar *context_type{nullptr};

        gst_message_parse_context_type(msg, &context_type);

        if (g_strcmp0(context_type, GST_GL_DISPLAY_CONTEXT_TYPE) == 0)
        {
            GstContext *display_context = gst_context_new(GST_GL_DISPLAY_CONTEXT_TYPE, TRUE);
            gst_context_set_gl_display(display_context, gl_display_);
            gst_element_set_context(GST_ELEMENT(msg->src), display_context);
            return TRUE;
        }
        else if (g_strcmp0(context_type, "gst.gl.app_context") == 0)
        {
            GstContext *app_context = gst_context_new("gst.gl.app_context", TRUE);
            GstStructure *s = gst_context_writable_structure(app_context);
            gst_structure_set(s, "context", GST_GL_TYPE_CONTEXT, gl_context_, NULL);
            gst_element_set_context(GST_ELEMENT(msg->src), app_context);
            return TRUE;
        }
        break;
    }
    default:
        break;
    }

    return FALSE;
}

void GstStreamer::onGstBuffer(GstElement *fakesink, GstBuffer *buf, GstPad *pad, gpointer data)
{
    Q_UNUSED(pad);
    Q_UNUSED(data);

    GAsyncQueue *queue_input_buf{nullptr};
    GAsyncQueue *queue_output_buf{nullptr};

    gst_buffer_ref(buf);
    queue_input_buf = (GAsyncQueue*)g_object_get_data(G_OBJECT(fakesink), "queue_input_buf");
    g_async_queue_push(queue_input_buf, buf);
    if (g_async_queue_length(queue_input_buf) > 3)
        g_idle_add(update, (gpointer)fakesink);

    queue_output_buf = (GAsyncQueue*)g_object_get_data(G_OBJECT(fakesink), "queue_output_buf");
    if (g_async_queue_length(queue_output_buf) > 3)
    {
        GstBuffer *buf_old = (GstBuffer*)g_async_queue_pop(queue_output_buf);
        gst_buffer_unref(buf_old);
    }
}

gboolean GstStreamer::update(void *sink)
{
    GstElement *fakesink = (GstElement*)sink;
    GAsyncQueue *queue_input_buf = (GAsyncQueue*)g_object_get_data(G_OBJECT(fakesink),
                                                                   "queue_input_buf");
    GAsyncQueue *queue_output_buf = (GAsyncQueue*)g_object_get_data(G_OBJECT(fakesink),
                                                                    "queue_output_buf");
    GstBuffer *buf = (GstBuffer*)g_async_queue_pop(queue_input_buf);

    std::string name(fakesink->object.name);

    if (name.c_str() == std::string("video1"))
        retrieveTexture(buf, 0);
    else if (name.c_str() == std::string("video2"))
        retrieveTexture(buf, 1);
    else
    {
        qDebug() << "GstStreamer::update(): Invalid video index.";
        throw Exception("GstStreamer::update()", "Invalid video index.");
    }

    g_async_queue_push(queue_output_buf, buf);

    return FALSE;
}

void GstStreamer::retrieveTexture(GstBuffer *buf, short index)
{
    switch (index)
    {
    case 0:
        video_0_buffer_ = fetchTexture(buf);
        break;
    case 1:
        video_1_buffer_ = fetchTexture(buf);
        break;
    }
}

guint GstStreamer::fetchTexture(GstBuffer *buf)
{
    GstVideoFrame video_frame;
    GstVideoInfo video_info;

    if (!video_flipped_)
        gst_video_info_set_format(&video_info, GST_VIDEO_FORMAT_RGBA, video_width_, video_height_);
    else
        gst_video_info_set_format(&video_info, GST_VIDEO_FORMAT_RGBA, video_height_, video_width_);

    if (!gst_video_frame_map(&video_frame, &video_info, buf,
                             static_cast<GstMapFlags>(GST_MAP_READ | GST_MAP_GL)))
    {
        qDebug() << "GstStreamer::fetchTexture(): Failed to fetch texture.";
        return 0;
    }

    guint texture = *(guint*)video_frame.data[0];
    gst_video_frame_unmap(&video_frame);

    return texture;
}