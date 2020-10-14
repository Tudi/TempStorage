#pragma once

bool socksLogin(SOCKET fd);
bool socksConnect(SOCKET fd, const in_addr& dest, unsigned short port);