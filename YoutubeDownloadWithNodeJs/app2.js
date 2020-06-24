//edit this segment to set up your download parameters
///////////////////////////////////////////////////////
//var url = 'https://www.youtube.com/watch?v=ijD35ymw8RU'; // no range support
//var url = 'https://www.youtube.com/watch?v=CehAKQL463M'; // supports range
//var url = "https://www.youtube.com/watch?v=f4Mc-NYPHaQ";// whole audio seems to be extracted
var url = "https://www.youtube.com/watch?v=bmmpFIIvXno";// the end of the audio is extracted

const ytdlcore = require('ytdl-core');

ytdlcore.getInfo(url);

var starttime = Date.now();
// as last step, reindex the audio file considering we cut out of the content. For some players, this might not be a necesarry step !
function FixWebmStructureCallback(AudioInfo)
{
	console.log("Finished appending audio blocks to header");
	var ffmpeg = require('./FixAudioFile.js');
	ffmpeg.FixAudioFile(AudioInfo, null);
	var Duration = Date.now() - starttime
	console.log("All done. Time it took: ", Duration / 1000, " seconds");
}

// at this point we have a valid audio header and audio file details. In this step we download the audio segment based on audio header details
function DownloadAudioSegmentCallback(AudioInfo) {
	console.log("Finished parsing audio header segment");
	var SegmentDownloader = require('./DownloadSement.js');
	SegmentDownloader.DownloadAudioSegment( AudioInfo, FixWebmStructureCallback);
}

// this is required because we guessed the size of the header in the previous download. In case we gave exact audio header size, this function should do nothing
// this function also returns audio details we need to be able to guess the amount of bytes to download for audio data
function TruncateAudioHeaderCallback(AudioInfo) {
	console.log("Finished downloading the audio header segment");
	var HeaderParser = require('./TruncateAudioHeaderOnly.js');
	HeaderParser.TruncateAudioHeader(AudioInfo, DownloadAudioSegmentCallback);
}

function StartDownloadChain(info, format, StartSec, LengthSec) {
	var HeaderDownloader = require('./DownloadAudioHeader.js')
	var AudioInfo = {};
	AudioInfo.info = info;
	AudioInfo.format = format;
	AudioInfo.TempFileName = 'temp.' + format.container;
	AudioInfo.OutputFileName = "out." + format.container;
	//you can force output type by setting the output extension
//	AudioInfo.OutputFileName = "out" + ".webm"; // maybe it's best to not change output. In my tests forcing mp4 -> webm worked. I wonder if it will work every time
	AudioInfo.StartSec = parseInt(StartSec); // does not work with MP4 that does not support 'begin'
	AudioInfo.LengthSec = parseInt(LengthSec);
	AudioInfo.ContainerType = format.container;
	
	//m4a container does not have info about bitrate, seek info or anything seek related. The codec inside the container has this info
	if( typeof format.audioBitrate != 'undefined' && format.audioBitrate > 0 )
		AudioInfo.AudioBitrate = format.audioBitrate * 1000;
	else if( typeof format.bitrate != 'undefined' && format.bitrate > 0 )
		AudioInfo.AudioBitrate = format.bitrate * 1000;
	else
		console.log("Error : Audio bitrate is required to know segment size");
	
	AudioInfo.Codecs = format.mimeType; // vorbis and opus does not need reencoding

	if( typeof format.indexRange != 'undefined' )
	{
		//live streams do not support indexing
		AudioInfo.SupportsRange = 1; // we presume if one format supports range, the others will support 'begin'
		console.log("File supports range. Probably does not support 'begin'");
	}
	else if( typeof format.isDashMPD == 'undefined' || format.isDashMPD != true )
	{
		console.log("Neither range or 'begin' is supported. Seek is not supported for these file types");
	}
	else
		console.log("File is considered a livestream. Does not support range. Should support 'begin'");
	
	AudioInfo.AproxDuration = parseInt(format.approxDurationMs/1000);
	if( StartSec == 0 && LengthSec >= AudioInfo.AproxDuration )
	{
		console.log("Length of the audio is ", AudioInfo.AproxDuration," we wanted to download ", LengthSec, ". Downloading whole file as is");
		DownloadAudioSegmentCallback(AudioInfo, FixWebmStructureCallback);
	}
	else 
		HeaderDownloader.DownloadAudioHeader(AudioInfo, TruncateAudioHeaderCallback);
}

function getFormat(info, excludeformats){
    
    let format;
    let audioFormats = ytdlcore.filterFormats(info.formats, 'audioonly');
	for (let i=0; i<audioFormats.length; i++) {            
		if (!excludeformats.includes(audioFormats[i].url) && (audioFormats[i].container === 'webm' || audioFormats[i].container === 'm4a' || audioFormats[i].container === 'mp4')) {
//		if (!excludeformats.includes(audioFormats[i].url) && (audioFormats[i].container === 'm4a' || audioFormats[i].container === 'mp4')) {
//		if (!excludeformats.includes(audioFormats[i].url) && (audioFormats[i].container === 'webm')) {
			format = audioFormats[i];
			console.log('audioonly: '+ format.container);             
			break;
		}
	}
	//try to get the Duration of this file	
	for (let i=0; i<info.formats.length; i++) 
		if( typeof info.formats[i].approxDurationMs != 'undefined' && info.formats[i].approxDurationMs > 0 )
		{
			format.approxDurationMs = info.formats[i].approxDurationMs;
			break;
		}
 
    return format;
};

//call function chain
ytdlcore.getInfo(url, (err, info) => {
	
	var excludeformats = [];    

	if (err) //throw err;
	{            
		return;            
	}

	let format = getFormat(info, excludeformats);

	if (!format) {
		return;      
	}               

	// length can change +/- 20 seconds because fragments might not allign perfectly with what we want
	StartDownloadChain(info, format, 0, 180);
});
		





