const path = require('path');
const fs   = require('fs');
const ytdl = require('..');

const url = 'https://www.youtube.com/watch?v=CehAKQL463M';

const output = path.resolve(__dirname, 'audio_header.webm');

const video = ytdl(url, { range: { start: 0, end: 732 }, filter: 'audioonly', quality: 'highestaudio' });
video.pipe(fs.createWriteStream(output));
