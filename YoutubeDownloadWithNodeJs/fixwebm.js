var ffmpeg = require("ffmpeg.js");
var fs = require("fs");
var inputFile = "audio_fragments.webm";
var testData = new Uint8Array(fs.readFileSync(inputFile));
var result = ffmpeg({
  MEMFS: [{name: inputFile, data: testData}],
  arguments: ["-i", inputFile, "-c", "copy", "out.webm"],
  stdin: function() {},
});
var out = result.MEMFS[0];
fs.writeFileSync(out.name, Buffer(out.data));