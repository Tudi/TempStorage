#!/usr/bin/env python3
import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstRtspServer', '1.0')

from gi.repository import Gst, GstRtspServer, GObject, GLib

Gst.init(None)

class RTSPServer:
    def __init__(self):
        # Create RTSP server and mount points
        self.server = GstRtspServer.RTSPServer()
        self.server.set_service("8554")

        self.mounts = self.server.get_mount_points()
        self.factory = GstRtspServer.RTSPMediaFactory()

        # 🔄 Updated to consume RTP H.264
        pipeline_description = (
            '(udpsrc port=5000 caps="application/x-rtp, media=video, encoding-name=H264, payload=96, clock-rate=90000" ! '
            'rtph264depay ! h264parse config-interval=1 ! rtph264pay pt=96 name=pay0 ) '
            '( udpsrc port=5001 caps="application/octet-stream" '
            '! klvtortp maxupm=30 klvallowed=1,2 ! identity sync=false ! rtppassthroughpay pt=97 name=pay1 )'
        )

        self.factory.set_launch(pipeline_description)
        self.factory.set_shared(True)
        self.factory.set_latency(200)

        self.mounts.add_factory("/vidwithklvcombined", self.factory)

        self.server.attach(None)

        print("✅ RTSP server is ready at rtsp://127.0.0.1:8554/vidwithklvcombined")

    def run(self):
        loop = GLib.MainLoop()
        try:
            loop.run()
        except KeyboardInterrupt:
            print("RTSP server stopped")

if __name__ == "__main__":
    server = RTSPServer()
    server.run()
