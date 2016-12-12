#ifndef GST_STREAMER_H
#define GST_STREAMER_H

#include <gst/gst.h>
#include <gst/gl/gl.h>

#include <QDebug>

#include <string>

#include "exception.h"

class GstStreamer
{
public:
    static GstStreamer& instance()
    {
        static GstStreamer singleton;
        return singleton;
    }

    enum class StreamInput
    {
        TEST_FILE,
        CAMERAS,
    };

    guint getTexture(short index) const
    {
        switch (index)
        {
        case 0:
            return video_0_buffer_;
        case 1:
            return video_1_buffer_;
        default:
            qDebug() << "GstStreamer::getTexture(): Invalid video index.";
            throw Exception("GstStreamer::getTexture()", "Invalid video index.");
        }
    }

    void createPipelines();
    void initialize(HGLRC gl_context, HDC device_context);

    void iterate() const
    {
        GMainContext *context = g_main_loop_get_context(loop_);
        g_main_context_iteration(context, FALSE);
    }

    void setVideoSize(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            qDebug() << "GstStreamer::setVideoSize(): Invalid video size (<= 0).";
            throw Exception("GstStreamer::setVideoSize()", "Invalid video size (<= 0).");
        }

        video_width_ = width;
        video_height_ = height;
    }

    void setVideoSource(StreamInput input)
    {
        input_ = input;
    }

protected:
    GstStreamer() {}
    GstStreamer(const GstStreamer &) = delete;
    void operator=(const GstStreamer &) = delete;

    // Gsreamer functions must be static!
    static gboolean syncBusCallback(GstBus *bus, GstMessage *msg, gpointer data);
    static gboolean update(void *sink);
    static guint fetchTexture(GstBuffer *buf);
    static void onGstBuffer(GstElement *fakesink, GstBuffer *buf, GstPad *pad, gpointer data);
    static void retrieveTexture(GstBuffer *buf, short index);

protected:
    const gchar *platform_{nullptr};

    static bool video_flipped_;
    static GstGLContext *gl_context_;
    static GstGLDisplay *gl_display_;
    static guint video_0_buffer_;
    static guint video_1_buffer_;
    static int video_height_;
    static int video_width_;

    GAsyncQueue *queue_input_buf_video_0_{nullptr};
    GAsyncQueue *queue_output_buf_video_0_{nullptr};
    GAsyncQueue *queue_input_buf_video_1_{nullptr};
    GAsyncQueue *queue_output_buf_video_1_{nullptr};
    GMainLoop *loop_{nullptr};
    GstBus *bus_video_0_{nullptr};
    GstBus *bus_video_1_{nullptr};
    GstElement *fakesink_video_0_{nullptr};
    GstElement *fakesink_video_1_{nullptr};
    GstPipeline *pipeline_video_0_{nullptr};
    GstPipeline *pipeline_video_1_{nullptr};
    GstState state_{GST_STATE_NULL};

    HDC hdc_{0};
    HGLRC hglrc_{0};

    StreamInput input_{StreamInput::TEST_FILE};
};

#endif // GST_STREAMER_H