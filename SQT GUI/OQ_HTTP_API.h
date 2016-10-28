#pragma once

// Start HTTP server
int OQAPI_HTTP_Start( int port );
// Stop HTTP server
void OQAPI_HTTP_Stop();
// Generic handler for HTTP GET URL
int HandleURL( struct MHD_Connection *connection, const char *Url );