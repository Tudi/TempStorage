module.exports = {
	DownloadAudioHeader : function (AudioInfo, CallBack, HeaderSize)
	{
		const path = require('path');
		const fs   = require('fs');
		const ytdlcore = require('ytdl-core');

		// would be great if someone could guess the size of the header. We will try to download more bytes than we require
		if( HeaderSize == null )
			HeaderSize = 1500;
		
		var video;
		if( AudioInfo.info != null )
			video = ytdlcore.downloadFromInfo(AudioInfo.info, { format: AudioInfo.format, range: { start: 0, end: HeaderSize } });
		else
			video = ytdlcore(AudioInfo.url, { format: AudioInfo.format, range: { start: 0, end: HeaderSize }, filter: 'audioonly', quality: 'highestaudio' });
		
		var BytesDownloadedSoFar = 0;
		OutStream = fs.createWriteStream(AudioInfo.TempFileName);
		video.pipe(OutStream);
		//live streams will not support"range" option. We need to cut the connection as soon as possible
		video.on('progress', (chunkLength, downloaded, total) => {
			BytesDownloadedSoFar += chunkLength;
		  //if we have enough data, end the downloading
		  if( BytesDownloadedSoFar > HeaderSize + 1)
		  {
			console.log("Interrupted download. We received ", BytesDownloadedSoFar, " bytes, but only asked for ", HeaderSize);
			video.destroy();
			//close the stream
			OutStream.end();
			// call next function in the chain
			CallBack(AudioInfo);
			return;
		  }
		  //else
		//	  console.log("Chunk length ",chunkLength," Downloaded so far: ",downloaded," Total: ",total);
		});
		//chain the next callback as soon as we are done downloading
		video.on('end', function() {
			CallBack(AudioInfo);
		});
        video.once('error', (err) => {
            
            callback(null, { status: "503", statusDescription: "downloadFromInfo" });
            return;             
        });
	},
	//dummy function for fast testing
	DownloadAudioHeader2 : function (url, OutputFileName, CallBack, HeaderSize)
	{
		CallBack(url, OutputFileName);
	}
};
