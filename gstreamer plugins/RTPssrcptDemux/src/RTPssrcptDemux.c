#include <string.h>
#include <gst/rtp/gstrtpbuffer.h>
#include <gst/rtp/gstrtcpbuffer.h>

#include "RTPssrcptDemux.h"
#include "utils.h"

GST_DEBUG_CATEGORY_STATIC(my_rtp_ssrcpt_demux_debug);
#define GST_CAT_DEFAULT my_rtp_ssrcpt_demux_debug

/* generic templates */
static GstStaticPadTemplate rtp_ssrcpt_demux_sink_template =
GST_STATIC_PAD_TEMPLATE("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("application/x-rtp")
);

static GstStaticPadTemplate rtp_ssrcpt_demux_rtcp_sink_template =
GST_STATIC_PAD_TEMPLATE("rtcp_sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("application/x-rtcp")
);

static GstStaticPadTemplate rtp_ssrcpt_demux_src_template =
GST_STATIC_PAD_TEMPLATE("src_%u",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS("application/x-rtp, payload=(int)[0,127]")
);

static GstStaticPadTemplate rtp_ssrcpt_demux_rtcp_src_template =
GST_STATIC_PAD_TEMPLATE("rtcp_src_%u",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS("application/x-rtcp")
);

#define INTERNAL_STREAM_LOCK(obj)   (g_rec_mutex_lock (&(obj)->padlock))
#define INTERNAL_STREAM_UNLOCK(obj) (g_rec_mutex_unlock (&(obj)->padlock))

#define GST_PAD_FLAG_STICKIES_SENT (GST_PAD_FLAG_LAST << 0)
#define GST_PAD_STICKIES_SENT(pad)  (GST_OBJECT_FLAG_IS_SET (pad, GST_PAD_FLAG_STICKIES_SENT))
#define GST_PAD_SET_STICKIES_SENT(pad) (GST_OBJECT_FLAG_SET (pad, GST_PAD_FLAG_STICKIES_SENT))

typedef enum
{
    RTP_PAD,
    RTCP_PAD
} PadType;

/* signals */
enum
{
    SIGNAL_NEW_SSRCPT_PAD,
    SIGNAL_REMOVED_SSRCPT_PAD,
    SIGNAL_CLEAR_SSRCPT,
    LAST_SIGNAL
};

enum {
    PLUGIN_PROPERTY_GUESSENCODING = 1,
}PluginProtperties;

#define my_rtp_ssrcpt_demux_parent_class parent_class
G_DEFINE_TYPE(GstRtpSsrcPtDemux, my_rtp_ssrcpt_demux, GST_TYPE_ELEMENT);

#define PACKAGE "rtpssrcptdemux"

//GST_ELEMENT_REGISTER_DEFINE(rtpssrcptdemux, "rtpssrcptdemux", GST_RANK_NONE, GST_TYPE_RTP_SSRCPT_DEMUX);

static gboolean plugin_init(GstPlugin* plugin)
{
    GST_DEBUG_CATEGORY_INIT(my_rtp_ssrcpt_demux_debug, "rtpssrcptdemux", 0, "RTP SSRC PT demuxer");
    g_type_class_ref(GST_TYPE_RTP_SSRCPT_DEMUX);
    return gst_element_register(plugin, PACKAGE, GST_RANK_NONE, GST_TYPE_RTP_SSRCPT_DEMUX);
}

GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    rtpssrcptdemux,
    "RTP SSRC PT Demux plugin",
    plugin_init,
    "1.0",
    "LGPL",
    PACKAGE,
    "https://buybitcoin.pls"
)

/* GObject vmethods */
static void my_rtp_ssrcpt_demux_dispose(GObject* object);
static void my_rtp_ssrcpt_demux_finalize(GObject* object);

/* GstElement vmethods */
static GstStateChangeReturn my_rtp_ssrcpt_demux_change_state(GstElement* element, GstStateChange transition);
static void my_rtp_ssrcpt_demux_clear_ssrc_pt(GstRtpSsrcPtDemux* demux, guint32 ssrc, guint8 pt);

/* sinkpad stuff */
static GstFlowReturn my_rtp_ssrcpt_demux_chain(GstPad* pad, GstObject* parent, GstBuffer* buf);
static gboolean my_rtp_ssrcpt_demux_sink_event(GstPad* pad, GstObject* parent, GstEvent* event);
static GstFlowReturn my_rtp_ssrcpt_demux_rtcp_chain(GstPad* pad, GstObject* parent, GstBuffer* buf);
static GstIterator* my_rtp_ssrcpt_demux_iterate_internal_links_sink(GstPad* pad, GstObject* parent);

/* srcpad stuff */
static gboolean my_rtp_ssrcpt_demux_src_event(GstPad* pad, GstObject* parent, GstEvent* event);
static GstIterator* my_rtp_ssrcpt_demux_iterate_internal_links_src(GstPad* pad, GstObject* parent);

static guint my_rtp_ssrcpt_demux_signals[LAST_SIGNAL] = { 0 };

/*
 * Item for storing GstPad <-> SSRCPT pairs.
 */
typedef struct
{
    guint32 ssrc;
    guint32 pt; // payload type
    GstPad* rtp_pad;
    GstCaps* caps;
    GstPad* rtcp_pad;
    GuessedCodec codec;
} GstRtpSsrcPtDemuxPads;

static void guess_state_free(gpointer data) {
    GuessState* state = (GuessState*)data;
    if (!state) return;

    if (state->buffers) {
        while (!g_queue_is_empty(state->buffers)) {
            GstBuffer* b = g_queue_pop_head(state->buffers);
            gst_buffer_unref(b);
        }
        g_queue_free(state->buffers);
    }

    g_free(state);
}

/* find a src pad for a given SSRCPT, returns NULL if the SSRCPT was not found
 * MUST be called with object lock
 */
static GstRtpSsrcPtDemuxPads* find_demux_pads_for_ssrc_pt(GstRtpSsrcPtDemux* demux, guint32 ssrc, guint8 pt)
{
    GSList* walk;

    if (demux == NULL) {
        return NULL;
    }

    for (walk = demux->srcpads; walk; walk = g_slist_next(walk)) {
        GstRtpSsrcPtDemuxPads* pad = (GstRtpSsrcPtDemuxPads*)walk->data;

        if (pad->ssrc == ssrc && pad->pt == pt)
            return pad;
    }
    return NULL;
}

/* returns a reference to the pad if found, %NULL otherwise */
static GstPad* get_demux_pad_for_ssrc_pt(GstRtpSsrcPtDemux* demux, guint32 ssrc, guint8 pt, PadType padtype)
{
    GstRtpSsrcPtDemuxPads* dpads;
    GstPad* retpad;

    if (demux == NULL) {
        return NULL;
    }

    GST_OBJECT_LOCK(demux);

    dpads = find_demux_pads_for_ssrc_pt(demux, ssrc, pt);
    if (!dpads) {
        GST_OBJECT_UNLOCK(demux);
        return NULL;
    }

    switch (padtype) {
    case RTP_PAD:
        retpad = gst_object_ref(dpads->rtp_pad);
        break;
    case RTCP_PAD:
        retpad = gst_object_ref(dpads->rtcp_pad);
        break;
    default:
        retpad = NULL;
        g_assert_not_reached();
    }

    GST_OBJECT_UNLOCK(demux);

    return retpad;
}

static GstEvent* add_ssrc_and_ref(GstEvent* event, guint32 ssrc, guint8 pt, GuessedCodec codec)
{
    /* Set the ssrc on the output caps */
    switch (GST_EVENT_TYPE(event)) {
    case GST_EVENT_CAPS:
    {
        GstCaps* caps;
        GstCaps* newcaps;
        GstStructure* s;

        gst_event_parse_caps(event, &caps);
        newcaps = gst_caps_copy(caps);

        s = gst_caps_get_structure(newcaps, 0);
//        gst_structure_set(s, "ssrc", G_TYPE_UINT, ssrc, NULL); // probably nobody cares about this
        gst_structure_set(s, "payload", G_TYPE_INT, pt, NULL);
        gst_structure_set(s, "clock-rate", G_TYPE_INT, 90000, NULL);
        if (codec != CODEC_UNKNOWN)
        {
            const gchar* encoding = NULL;

            switch (codec) {
            case CODEC_H264: encoding = "H264"; break;
            case CODEC_H265: encoding = "H265"; break;
            case CODEC_VP8:  encoding = "VP8";  break;
            case CODEC_AV1:  encoding = "AV1";  break;
            default: break;
            }
            if (encoding) {
                gst_structure_set(s, "media", G_TYPE_STRING, "video", NULL);
                gst_structure_set(s, "encoding-name", G_TYPE_STRING, encoding, NULL);
            }
        }
        event = gst_event_new_caps(newcaps);
        gst_caps_unref(newcaps);
        break;
    }
    default:
        gst_event_ref(event);
        break;
    }

    return event;
}

struct ForwardStickyEventData
{
    GstPad* pad;
    guint32 ssrc;
    guint8 pt;
    GuessedCodec codec;
};

/* With internal stream lock held */
static gboolean forward_sticky_events(GstPad* pad, GstEvent** event, gpointer user_data)
{
    struct ForwardStickyEventData* data = user_data;
    GstEvent* newevent;

    newevent = add_ssrc_and_ref(*event, data->ssrc, data->pt, data->codec);
    gst_pad_push_event(data->pad, newevent);

    return TRUE;
}

static void forward_initial_events(GstRtpSsrcPtDemux* demux, guint32 ssrc, guint8 pt, GuessedCodec codec, GstPad* pad, PadType padtype)
{
    struct ForwardStickyEventData fdata;
    GstPad* sinkpad = NULL;

    if (demux == NULL) {
        return;
    }

    if (padtype == RTP_PAD)
        sinkpad = demux->rtp_sink;
    else if (padtype == RTCP_PAD)
        sinkpad = demux->rtcp_sink;
    else
        g_assert_not_reached();

    fdata.ssrc = ssrc;
    fdata.pt = pt;
    fdata.pad = pad;
    fdata.codec = codec;

    gst_pad_sticky_events_foreach(sinkpad, forward_sticky_events, &fdata);
}

/* MUST only be called from streaming thread */
static GstPad* find_or_create_demux_pad_for_ssrc_pt(GstRtpSsrcPtDemux* demux, guint32 ssrc, guint8 pt, GuessedCodec codec, PadType padtype)
{
    GstPad* rtp_pad, * rtcp_pad;
    GstElementClass* klass;
    GstPadTemplate* templ;
    gchar* padname;
    GstRtpSsrcPtDemuxPads* dpads;
    GstPad* retpad;

    if (demux == NULL) {
        return NULL;
    }

    INTERNAL_STREAM_LOCK(demux);

    retpad = get_demux_pad_for_ssrc_pt(demux, ssrc, pt, padtype);
    if (retpad != NULL) {
        INTERNAL_STREAM_UNLOCK(demux);
        return retpad;
    }

    // limit the number of pads created to X. Required if you feed non RTP traffic to the plugin
    guint16 num_streams = (GST_ELEMENT_CAST(demux)->numsrcpads) >> 1;
    if (num_streams >= MAX_PADS_LIMIT) {
        INTERNAL_STREAM_UNLOCK(demux);
        return NULL;
    }

    GST_DEBUG_OBJECT(demux, "creating new pad for SSRC=%08x pt=%u", ssrc, pt);

    klass = GST_ELEMENT_GET_CLASS(demux);

    guint32 pad_index = demux->PadsUsedForPT[pt] * 1000 + pt;
    demux->PadsUsedForPT[pt] += 1;

    templ = gst_element_class_get_pad_template(klass, "src_%u");
    padname = g_strdup_printf("src_%u", pad_index);
    rtp_pad = gst_pad_new_from_template(templ, padname);
    g_free(padname);

    templ = gst_element_class_get_pad_template(klass, "rtcp_src_%u");
    padname = g_strdup_printf("rtcp_src_%u", pad_index);
    rtcp_pad = gst_pad_new_from_template(templ, padname);
    g_free(padname);

    /* wrap in structure and add to list */
    dpads = g_new0(GstRtpSsrcPtDemuxPads, 1);
    dpads->ssrc = ssrc;
    dpads->pt = pt;
    dpads->codec = codec;
    dpads->rtp_pad = rtp_pad;
    dpads->rtcp_pad = rtcp_pad;

    GST_OBJECT_LOCK(demux);
    demux->srcpads = g_slist_prepend(demux->srcpads, dpads);
    GST_OBJECT_UNLOCK(demux);

    gst_pad_set_iterate_internal_links_function(rtp_pad, my_rtp_ssrcpt_demux_iterate_internal_links_src);
    gst_pad_set_event_function(rtp_pad, my_rtp_ssrcpt_demux_src_event);
    gst_pad_use_fixed_caps(rtp_pad);
    gst_pad_set_active(rtp_pad, TRUE);

    gst_pad_set_event_function(rtcp_pad, my_rtp_ssrcpt_demux_src_event);
    gst_pad_set_iterate_internal_links_function(rtcp_pad, my_rtp_ssrcpt_demux_iterate_internal_links_src);
    gst_pad_use_fixed_caps(rtcp_pad);
    gst_pad_set_active(rtcp_pad, TRUE);

    gst_element_add_pad(GST_ELEMENT_CAST(demux), rtp_pad);
    gst_element_add_pad(GST_ELEMENT_CAST(demux), rtcp_pad);

    switch (padtype) {
    case RTP_PAD:
        retpad = gst_object_ref(dpads->rtp_pad);
        break;
    case RTCP_PAD:
        retpad = gst_object_ref(dpads->rtcp_pad);
        break;
    default:
        retpad = NULL;
        g_assert_not_reached();
    }

    g_signal_emit(G_OBJECT(demux), my_rtp_ssrcpt_demux_signals[SIGNAL_NEW_SSRCPT_PAD], 0, ssrc, pt, rtp_pad);

    INTERNAL_STREAM_UNLOCK(demux);

    return retpad;
}

static void my_rtp_ssrcpt_demux_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec)
{
    GstRtpSsrcPtDemux* demux = GST_RTP_SSRCPT_DEMUX(object);
    g_return_if_fail(GST_IS_RTP_SSRCPT_DEMUX(demux));

    switch (prop_id) {
    case PLUGIN_PROPERTY_GUESSENCODING:
        demux->guessmedia = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void my_rtp_ssrcpt_demux_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec)
{
    GstRtpSsrcPtDemux* demux = GST_RTP_SSRCPT_DEMUX(object);
    g_return_if_fail(GST_IS_RTP_SSRCPT_DEMUX(demux));

    switch (prop_id) {
    case PLUGIN_PROPERTY_GUESSENCODING:
        g_value_set_boolean(value, demux->guessmedia);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void my_rtp_ssrcpt_demux_class_init(GstRtpSsrcPtDemuxClass* klass)
{
    GObjectClass* gobject_klass;
    GstElementClass* gstelement_klass;
    GstRtpSsrcPtDemuxClass* gstRtpSsrcPtDemux_klass;

    gobject_klass = (GObjectClass*)klass;
    gstelement_klass = (GstElementClass*)klass;
    gstRtpSsrcPtDemux_klass = (GstRtpSsrcPtDemuxClass*)klass;

    gobject_klass->dispose = my_rtp_ssrcpt_demux_dispose;
    gobject_klass->finalize = my_rtp_ssrcpt_demux_finalize;

    /**
     * GstRtpSsrcPtDemux::new-ssrc-pad:
     * @demux: the object which received the signal
     * @ssrc: the SSRC of the pad
     * @pad: the new pad.
     *
     * Emitted when a new SSRC pad has been created.
     */
    my_rtp_ssrcpt_demux_signals[SIGNAL_NEW_SSRCPT_PAD] =
        g_signal_new("new-ssrcpt-pad",
            G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(GstRtpSsrcPtDemuxClass, new_ssrcpt_pad),
            NULL, NULL, NULL, G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_UINT, GST_TYPE_PAD);

    /**
     * GstRtpSsrcPtDemux::removed-ssrc-pad:
     * @demux: the object which received the signal
     * @ssrc: the SSRC of the pad
     * @pad: the removed pad.
     *
     * Emitted when a SSRC pad has been removed.
     */
    my_rtp_ssrcpt_demux_signals[SIGNAL_REMOVED_SSRCPT_PAD] =
        g_signal_new("removed-ssrcpt-pad",
            G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(GstRtpSsrcPtDemuxClass, removed_ssrcpt_pad),
            NULL, NULL, NULL, G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_UINT, GST_TYPE_PAD);

    /**
     * GstRtpSsrcPtDemux::clear-ssrc:
     * @demux: the object which received the signal
     * @ssrc: the SSRC of the pad
     *
     * Action signal to remove the pad for SSRC.
     */
    my_rtp_ssrcpt_demux_signals[SIGNAL_CLEAR_SSRCPT] =
        g_signal_new("clear-ssrcpt",
            G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET(GstRtpSsrcPtDemuxClass, clear_ssrcpt),
            NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_UINT);

    gstelement_klass->change_state = GST_DEBUG_FUNCPTR(my_rtp_ssrcpt_demux_change_state);
    gstRtpSsrcPtDemux_klass->clear_ssrcpt = GST_DEBUG_FUNCPTR(my_rtp_ssrcpt_demux_clear_ssrc_pt);

    gst_element_class_add_static_pad_template(gstelement_klass, &rtp_ssrcpt_demux_sink_template);
    gst_element_class_add_static_pad_template(gstelement_klass, &rtp_ssrcpt_demux_rtcp_sink_template);
    gst_element_class_add_static_pad_template(gstelement_klass, &rtp_ssrcpt_demux_src_template);
    gst_element_class_add_static_pad_template(gstelement_klass, &rtp_ssrcpt_demux_rtcp_src_template);

    gst_element_class_set_static_metadata(gstelement_klass, "RTP SSRCPT Demux",
        "Demux/Network/RTP",
        "Splits RTP streams based on the SSRC + PT",
        "Jozsa Istvan <jozsab1@gmail.com>");

    GST_DEBUG_CATEGORY_INIT(my_rtp_ssrcpt_demux_debug,"rtpssrcptdemux", 0, "RTP SSRC PT demuxer");

    GST_DEBUG_REGISTER_FUNCPTR(my_rtp_ssrcpt_demux_chain);
    GST_DEBUG_REGISTER_FUNCPTR(my_rtp_ssrcpt_demux_rtcp_chain);

    gobject_klass->set_property = my_rtp_ssrcpt_demux_set_property;
    gobject_klass->get_property = my_rtp_ssrcpt_demux_get_property;

    g_object_class_install_property(gobject_klass, PLUGIN_PROPERTY_GUESSENCODING,
        g_param_spec_boolean("guessmedia", "Guess Media",
            "Attempt to guess the media format from early RTP packets",
            FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void my_rtp_ssrcpt_demux_init(GstRtpSsrcPtDemux* demux)
{
    if (demux == NULL) {
        GST_ERROR("Class init with missing plugin. Skipping init");
        return;
    }

    GstElementClass* klass = GST_ELEMENT_GET_CLASS(demux);

    demux->rtp_sink = gst_pad_new_from_template(gst_element_class_get_pad_template(klass, "sink"), "sink");
    gst_pad_set_chain_function(demux->rtp_sink, my_rtp_ssrcpt_demux_chain);
    gst_pad_set_event_function(demux->rtp_sink, my_rtp_ssrcpt_demux_sink_event);
    gst_pad_set_iterate_internal_links_function(demux->rtp_sink, my_rtp_ssrcpt_demux_iterate_internal_links_sink);
    gst_element_add_pad(GST_ELEMENT_CAST(demux), demux->rtp_sink);

    demux->rtcp_sink = gst_pad_new_from_template(gst_element_class_get_pad_template(klass, "rtcp_sink"), "rtcp_sink");
    gst_pad_set_chain_function(demux->rtcp_sink, my_rtp_ssrcpt_demux_rtcp_chain);
    gst_pad_set_event_function(demux->rtcp_sink, my_rtp_ssrcpt_demux_sink_event);
    gst_pad_set_iterate_internal_links_function(demux->rtcp_sink, my_rtp_ssrcpt_demux_iterate_internal_links_sink);
    gst_element_add_pad(GST_ELEMENT_CAST(demux), demux->rtcp_sink);

    demux->pt_guess_states = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)guess_state_free);

    g_rec_mutex_init(&demux->padlock);
}

static void my_rtp_ssrcpt_demux_pads_free(GstRtpSsrcPtDemuxPads* dpads)
{
    gst_pad_set_active(dpads->rtp_pad, FALSE);
    gst_pad_set_active(dpads->rtcp_pad, FALSE);

    gst_element_remove_pad(GST_PAD_PARENT(dpads->rtp_pad), dpads->rtp_pad);
    gst_element_remove_pad(GST_PAD_PARENT(dpads->rtcp_pad), dpads->rtcp_pad);

    g_free(dpads);
}

static void my_rtp_ssrcpt_demux_reset(GstRtpSsrcPtDemux* demux)
{
    if (demux == NULL) {
        return;
    }

    g_slist_free_full(demux->srcpads, (GDestroyNotify)my_rtp_ssrcpt_demux_pads_free);
    demux->srcpads = NULL;

    g_hash_table_destroy(demux->pt_guess_states);
    demux->pt_guess_states = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)guess_state_free);
}

static void my_rtp_ssrcpt_demux_dispose(GObject* object)
{
    GstRtpSsrcPtDemux* demux = GST_RTP_SSRCPT_DEMUX(object);
    g_return_if_fail(GST_IS_RTP_SSRCPT_DEMUX(demux));

    my_rtp_ssrcpt_demux_reset(demux);

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void my_rtp_ssrcpt_demux_finalize(GObject* object)
{
    GstRtpSsrcPtDemux* demux = GST_RTP_SSRCPT_DEMUX(object);
    g_return_if_fail(GST_IS_RTP_SSRCPT_DEMUX(demux));

    g_rec_mutex_clear(&demux->padlock);

    g_hash_table_destroy(demux->pt_guess_states);

    G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void my_rtp_ssrcpt_demux_clear_ssrc_pt(GstRtpSsrcPtDemux* demux, guint32 ssrc, guint8 pt)
{
    GstRtpSsrcPtDemuxPads* dpads;

    if (demux == NULL) {
        return;
    }

    GST_OBJECT_LOCK(demux);
    dpads = find_demux_pads_for_ssrc_pt(demux, ssrc, pt);
    if (dpads == NULL) {
        GST_OBJECT_UNLOCK(demux);
        goto unknown_pad;
    }

    GST_DEBUG_OBJECT(demux, "clearing pad for SSRC=%08x pt=%u", ssrc, pt);

    demux->srcpads = g_slist_remove(demux->srcpads, dpads);
    GST_OBJECT_UNLOCK(demux);

    g_signal_emit(G_OBJECT(demux), my_rtp_ssrcpt_demux_signals[SIGNAL_REMOVED_SSRCPT_PAD], 0, ssrc, pt, dpads->rtp_pad);

    my_rtp_ssrcpt_demux_pads_free(dpads);

    return;

    /* ERRORS */
unknown_pad:
    {
        GST_WARNING_OBJECT(demux, "unknown SSRC=%08x pt=%u", ssrc, pt);
        return;
    }
}

struct ForwardEventData
{
    GstRtpSsrcPtDemux* demux;
    GstEvent* event;
    gboolean res;
    GstPad* pad;
};

static gboolean forward_event(GstPad* pad, gpointer user_data)
{
    struct ForwardEventData* fdata = user_data;
    GSList* walk = NULL;
    GstEvent* newevent = NULL;

    /* special case for EOS */
    if (GST_EVENT_TYPE(fdata->event) == GST_EVENT_EOS)
        GST_PAD_SET_STICKIES_SENT(pad);

    if (GST_EVENT_IS_STICKY(fdata->event) && !GST_PAD_STICKIES_SENT(pad))
        return FALSE;

    if (fdata->demux) {
        GST_OBJECT_LOCK(fdata->demux);
        for (walk = fdata->demux->srcpads; walk; walk = walk->next) {
            GstRtpSsrcPtDemuxPads* dpads = (GstRtpSsrcPtDemuxPads*)walk->data;

            if (pad == dpads->rtp_pad || pad == dpads->rtcp_pad) {
                newevent = add_ssrc_and_ref(fdata->event, dpads->ssrc, dpads->pt, dpads->codec);
                break;
            }
        }
        GST_OBJECT_UNLOCK(fdata->demux);
    }

    if (newevent)
        fdata->res &= gst_pad_push_event(pad, newevent);

    return FALSE;
}

static gboolean my_rtp_ssrcpt_demux_sink_event(GstPad* pad, GstObject* parent, GstEvent* event)
{
    GstRtpSsrcPtDemux* demux;
    struct ForwardEventData fdata;

    demux = GST_RTP_SSRCPT_DEMUX(parent);
    g_return_val_if_fail(GST_IS_RTP_SSRCPT_DEMUX(demux), FALSE);

    fdata.demux = demux;
    fdata.pad = pad;
    fdata.event = event;
    fdata.res = TRUE;

    if (GST_EVENT_TYPE(event) == GST_EVENT_EOS) {
        // Copy the srcpads list to avoid modifying while iterating
        GSList* pads_copy;

        GST_OBJECT_LOCK(demux);
        pads_copy = g_slist_copy(demux->srcpads);
        GST_OBJECT_UNLOCK(demux);

        GstRtpSsrcPtDemuxClass* demuxclass = GST_RTP_SSRCPT_DEMUX_CLASS(G_OBJECT_GET_CLASS(demux));
        for (GSList* l = pads_copy; l != NULL; l = l->next) {
            GstRtpSsrcPtDemuxPads* dpads = (GstRtpSsrcPtDemuxPads*)l->data;
            if ((pad == demux->rtp_sink && dpads->rtp_pad) ||
                (pad == demux->rtcp_sink && dpads->rtcp_pad)) {
                demuxclass->clear_ssrcpt(demux, dpads->ssrc, dpads->pt);
            }
        }

        g_slist_free(pads_copy);
    }

    gst_pad_forward(pad, forward_event, &fdata);

    gst_event_unref(event);

    return fdata.res;
}

static GstFlowReturn my_rtp_ssrcpt_demux_chain(GstPad* pad, GstObject* parent, GstBuffer* buf)
{
    GstFlowReturn ret;
    GstRtpSsrcPtDemux* demux;
    guint32 ssrc;
    guint8 pt;
    GstRTPBuffer rtp = { NULL };
    GstPad* srcpad;

    demux = GST_RTP_SSRCPT_DEMUX(parent);
    g_return_val_if_fail(GST_IS_RTP_SSRCPT_DEMUX(demux), FALSE);

    if (!gst_rtp_buffer_map(buf, GST_MAP_READ, &rtp))
        goto invalid_payload;

    ssrc = gst_rtp_buffer_get_ssrc(&rtp);
    pt = gst_rtp_buffer_get_payload_type(&rtp);
    guint8* payload = gst_rtp_buffer_get_payload(&rtp);
    guint payload_size = gst_rtp_buffer_get_payload_len(&rtp);
    gst_rtp_buffer_unmap(&rtp);

    GST_DEBUG_OBJECT(demux, "received buffer of SSRC=%08x pt=%u", ssrc, pt);

    // swallow up to GUESS_MAX_PACKETS packets and try to guess codec type for media
    GQueue* buffers = NULL;
    GuessedCodec codec = CODEC_UNKNOWN;
    if (demux->guessmedia && pt == GUESS_MEDIA_FOR_PT_VIDEO) {

        gpointer key = GUINT_TO_POINTER((ssrc << 8) | pt);
        GuessState* state = g_hash_table_lookup(demux->pt_guess_states, key);
        if (!state) {
            state = g_new0(GuessState, 1);
            state->buffers = g_queue_new();
            state->dropped_frames = 0;
            state->guessed = FALSE;
            state->startedQueue = FALSE;
            g_hash_table_insert(demux->pt_guess_states, key, state);
        }
        if (state->guessed == FALSE) {        
            // queue X packets that we will investigate to try to guess media type
            // note that an I frame can have 2MB data / 1400 .. 1500 RTP packets
            guint guessedCodecs[CODEC_MAX_TYPE];
            guessedCodecs[CODEC_H264] = guess_h264(payload, payload_size);
            guessedCodecs[CODEC_H265] = guess_h265(payload, payload_size);
            guessedCodecs[CODEC_VP8] = guess_vp8(payload, payload_size);
            guessedCodecs[CODEC_AV1] = guess_av1(payload, payload_size);
            gboolean is_guessable = guessedCodecs[CODEC_H264]
                || guessedCodecs[CODEC_H265]
                || guessedCodecs[CODEC_VP8]
                || guessedCodecs[CODEC_AV1];

            // once first guessable packet has been seen, start queueing packets
            if (is_guessable && state->startedQueue == FALSE) {
                GST_DEBUG_OBJECT(demux, "Skipped %u RTP packets for SSRC=%08x PT=%u to find first guessable", state->dropped_frames, ssrc, pt);
                state->startedQueue = TRUE;
            }

            // wait until we see the first guessable packet and only than start queueing
            if (!is_guessable && state->startedQueue == FALSE && state->dropped_frames < GUESS_MAX_FRAMES_DROPPED_NO_GUESS) {
                state->dropped_frames++;
                if((state->dropped_frames % 100) == 0 )
                    GST_DEBUG_OBJECT(demux, "Skipping %u non-guessable RTP packet for SSRC=%08x PT=%u", state->dropped_frames, ssrc, pt);
                gst_buffer_unref(buf);
                return GST_FLOW_OK;
            }/**/

            // merge in current guesses with previous ones
            state->guessedCodecs[CODEC_H264] += guessedCodecs[CODEC_H264];
            state->guessedCodecs[CODEC_H265] += guessedCodecs[CODEC_H265];
            state->guessedCodecs[CODEC_VP8] += guessedCodecs[CODEC_VP8];
            state->guessedCodecs[CODEC_AV1] += guessedCodecs[CODEC_AV1];

            codec = try_guess_codec(state->guessedCodecs);

            // queue X packets. Crossing fingers we will catch more than 1 full frame
            if (codec == CODEC_UNKNOWN && g_queue_get_length(state->buffers) < GUESS_MAX_PACKETS) {
                g_queue_push_tail(state->buffers, gst_buffer_ref(buf));
                gst_buffer_unref(buf);
                return GST_FLOW_OK;
            }

            // stop trying to queue up packets
            state->guessed = TRUE;

            // we buffered enough packets. try to make an educated guess of codec type
            GstPad* pad = NULL;
            if (codec != CODEC_UNKNOWN) {
                GST_DEBUG_OBJECT(demux, "guessed codecs h264=%u h265=%u vp8=%u av1=%u. Picked %d", 
                    state->guessedCodecs[CODEC_H264], state->guessedCodecs[CODEC_H265], state->guessedCodecs[CODEC_VP8], state->guessedCodecs[CODEC_AV1], codec);

                GstCaps* caps = NULL;
            }
            GST_DEBUG_OBJECT(demux, "Stopped guessing after %u RTP packets for SSRC=%08x PT=%u", g_queue_get_length(state->buffers), ssrc, pt);
        }
    }

    srcpad = find_or_create_demux_pad_for_ssrc_pt(demux, ssrc, pt, codec, RTP_PAD);
    if (srcpad == NULL)
        goto create_failed;

    if (!GST_PAD_STICKIES_SENT(srcpad)) {
        forward_initial_events(demux, ssrc, pt, codec, srcpad, RTP_PAD);
        GST_PAD_SET_STICKIES_SENT(srcpad);
    }

    if (!gst_pad_is_linked(srcpad)) {
        //GST_DEBUG_OBJECT(demux, "Pad %s not linked, dropping buffer", GST_PAD_NAME(srcpad));
        gst_buffer_unref(buf);
        return GST_FLOW_OK;
    }

    // flushed queued buffers we used to guess media type
    if (buffers) {
        while (!g_queue_is_empty(buffers)) {
            GstBuffer* b = g_queue_pop_head(buffers);
            gst_pad_push(srcpad, b);
        }
    }

    /* push to srcpad */
    ret = gst_pad_push(srcpad, buf);

    if (ret != GST_FLOW_OK) {
        GstPad* active_pad;

        /* check if the ssrc still there, may have been removed */
        active_pad = get_demux_pad_for_ssrc_pt(demux, ssrc, pt, RTP_PAD);

        if (active_pad == NULL || active_pad != srcpad) {
            /* SSRC was removed during the push ... ignore the error */
            ret = GST_FLOW_OK;
        }

        g_clear_object(&active_pad);
    }

    gst_object_unref(srcpad);

    return ret;

    /* ERRORS */
invalid_payload:
    {
        GST_DEBUG_OBJECT(demux, "Dropping invalid RTP packet");
        gst_buffer_unref(buf);
        return GST_FLOW_OK;
    }
create_failed:
    {
        gst_buffer_unref(buf);
        GST_WARNING_OBJECT(demux, "Dropping buffer SSRC=%08x pt=%u", ssrc, pt);
        return GST_FLOW_OK;
    }
}

static GstFlowReturn my_rtp_ssrcpt_demux_rtcp_chain(GstPad* pad, GstObject* parent, GstBuffer* buf)
{
    GstFlowReturn ret;
    GstRtpSsrcPtDemux* demux;
    guint32 ssrc;
    guint8 pt = 0;
    GstRTCPPacket packet;
    GstRTCPBuffer rtcp = { NULL, };
    GstPad* srcpad;

    demux = GST_RTP_SSRCPT_DEMUX(parent);
    g_return_val_if_fail(GST_IS_RTP_SSRCPT_DEMUX(demux), FALSE);

    if (!gst_rtcp_buffer_validate_reduced(buf))
        goto invalid_rtcp;

    gst_rtcp_buffer_map(buf, GST_MAP_READ, &rtcp);
    if (!gst_rtcp_buffer_get_first_packet(&rtcp, &packet)) {
        gst_rtcp_buffer_unmap(&rtcp);
        goto invalid_rtcp;
    }

    /* first packet must be SR or RR, or in case of a reduced size RTCP packet
     * it must be APP, RTPFB or PSFB feeadback, or else the validate would
     * have failed */
    switch (gst_rtcp_packet_get_type(&packet)) {
    case GST_RTCP_TYPE_SR:
        /* get the ssrc so that we can route it to the right source pad */
        gst_rtcp_packet_sr_get_sender_info(&packet, &ssrc, NULL, NULL, NULL, NULL);
        break;
    case GST_RTCP_TYPE_RR:
        ssrc = gst_rtcp_packet_rr_get_ssrc(&packet);
        break;
    case GST_RTCP_TYPE_APP:
        ssrc = gst_rtcp_packet_app_get_ssrc(&packet);
        break;
    case GST_RTCP_TYPE_RTPFB:
    case GST_RTCP_TYPE_PSFB:
        ssrc = gst_rtcp_packet_fb_get_sender_ssrc(&packet);
        break;
    default:
        goto unexpected_rtcp;
    }
    gst_rtcp_buffer_unmap(&rtcp);

    GST_DEBUG_OBJECT(demux, "received RTCP of SSRC=%08x pt=%u", ssrc, pt);

    srcpad = find_or_create_demux_pad_for_ssrc_pt(demux, ssrc, pt, CODEC_UNKNOWN, RTCP_PAD);
    if (srcpad == NULL)
        goto create_failed;

    if (!GST_PAD_STICKIES_SENT(srcpad)) {
        forward_initial_events(demux, ssrc, pt, CODEC_UNKNOWN, srcpad, RTCP_PAD);
        GST_PAD_SET_STICKIES_SENT(srcpad);
    }

    if (!gst_pad_is_linked(srcpad)) {
        //GST_WARNING_OBJECT(demux, "Pad %s not linked, dropping buffer", GST_PAD_NAME(srcpad));
        gst_buffer_unref(buf);
        return GST_FLOW_OK;
    }

    /* push to srcpad */
    ret = gst_pad_push(srcpad, buf);

    if (ret != GST_FLOW_OK) {
        GstPad* active_pad;

        /* check if the ssrc still there, may have been removed */
        active_pad = get_demux_pad_for_ssrc_pt(demux, ssrc, pt, RTCP_PAD);
        if (active_pad == NULL || active_pad != srcpad) {
            /* SSRC was removed during the push ... ignore the error */
            ret = GST_FLOW_OK;
        }

        g_clear_object(&active_pad);
    }

    gst_object_unref(srcpad);

    return ret;

    /* ERRORS */
invalid_rtcp:
    {
        GST_DEBUG_OBJECT(demux, "Dropping invalid RTCP packet");
        gst_buffer_unref(buf);
        return GST_FLOW_OK;
    }
unexpected_rtcp:
    {
        GST_DEBUG_OBJECT(demux, "dropping unexpected RTCP packet");
        gst_buffer_unref(buf);
        return GST_FLOW_OK;
    }
create_failed:
    {
        gst_buffer_unref(buf);
        GST_WARNING_OBJECT(demux, "Dropping buffer SSRC=%08x pt=%u", ssrc, pt);
        return GST_FLOW_OK;
    }
}

static GstRtpSsrcPtDemuxPads* find_demux_pad_for_pad(GstRtpSsrcPtDemux* demux, GstPad* pad)
{
    GSList* walk;

    if (demux == NULL) {
        return NULL;
    }

    for (walk = demux->srcpads; walk; walk = g_slist_next(walk)) {
        GstRtpSsrcPtDemuxPads* dpads = (GstRtpSsrcPtDemuxPads*)walk->data;
        if (dpads->rtp_pad == pad || dpads->rtcp_pad == pad) {
            return dpads;
        }
    }

    return NULL;
}


static gboolean my_rtp_ssrcpt_demux_src_event(GstPad* pad, GstObject* parent, GstEvent* event)
{
    GstRtpSsrcPtDemux* demux;
    const GstStructure* s;

    demux = GST_RTP_SSRCPT_DEMUX(parent);
    g_return_val_if_fail(GST_IS_RTP_SSRCPT_DEMUX(demux), FALSE);

    switch (GST_EVENT_TYPE(event)) {
    case GST_EVENT_CUSTOM_UPSTREAM:
    case GST_EVENT_CUSTOM_BOTH:
    case GST_EVENT_CUSTOM_BOTH_OOB:
        s = gst_event_get_structure(event);
        if (s ){
            GstRtpSsrcPtDemuxPads* dpads = NULL;
            GstStructure* ws = NULL;
            if (!gst_structure_has_field(s, "ssrc")) {
                if (dpads == NULL)
                    dpads = find_demux_pad_for_pad(demux, pad);
                if (dpads) {
                    if (ws == NULL) {
                        event = gst_event_make_writable(event);
                        ws = gst_event_writable_structure(event);
                    }
                    gst_structure_set(ws, "ssrc", G_TYPE_UINT, dpads->ssrc, NULL);
                }
            }
            if (!gst_structure_has_field(s, "payload")) {
                if (dpads == NULL)
                    dpads = find_demux_pad_for_pad(demux, pad);
                if (dpads) {
                    if (ws == NULL) {
                        event = gst_event_make_writable(event);
                        ws = gst_event_writable_structure(event);
                    }
                    gst_structure_set(ws, "payload", G_TYPE_UINT, dpads->pt, NULL);
                }
            }
            if (!gst_structure_has_field(s, "encoding-name")) {
                if (dpads == NULL)
                    dpads = find_demux_pad_for_pad(demux, pad);
                if (dpads && dpads->codec != CODEC_UNKNOWN) {
                    if (ws == NULL) {
                        event = gst_event_make_writable(event);
                        ws = gst_event_writable_structure(event);
                    }
                    const gchar* encoding = NULL;
                    switch (dpads->codec) {
                    case CODEC_H264: encoding = "H264"; break;
                    case CODEC_H265: encoding = "H265"; break;
                    case CODEC_VP8:  encoding = "VP8";  break;
                    case CODEC_AV1:  encoding = "AV1";  break;
                    default: break;
                    }
                    if (encoding) {
                        gst_structure_set(ws, "media", G_TYPE_STRING, "video", NULL);
                        gst_structure_set(ws, "encoding-name", G_TYPE_STRING, encoding, NULL);
                    }
                }
            }
        }
        break;
    default:
        break;
    }

    return gst_pad_event_default(pad, parent, event);
}

static GstIterator* my_rtp_ssrcpt_demux_iterate_internal_links_src(GstPad* pad, GstObject* parent)
{
    GstRtpSsrcPtDemux* demux;
    GstPad* otherpad = NULL;
    GstIterator* it = NULL;
    GSList* current;

    demux = GST_RTP_SSRCPT_DEMUX(parent);
    g_return_val_if_fail(GST_IS_RTP_SSRCPT_DEMUX(demux), FALSE);

    GST_OBJECT_LOCK(demux);
    for (current = demux->srcpads; current; current = g_slist_next(current)) {
        GstRtpSsrcPtDemuxPads* dpads = (GstRtpSsrcPtDemuxPads*)current->data;

        if (pad == dpads->rtp_pad) {
            otherpad = demux->rtp_sink;
            break;
        }
        else if (pad == dpads->rtcp_pad) {
            otherpad = demux->rtcp_sink;
            break;
        }
    }
    if (otherpad) {
        GValue val = { 0, };

        g_value_init(&val, GST_TYPE_PAD);
        g_value_set_object(&val, otherpad);
        it = gst_iterator_new_single(GST_TYPE_PAD, &val);
        g_value_unset(&val);

    }
    GST_OBJECT_UNLOCK(demux);

    return it;
}

/* Should return 0 for elements to be included */
static gint src_pad_compare_func(gconstpointer a, gconstpointer b)
{
    GstPad* pad = GST_PAD(g_value_get_object(a));
    const gchar* prefix = g_value_get_string(b);
    gint res;

    /* 0 means equal means we accept the pad, accepted if there is a name
     * and it starts with the prefix */
    GST_OBJECT_LOCK(pad);
    res = !GST_PAD_NAME(pad) || !g_str_has_prefix(GST_PAD_NAME(pad), prefix);
    GST_OBJECT_UNLOCK(pad);

    return res;
}

static GstIterator* my_rtp_ssrcpt_demux_iterate_internal_links_sink(GstPad* pad, GstObject* parent)
{
    GstRtpSsrcPtDemux* demux;
    GstIterator* it = NULL;
    GValue gval = { 0, };

    demux = GST_RTP_SSRCPT_DEMUX(parent);
    g_return_val_if_fail(GST_IS_RTP_SSRCPT_DEMUX(demux), FALSE);

    g_value_init(&gval, G_TYPE_STRING);
    if (pad == demux->rtp_sink)
        g_value_set_static_string(&gval, "src_");
    else if (pad == demux->rtcp_sink)
        g_value_set_static_string(&gval, "rtcp_src_");
    else
        g_assert_not_reached();

    it = gst_element_iterate_src_pads(GST_ELEMENT_CAST(demux));
    it = gst_iterator_filter(it, src_pad_compare_func, &gval);

    return it;
}

static GstStateChangeReturn my_rtp_ssrcpt_demux_change_state(GstElement* element, GstStateChange transition)
{
    GstStateChangeReturn ret;
    GstRtpSsrcPtDemux* demux;

    demux = GST_RTP_SSRCPT_DEMUX(element);
    g_return_val_if_fail(GST_IS_RTP_SSRCPT_DEMUX(demux), FALSE);

    switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
    case GST_STATE_CHANGE_READY_TO_PAUSED:
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
    default:
        break;
    }

    ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);

    switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
        break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
    case GST_STATE_CHANGE_READY_TO_NULL:
        my_rtp_ssrcpt_demux_reset(demux);
        break;
    default:
        break;
    }
    return ret;
}