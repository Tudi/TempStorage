import struct
import sys

def parse_rtp_file(filename):
    with open(filename, 'rb') as f:
        data = f.read()

    offset = 0
    packet_count = 0
    min_packet_size = 12  # minimum RTP header size

    expected_ssrc = None
    expected_pt = 97  # matches your HardcodedRTPPayloadType

    while offset + min_packet_size <= len(data):
        header = data[offset:offset+12]
        if len(header) < 12:
            break

        b1, b2, seqnum, timestamp, ssrc = struct.unpack('!BBHII', header)

        version = (b1 >> 6) & 0x03
        padding = (b1 >> 5) & 0x01
        extension = (b1 >> 4) & 0x01
        cc = b1 & 0x0F
        marker = (b2 >> 7) & 0x01
        payload_type = b2 & 0x7F

        if version != 2:
            print(f"[{offset}] Invalid RTP version: {version}")
            break

        if expected_ssrc is None:
            expected_ssrc = ssrc
        elif ssrc != expected_ssrc:
            print(f"[{offset}] Unexpected SSRC: {ssrc} (expected {expected_ssrc})")
            break

        if payload_type != expected_pt:
            print(f"[{offset}] Unexpected payload type: {payload_type} (expected {expected_pt})")
            break

        header_len = 12 + cc * 4
        next_offset = offset + header_len

        while next_offset + 12 <= len(data):
            candidate_header = data[next_offset:next_offset+12]
            cb1, cb2, cseq, cts, cssrc = struct.unpack('!BBHII', candidate_header)
            cversion = (cb1 >> 6) & 0x03
            cpt = cb2 & 0x7F

            if (
                cversion == 2 and
                cssrc == expected_ssrc and
                cpt == expected_pt
            ):
                break  # Found next RTP packet

            next_offset += 1

        payload_len = next_offset - (offset + header_len)
        total_len = header_len + payload_len

        print(f"Packet #{packet_count}: offset={offset}, v={version}, p={padding}, x={extension}, "
              f"cc={cc}, m={marker}, pt={payload_type}, seq={seqnum}, ts={timestamp}, ssrc={ssrc}, "
              f"header_len={header_len}, payload_len={payload_len}")

        offset += total_len
        packet_count += 1


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python check_output_rtp.py <rtp-file>")
    else:
        parse_rtp_file(sys.argv[1])
