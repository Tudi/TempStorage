#include <memory.h>
#include "SessionStore.h"

SessionVariableStore *sSession = NULL;

void InitSessionStore()
{
    if(sSession != NULL)
        return;
    sSession = new SessionVariableStore;
}

SessionVariableStore *GetSession()
{
    InitSessionStore();
    return sSession;
}

void SessionVariableStore::SetSessionToken(QString pToken)
{
    SessionToken = pToken;
}
