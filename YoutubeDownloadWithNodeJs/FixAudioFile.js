module.exports = {
	FixAudioFile : function(AudioInfo, Callback, OutputAsBase64 = false, DeleteOldFile = true)
	{
		const fs   = require('fs');
		const InputFileName = AudioInfo.TempFileName;
		const OutputFileName = AudioInfo.OutputFileName;

		return FixWebmStruture(InputFileName, OutputFileName, OutputAsBase64, DeleteOldFile, Callback, AudioInfo);

		function FixWebmStruture(InputFileName, OutputFileName, OutputAsBase64 = false, DeleteOldFile = true, Callback, AudioInfo)
		{
			var ffmpeg;
			var testData = new Uint8Array(fs.readFileSync(InputFileName));
			var args;
			
			// need to tell ffmpeg that we want to use the additional module and force output to mp4
			if( OutputFileName.indexOf(".mp4") > 0 || OutputFileName.indexOf(".m4a") > 0)
			{
				ffmpeg = require("ffmpeg.js/ffmpeg-mp4.js");
				args = ["-i", InputFileName, "-c", "copy", "-f", "mp4", OutputFileName];
			}
			else
			{
				ffmpeg = require("ffmpeg.js");
				if( AudioInfo.Codecs.indexOf('vorbis') > 0 || AudioInfo.Codecs.indexOf('opus') > 0 )
					args = ["-i", InputFileName, "-c", "copy", OutputFileName];
				else
					args = ["-i", InputFileName, "-c:a", "libopus", OutputFileName];
			}

			var result = ffmpeg({
			  MEMFS: [{name: InputFileName, data: testData}],
			  arguments: args,
			  stdin: function() {},
			});
			//console.log("FFMPEG result :",result, result.MEMFS.length);
			var out = result.MEMFS[0];
			//format the output as file or as a buffer
			if( OutputAsBase64 == false || OutputAsBase64 == null)
			{
				console.log("Create output file :", out.name);
				fs.writeFileSync(out.name, Buffer(out.data));
			}
			else
			{
				OutputAsBase64 = Buffer(out.data).toString('base64');
				//you need to add code what to do with the output or else nothing will happen
				//console.log(OutputAsBase64.substr(0,100));
			}
			// no longer needed. Stores the header of the file
			if(DeleteOldFile == true)
				fs.unlinkSync(InputFileName);
			
			// chain the next function as soon as we are done
			if( typeof Callback == 'function' )
				Callback(AudioInfo);
			
			return OutputAsBase64;
		}
	}
};