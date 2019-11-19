const path = require('path');
const fs   = require('fs');
const ytdl = require('..');

const url = 'https://www.youtube.com/watch?v=CehAKQL463M';

//you can adjust the size of the range by adjusting the "downloaded" or the "chunksize" parameter
//you can adjust the start time if you hook the video info event and get the duration of the video....
var VideoLength = 0
const output = path.resolve(__dirname, 'temp.webm');

const video = ytdl(url, { range: { start: 0, end: 732 }, filter: 'audioonly', quality: 'highestaudio' });
video.pipe(fs.createWriteStream(output));

video.on('end', () => {
  ytdl(url, { range: { start: 522929, end: 713676 }, filter: 'audioonly', quality: 'highestaudio' })
    .pipe(fs.createWriteStream(output, { flags: 'a' }));
});