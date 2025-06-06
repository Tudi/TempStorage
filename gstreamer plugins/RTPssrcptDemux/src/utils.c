#include "utils.h"
#include "RTPssrcptDemux.h"

typedef struct {
    const guint8 *data;
    gsize size;
    gsize byte_pos;
    guint8 bit_pos;
} BitReader;

static void bitreader_init(BitReader *br, const guint8 *data, gsize size) {
    br->data = data;
    br->size = size;
    br->byte_pos = 0;
    br->bit_pos = 0;
}

static guint bitreader_read_bits(BitReader *br, guint n) {
    guint val = 0;
    for (guint i = 0; i < n; ++i) {
        if (br->byte_pos >= br->size) return 0;
        val <<= 1;
        val |= (br->data[br->byte_pos] >> (7 - br->bit_pos)) & 1;
        if (++br->bit_pos == 8) {
            br->bit_pos = 0;
            ++br->byte_pos;
        }
    }
    return val;
}

static guint bitreader_read_ue(BitReader *br) {
    guint zeroes = 0;
    while (bitreader_read_bits(br, 1) == 0 && zeroes < 32)
        zeroes++;
    return (1 << zeroes) - 1 + bitreader_read_bits(br, zeroes);
}

static gboolean parse_h264_sps(const guint8 *data, gsize size) {
    BitReader br;
    bitreader_init(&br, data, size);

    guint profile_idc = bitreader_read_bits(&br, 8);
    guint constraint_set_flags = bitreader_read_bits(&br, 8); // constraint + reserved bits
    guint level_idc = bitreader_read_bits(&br, 8);
    guint sps_id = bitreader_read_ue(&br);

    if (sps_id > 31) return FALSE;

    // Optional chroma, scaling matrices depending on profile
    if (profile_idc == 100 || profile_idc == 110 ||
        profile_idc == 122 || profile_idc == 244 || 
        profile_idc == 44  || profile_idc == 83  || 
        profile_idc == 86  || profile_idc == 118 || 
        profile_idc == 128 || profile_idc == 138 || 
        profile_idc == 139 || profile_idc == 134 || 
        profile_idc == 135) {
        guint chroma_format_idc = bitreader_read_ue(&br);
        if (chroma_format_idc > 3)
            return FALSE;

        if (chroma_format_idc == 3)
            bitreader_read_bits(&br, 1); // separate_colour_plane_flag

        guint bit_depth_luma_minus8 = bitreader_read_ue(&br);
        guint bit_depth_chroma_minus8 = bitreader_read_ue(&br);

        if (bit_depth_luma_minus8 > 6 || bit_depth_chroma_minus8 > 6)
            return FALSE;

        bitreader_read_bits(&br, 1); // qpprime_y_zero_transform_bypass_flag

        gboolean seq_scaling_matrix_present_flag = bitreader_read_bits(&br, 1);
        if (seq_scaling_matrix_present_flag) {
            // Skip scaling matrices (could be implemented)
            return FALSE;
        }
    }

    guint log2_max_frame_num_minus4 = bitreader_read_ue(&br);
    if (log2_max_frame_num_minus4 > 12)
        return FALSE;

    // If all fields read successfully and within expected range:
    return TRUE;
}

gboolean guess_h264(const guint8* payload, gsize size) {
    if (size < 1)
        return FALSE;

    guint8 nal_header = payload[0];
    guint8 nal_type = nal_header & 0x1F;
    guint8 nal_ref_idc = (nal_header >> 5) & 0x03;

    // Early rejection: forbidden_zero_bit must be 0
    if ((nal_header & 0x80) != 0)
        return FALSE;

    // Strong candidate types
    if (nal_type == 7 || nal_type == 8 || nal_type == 5) {
        // Optional: add profile validation for SPS
        if (nal_type == 7 && size >= 4) {
            if (parse_h264_sps(payload + 1, size - 1)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

static guint unescape_h265(const guint8* src, gsize src_size, guint8* dst, gsize dst_max) {
    guint src_i = 0, dst_i = 0;
    while (src_i + 2 < src_size && dst_i < dst_max) {
        if (src[src_i] == 0 && src[src_i + 1] == 0 && src[src_i + 2] == 3) {
            dst[dst_i++] = 0;
            dst[dst_i++] = 0;
            src_i += 3;
        }
        else {
            dst[dst_i++] = src[src_i++];
        }
    }
    while (src_i < src_size && dst_i < dst_max)
        dst[dst_i++] = src[src_i++];
    return dst_i;
}

static gboolean bitreader_align_and_check_trailing_bits(BitReader* br) {
    // We're mid-byte — read and validate rbsp_stop_one_bit
    if (br->bit_pos != 0) {
        guint stop_bit = bitreader_read_bits(br, 1);
        if (stop_bit != 1)
            return FALSE;

        // Remaining bits in this byte should all be 0
        while (br->bit_pos != 0) {
            guint pad = bitreader_read_bits(br, 1);
            if (pad != 0)
                return FALSE;
        }
    }

    // Rest of RBSP must be empty (we're now aligned)
    return TRUE;
}

static gboolean parse_h265_sps(const guint8* data, gsize size) {
    guint8 unescaped[256];
    gsize actual = unescape_h265(data, size, unescaped, sizeof(unescaped));
    if (actual < 8)
        return FALSE;

    BitReader br;
    bitreader_init(&br, unescaped, actual);

    guint sps_video_parameter_set_id = bitreader_read_bits(&br, 4);
    guint sps_max_sub_layers_minus1 = bitreader_read_bits(&br, 3);
    bitreader_read_bits(&br, 1); // sps_temporal_id_nesting_flag

    // profile_tier_level()
    guint profile_space = bitreader_read_bits(&br, 2);
    guint tier_flag = bitreader_read_bits(&br, 1);
    guint profile_idc = bitreader_read_bits(&br, 5);
    guint profile_compatibility_flags = bitreader_read_bits(&br, 32);
    guint64 constraint_flags = ((guint64)bitreader_read_bits(&br, 16) << 32) | bitreader_read_bits(&br, 32);
    guint level_idc = bitreader_read_bits(&br, 8);

    // Validation
    if (profile_idc > 31 || level_idc > 255)
        return FALSE;

    // Basic compatibility check: must support the signaled profile
    if ((profile_compatibility_flags & (1 << profile_idc)) == 0)
        return FALSE;

    // Check sub-layer flags
    for (guint i = 0; i < sps_max_sub_layers_minus1; i++) {
        bitreader_read_bits(&br, 1); // sub_layer_profile_present_flag
        bitreader_read_bits(&br, 1); // sub_layer_level_present_flag
    }

    guint sps_seq_parameter_set_id = bitreader_read_ue(&br);
    if (sps_seq_parameter_set_id > 15)
        return FALSE;

    guint chroma_format_idc = bitreader_read_ue(&br);
    if (chroma_format_idc > 3)
        return FALSE;

    if (chroma_format_idc == 3)
        bitreader_read_bits(&br, 1); // separate_colour_plane_flag

    guint pic_width_in_luma_samples = bitreader_read_ue(&br);
    guint pic_height_in_luma_samples = bitreader_read_ue(&br);

    if (pic_width_in_luma_samples < 16 || pic_width_in_luma_samples > 8192)
        return FALSE;
    if (pic_height_in_luma_samples < 16 || pic_height_in_luma_samples > 4320)
        return FALSE;

    return bitreader_align_and_check_trailing_bits(&br);
}

static gboolean parse_h265_vps(const guint8* data, gsize size) {
    guint8 unescaped[256];
    gsize actual = unescape_h265(data, size, unescaped, sizeof(unescaped));
    if (actual < 8)
        return FALSE;

    BitReader br;
    bitreader_init(&br, unescaped, actual);

    guint vps_id = bitreader_read_bits(&br, 4);
    guint vps_reserved_three_2bits = bitreader_read_bits(&br, 2); // should be 3
    guint vps_max_layers_minus1 = bitreader_read_bits(&br, 6);

    if (vps_reserved_three_2bits != 3 || vps_max_layers_minus1 > 62)
        return FALSE;

    guint vps_max_sub_layers_minus1 = bitreader_read_bits(&br, 3);
    guint vps_temporal_id_nesting_flag = bitreader_read_bits(&br, 1);
    bitreader_read_bits(&br, 16); // vps_reserved_0xffff_16bits

    // profile_tier_level() starts here
    guint profile_space = bitreader_read_bits(&br, 2);
    guint tier_flag = bitreader_read_bits(&br, 1);
    guint profile_idc = bitreader_read_bits(&br, 5);
    guint profile_compatibility_flags = bitreader_read_bits(&br, 32);
    guint64 constraint_flags = ((guint64)bitreader_read_bits(&br, 16)) << 32;
    constraint_flags |= bitreader_read_bits(&br, 32);
    guint level_idc = bitreader_read_bits(&br, 8);

    // Validate basic properties
    if (profile_idc > 31 || level_idc > 255)
        return FALSE;

    guint sub_layer_profile_present_flags = 0;
    guint sub_layer_level_present_flags = 0;
    for (guint i = 0; i < vps_max_sub_layers_minus1; i++) {
        sub_layer_profile_present_flags <<= 1;
        sub_layer_profile_present_flags |= bitreader_read_bits(&br, 1);
        sub_layer_level_present_flags <<= 1;
        sub_layer_level_present_flags |= bitreader_read_bits(&br, 1);
    }

    if (!bitreader_align_and_check_trailing_bits(&br))
        return FALSE;

    return TRUE;
}


static gboolean parse_h265_pps(const guint8* data, gsize size) {
    guint8 unescaped[256];
    gsize actual = unescape_h265(data, size, unescaped, sizeof(unescaped));
    if (actual < 4)
        return FALSE;

    BitReader br;
    bitreader_init(&br, unescaped, actual);

    guint pps_id = bitreader_read_ue(&br);
    guint sps_id = bitreader_read_ue(&br);

    if (pps_id > 63 || sps_id > 15)
        return FALSE;

    return TRUE;
}

gboolean guess_h265_ap(const guint8* payload, gsize size) {
    if (size < 3)
        return FALSE;

    guint offset = 2; // skip NAL header and 1 reserved byte if needed

    while (offset + 2 <= size) {
        guint16 nal_size = (payload[offset] << 8) | payload[offset + 1];
        offset += 2;
        if (offset + nal_size > size)
            break;

        const guint8* nal = payload + offset;
        guint8 nal_type = (nal[0] >> 1) & 0x3F;

        switch (nal_type) {
        case 32:
            if (parse_h265_vps(nal + 2, nal_size - 2)) return TRUE;
            break;
        case 33:
            if (parse_h265_sps(nal + 2, nal_size - 2)) return TRUE;
            break;
        case 34:
//            if (parse_h265_pps(nal + 2, nal_size - 2)) return TRUE;
            parse_h265_pps(nal + 2, nal_size - 2); // too vague to be reliable
            break;
        }

        offset += nal_size;
    }

    return FALSE;
}

gboolean guess_h265(const guint8* payload, gsize size) {
    if (size < 4)
        return FALSE;

    guint8 forbidden_zero_bit = (payload[0] >> 7) & 0x01;
    guint8 nal_unit_type = (payload[0] >> 1) & 0x3F;
    guint8 temporal_id_plus1 = payload[1] & 0x07;

    // Reject if forbidden bit is set (invalid)
    if (forbidden_zero_bit != 0)
        return FALSE;

    // Check that temporal ID is non-zero (must be >= 1)
    if (temporal_id_plus1 == 0)
        return FALSE;

    switch (nal_unit_type) {
    case 32: // VPS
        return parse_h265_vps(payload + 2, size - 2);
    case 34: // PPS
//        return parse_h265_pps(payload + 2, size - 2);
        return FALSE; // too vague to be reliable
    case 33: // SPS
        if (size >= 6) {
            if (parse_h265_sps(payload + 2, size - 2)) {
                return TRUE;
            }
        }
        break;
    case 48: // AP
        return guess_h265_ap(payload, size);
    default:
        break;
    }

    return FALSE; // VPS/PPS
}

gboolean guess_vp8(const guint8* payload, gsize size) {
    if (size < 10) return FALSE;

    guint8 desc = payload[0];
    guint8 X = (desc >> 7) & 1;
    guint8 N = (desc >> 5) & 1;
    guint8 S = (desc >> 4) & 1;

    guint offset = 1;

    // Handle extended control bits
    if (X) {
        if (offset >= size) return FALSE;
        guint8 ext = payload[offset++];
        if (ext & 0x80) offset++; // I
        if (ext & 0x40) offset++; // L
        if (ext & 0x20) offset += 2; // T & K
    }

    if (offset + 10 >= size) return FALSE;

    // Actual VP8 frame should start here
    const guint8* vp8_data = payload + offset;

    // Keyframe: first bit of byte 0 is 0
    gboolean is_keyframe = (vp8_data[0] & 0x01) == 0;

    if (!is_keyframe)
        return FALSE;

    // Validate VP8 sync code: 0x9d012a
    if (vp8_data[3] != 0x9d || vp8_data[4] != 0x01 || vp8_data[5] != 0x2a)
        return FALSE;

    return TRUE;
}

gboolean guess_av1(const guint8* payload, gsize size) {
    if (size < 5)
        return FALSE;

    guint offset = 1;

    guint8 first_byte = payload[offset];
    guint8 obu_forbidden_bit = (first_byte >> 7) & 0x1;
    guint8 obu_type = (first_byte >> 3) & 0xF;
    guint8 obu_extension_flag = (first_byte >> 2) & 0x1;
    guint8 obu_has_size_field = (first_byte >> 1) & 0x1;

    if (obu_forbidden_bit != 0)
        return FALSE;

    if (obu_type != 1 && obu_type != 2) // Must be Sequence Header OBU
        return FALSE;

    offset++;

    if (obu_extension_flag) {
        if (offset >= size)
            return FALSE;
        offset++; // Skip OBU extension header
    }

    if (!obu_has_size_field)
        return FALSE;

    // Read leb128-encoded OBU size
    guint obu_size = 0;
    guint shift = 0;
    while (offset < size && shift < 28) {
        guint8 leb = payload[offset++];
        obu_size |= (leb & 0x7F) << shift;
        if ((leb & 0x80) == 0)
            break;
        shift += 7;
    }

    if (obu_size == 0 || offset + obu_size > size)
        return FALSE;

    // Minimal validation of the sequence header fields
    const guint8* obu_payload = payload + offset;
    guint8 seq_profile = (obu_payload[0] >> 5) & 0x3;
    guint8 still_picture = (obu_payload[0] >> 4) & 0x1;
    guint8 reduced_still_picture_header = (obu_payload[0] >> 3) & 0x1;

    if (seq_profile > 2)
        return FALSE;

    return TRUE;
}

GuessedCodec try_guess_codec(guint* scores) {
    guint best_score = 0;
    GuessedCodec best_codec = CODEC_UNKNOWN;
    guint second_best = 0;

    for (int i = 0; i < CODEC_MAX_TYPE; ++i) {
        if (scores[i] > best_score) {
            second_best = best_score;
            best_score = scores[i];
            best_codec = (GuessedCodec)i;
        }
        else if (scores[i] > second_best) {
            second_best = scores[i];
        }
    }

    if (best_score >= GUESS_REQUIRED_MATCHES && best_score >= second_best + GUESS_REQUIRED_MATCHES_DIFF)
        return best_codec;

    return CODEC_UNKNOWN;
}
