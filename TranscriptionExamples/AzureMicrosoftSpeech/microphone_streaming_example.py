#!/usr/bin/env python
# coding: utf-8

# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE.md file in the project root for full license information.
"""
Speech recognition samples for the Microsoft Cognitive Services Speech SDK
"""

import time
import wave
import json

try:
    import azure.cognitiveservices.speech as speechsdk
except ImportError:
    print("""
    Importing the Speech SDK for Python failed.
    Refer to
    https://docs.microsoft.com/azure/cognitive-services/speech-service/quickstart-python for
    installation instructions.
    """)
    import sys
    sys.exit(1)


# Set up the subscription info for the Speech Service:
# Replace with your own subscription key and service region (e.g., "westus").
speech_key, service_region = "2ef861e6c1334efc931c630327bd8a66", "eastus"

def speech_recognize_from_microphone():
    """performs keyword-triggered speech recognition with input microphone"""
    speech_config = speechsdk.SpeechConfig(subscription=speech_key, region=service_region)
    speech_config.request_word_level_timestamps()
    speech_config.output_format = speechsdk.OutputFormat(1)

    speech_recognizer = speechsdk.SpeechRecognizer(speech_config=speech_config)

    done = False

    def stop_cb(evt):
        """callback that signals to stop continuous recognition upon receiving an event `evt`"""
        print('CLOSING on {}'.format(evt))
        nonlocal done
        done = True

    def recognized_cb(evt):
        """callback for recognized event"""
        if evt.result.reason == speechsdk.ResultReason.RecognizedSpeech:
            #print('RECOGNIZED: {}'.format(evt.result.text))
            #print('All params: {}'.format(evt.result))
            #print(evt.result.json)
            response = json.loads(evt.result.json)
            #print('All params: {}'.format(response))
            Text = response["DisplayText"]
            duration = 0;
            for word in response["NBest"][0]["Words"]:
                duration += word["Duration"]
            duration = duration / 10000000
            print("dur :"+str(duration)+" text: " + Text)

    # Connect callbacks to the events fired by the speech recognizer
    speech_recognizer.recognized.connect(recognized_cb)
    speech_recognizer.session_started.connect(lambda evt: print('SESSION STARTED: {}'.format(evt)))
    speech_recognizer.session_stopped.connect(lambda evt: print('SESSION STOPPED {}'.format(evt)))
    speech_recognizer.canceled.connect(lambda evt: print('CANCELED {}'.format(evt)))
    # stop continuous recognition on either session stopped or canceled events
    speech_recognizer.session_stopped.connect(stop_cb)
    speech_recognizer.canceled.connect(stop_cb)

    # Start keyword recognition
    speech_recognizer.start_continuous_recognition()

    while not done:
        time.sleep(.5)

    speech_recognizer.stop_continuous_recognition()

speech_recognize_from_microphone()