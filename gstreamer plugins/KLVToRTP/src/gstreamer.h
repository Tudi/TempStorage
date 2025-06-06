#ifndef __GST_MY_ELEMENT_H__
#define __GST_MY_ELEMENT_H__

#ifdef _DEBUG
    #undef G_DISABLE_CHECKS
    #undef G_DISABLE_CAST_CHECKS
    #undef G_DISABLE_ASSERT
#endif

// change this if payload type 96 is not suitable
#define HardcodedRTPPayloadType 97
#define HardcodedRTPClockRate 90000
#define HardcodedRTPMaxMTUSize 1200

// helper macro to turn the INT to STR
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

G_BEGIN_DECLS

typedef struct KLVFilterEntry {
    guint64 keyVal; // we filter based on this value
    guint64 timestampNextSend;
}KLVFilterEntry;

struct _GstMyElement {
    GstElement parent;  // Always the first member!

    GstPad* sinkpad;      
    GstPad* srcpad;

    // init params
    gchar* klv_allowed; // string (must be freed on finalize)
    gint max_upm;

    // so we can afford non fixed clock rate input
    guint64 stampPrevSrcPacket;

    // RTP packet related info. Consider these variable as RTP session stores
    guint ssrc;
    guint16 seqnum;
    guint32 timestamp;

    // klv filtering
    KLVFilterEntry* KLVsAllowed;
    guint64 KLVsAllowedCount;

#ifdef _DEBUG
    guint debug_ssrc;
    guint16 debug_seqnum;
    guint32 debug_timestamp;
#endif
};

#define GST_TYPE_KLVTORTP (gst_my_element_get_type())
G_DECLARE_FINAL_TYPE(GstMyElement, gst_my_element, GST, KLVTORTP, GstElement)

G_END_DECLS

#endif
