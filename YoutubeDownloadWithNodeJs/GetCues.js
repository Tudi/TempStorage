const fs = require('fs')
const ebml = require('ebml')
const ebmlBlock = require('ebml-block')
 
var decoder = new ebml.Decoder()
 
decoder.on('data', function (chunk) {
/*  if (chunk[1].name === 'Block' || chunk[1].name === 'SimpleBlock') {
    var block = ebmlBlock(chunk[1].data)
    console.log(block)
  }*/
  var block = ebmlBlock(chunk[1].data)
  console.log(chunk[1].name, block)
})

decoder.on('error', function (err) {
})

fs.createReadStream('audio_header.webm').pipe(decoder)