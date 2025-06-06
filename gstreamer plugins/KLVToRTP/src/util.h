#ifndef _UTIL_H_
#define _UTIL_H_

/// <summary>
/// Get ms precise time
/// </summary>
/// <returns></returns>
uint64_t get_time_ms();

/// <summary>
/// using our element as a session store, wrap incomming buffers into RTP packets
/// </summary>
/// <param name="self"></param>
/// <param name="raw_buf"></param>
/// <returns></returns>
GstBuffer* wrap_in_rtp(GstMyElement* self, GstBuffer* raw_buf);

/// <summary>
/// Double check that what we send out is according to the RTP standard
/// </summary>
/// <param name="self"></param>
/// <param name="buffer"></param>
void debug_check_rtp_validity(GstMyElement* self, GstBuffer* buffer);

/// <summary>
/// Convert param list of string ints to ints
/// </summary>
/// <param name="input"></param>
/// <param name="out_entries"></param>
/// <param name="out_count"></param>
/// <returns></returns>
gboolean parse_klv_allowed_list(const gchar* input, KLVFilterEntry** out_entries, guint64* out_count);

/// <summary>
/// Filter incomming MISB 601.8 KLV data based on filter params
/// </summary>
/// <param name="self"></param>
/// <param name="buf"></param>
/// <returns></returns>
GstBuffer* filter_klv_buffer(GstMyElement* self, GstBuffer* buf);
#endif