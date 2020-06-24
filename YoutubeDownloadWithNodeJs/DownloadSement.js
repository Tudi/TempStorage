module.exports = {
	DownloadAudioSegment : function( AudioInfo, CallBack)
	{
		const fs = require('fs');
		const ytdlcore = require('ytdl-core');

		if( AudioInfo.StartSec == 0 && AudioInfo.LengthSec >= AudioInfo.AproxDuration )
		{
			DownloadFullAudio(AudioInfo, CallBack);
			return;
		}

		DecideDownloadParameters( AudioInfo );

		if( AudioInfo.AudioHasSeekInfo == 0 )
		{
			var AudioByteRate;
			if( AudioInfo.AudioSamplingRate > 0 && AudioInfo.AudioChannelCount > 0 && AudioInfo.AudioBitDepth > 0 )
				AudioByteRate = AudioInfo.AudioSamplingRate * AudioInfo.AudioBitDepth * AudioInfo.AudioChannelCount / 8 / 10;
			else 
				AudioByteRate =  AudioInfo.AudioBitrate / 8;
			const AudioChunkCount = AudioInfo.LengthSec * 1000 / 10000;// lucky guess :(
			var ChunkHeaderSize;
			if( AudioInfo.ContainerType === 'webm' )
				ChunkHeaderSize = AudioChunkCount * (7 + 4); // have to add 4 bytes for each stream chunk as blocksize+timestamp store
			else
				ChunkHeaderSize = AudioChunkCount * (1900);  // moof size might be around 1800
			const AudioDownloadByteCount = parseInt(AudioByteRate * AudioInfo.LengthSec + ChunkHeaderSize); 
			
			AudioInfo.DownloadStartAtByte = parseInt(AudioByteRate * AudioInfo.StartSec + ChunkHeaderSize);
			AudioInfo.DownloadStopAtByte = AudioInfo.DownloadStartAtByte + AudioDownloadByteCount;
		}

		if( AudioInfo.AudioHasSeekInfo == 1 )
			DownloadAudioSegment_ByteRange(AudioInfo, CallBack);
		else
		{
			if( AudioInfo.ContainerType === 'm4a' || AudioInfo.ContainerType === 'mp4' || AudioInfo.ContainerType === 'm4p')
				DownloadAudioSegment_NoRangeSupport(AudioInfo, CallBack);
			else if( AudioInfo.SupportsRange == 1)
				DownloadAudioSegment_ByteRange(AudioInfo, CallBack);
			else
				DownloadAudioSegment_NoRangeSupport(AudioInfo, CallBack);
		}

		function DownloadFullAudio(AudioInfo, CallBack)
		{	
			var video = ytdlcore.downloadFromInfo(AudioInfo.info, { format: AudioInfo.format });
			video.pipe(fs.createWriteStream(AudioInfo.TempFileName));
			video.on('end', function() {
				CallBack(AudioInfo);
			});
			video.once('error', (err) => {			
				console.log("Error : ", err);
				callback(null, { status: "503", statusDescription: "DownloadFullAudio" });
				return;             
			});
		}
		
		function DownloadAudioSegment_NoRangeSupport( AudioInfo, CallBack)
		{
			const OutputFile = AudioInfo.TempFileName;
			const HeaderSize = AudioInfo.HeaderSize;
			var AudioDownloadByteCount = AudioInfo.DownloadStopAtByte - AudioInfo.DownloadStartAtByte;

			console.log("Try to download ", AudioDownloadByteCount, " bytes as audio chuncks. Start is at ", AudioInfo.StartSec);
			
			var BytesDownloadedSoFar = 0;
			var video;
			if( AudioInfo.info != null )
				video = ytdlcore.downloadFromInfo(AudioInfo.info, { format: AudioInfo.format, begin: '\'' + AudioInfo.StartSec + 's\'' });
			else
				video = ytdlcore(AudioInfo.url, { format: AudioInfo.format, begin: '\'' + AudioInfo.StartSec + 's\'', filter: 'audioonly', quality: 'highestaudio'});
			
			var OutStream = fs.createWriteStream(OutputFile, { 'flags': 'a', 'encoding': null, 'mode': 0666});
			video.pipe(OutStream);
			video.on('progress', (chunkLength, downloaded, total) => {
			  BytesDownloadedSoFar += chunkLength;
			  //if we have enough data, end the downloading
			  if( BytesDownloadedSoFar > AudioDownloadByteCount + 1)
			  {
				console.log("Interrupted download. We received ", BytesDownloadedSoFar, " bytes, but only asked for ", AudioDownloadByteCount);
				video.destroy();
				//close the stream
				OutStream.end();
				// required due to chuncks having fixed sizes that we do not need
				TruncateSegmentToByteCount(OutputFile, HeaderSize + AudioDownloadByteCount);
				CallBack(AudioInfo);
				return;
			  }
			  //else
			//	  console.log("Chunk length ",chunkLength," Downloaded so far: ",downloaded," Total: ",total);
			});
			video.on('end', function() {
				console.log("Triggered on end event");
				CallBack(AudioInfo);
			});
			video.once('error', (err) => {
				console.log("Error: ", err);
				callback(null, { status: "503", statusDescription: "downloadFromInfo" });
				return;             
			});
			//if we got here, it means we needed more bytes than the whole audio file :O
			//console.log("Done downloading");
		}

		function TruncateSegmentToByteCount(OutputFileName, AudioDownloadByteCount)
		{
			console.log("Started truncating. Expected file size is ", AudioDownloadByteCount);
			//if we downloaded more bytes than we needed, truncate the file to the desired byte count
			const fd = fs.openSync(OutputFileName, 'r+');
			fs.ftruncateSync(fd, parseInt(AudioDownloadByteCount), (err) => { if(err)console.log(err);});
			fs.closeSync(fd);
		}
		
		function DownloadAudioSegment_ByteRange(AudioInfo, CallBack)
		{
			const DownloadStartAtByte = AudioInfo.DownloadStartAtByte;
			const DownloadStopAtByte = AudioInfo.DownloadStopAtByte;
			const OutputFile = AudioInfo.TempFileName;
			console.log("Try to download ", DownloadStartAtByte, " - ", DownloadStopAtByte, " as audio chuncks");
			
			var BytesDownloadedSoFar = 0;
			var AudioDownloadByteCount = DownloadStopAtByte - DownloadStartAtByte + 1;
		
			var video;
			if( AudioInfo.info != null )
				video = ytdlcore.downloadFromInfo(AudioInfo.info, { format: AudioInfo.format, range: { start: DownloadStartAtByte, end: DownloadStopAtByte } });
			else
				video = ytdlcore(AudioInfo.url, { format: AudioInfo.format, range: { start: DownloadStartAtByte, end: DownloadStopAtByte }, filter: 'audioonly', quality: 'highestaudio'});
			
			var OutStream = fs.createWriteStream(OutputFile, { 'flags': 'a', 'encoding': null, 'mode': 0666});
			video.pipe(OutStream);
			video.on('progress', (chunkLength, downloaded, total) => {
			  BytesDownloadedSoFar += chunkLength;
			  //if we have enough data, end the downloading
			  if( BytesDownloadedSoFar > AudioDownloadByteCount )
			  {
				console.log("Interrupted download. We received ", BytesDownloadedSoFar, " bytes, but only asked for ", AudioDownloadByteCount);
				video.destroy();
				//close the stream
				OutStream.end();
				// required due to chuncks having fixed sizes that we do not need
				TruncateSegmentToByteCount(OutputFile, HeaderSize + AudioDownloadByteCount);
				CallBack(AudioInfo);
				return;
			  }
			  //else
			//	  console.log("Chunk length ",chunkLength," Downloaded so far: ",downloaded," Total: ",total);
			});
			video.on('end', function() {
				//console.log("Triggered on end event");
				CallBack(AudioInfo);
			});
			//if we got here, it means we needed more bytes than the whole audio file :O
			//console.log("Done downloading");
			video.once('error', (err) => {
				
				callback(null, { status: "503", statusDescription: "downloadFromInfo" });
				return;             
			});
		}
		
		//Most of the parameters are unknown for MPEG Dash formats !!
		function DecideDownloadParameters(AudioInfo)
		{
			var AudioLen = AudioInfo.AudioLen;
			var segmentoffset = AudioInfo.StartSec;
			var segmentduration = AudioInfo.LengthSec;
			var segmentstart = "start";
			
			if(segmentduration == null || segmentduration == 0)
				segmentduration = AudioLen / 1000 - segmentoffset;
			if( AudioInfo.LengthSec == null || AudioInfo.LengthSec <= 0 )
				AudioInfo.LengthSec = AudioInfo.AudioLen;
			
			//has highest priority according to specification. If segment is larger than the whole duration, we download everything
			/*if(segmentduration * 1000 >= AudioLen )
			{
				//no need to risk corrupting the audio file. Download as youtube stores it
				DownloadFullAudio();
				return;
			}*/
			
			var SegmentStartMS = 0;
			
			//from what Millisecond do we need to download from ?
			if(segmentstart == "start")
				SegmentStartMS = 0;
			else if(segmentstart == "middle")
				SegmentStartMS = AudioLen / 2;
			
			//add the offset to the start marker
			SegmentStartMS = SegmentStartMS + segmentoffset * 1000;
			
			//if offset points before the start of the audio, we pinch it off
			if( SegmentStartMS < 0 )
				SegmentStartMS = 0;
			
			//Most respected parameter is still segment duration. Try to make it so it will have the desired duration
			if(SegmentStartMS + segmentduration * 1000 > AudioLen )
				SegmentStartMS = AudioLen - segmentduration * 1000;
			
			//if offset points before the start of the audio, we pinch it off
			if( SegmentStartMS < 0 )
				SegmentStartMS = 0;
			
			//non live streams will have seek info
			if( AudioInfo.AudioHasSeekInfo == 1)
			{
				//check byte download start position
				var DownloadStartAtByte = 0;
				var DownloadStopAtByte = 0;
				var SegmentEndMS = SegmentStartMS + segmentduration * 1000;
				for(i=1;i<AudioInfo.KeyFrameStartPositions.length;i++)
				{
					if(DownloadStartAtByte == 0 && AudioInfo.KeyFrameStartPositions[i]['time'] >= SegmentStartMS)
						DownloadStartAtByte = AudioInfo.KeyFrameStartPositions[i]['pos'];
					if(DownloadStopAtByte == 0 && AudioInfo.KeyFrameStartPositions[i]['time'] >= SegmentEndMS && DownloadStartAtByte != AudioInfo.KeyFrameStartPositions[i]['pos'])
						DownloadStopAtByte = AudioInfo.KeyFrameStartPositions[i]['pos'];
				}
				if(DownloadStartAtByte == 0)
					DownloadStartAtByte = AudioInfo.KeyFrameStartPositions[0]['pos'];
				if(DownloadStopAtByte == 0)
					DownloadStopAtByte = AudioInfo.KeyFrameStartPositions[AudioInfo.KeyFrameStartPositions.length-1]['pos'] + 500000; // magic number 500k should point after the end of the file
				AudioInfo.DownloadStartAtByte = DownloadStartAtByte;
				AudioInfo.DownloadStopAtByte = DownloadStopAtByte;
				//console.log("According to seek info, we should download byte range ", DownloadStartAtByte," - ", DownloadStopAtByte);
			}

			AudioInfo.SegmentStartMS = SegmentStartMS;	
		}
	},
};