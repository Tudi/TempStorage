# ImageWaterMark

## Name
Burn or extract watermark into/from an image

## Description
Watermark string will get encrypted using the encryption key. It is added to metadata and to the image itself (humanly invisible).
When the watermark is extracted, metadata and invisible image is compared. If they do not match, the image is considered modified and refuse to extract the watermark.
Optionally you can use the program to 2way encrypt strings. In case the encrypted message needs to be regenerated.

## Installation
This program is a console application that relies on FreeImage dynamic library alone. No special instalation is required.

## Usage
Usage 1: Encrypt/ Decrypt string : WaterMark.exe 1 WaterMarkString EncryptorString
Usage 2: Extract WaterMark : WaterMark.exe 2 SourceFile DecryptorString
Usage 3: Burn WaterMark : WaterMark.exe 3 SourceFile DestFile WaterMarkString DecryptorString

## Support
Contact me at jozsa.istvan@toptal.com

## Roadmap
There are no future plans for this program. Could add features like :
 - partial watermark extraction
 - detect recompressed image
 - different encryption methods

## Contributing
Feel free to make changes to it

## Authors and acknowledgment
Extra thanks goes to the FreeImage team.

## License
Licensed to Rev3al

## Project status
Frozen
