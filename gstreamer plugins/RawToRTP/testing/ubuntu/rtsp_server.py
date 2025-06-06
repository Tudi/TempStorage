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

        # Create media factory with two streams
        self.factory = GstRtspServer.RTSPMediaFactory()

        '''
        # this works. Tested
        pipeline_description = (
            'udpsrc port=5000 caps="application/x-rtp,media=video,encoding-name=AV1,payload=96,clock-rate=90000" ! '
            'identity sync=false ! rtppassthroughpay pt=96 name=pay0 '
        )
        '''
        '''
        # confirmed working udp src with raw data into RTP stream
        pipeline_description = (
            '( udpsrc port=5001 caps="application/octet-stream" '
            '! rawtortp ! identity sync=false ! rtppassthroughpay pt=97 name=pay0 )'
        )
        '''
        
        # Combined pipeline: AV1 video + BIN data       
        pipeline_description = (
            '( udpsrc port=5000 caps="application/x-rtp,media=video,encoding-name=AV1,payload=96,clock-rate=90000" '
            '! identity sync=false ! rtppassthroughpay pt=96 name=pay0 ) '
            '( udpsrc port=5001 caps="application/octet-stream" '
            '! rawtortp ! identity sync=false ! rtppassthroughpay pt=97 name=pay1 )'
        )
                
        self.factory.set_launch(pipeline_description)

        # Ensure live mode and shared factory
        self.factory.set_shared(True)
        self.factory.set_latency(200)

        # Mount the factory on a stream path
        self.mounts.add_factory("/av1withrawcombined", self.factory)

        # Attach server to the default main context
        self.server.attach(None)

        print("RTSP server is ready at rtsp://127.0.0.1:8554/av1withrawcombined")

    def run(self):
        loop = GLib.MainLoop()
        try:
            loop.run()
        except KeyboardInterrupt:
            print("RTSP server stopped")


if __name__ == "__main__":
    server = RTSPServer()
    server.run()
