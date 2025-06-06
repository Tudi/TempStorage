import socket
import os
import random
import time

# Configuration
UDP_IP = "127.0.0.1"
UDP_PORT = 5001

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Sending data to {UDP_IP}:{UDP_PORT} — press Ctrl+C to stop")

try:
    counter = 1
    while True:
        # Generate random byte payload between 10 and 50 bytes
        pkt_header_visual_marker = "pkthdr"
        size = 2 + 8 + len(pkt_header_visual_marker)
        print(f"\rChunk = {counter} Size = {size}", end="")

        # Convert counter to 8-byte ASCII
        size_bytes = str(size).encode('ascii').ljust(2, b' ')[:2]
        pkt_hdr_bytes = str(pkt_header_visual_marker).encode('ascii').ljust(len(pkt_header_visual_marker), b' ')[:len(pkt_header_visual_marker)]
        counter_bytes = str(counter).encode('ascii').ljust(8, b' ')[:8]

        # Final payload
        data = size_bytes + pkt_hdr_bytes + counter_bytes

        # Send over UDP
        sock.sendto(data, (UDP_IP, UDP_PORT))

        counter += 1

        # Sleep 1–20 ms
        delay = random.uniform(0.001, 0.020)
        time.sleep(delay)

except KeyboardInterrupt:
    print("\nStopped by user.")

finally:
    sock.close()
