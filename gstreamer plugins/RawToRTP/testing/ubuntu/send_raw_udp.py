import socket
import os
import random
import time

# Configuration
UDP_IP = "127.0.0.1"
UDP_PORT = 5001

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Sending random data to {UDP_IP}:{UDP_PORT} — press Ctrl+C to stop")

try:
    chunks_sent = 0
    while True:
        # Generate random byte payload between 10 and 50 bytes
        size = random.randint(10, 50)
        print(f"\rChunk = {chunks_sent} Size = {size}", end = "")
        chunks_sent += 1
        data = os.urandom(size)

        # Send over UDP
        sock.sendto(data, (UDP_IP, UDP_PORT))

        # Sleep 1–20 ms
        delay = random.uniform(0.001, 0.020)
        time.sleep(delay)

except KeyboardInterrupt:
    print("\nStopped by user.")

finally:
    sock.close()
