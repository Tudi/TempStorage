#include <inttypes.h>

#include <gst/gst.h>
#include <gst/gstpadtemplate.h>
#include <gst/gstelement.h>
#include <gst/rtp/gstrtpbuffer.h>

#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include "gstreamer.h"
#include "util.h"

enum {
    PROP_0,
    PROP_KLV_ALLOWED,
    PROP_MAX_UPM,
};

GST_DEBUG_CATEGORY_STATIC(klvtortp_debug_category);
#define GST_CAT_DEFAULT klvtortp_debug_category

#define PACKAGE "klvtortp"

static gboolean plugin_init(GstPlugin* plugin) {
#ifdef _DEBUG
    g_print("klvtortp: plugin_init() starting\n");
    fprintf(stderr, "plugin_init() running...\n");
#endif

    // allow debugging outputs
    GST_DEBUG_CATEGORY_INIT(klvtortp_debug_category, "klvtortp", 0, "Debug for KLVToRTP plugin");

    // register our plugin
    gboolean success = gst_element_register(plugin, "klvtortp", GST_RANK_NONE, GST_TYPE_KLVTORTP);

#ifdef _DEBUG
    g_print("klvtortp: gst_element_register() returned %d\n", success);
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
    klvtortp,
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

static void gst_my_element_set_property(GObject* object, guint prop_id,
    const GValue* value, GParamSpec* pspec)
{
    GstMyElement* self = GST_KLVTORTP(object);

    switch (prop_id) {
    case PROP_KLV_ALLOWED:
        g_free(self->klv_allowed); // free previous value
        self->klv_allowed = g_value_dup_string(value);
        parse_klv_allowed_list(self->klv_allowed, &self->KLVsAllowed, &self->KLVsAllowedCount);
        GST_DEBUG_OBJECT(self, "klvallowed = '%s' val count = %" PRIu64, self->klv_allowed, self->KLVsAllowedCount);
        break;

    case PROP_MAX_UPM:
        self->max_upm = g_value_get_int(value);
        if (self->max_upm <= 0)
            self->max_upm = 1;
        GST_DEBUG_OBJECT(self, "maxupm = %d", self->max_upm);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gst_my_element_get_property(GObject* object, guint prop_id,
    GValue* value, GParamSpec* pspec)
{
    GstMyElement* self = GST_KLVTORTP(object);

    switch (prop_id) {
    case PROP_KLV_ALLOWED:
        g_value_set_string(value, self->klv_allowed);
        break;

    case PROP_MAX_UPM:
        g_value_set_int(value, self->max_upm);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gst_my_element_finalize(GObject* object)
{
    GstMyElement* self = GST_KLVTORTP(object);

    // Free dynamically allocated klv_allowed string
    if (self->klv_allowed) {
        g_free(self->klv_allowed);
        self->klv_allowed = NULL;
    }

    if (self->KLVsAllowed) {
        free(self->KLVsAllowed);
        self->KLVsAllowed = NULL;
    }

    // Chain up to the parent class
    G_OBJECT_CLASS(gst_my_element_parent_class)->finalize(object);
}

static void gst_my_element_class_init(GstMyElementClass* klass) {
    GstElementClass* element_class = GST_ELEMENT_CLASS(klass);

    gst_element_class_set_static_metadata(element_class,
        "KLVToRTP Packetizer", "KLVToRTP clasification",
        "Takes in an UDP klv bytestream source and wraps it in RTP packets",
        "Jozsa Istvan <jozsab1@gmail.com>");

    gst_element_class_add_pad_template(
        element_class,
        gst_static_pad_template_get(&sink_template)
    );

    gst_element_class_add_pad_template(
        element_class,
        gst_static_pad_template_get(&src_template)
    );

    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->set_property = gst_my_element_set_property;
    gobject_class->get_property = gst_my_element_get_property;

    g_object_class_install_property(
        gobject_class,
        PROP_KLV_ALLOWED,
        g_param_spec_string("klvallowed",
            "KLV Allowed List",
            "Comma-separated list of allowed KLV keys",
            NULL,  // default value
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)
    );

    g_object_class_install_property(
        gobject_class,
        PROP_MAX_UPM,
        g_param_spec_int("maxupm",
            "Max Updates Per Minute",
            "Maximum number of KLV update to send per minute",
            0,
            60000,
            60,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)
    );

    gobject_class->finalize = gst_my_element_finalize;
}

static GstFlowReturn gst_my_element_chain(GstPad* pad, GstObject* parent, GstBuffer* buf)
{
    GstMyElement* self = GST_KLVTORTP(parent);

    // sanity. Should never happen
    if (!buf) {
        GST_ERROR_OBJECT(self, "Received NULL buffer");
        return GST_FLOW_ERROR;
    }

    // debug. Can be deleted
    GST_LOG_OBJECT(self, "Processing buffer of size %" G_GSIZE_FORMAT, gst_buffer_get_size(buf));

    // break input into workable chuncks
    guint64 offset = 0;

    GstBuffer* filtered = filter_klv_buffer(self, buf);
    if (!filtered) {
        GST_ERROR_OBJECT(self, "KLV filtering failed");
        gst_buffer_unref(buf);
        return GST_FLOW_ERROR;
    }
    // done with original
    gst_buffer_unref(buf);

    guint64 remaining = gst_buffer_get_size(filtered);
    if (remaining > HardcodedRTPMaxMTUSize) {
        GST_WARNING_OBJECT(self, "Input buffer too large (%" G_GSIZE_FORMAT " bytes) — may need fragmentation", remaining);
    }

    GstFlowReturn retCode = GST_FLOW_OK;
    while (remaining > 0) {
        // get the next chunk to be wrapped
        guint64 chunk_size = MIN(remaining, HardcodedRTPMaxMTUSize);
        GstBuffer* chunk = gst_buffer_copy_region(filtered, GST_BUFFER_COPY_ALL, offset, chunk_size);
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

    return retCode;
}

static gboolean gst_my_element_sink_event(GstPad* pad, GstObject* parent, GstEvent* event)
{
    GstMyElement* self = GST_KLVTORTP(parent);

    switch (GST_EVENT_TYPE(event)) {
    case GST_EVENT_SEGMENT:
        GST_DEBUG_OBJECT(self, "Received segment event");
        break;
    
    case GST_EVENT_EOS:
        GST_DEBUG_OBJECT(self, "Received EOS");
        return gst_pad_push_event(self->srcpad, event);
    
    case GST_EVENT_FLUSH_START:
        GST_DEBUG_OBJECT(self, "Received FLUSH_START");
        return gst_pad_push_event(self->srcpad, event);

    case GST_EVENT_FLUSH_STOP:
        GST_DEBUG_OBJECT(self, "Received FLUSH_STOP");
        // Optional: reset any internal buffer queues or state here
        return gst_pad_push_event(self->srcpad, event);

	case GST_EVENT_CAPS: {
		GST_DEBUG_OBJECT(self, "Received CAPS event (ignored input caps, pushing RTP caps instead)");

		// Generate output RTP caps
		GstCaps* rtp_caps = gst_caps_from_string(
			"application/x-rtp, "
			"payload=" STR(HardcodedRTPPayloadType) ", "
			"clock-rate=" STR(HardcodedRTPClockRate) ", "
			"media=application, "
			"encoding-name=octet-stream"
		);

		GstEvent* new_caps_event = gst_event_new_caps(rtp_caps);
		gst_caps_unref(rtp_caps);

		return gst_pad_push_event(self->srcpad, new_caps_event);
	}

    default:
        break;
    }

    // Forward the event downstream
    return gst_pad_event_default(pad, parent, event);
}

static void gst_my_element_init(GstMyElement* self) {
    GST_LOG_OBJECT(self, "KLVToRTP is initializing");

    // Create sink pad
    self->sinkpad = gst_pad_new_from_static_template(&sink_template, "sink");
    gst_element_add_pad(GST_ELEMENT(self), self->sinkpad);
    // worker function on the input buffers that will be sent to the out pad
    gst_pad_set_chain_function(self->sinkpad, GST_DEBUG_FUNCPTR(gst_my_element_chain));
    gst_pad_set_event_function(self->sinkpad, GST_DEBUG_FUNCPTR(gst_my_element_sink_event));

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

    gst_pad_set_active(self->sinkpad, TRUE);
    gst_pad_set_active(self->srcpad, TRUE);

    // param values not yet initialized
    self->klv_allowed = NULL; // or strdup default if needed
    self->max_upm = 60;
    self->KLVsAllowed = NULL;
    self->KLVsAllowedCount = 0;

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
