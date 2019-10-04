#pragma once

int		StartCapturePackets(int AutoPickAdapter = -1);
void	StopCapturePackets();
void	SendPacket(unsigned char *Data, int Len);