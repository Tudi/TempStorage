import socket
import os
import random
import time

def add_klv(buffer: bytes, key: int, value: bytes) -> bytes:
    """Append a KLV entry to the buffer."""
    if not (0 <= key <= 255):
        raise ValueError("Key must be a single byte (0-255).")
    if len(value) > 255:
        raise ValueError("Value too long for 1-byte length field.")
    return buffer + bytes([key, len(value)]) + value



# Configuration
UDP_IP = "127.0.0.1"
UDP_PORT = 5001

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Sending data to {UDP_IP}:{UDP_PORT} — press Ctrl+C to stop")

try:
    counter = 1
    while True:
        buffer = b''

        pkt_header_visual_marker = "pkthdr"
        counter_bytes = str(counter).encode('ascii').ljust(8, b' ')[:8]
        pkt_hdr_bytes = pkt_header_visual_marker.encode('ascii')

        # Add KLV entries (keys are arbitrary and unique per field)
        buffer = add_klv(buffer, key=0x01, value=pkt_hdr_bytes)
        buffer = add_klv(buffer, key=0x02, value=counter_bytes)

        # Send over UDP
        sock.sendto(buffer, (UDP_IP, UDP_PORT))

        print(f"\rChunk = {counter} Size = {len(buffer)}", end="")

        counter += 1
        time.sleep(random.uniform(0.001, 0.020))

except KeyboardInterrupt:
    print("\nStopped by user.")
finally:
    sock.close()
