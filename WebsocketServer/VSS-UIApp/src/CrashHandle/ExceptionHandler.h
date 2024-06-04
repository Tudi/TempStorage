#pragma once

/// <summary>
/// Dump the stack to file so client may send it to us
/// </summary>
void ExceptionHandlerInit();

/// <summary>
/// Iterate through crashes directory and send them to the server
/// </summary>
void ExceptionHandler_SendCrashesToServer();
