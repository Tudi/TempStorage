var matroska = require('matroska');

var url="audio_header.webm";

var decoder = new matroska.Decoder();
decoder.parse(url, function(error, document) 
{
    if (error) 
	{
        //console.error(error);
        return;
    }
	//there has got to be a better way to parse this
	var FormatedHeader = document.print();
	var AudioLen = GetStrValFromDoc("Duration  f[4]=",FormatedHeader);
	console.log("Audio has length in ms : ", AudioLen);
	
	//this should be about 10 seconds length
	var OneCueLen = GetStrValFromDoc("CueTime  u[2]=",FormatedHeader);
	console.log("Cue has length in ms : ", OneCueLen);
	if(OneCueLen<1000)
		OneCueLen = 10001;
	
	var SkipCueCount = parseInt(AudioLen / OneCueLen / 2);
	var DownloadCount = parseInt((30000 + OneCueLen / 2 ) / OneCueLen);
	console.log("Number of cues to download :",DownloadCount);
	
	var Parts = FormatedHeader.split("CueClusterPosition  u[3]=");
	var CueCount = 0;
	for(let val of Parts)
	{
		var subpart = val.split("\n");
		var CueIndex = subpart[0];
		if(CueCount == SkipCueCount)
			console.log("Download from :",CueIndex);
		else if(CueCount == SkipCueCount + DownloadCount)
			console.log("Download to :",CueIndex);
		CueCount++;
	}
});

function GetStrValFromDoc(key, doc)
{
	var parts = doc.split(key);
	var part = parts[1].split("\n")
	return part[0];
}