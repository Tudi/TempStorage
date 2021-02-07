import asyncio
import time
import pyaudio
import wave
import socket
import sys
import struct
import binascii

from amazon_transcribe.client import TranscribeStreamingClient
from amazon_transcribe.handlers import TranscriptResultStreamHandler
from amazon_transcribe.model import TranscriptEvent

FORMAT = pyaudio.paInt16 # data type formate
CHANNELS = 1 # Adjust to your number of channels
RATE = 16000 # Sample Rate
CHUNK = 1024 # Block Size
SpeakerTimeShares=dict()
ProgramIsRunning=1
sock=0
InputType=2 #microphone or PC

def GetTotalSpeakTime():
    TotalTime=0
    for speaker in SpeakerTimeShares:
        TotalTime += SpeakerTimeShares[speaker]
    return TotalTime
    
def SendSpeakerStatusOverNetwork():
    if sock == 0:
        return
    try:         
        struct_setup = "I I"
        values = [int(len(SpeakerTimeShares))]
        values.append(int(InputType))
        for speaker in SpeakerTimeShares:
            values.append(int(SpeakerTimeShares[speaker] * 10000))
            struct_setup = struct_setup + " I"
        #print("Struct layout : " + str(struct_setup))
        packer = struct.Struct(struct_setup)
        packed_data = packer.pack(*values)
        size_packer = struct.Struct("I")
        size_packed = size_packer.pack((len(packed_data)+4))
        #print("would send over network size %d" % len(packed_data))
        
        #now send each of the timeshares
        sock.sendall(size_packed)
        sock.sendall(packed_data)
    except:
        exit(1)
    
class MyEventHandler(TranscriptResultStreamHandler):
    async def handle_transcript_event(self, transcript_event: TranscriptEvent):
        #print("Received a response from AWS. Listing transcription results : ")
        results = transcript_event.transcript.results
        GotSomeValue = 0
        for result in results:
            if result.is_partial == True :
                continue
            for alt in result.alternatives:
                for item in alt.items:
                    duration = item.end_time - item.start_time
                    if duration == 0:
                        continue
                    if item.speaker in SpeakerTimeShares:
                        SpeakerTimeShares[item.speaker] = SpeakerTimeShares[item.speaker] + duration
                    else:
                        SpeakerTimeShares[item.speaker] = duration
                    TimeShare = SpeakerTimeShares[item.speaker] / GetTotalSpeakTime()
                    if InputType == 2:
                        print("PC Speaker : Label:" + str(item.speaker) + " Timeshare:" + str(TimeShare) + " Content:" + str(item.content))
                    GotSomeValue = 1
        if GotSomeValue == 1:
            SendSpeakerStatusOverNetwork()
            pass

async def basic_transcribe():
    # Setup up our client with our chosen AWS region
    client = TranscribeStreamingClient(region="us-west-2")

    # Start transcription to generate our async stream
    stream = await client.start_stream_transcription(
        language_code="en-US",
        media_sample_rate_hz=RATE,
        media_encoding="pcm",
        show_speaker_label=True,
    )

    async def write_chunks():
        print("listening to audio")
        while ProgramIsRunning:
            chunk = audiostream.read(CHUNK)
            await stream.input_stream.send_audio_event(audio_chunk=chunk)
            #print("Sent 1k audio")
        await stream.input_stream.end_stream()
  
    # Instantiate our handler and start processing events
    handler = MyEventHandler(stream.output_stream)
    await asyncio.gather(write_chunks(), handler.handle_events())
    
def ConnectToServer():
    # Create a TCP/IP socket
    global sock
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Connect the socket to the port where the server is listening
    server_address = ('localhost', 10000)
    print('connecting to %s port %s' % server_address)
    sock.connect(server_address)
    
#try to send values to the server
ConnectToServer()

# Startup pyaudio instance
audio = pyaudio.PyAudio()

dev_index = audio.get_default_input_device_info()["hostApi"]
for i in range(audio.get_device_count()):
    dev = audio.get_device_info_by_index(i)
    #print('dev_index', dev['index'], " ", dev['name'], " hostapi ", dev['hostApi'])
    #is_format_supported(rate, input_device=None, input_channels=None, input_format=None, output_device=None, output_channels=None, output_format=None)
    if (dev['name'].find('Stereo Mix') == 0 and dev['hostApi'] == 0):
        dev_index = dev['index'];
        print('Selecting input device : dev_index', dev_index, " ", dev['name'])
        
audiostream = audio.open(format=FORMAT, channels=CHANNELS,rate=RATE, input=True, input_device_index=dev_index,frames_per_buffer=CHUNK)

#start the transcription
loop = asyncio.get_event_loop()
loop.run_until_complete(basic_transcribe())
loop.close()

# Stop Recording
audiostream.stop_stream()
audiostream.close()
audio.terminate()
