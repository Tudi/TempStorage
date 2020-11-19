#ifndef SESSIONSTORE_H
#define SESSIONSTORE_H

#include "qstring.h"

class SessionVariableStore
{
public:
    void SetSessionToken( QString pToken);
    QString GetSessionToken(){ return SessionToken; }
private:
    QString SessionToken;
};

void InitSessionStore();
SessionVariableStore *GetSession();

#endif // SESSIONSTORE_H
