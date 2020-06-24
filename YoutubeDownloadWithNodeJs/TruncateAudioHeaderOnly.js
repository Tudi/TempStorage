module.exports = {
	TruncateAudioHeader : function(AudioInfo, CallBack)
	{
		const path = require('path');
		const fs   = require('fs');
		
		if( AudioInfo.ContainerType === 'webm' )
			TruncateWEBMHeader(AudioInfo, CallBack);
		else if( AudioInfo.ContainerType === 'm4a' || AudioInfo.ContainerType === 'mp4' || AudioInfo.ContainerType === 'm4p')
			TruncateMP4Header(AudioInfo, CallBack);
		else
			console.log("Unsupported container type : ", AudioInfo.ContainerType);
		
		// mp4 files are containers and might not contain info about the data stream. The data stream itself might contain all audio details. We need to decode in order to download ... not good
		// youtube solves this by creating a manifest file. Presence of this manifest file can be signaled by isDashMPD=true . This file will contain seek information for all bitrates
		// chances are that this file will contain seek information and all we need to do is to jump to specific second
		function TruncateMP4Header(AudioInfo, CallBack)
		{
			console.log("File type is mp4");
			var mp4 = require('mp4-stream');
			AudioInfo.AudioSamplingRate = 0;
			AudioInfo.AudioChannelCount = 0;
			AudioInfo.AudioBitDepth = 0;
			var decode = mp4.decode()
			decode = mp4.decode();
			var ReadStream = fs.createReadStream(AudioInfo.TempFileName)
			ReadStream.pipe(decode)
			AudioInfo.HeaderSize = 0;
			decode.on('box', function (headers) 
			  {
					if (headers.type === 'mdat' || headers.type === 'moof')
					{
					}
					else
					{
						AudioInfo.HeaderSize += headers.length;
					}
					decode.ignore();
				}
			  )
			ReadStream.on("end", () => { 
				console.log("Header size in bytes :", AudioInfo.HeaderSize);
				
				const fd = fs.openSync(AudioInfo.TempFileName, 'r+');
				fs.ftruncate(fd, parseInt(AudioInfo.HeaderSize), (err) => { if(err)console.log(err);});

				console.log("Audio bitrate : ", AudioInfo.AudioBitrate);

				AudioInfo.AudioHasSeekInfo = 0;
				AudioInfo.KeyFrameStartPositions = null;
				console.log("Audio has NO seek info");
			
				//continue execution with the next function in chain
				CallBack(AudioInfo);
			});		
		}
		
		function TruncateWEBMHeader(AudioInfo, CallBack)
		{
			console.log("File type is EBL");
			const matroska = require('matroska');

			var decoder = new matroska.Decoder();
			decoder.parse(AudioInfo.TempFileName, function ParseHeaderCallback(error, document){
				if (error) 
				{
					//console.error(error);
					return;
				}
				//there has got to be a better way to parse this
				var FormatedHeader = document.print();
				//console.log("Full header : ", FormatedHeader);
				AudioInfo.HeaderSize = GetHeaderSize(FormatedHeader);
				console.log("Header size in bytes :", AudioInfo.HeaderSize);
				
				AudioInfo.AudioLen = GetIntValFromDoc("Duration  f[4]=",FormatedHeader);
				console.log("Audio has length in ms : ", AudioInfo.AudioLen);
			
				AudioInfo.AudioSamplingRate = GetIntValFromDoc("SamplingFrequency  f[4]=",FormatedHeader);
				console.log("Audio sampling rate : ", AudioInfo.AudioSamplingRate);
			
				AudioInfo.AudioChannelCount = GetIntValFromDoc("Channels  u[1]=",FormatedHeader);
				console.log("Audio channel count : ", AudioInfo.AudioChannelCount);
			
				AudioInfo.AudioBitDepth = GetIntValFromDoc("BitDepth  u[1]=",FormatedHeader);
				console.log("Audio BitDeth : ", AudioInfo.AudioBitDepth);

				if( GetIntValFromDoc("CueClusterPosition  u[",FormatedHeader) != null )
				{
					AudioInfo.AudioHasSeekInfo = 1; 
					AudioInfo.KeyFrameStartPositions = GenerateChunkIndexes(FormatedHeader);
					console.log("Audio has seek info");
				}
				else
				{
					AudioInfo.AudioHasSeekInfo = 0;
					AudioInfo.KeyFrameStartPositions = null;
					console.log("Audio has NO seek info");
				}

				const fd = fs.openSync(AudioInfo.TempFileName, 'r+');
				fs.ftruncate(fd, parseInt(AudioInfo.HeaderSize), (err) => { if(err)console.log(err);});
				
				//continue execution with the next function in chain
				CallBack(AudioInfo);
			});

			function GenerateChunkIndexes(FormatedHeader)
			{
				var CuePositionInfoArray = FormatedHeader.split("CueClusterPosition  u[");
				var CueCount = 0;
				var ret = [];
				for(let val of CuePositionInfoArray)
				{
					var subpart = val.split("\n");
					var CueIndex = subpart[0];
					CueIndex = CueIndex.replace("1]=","");
					CueIndex = CueIndex.replace("2]=","");
					CueIndex = CueIndex.replace("3]=","");
					CueIndex = CueIndex.replace("4]=","");
					ret[CueCount]={};
					ret[CueCount]['pos']=strToInt(CueIndex);
					CueCount++;
				}
				//now generate timing info for the positions
				var CueTimeInfoArray = FormatedHeader.split("CueTime  u[");
				CueCount = 0;
				for(let val of CueTimeInfoArray)
				{
					var subpart = val.split("\n");
					var CueIndex = subpart[0];
					CueIndex = CueIndex.replace("1]=","");
					CueIndex = CueIndex.replace("2]=","");
					CueIndex = CueIndex.replace("3]=","");
					CueIndex = CueIndex.replace("4]=","");
					ret[CueCount]['time']=strToInt(CueIndex);
					CueCount++;
				}
				ret.shift(1);
				return ret;			
			}
			
			function strToInt(val)
			{
				ret = val.replace(" ","");
				ret = ret.replace("\t","");
				ret = parseInt(ret);
				return ret;
			}
				
			function GetHeaderSize(doc)
			{
				var Parts = doc.split("\n");
				for(let val of Parts)
					if(val.indexOf("* Cluster  children") > 0 )
					{
						var subparts = val.split("#");
						if( subparts.length > 0)
						{
							return strToInt(subparts[0]);
						}
						else
							return 0;
					}
				return 0;
			}
			
			function GetIntValFromDoc(key, doc)
			{
				var parts = doc.split(key);
				if( parts.length < 2 )
					return null;
				var part = parts[1].split("\n")
				if( parts.length < 1 )
					return null;
				return strToInt(part[0]);
			}
		}
	}
};