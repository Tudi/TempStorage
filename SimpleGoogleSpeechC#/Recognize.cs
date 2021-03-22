using CommandLine;
using Google.Cloud.Speech.V1;
using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;

namespace GoogleCloudSamples
{
    public class Recognize
    {
        static async Task<object> StreamingMicRecognizeAsync(int seconds)
        {
/*            SpeechClientBuilder builder = new SpeechClientBuilder
            {
                CredentialsPath = credentialsFilePath
            };
            SpeechClient speech = builder.Build();*/
            var speech = SpeechClient.Create();
            var streamingCall = speech.StreamingRecognize();
            // Write the initial request with the config.
            await streamingCall.WriteAsync(
                new StreamingRecognizeRequest()
                {
                    StreamingConfig = new StreamingRecognitionConfig()
                    {
                        Config = new RecognitionConfig()
                        {
                            Encoding =
                            RecognitionConfig.Types.AudioEncoding.Linear16,
                            SampleRateHertz = 16000,
                            LanguageCode = "en-GB",
                            // The `model` value must be one of the following:
                            // "video", "phone_call", "command_and_search", "default"
                            Model = "phone_call",
//                            EnableWordTimeOffsets = true,
/*                            DiarizationConfig = new SpeakerDiarizationConfig()
                            {
                                EnableSpeakerDiarization = true,
                                MinSpeakerCount = 2
                            }*/
                            UseEnhanced = true,
                        },
                        InterimResults = false,
                    }
                });
            // Print responses as they arrive.
            Task printResponses = Task.Run(async () =>
            {
                var responseStream = streamingCall.GetResponseStream();
                while (await responseStream.MoveNextAsync())
                {
                    StreamingRecognizeResponse response = responseStream.Current;
                    foreach (StreamingRecognitionResult result in response.Results)
                    {
                        foreach (SpeechRecognitionAlternative alternative in result.Alternatives)
                        {
                            Console.WriteLine(alternative.Transcript);
                        }
                    }
                }
            });
            // Read from the microphone and stream to API.
            object writeLock = new object();
            bool writeMore = true;
            var waveIn = new NAudio.Wave.WaveInEvent();
            waveIn.DeviceNumber = 0;
            waveIn.WaveFormat = new NAudio.Wave.WaveFormat(16000, 1);
            waveIn.DataAvailable +=
                (object sender, NAudio.Wave.WaveInEventArgs args) =>
                {
                    lock (writeLock)
                    {
                        if (!writeMore)
                        {
                            return;
                        }

                        streamingCall.WriteAsync(
                            new StreamingRecognizeRequest()
                            {
                                AudioContent = Google.Protobuf.ByteString
                                    .CopyFrom(args.Buffer, 0, args.BytesRecorded)
                            }).Wait();
                    }
                };
            waveIn.StartRecording();
            Console.WriteLine("Speak now.");
            await Task.Delay(TimeSpan.FromSeconds(seconds));
            // Stop recording and shut down.
            waveIn.StopRecording();
            lock (writeLock)
            {
                writeMore = false;
            }

            await streamingCall.WriteCompleteAsync();
            await printResponses;
            return 0;
        }

        public static int Main(string[] args)
        {
            var result = Task.Run(async() => await StreamingMicRecognizeAsync(100000)).Result;
            Console.WriteLine("Finished listening to the microphone.");
            return 0;
        }
    }
}