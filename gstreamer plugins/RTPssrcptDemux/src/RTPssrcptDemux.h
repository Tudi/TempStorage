#ifndef __GST_RTP_SSRCPT_DEMUX_H__
#define __GST_RTP_SSRCPT_DEMUX_H__

/*
Demux a stream of RTP packets based on PT and ssrc
The created pads will try to be based on PT, but if have multiple same PT, SSRC will be used to differentiate them
RTCP will always have PT=0
This plugin is based on RtpSsrcPtDemux plugin from gstreamer
*/

#include <gst/gst.h>
#include <glib.h>

#define MAX_PADS_LIMIT						10 // if you feed the plugin non rtp data, it would create a lot of pads

#define GST_TYPE_RTP_SSRCPT_DEMUX            (my_rtp_ssrcpt_demux_get_type())
#define GST_RTP_SSRCPT_DEMUX(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTP_SSRCPT_DEMUX,GstRtpSsrcPtDemux))
#define GST_RTP_SSRCPT_DEMUX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTP_SSRCPT_DEMUX,GstRtpSsrcPtDemuxClass))
#define GST_IS_RTP_SSRCPT_DEMUX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTP_SSRCPT_DEMUX))
#define GST_IS_RTP_SSRCPT_DEMUX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTP_SSRCPT_DEMUX))

typedef struct _GstRtpSsrcPtDemux GstRtpSsrcPtDemux;
typedef struct _GstRtpSsrcPtDemuxClass GstRtpSsrcPtDemuxClass;

struct _GstRtpSsrcPtDemux
{
  GstElement parent;

  GstPad *rtp_sink;
  GstPad *rtcp_sink;

  GRecMutex padlock;
  GSList *srcpads;
  // in case we find more than one PT, we will create a pad with indexed PT. Ex : PT, 1000 + PT, 2000 + PT
  guint8 PadsUsedForPT[UCHAR_MAX];

  gboolean guessmedia;  // whether to guess the media type based on payload
  GHashTable* pt_guess_states; // key = GUINT_TO_POINTER((ssrc << 8) | pt), value = GuessState*
};

struct _GstRtpSsrcPtDemuxClass
{
  GstElementClass parent_class;

  /* signals */
  void (*new_ssrcpt_pad)     (GstRtpSsrcPtDemux *demux, guint32 ssrc, guint8 pt, GstPad *pad);
  void (*removed_ssrcpt_pad) (GstRtpSsrcPtDemux *demux, guint32 ssrc, guint8 pt, GstPad *pad);

  /* actions */
  void (*clear_ssrcpt)       (GstRtpSsrcPtDemux *demux, guint32 ssrc, guint8 pt);
};

GType my_rtp_ssrcpt_demux_get_type (void);

GST_ELEMENT_REGISTER_DECLARE (RtpSsrcPtDemux);

typedef enum {
	CODEC_UNKNOWN = 0,
	CODEC_H264,
	CODEC_H265,
	CODEC_VP8,
	CODEC_AV1,
	CODEC_MAX_TYPE
} GuessedCodec;

typedef struct {
	GQueue* buffers;               // small buffer of guessable RTP packets
	guint dropped_frames;          // non-guessable packets skipped
	gboolean guessed;              // set to TRUE once guess is made
	gboolean startedQueue;         // set to TRUE once guess is made
	guint guessedCodecs[CODEC_MAX_TYPE]; // number of valid frames (e.g., IDR, keyframe)
} GuessState;

#define GUESS_REQUIRED_MATCHES				2
#define GUESS_REQUIRED_MATCHES_DIFF			1		// leading vote should have this advantage
#define GUESS_MAX_PACKETS					3000	// ex 2MB frame = 1428 RTP packets
#define GUESS_MEDIA_FOR_PT_VIDEO			96
#define GUESS_MAX_FRAMES_DROPPED_NO_GUESS	3000

#endif /* __GST_RTP_SSRCPT_DEMUX_H__ */