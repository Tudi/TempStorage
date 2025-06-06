#include <gst/gst.h>
#include <gst/gstpadtemplate.h>
#include <gst/gstelement.h>
#include <gst/rtp/gstrtpbuffer.h>

#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include "gstreamer.h"
#include "util.h"

GST_DEBUG_CATEGORY_STATIC(rawtortp_debug_category);
#define GST_CAT_DEFAULT rawtortp_debug_category

#define PACKAGE "rawtortp"

static gboolean plugin_init(GstPlugin* plugin) {
#ifdef _DEBUG
    g_print("rawtortp: plugin_init() starting\n");
    fprintf(stderr, "plugin_init() running...\n");
#endif

    // allow debugging outputs
    GST_DEBUG_CATEGORY_INIT(rawtortp_debug_category, "rawtortp", 0, "Debug for RawToRTP plugin");

    // register our plugin
    gboolean success = gst_element_register(plugin, "rawtortp", GST_RANK_NONE, GST_TYPE_RAWTORTP);

#ifdef _DEBUG
    g_print("rawtortp: gst_element_register() returned %d\n", success);
    if (!success) {
        fprintf(stderr, "plugin_init(): gst_element_register failed!\n");
    }
    else {
        fprintf(stderr, "plugin_init(): registered element!\n");
    }
#endif

    return success;
}

GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    rawtortp,
    "Convert an UDP source into an RTP packetized source",
    plugin_init,
    "1.0",
    "LGPL",
    PACKAGE,
    "https://gstreamer.freedesktop.org/"
)

G_DEFINE_TYPE(GstMyElement, gst_my_element, GST_TYPE_ELEMENT)

//Input could be whatever random stream
static GstStaticPadTemplate sink_template =
GST_STATIC_PAD_TEMPLATE("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("application/octet-stream") 
);

// output is always an RTP stream
static GstStaticPadTemplate src_template =
GST_STATIC_PAD_TEMPLATE("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("application/x-rtp, "
        "payload=(int)" STR(HardcodedRTPPayloadType) ", "
        "clock-rate=(int)" STR(HardcodedRTPClockRate) ", "
        "media=(string)application, "
        "encoding-name=(string)octet-stream"
    ) // RTP caps
);

static void gst_my_element_class_init(GstMyElementClass* klass) {
    GstElementClass* element_class = GST_ELEMENT_CLASS(klass);

    gst_element_class_set_static_metadata(element_class,
        "RawToRTP Packetizer", "RawToRTP clasification",
        "Takes in an UDP raw bytestream source and wraps it in RTP packets",
        "Jozsa Istvan <jozsab1@gmail.com>");

    gst_element_class_add_pad_template(
        element_class,
        gst_static_pad_template_get(&sink_template)
    );

    gst_element_class_add_pad_template(
        element_class,
        gst_static_pad_template_get(&src_template)
    );
}

static GstFlowReturn gst_my_element_chain(GstPad* pad, GstObject* parent, GstBuffer* buf)
{
    GstMyElement* self = GST_RAWTORTP(parent);

    // sanity. Should never happen
    if (!buf) {
        GST_ERROR_OBJECT(self, "Received NULL buffer");
        return GST_FLOW_ERROR;
    }

    // for some reason output caps is not getting set properly. Try to force it
    if (!gst_pad_has_current_caps(self->srcpad)) {
        GstCaps* rtp_caps = gst_caps_from_string(
            "application/x-rtp, "
            "payload="STR(HardcodedRTPPayloadType)", "
            "clock-rate="STR(HardcodedRTPClockRate)", "
            "media=application, "
            "encoding-name=octet-stream"
        );
        gst_pad_set_caps(self->srcpad, rtp_caps);
        gst_caps_unref(rtp_caps);
        g_print(">>> rawtortp: set output caps on srcpad at runtime\n");
    }

    // debug. Can be deleted
    GST_LOG_OBJECT(self, "Processing buffer of size %" G_GSIZE_FORMAT, gst_buffer_get_size(buf));

    // break input into workable chuncks
    guint64 offset = 0;
    guint64 remaining = gst_buffer_get_size(buf);
    if (remaining > HardcodedRTPMaxMTUSize) {
        GST_WARNING_OBJECT(self, "Input buffer too large (%" G_GSIZE_FORMAT " bytes) — may need fragmentation", remaining);
    }

    GstFlowReturn retCode = GST_FLOW_OK;
    while (remaining > 0) {
        // get the next chunk to be wrapped
        guint64 chunk_size = MIN(remaining, HardcodedRTPMaxMTUSize);
        GstBuffer* chunk = gst_buffer_copy_region(buf, GST_BUFFER_COPY_ALL, offset, chunk_size);
        // should not happen
        if (!chunk) {
            GST_ERROR_OBJECT(self, "Failed to copy chunk region at offset %" G_GUINT64_FORMAT, offset);
            retCode = GST_FLOW_ERROR;
            break;
        }

        // wrap the chunk into an RTP packet
        GstBuffer* rtp = wrap_in_rtp(self, chunk);
        // sanity check to avoid pushing bad buff to pad
        if (!rtp) {
            GST_ERROR_OBJECT(self, "Failed to wrap chunk in RTP");
            gst_buffer_unref(chunk);
            retCode = GST_FLOW_ERROR;
            break;
        }

#ifdef _DEBUG
        debug_check_rtp_validity(self, rtp);
#endif

        // push chunk to output pad
        retCode = gst_pad_push(self->srcpad, rtp);
        // sanity check to not repeatedly push junk
        if (retCode < 0) {
            break;
        }

        // progress to next chunk to be processed
        offset += chunk_size;
        remaining -= chunk_size;
    }

    // done with original
    gst_buffer_unref(buf); 

    return retCode;
}

static void gst_my_element_init(GstMyElement* self) {
    GST_LOG_OBJECT(self, "RawToRTP is initializing");

    // Create sink pad
    self->sinkpad = gst_pad_new_from_static_template(&sink_template, "sink");
    gst_element_add_pad(GST_ELEMENT(self), self->sinkpad);
    // worker function on the input buffers that will be sent to the out pad
    gst_pad_set_chain_function(self->sinkpad, GST_DEBUG_FUNCPTR(gst_my_element_chain));

    // Create source pad
    self->srcpad = gst_pad_new_from_static_template(&src_template, "src");
    gst_element_add_pad(GST_ELEMENT(self), self->srcpad);

    GstCaps* rtp_caps = gst_caps_from_string(
        "application/x-rtp, "
        "payload="STR(HardcodedRTPPayloadType)", "
        "clock-rate="STR(HardcodedRTPClockRate)", "
        "media=application, "
        "encoding-name=octet-stream"
    );
    gst_pad_use_fixed_caps(self->srcpad);
    gst_pad_set_caps(self->srcpad, rtp_caps);
    gst_caps_unref(rtp_caps);

    // RTP session initialization
    self->stampPrevSrcPacket = 0;
    self->seqnum = 0;
    self->timestamp = 0;
    self->ssrc = g_random_int();

    // lagging session variables. Used to verify what we send out is indeed RTP compliant
#ifdef _DEBUG
    self->debug_ssrc = 0;
    self->debug_seqnum = 0;
    self->debug_timestamp = 0;
#endif
}
