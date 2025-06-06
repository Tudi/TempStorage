#pragma once

#include <gst/gst.h>
#include <glib.h>

gboolean guess_h264(const guint8* payload, gsize size);
gboolean guess_h265(const guint8* payload, gsize size);
gboolean guess_vp8(const guint8* payload, gsize size);
gboolean guess_av1(const guint8* payload, gsize size);
enum GuessedCodec try_guess_codec(guint* scores);