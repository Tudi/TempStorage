'use strict';

var http = require('http');
var https = require('https');
const querystring = require('querystring');
const stream = require('stream');
const AWS = require('aws-sdk');
const urlLib = require('url');


const youtubedl = require('ytdl-core');
//youtubedl: https://github.com/fent/node-ytdl-core

const matroska = require('matroska');
const ffmpeg = require("ffmpeg.js");
const path = require('path');
const fs   = require('fs');

const SecondsToDownload = 180; // this should be a multiply of 10 seconds

exports.handler = (event, context, callback) => {
    
    var excludeformats = [];    
    
    ///////////////////////////////////////////
    //REQUEST
    ///////////////////////////////////////////     
    const request = event.Records[0].cf.request;
    
    /*
    const bodytest = request.body;    
    const response = {
       status: '200',
       statusDescription: 'Test',
       headers: {
           'cache-control': [{
               key: 'Cache-Control',
               value: 'max-age=100'
           }],
           'content-type': [{
               key: 'Content-Type',
               value: 'text/html'
           }],
           'content-encoding': [{
               key: 'Content-Encoding',
               value: 'UTF-8'
           }],
       },
       body: JSON.stringify(bodytest),
    };
    callback(null, response);    
    return;
    */
    ///////////////////////////////////////////
    //VALIDATION
    ///////////////////////////////////////////
    if (!(request.headers["authorisation"] && request.headers["authorisation"][0] && validateAuthorisation(request.headers["authorisation"][0].value))) {
        callback(null, { status: "401", statusDescription: "Unauthorised" });
        return;
    }

    if (request.method != 'POST') {
        callback(null, { status: "405", statusDescription: "Method Not Allowed" });
        return;        
    }    

    ///////////////////////////////////////////
    //PARAMS
    ///////////////////////////////////////////
    const body = JSON.parse(Buffer.from(request.body.data, 'base64').toString());
    
    for (let param in body) {
        console.log(`${param}: ${body[param]}.`);
    }
    //console.log('URI: ', request.uri);
    console.log('VideoUrl: ', body.VideoUrl);

    if (!body.VideoUrl) {
        callback(null, { status: "405", statusDescription: "VideoUrl missing" });
        return;      
    }    

    ///////////////////////////////////////////
    // CALL BACK "S3"
    ///////////////////////////////////////////
    var callbackS3 = function(err, info, ip, segment){
        
        if(err){
            callback(null, { status: "503", statusDescription: "callbackS3" });
            return;   
        }
        
        var responseBody = {
        "ytdl-extract": {
            "lambda-ip": "",
            "video-title" : "",
            "video-author" : "",            
            "audio": {
                "segment": [{
                            "bucketName": "cdnbeta.harvestmedia.net",
                            "key": "AudioExtract/4b63c93d-ea2b-469a-af34-cb94b59fdc9f.mp3",
                            "url": "s3://cdnbeta.harvestmedia.net/AudioExtract/4b63c93d-ea2b-469a-af34-cb94b59fdc9f.mp3"
                            }]
            		}
            	}
            };       

        responseBody["ytdl-extract"]["video-title"] = info.title;
        responseBody["ytdl-extract"]["video-author"] = info.author.name;
        responseBody["ytdl-extract"]["audio"]["segment"][0] = segment;

        if (ip)
            responseBody["ytdl-extract"]["lambda-ip"] = ip;
        else 
            delete responseBody["ytdl-extract"]["lambda-ip"];


        const response = {
            status: '200',
            statusDescription: 'OK',
            headers: {
                'cache-control': [{
                    key: 'Cache-Control',
                    value: 'max-age=0'
                }],
                'content-type': [{
                    key: 'Content-Type',
                    value: 'application/json'
                }],
                'content-encoding': [{
                    key: 'Content-Encoding',
                    value: 'UTF-8'
                }],
            },
            body: JSON.stringify(responseBody)
            
        };
        callback(null, response);             
        //callback(null, { status: "200", statusDescription: "DONE" });

        console.log('DONE');
        return;                 
    }
    

    ///////////////////////////////////////////
    // CALL BACK "ParseHeader"
    ///////////////////////////////////////////
/*    var callbackParseHeader = function(context, info, format, ip)
    {
		
        const input = path.resolve('/tmp/', `${context.awsRequestId}_build.webm`);       
    
    	var decoder = new matroska.Decoder();
    	decoder.parse(input, function ParseHeaderCallback(error, document){
    		if (error) 
    		{
    			//console.error(error);
    			return;
    		}
    		//there has got to be a better way to parse this
    		var FormatedHeader = document.print();
    		//console.log('Full header : '+ FormatedHeader);
    		var AudioLen = GetStrValFromDoc("Duration  f[4]=",FormatedHeader);
    		console.log("Audio has length in ms : ", AudioLen);
    		
    		//this should be about 10 seconds length
    		var OneCueLen = GetStrValFromDoc("CueTime  u[2]=",FormatedHeader);
    		console.log('Cue has length in ms : ', OneCueLen);
    		if(OneCueLen<1000)
    			OneCueLen = 10001;
    		
    		var SkipCueCount = parseInt(AudioLen / OneCueLen / 2);
    		var DownloadCount = parseInt((SecondsToDownload * 1000 + OneCueLen / 2 ) / OneCueLen);
    		console.log('Number of cues to download :',DownloadCount);
    		
    		var Parts = FormatedHeader.split("CueClusterPosition  u[3]=");
    		var CueCount = 0;
    		var DownloadStarAt = 0;
    		var DownloadEndAt = 0;
    		var LastCue = 0;
    		for(let val of Parts)
    		{
    			var subpart = val.split("\n");
    			var CueIndex = subpart[0];
    			if(CueCount == SkipCueCount)
    			{
    				console.log("Download from :",CueIndex);
    				DownloadStarAt = CueIndex;
    			}
    			else if(CueCount == SkipCueCount + DownloadCount)
    			{
    				console.log("Download to :",CueIndex);
    				DownloadEndAt = CueIndex;
    			}
    			CueCount++;
    			LastCue = CueIndex;
    		}
    		if(DownloadEndAt == 0)
    			DownloadEndAt = LastCue;
    		var HeaderSize = GetHeaderSize(FormatedHeader);
    
    		//console.log('Header size in bytes :',HeaderSize);
    		//GenHeaderAndChunks(context, info, format, ip, HeaderSize, DownloadStarAt, DownloadEndAt);
            //const output = path.resolve('/tmp/', `${context.awsRequestId}_build.webm`);        
        
        	const fd = fs.openSync(input, 'r+');
        	fs.ftruncate(fd, parseInt(HeaderSize), (err) => { if(err)console.log(err);});
        
        	const dl = youtubedl.downloadFromInfo(info, { format: format, range: { start: DownloadStarAt, end: DownloadEndAt } })
        	dl.pipe(fs.createWriteStream(input, { flags: 'a' }));
        	dl.on('end', () => {
        		callbackFixWebmStruture(context, info, format, ip);	
        	});			
    	});
    }*/
    
    ///////////////////////////////////////////
    // CALL BACK "FixWebmStruture"
    ///////////////////////////////////////////    
    var callbackFixWebmStruture = function(context, info, format, ip)
    {
        const input = path.resolve('/tmp/', `${context.awsRequestId}_build.webm`);
        const inputname = `${context.awsRequestId}_build.webm`;
        const output = path.resolve('/tmp/', `${context.awsRequestId}_output.webm`);      
        const outputname = `${context.awsRequestId}_output.webm`;
        
/*    	var testData = new Uint8Array(fs.readFileSync(input));
    	var result = ffmpeg({
    	  MEMFS: [{name: inputname, data: testData}],
    	    arguments: ["-i", inputname, "-c", "copy", outputname],
    	  stdin: function() {},
    	});
    	var out = result.MEMFS[0];
    	fs.writeFileSync(output, Buffer(out.data), { flags: 'w' });
    	fs.unlinkSync(input);
    	*/
    	var stream = fs.createReadStream(output);
    	
    	
        const s3 = new AWS.S3({apiVersion: '2006-03-01'});
        var bucketName = "cdnbeta.harvestmedia.net";
        const key = `AudioExtract/YouTube/${context.awsRequestId}.${format.container}`;
        const upload = new AWS.S3.ManagedUpload({
        params: {
            Bucket: bucketName,
            Key: key,
            Body: stream
        },
        partSize: 1024 * 1024 * 64 // in bytes
        });
        upload.on('httpUploadProgress', (progress) => {
            console.log(`copying audio ...`, progress);
        });
        upload.send((err) => {
            if (err) {
                callback(null, { status: "503", statusDescription: "upload" });
                return;    
            } else {
              callbackS3(null, info, ip, {
                bucketName: bucketName,
                key: key,
                url: `s3://${bucketName}/${key}`,
                start: "0",
                end: `180`
              });
            }
        })	
    }
    
	function UploadToS3Callback(AudioInfo)
	{
		callbackFixWebmStruture(AudioInfo.context, AudioInfo.info, AudioInfo.format, AudioInfo.ip );
	}
	
	function FixWebmStructureCallback(AudioInfo)
	{
		var ffmpeg = require('./FixAudioFile.js');
		ffmpeg.FixAudioFile(AudioInfo, UploadToS3Callback);
	}
	
    function DownloadAudioSegmentCallback(AudioInfo) 
	{
		var SegmentDownloader = require('./DownloadSement.js');
		SegmentDownloader.DownloadAudioSegment( AudioInfo, FixWebmStructureCallback);
	}

	function TruncateAudioHeaderCallback(AudioInfo) 
	{
		var HeaderParser = require('./TruncateAudioHeaderOnly.js');
		HeaderParser.TruncateAudioHeader(AudioInfo, DownloadAudioSegmentCallback);
	}

    ///////////////////////////////////////////
    // "Content Length" CALLBACK
    ///////////////////////////////////////////
    var callbackContentLength = function(err, info, ip, format, contentlengthbytes){
        
        if(err){
            callback(null, { status: "503", statusDescription: "callbackContentLength" });
            return;             
        }
		
		var HeaderDownloader = require('./DownloadAudioHeader.js')
		var AudioInfo = {};
		AudioInfo.StartSec = 0;
		AudioInfo.LengthSec = 180;	// same as SecondsToDownload ? Or get the value from somewhere else ?	
		AudioInfo.info = info;
		AudioInfo.format = format;
		AudioInfo.TempFileName = path.resolve('/tmp/', `${context.awsRequestId}_build.webm`);
		AudioInfo.OutputFileName = path.resolve('/tmp/', `${context.awsRequestId}_output.webm`);
		AudioInfo.context = context;
		AudioInfo.ip = ip;
		HeaderDownloader.DownloadAudioHeader(AudioInfo, TempFileName, TruncateAudioHeaderCallback);
    }

    ///////////////////////////////////////////
    // "GetInfo" CALLBACK 
    ///////////////////////////////////////////
    var callbackGetInfo = function(err, info, ip, format, location){

        if (err){
            callback(null, { status: "503", statusDescription: "callbackGetInfo" });
            return;             
        }

        console.log('format: ' + format.url);
        //callbackContentLength(null, info, ip, format, 100000);

        let tUrl = format.url;
        if (location)
            tUrl = location;

        https.request(tUrl, function(res) {
            
            console.log('total length:', res.headers['content-length']);
            console.log('Status:', res.statusCode);
            console.log('location:', res.headers['location']);

            let statusCode = [301,302,303,304,307,308];

            if (res.headers['content-length'] == 0 && statusCode.includes(res.statusCode)){
                callbackGetInfo(null, info, ip, format, res.headers['location']);
                //callback(null, { status: "503", statusDescription: "No content-length" });
                return;
            }
            else if (res.headers['content-length'] == 0){
                excludeformats.push(format.url);
                let newformat = getFormat(info, excludeformats)
                if (!newformat) {
                    console.log('webm/m4a/mp4 formats not found');
                    callback(null, { status: "405", statusText: "No valid webm/m4a/mp4 formats." });
                    return;
                }
                callbackGetInfo(null, info, ip, newformat);
                return;
            }

            callbackContentLength(null, info, ip, format, res.headers['content-length']);
        }).end();        
        /*
        var parsed = urlLib.parse(format.url);
        parsed.method = 'HEAD';
        https.request(parsed, (res) => {
            console.log('total length:', res.headers['content-length']);
            callbackContentLength(null, info, ip, format, res.headers['content-length']);
        })
        .end();               
        console.log('https.request end');
        */
        /*        
        const audio = youtubedl.downloadFromInfo(info, { format: format, range: {start: 0, end: 1000} })
        
        audio.once('error', (err) => {
            callback(err);
        });   
        

        audio.on('info', (audioinfo) => {

            //console.log(audioformat.url);

        });
        console.log('audio end');
        */
    };


    ///////////////////////////////////////////
    // "IP" CALLBACK
    ///////////////////////////////////////////
    var callbackIP = function(err, ip){
        
        if(err){
            callback(null, { status: "503", statusDescription: "callbackIP" });
            return;   
        }
        
        if (ip) 
            console.log('Lambda public IP: ', ip);

        // START YTDL
        youtubedl.getInfo(body.VideoUrl, (err, info) => {
            
            if (err) //throw err;
            {            
                callback(null, { status: "503", statusDescription: "YT getInfo ERR, bad VideoUrl or access denied" });
                return;            
            }

            console.log(`"${info.title}" by "${info.author.name}" is being inspected`)
            console.log('length_seconds: '+ info.length_seconds);              

            let format = getFormat(info, excludeformats)

            if (!format) {
                console.log('webm/m4a/mp4 formats not found');
                callback(null, { status: "405", statusText: "No valid webm/m4a/mp4 formats." });
                return;      
            }               

            //console.log('Format url'+ format.url);
            /*
            let audioFormats = info.formats;
            console.log('Formats: ' + audioFormats.length);
            for (let i=0; i<audioFormats.length; i++) {
              if (audioFormats[i].audioBitrate) {
                console.log('audioonly - audioBitrate: ' + audioFormats[i].audioBitrate.toString());              
              }
              if (audioFormats[i].audioEncoding) {
                console.log('audioonly - audioEncoding: '+ audioFormats[i].audioEncoding);              
              }
              if (audioFormats[i].audio_sample_rate) {
                console.log('audioonly - audio_sample_rate: '+ audioFormats[i].audio_sample_rate);              
              }
              if (audioFormats[i].container) {
                console.log('audioonly - container: '+ audioFormats[i].container);              
              }      
            }    
            */

            callbackGetInfo(null, info, ip, format, null);
        });
    };
    
    
    ///////////////////////////////////////////
    // START 
    ///////////////////////////////////////////
    if (!(body.CheckIP && body.CheckIP === true))
        callbackIP(null, null);
    else
    {
        ///////////////////////////////////////////
        // GET IP 
        ///////////////////////////////////////////
        http.get({
            host: 'checkip.amazonaws.com',
        }, function(response) {
            var ip = '';
            response.on('data', function(d) {
                ip += d;
            });
            response.on('end', function() {
                if(ip){
                    callbackIP(null, ip.replace(/\n/g, ''));
                } else {
                    callbackIP('could not get public ip address');
                }
            });
        });
    }

        
};

function validateAuthorisation(authorisation) {
    return authorisation==='H3l3copt3r';
};

function getFormat(info, excludeformats){
    
    let format;
    let audioFormats = youtubedl.filterFormats(info.formats, 'audioonly');
    for (let i=0; i<audioFormats.length; i++) {            
        if (!excludeformats.includes(audioFormats[i].url) && audioFormats[i].url.indexOf('ratebypass=yes') !== -1 && (audioFormats[i].container === 'webm' || audioFormats[i].container === 'm4a' || audioFormats[i].container === 'mp4')) {
            format = audioFormats[i];
            console.log('ratebypass audioonly: '+ format.container);              
            break;
        }
    }          
    if (!format) {
       let audioFormats = youtubedl.filterFormats(info.formats, 'audio');
        for (let i=0; i<audioFormats.length; i++) {            
            if (!excludeformats.includes(audioFormats[i].url) && audioFormats[i].url.indexOf('ratebypass=yes') !== -1 && (audioFormats[i].container === 'webm' || audioFormats[i].container === 'm4a' || audioFormats[i].container === 'mp4')) {
                format = audioFormats[i];
                console.log('no ratepass audio: '+ format.container);             
                break;
            }
        }                 
    }              

    if (!format) {
       let audioFormats = youtubedl.filterFormats(info.formats, 'audioonly');
        for (let i=0; i<audioFormats.length; i++) {            
            if (!excludeformats.includes(audioFormats[i].url) && (audioFormats[i].container === 'webm' || audioFormats[i].container === 'm4a' || audioFormats[i].container === 'mp4')) {
                format = audioFormats[i];
                console.log('no ratepass audioonly: '+ format.container);             
                break;
            }
        }                 
    }               

    if (!format) {
       let audioFormats = youtubedl.filterFormats(info.formats, 'audio');
        for (let i=0; i<audioFormats.length; i++) {            
            if (!excludeformats.includes(audioFormats[i].url) && (audioFormats[i].container === 'webm' || audioFormats[i].container === 'm4a' || audioFormats[i].container === 'mp4')) {
                format = audioFormats[i];
                console.log('no ratepass audio: '+ format.container);             
                break;
            }
        }                 
    }   
    
    if (!format) {
       let audioFormats = youtubedl.filterFormats(info.formats, 'audioandvideo');
        for (let i=0; i<audioFormats.length; i++) {            
            if (!excludeformats.includes(audioFormats[i].url) && (audioFormats[i].container === 'webm' || audioFormats[i].container === 'm4a' || audioFormats[i].container === 'mp4')) {
                format = audioFormats[i];
                console.log('no ratepass audio: '+ format.container);             
                break;
            }
        }                 
    }   
    
    if (!format) {
       let audioFormats = youtubedl.filterFormats(info.formats, 'audioandvideo');
        for (let i=0; i<audioFormats.length; i++) {            
            if (!excludeformats.includes(audioFormats[i].url)) {
                format = audioFormats[i];
                console.log('catch all: '+ format.container);             
                break;
            }
        }                 
    }         
    return format;
};

/*
function GetStrValFromDoc(key, doc)
{
	var parts = doc.split(key);
	var part = parts[1].split("\n")
	return part[0];
}

function GetHeaderSize(doc)
{
	var Parts = doc.split("\n");
	for(let val of Parts)
		if(val.indexOf("* Cluster  children") > 0 )
		{
			var subparts = val.split("#");
			return subparts[0];
		}
	return "";
}
*/

