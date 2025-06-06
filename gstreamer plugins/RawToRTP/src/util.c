#include <gst/gst.h>
#include <gst/gstpadtemplate.h>
#include <gst/gstelement.h>
#include <gst/rtp/gstrtpbuffer.h>

#include <stdint.h>
#include <time.h>

#include "gstreamer.h"

#ifdef _WIN32
#include <windows.h>
#include <stdint.h>

uint64_t get_time_ms() {
    FILETIME ft;
    ULARGE_INTEGER uli;
    GetSystemTimeAsFileTime(&ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uli.QuadPart / 10000ULL); // Convert to ms
}
#else
#include <time.h>
#include <stdint.h>

uint64_t get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}
#endif

GstBuffer* wrap_in_rtp(GstMyElement* self, GstBuffer* raw_buf) {
    guint payload_type = HardcodedRTPPayloadType;

    // first time we see a src packet
    if (self->stampPrevSrcPacket == 0) {
        self->stampPrevSrcPacket = get_time_ms();
    }

    // in case our input src does not come at a fixed FPS
    // calculate how much time units passed since last time we received a packet
    guint64 timeDiff = get_time_ms() - self->stampPrevSrcPacket;
    guint64 packetTimeIncrease = timeDiff * HardcodedRTPClockRate / 1000;
    self->timestamp += (guint32)packetTimeIncrease;
    // remember last received pack stamp
    self->stampPrevSrcPacket = get_time_ms();

    // Allocate RTP buffer (same size as input + header)
    gsize payload_len = gst_buffer_get_size(raw_buf);
    GstBuffer* rtp_buf = gst_rtp_buffer_new_allocate((guint)payload_len, 0, 0);
    GstRTPBuffer rtp = GST_RTP_BUFFER_INIT;
    if (!gst_rtp_buffer_map(rtp_buf, GST_MAP_WRITE, &rtp)) {
        GST_ERROR_OBJECT(self, "Failed to map RTP buffer");
        gst_buffer_unref(rtp_buf);
        return NULL;
    }

    // actual RTP packet constuction
    gst_rtp_buffer_set_payload_type(&rtp, payload_type);
    gst_rtp_buffer_set_ssrc(&rtp, self->ssrc);
    gst_rtp_buffer_set_seq(&rtp, (self->seqnum % 65536));
    gst_rtp_buffer_set_timestamp(&rtp, (self->timestamp % ((guint64)1 << 32)));

    // Copy payload
    GstMapInfo map_src, map_dst;
    gst_buffer_map(raw_buf, &map_src, GST_MAP_READ);
    gst_buffer_map(rtp_buf, &map_dst, GST_MAP_WRITE);

    gsize src_size = map_src.size;
    gsize dst_size = gst_rtp_buffer_get_payload_len(&rtp);
    if (src_size > dst_size) {
        GST_WARNING_OBJECT(self, "Truncating input buffer from %" G_GSIZE_FORMAT " to %" G_GSIZE_FORMAT, src_size, dst_size);
        src_size = dst_size;
    }

    guint8* payload = gst_rtp_buffer_get_payload(&rtp);
    memcpy(payload, map_src.data, src_size);

    gst_buffer_unmap(raw_buf, &map_src);
    gst_buffer_unmap(rtp_buf, &map_dst);

    gst_rtp_buffer_unmap(&rtp);

    // update packet related info for next packet
    self->seqnum += 1;

    return rtp_buf;
}

#ifdef _DEBUG
void debug_check_rtp_validity(GstMyElement* self, GstBuffer* buffer) {
    GstRTPBuffer rtp = GST_RTP_BUFFER_INIT;

    if (!gst_rtp_buffer_map(buffer, GST_MAP_READ, &rtp)) {
        GST_WARNING_OBJECT(self, "Could not map RTP buffer for validation");
        return;
    }

    gboolean valid = TRUE;

    // Check RTP version
    guint8 version = gst_rtp_buffer_get_version(&rtp);
    if (version != 2) {
        GST_WARNING_OBJECT(self, "Invalid RTP version: %u (expected 2)", version);
        valid = FALSE;
    }

    // Check for payload length
    guint payload_len = gst_rtp_buffer_get_payload_len(&rtp);
    if (payload_len == 0) {
        GST_WARNING_OBJECT(self, "RTP packet has zero-length payload");
        valid = FALSE;
    }

    gsize expected_len = gst_rtp_buffer_get_packet_len(&rtp); // includes header and payload
    gsize actual_len = gst_buffer_get_size(buffer);

    if (actual_len != expected_len) {
        GST_WARNING_OBJECT(self, "RTP buffer size mismatch: expected %" G_GSIZE_FORMAT
            ", got %" G_GSIZE_FORMAT, expected_len, actual_len);
        valid = FALSE;
    }

    // ssrc should persist throughout the session
    if (self->debug_ssrc == 0) {
        self->debug_ssrc = gst_rtp_buffer_get_ssrc(&rtp);
    }
    else if (self->debug_ssrc != gst_rtp_buffer_get_ssrc(&rtp)) {
        GST_WARNING_OBJECT(self, "RTP buffer ssrc mismatch: expected %" G_GSIZE_FORMAT
            ", got %" G_GSIZE_FORMAT, self->debug_ssrc, gst_rtp_buffer_get_ssrc(&rtp));
        valid = FALSE;
    }

    // seq num should increase for every new packet
    if (self->debug_seqnum == 0) {
        self->debug_seqnum = gst_rtp_buffer_get_seq(&rtp);
    }
    else if (self->debug_seqnum + 1 != gst_rtp_buffer_get_seq(&rtp)) {
        GST_WARNING_OBJECT(self, "RTP buffer seq mismatch: expected %" G_GSIZE_FORMAT
            ", got %" G_GSIZE_FORMAT, self->debug_seqnum + 1, gst_rtp_buffer_get_seq(&rtp));
        valid = FALSE;
    }
    self->debug_seqnum = gst_rtp_buffer_get_seq(&rtp);

    // timestamp should increase over time
    if (self->debug_timestamp == 0) {
        self->debug_timestamp = gst_rtp_buffer_get_timestamp(&rtp);
    }
    else if (self->debug_timestamp > gst_rtp_buffer_get_timestamp(&rtp)) {
        GST_WARNING_OBJECT(self, "RTP buffer timestamp mismatch: expected %" G_GSIZE_FORMAT
            ", got %" G_GSIZE_FORMAT, self->debug_timestamp, gst_rtp_buffer_get_timestamp(&rtp));
        valid = FALSE;
    }
    self->debug_timestamp = gst_rtp_buffer_get_timestamp(&rtp);

    // might be an overkill, but we are in debug anyway
    if (valid) {
        GST_LOG_OBJECT(self, "RTP packet looks valid: seq=%u ts=%u len=%u ssrc=%u",
            gst_rtp_buffer_get_seq(&rtp),
            gst_rtp_buffer_get_timestamp(&rtp),
            actual_len,
            self->debug_ssrc);
    }

    gst_rtp_buffer_unmap(&rtp);
}
#endif