#ifdef __cplusplus
extern "C" {
#endif

#include <gst/gst.h>
#include <stdio.h>

GstElement *funnel;
GstElement *pipeline;

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    GMainLoop *loop = (GMainLoop *)data;
    GstState oldestate, newstate;
    // if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_STATE_CHANGED){
    //   gst_message_parse_state_changed(msg, &oldestate, &newstate, nullptr);
    //   g_print("++++++++%s state from %d to %d",GST_MESSAGE_SRC_NAME(msg),  oldestate, newstate);
    // }
    // g_print("---------Receive msg on bus from (%s) type (%s)\n", GST_MESSAGE_SRC_NAME(msg),
    //                    GST_MESSAGE_TYPE_NAME(msg));
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        g_print(" End of stream from %s\n", GST_MESSAGE_SRC_NAME(msg));

        // gst_element_seek_simple(pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, 0);

        // if (gst_nvmessage_is_stream_eos(msg)) {
        //     guint stream_id;
        //     if (gst_nvmessage_parse_stream_eos(msg, &stream_id)) {
        //         g_print("Got EOS from stream %d", stream_id);
        //     }
        // }
        // else{
        //     g_print(" not stream eos");
        // }

        break;
    case GST_MESSAGE_WARNING: {
        gchar *debug;
        GError *error;
        gst_message_parse_warning(msg, &error, &debug);
        g_print("WARNING from element %s: %s\n", GST_OBJECT_NAME(msg->src), error->message);
        g_free(debug);
        g_print("Warning: %s\n", error->message);
        g_error_free(error);
        break;
    }
    case GST_MESSAGE_ERROR: {

        gchar *debug;
        GError *error;
        gst_message_parse_error(msg, &error, &debug);

        gchar *name = GST_OBJECT_NAME(msg->src);

        g_print("ERROR from element %s: %s\n", GST_OBJECT_NAME(msg->src), error->message);

        if (debug)
            g_print(" Error details: %s\n", debug);

        // g_free(name);

        g_free(debug);
        g_error_free(error);
        // g_main_loop_quit(loop);
        break;
    }
    case GST_MESSAGE_STATE_CHANGED: {
        GstState oldstate, newstate;
        gst_message_parse_state_changed(msg, &oldstate, &newstate, nullptr);
        // g_print("%s state change, ollld state: %s, new state: %s.\n", GST_MESSAGE_SRC_NAME(msg),
        //         gst_element_state_get_name(oldstate), gst_element_state_get_name(newstate));
        if (GST_ELEMENT(GST_MESSAGE_SRC(msg)) == pipeline) {
            switch (newstate) {
            case GST_STATE_PLAYING:
                g_print(" Pipeline running.\n");
                GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline-playing");

                break;
            case GST_STATE_PAUSED:
                if (oldstate == GST_STATE_PLAYING) {
                    g_print(" Pipeline paused.\n");
                }
                break;
            case GST_STATE_READY:
                GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline-ready");
                if (oldstate == GST_STATE_NULL) {
                    g_print(" Pipeline ready.\n");
                } else {
                    g_print(" Pipeline stopped.\n");
                }
                break;
            case GST_STATE_NULL:
                g_print(" Pipeline is null.\n");
                break;
            default:
                break;
            }
        }

        break;
    }
    case GST_MESSAGE_ELEMENT: {

        gchar *name = GST_OBJECT_NAME(msg->src);
        g_print("GST_MESSAGE_ELEMENT from element %s: %s\n", GST_OBJECT_NAME(msg->src),
                gst_structure_get_name(gst_message_get_structure(msg)));

        break;
    }
    default:
        break;
    }
    return true;
}

static gboolean seek_decode(gpointer data) {
    GstElement *bin = (GstElement *)data;
    gboolean ret = TRUE;

    g_print("set source bin to GST_STATE_NULL\n");

    gst_element_set_state(bin, GST_STATE_NULL);

    g_print("set source bin to GST_STATE_NULL end, %s\n", GST_ELEMENT_NAME(bin));

    // ret = gst_element_seek(bin, 1.0, GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH),
    //                        GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

    // if (!ret)
    //     GST_WARNING("Error in seeking pipeline\n");

    g_print("set source bin to GST_STATE_PLAYING\n");

    gst_element_set_state(bin, GST_STATE_PLAYING);

    g_print("set source bin to GST_STATE_PLAYING end\n");

    return FALSE;
}

static GstPadProbeReturn restart_stream_buf_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data) {
    GstEvent *event = GST_EVENT(info->data);

    if ((info->type & GST_PAD_PROBE_TYPE_EVENT_BOTH)) {

        if (GST_EVENT_TYPE(event) == GST_EVENT_EOS) {
            g_print("restart_stream_buf_probe eos.\n");
            g_timeout_add(5000, seek_decode, u_data);
            // return GST_PAD_PROBE_DROP;
        }

        switch (GST_EVENT_TYPE(event)) {
        case GST_EVENT_EOS:
            // g_print("GST_EVENT_EOS\n");
            /* QOS events from downstream sink elements cause decoder to drop
             * frames after looping the file since the timestamps reset to 0.
             * We should drop the QOS events since we have custom logic for
             * looping individual sources. */
        // case GST_EVENT_QOS:
        //     g_print("GST_EVENT_QOS\n");
        //     g_print("GST_PAD_PROBE_DROP\n");
        //     return GST_PAD_PROBE_DROP;
        // case GST_EVENT_SEGMENT:
        //     g_print("GST_EVENT_SEGMENT\n");
        //     g_print("GST_PAD_PROBE_DROP\n");
        //     return GST_PAD_PROBE_DROP;
        case GST_EVENT_FLUSH_START:
            g_print("GST_EVENT_FLUSH_START\n");
            g_print("GST_PAD_PROBE_DROP\n");
            return GST_PAD_PROBE_DROP;
        case GST_EVENT_FLUSH_STOP:
            g_print("GST_EVENT_FLUSH_STOP\n");
            g_print("GST_PAD_PROBE_DROP\n");
            return GST_PAD_PROBE_DROP;
        default:
            break;
        }
    }

    return GST_PAD_PROBE_OK;
}

static void cb_newpad(GstElement *decodebin, GstPad *pad, gpointer data) {
    GstCaps *caps = gst_pad_query_caps(pad, NULL);
    const GstStructure *str = gst_caps_get_structure(caps, 0);
    const gchar *name = gst_structure_get_name(str);

    if (!strncmp(name, "video", 5)) {
        GstPad *sinkpad = gst_element_get_request_pad(funnel, "sink_%u");
        if (gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK) {
            g_print("Failed to link decodebin to pipeline\n");
        } else {
            gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_EVENT_BOTH, restart_stream_buf_probe, decodebin, NULL);
        }
        gst_object_unref(sinkpad);
    }
}

int main(int argc, char *argv[]) {
    g_print("start\n");
    guint i;
    gst_init(&argc, &argv);
    pipeline = gst_pipeline_new("pipeline");
    funnel = gst_element_factory_make("funnel", NULL);
    GstElement *sink = gst_element_factory_make("xvimagesink", NULL);

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    if (pipeline == NULL) {
        g_print("Failed create element\n");
        return -1;
    }

    if (funnel == NULL) {
        g_print("Failed create element funnel\n");
        return -1;
    }

    if (sink == NULL) {
        g_print("Failed create element sink\n");
        return -1;
    }

    gchar name[32];

    sprintf(name, "bin%u", 0);
    GstElement *uribin = gst_element_factory_make("uridecodebin", NULL);
    if (uribin == NULL) {
        g_print("Failed create uridecodebin element\n");
        return -1;
    }

    // g_object_set(G_OBJECT(uribin), "uri",
    // "file:///home/ptop/Programs/resources/streams/vehicle_plate/plate_test.mp4",
    //              NULL);

    g_object_set(G_OBJECT(uribin), "uri", "rtsp://192.168.10.146:9554/camera/tuiliu_test_02", NULL);

    gst_bin_add(GST_BIN(pipeline), uribin);

    g_signal_connect(G_OBJECT(uribin), "pad-added", G_CALLBACK(cb_newpad), funnel);

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));

    guint bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    gst_bin_add_many(GST_BIN(pipeline), funnel, sink, NULL);

    g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);

    gst_element_link(funnel, sink);

    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);

    g_main_loop_run(loop);

    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
