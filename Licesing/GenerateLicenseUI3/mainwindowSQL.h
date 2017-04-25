#ifndef MAINWINDOWSQL_H
#define MAINWINDOWSQL_H

class FeatureStore
{
public:
    FeatureStore( int pid, char *pName, char *pDesc, int pProductId, char *pActivationKey)
    {
        id = pid;
        ProductId = pProductId;
        Name = NULL;
        desc = NULL;
        ActivationKey = NULL;
        if(pName)
        {
            int Len = strlen(pName)+1;
            Name = new char[Len];
            strcpy_s(Name,Len,pName);
        }
        if(pDesc)
        {
            int Len = strlen(pDesc)+1;
            desc = new char[Len];
            strcpy_s(desc,Len,pDesc);
        }
        if(pActivationKey)
        {
            int Len = strlen(pActivationKey)+1;
            ActivationKey = new char[Len];
            strcpy_s(ActivationKey,Len,pActivationKey);
        }
        IsSelected = 0;
    }
    FeatureStore(FeatureStore *cpy)
    {
        FeatureStore(cpy->id,cpy->Name,cpy->desc,cpy->ProductId,cpy->ActivationKey);
    }
    ~FeatureStore()
    {
        if(Name!=NULL)
        {
            delete []Name;
            Name = NULL;
        }
        if(desc!=NULL)
        {
            delete []desc;
            desc = NULL;
        }
        if(ActivationKey!=NULL)
        {
            delete []ActivationKey;
            ActivationKey = NULL;
        }
    }

    int     id;
    int     ProductId;
    char    *Name;
    char    *desc;
    char    *ActivationKey;
    int     IsSelected;
};

class ProductStore
{
public:
    ProductStore( int pid, char *pName, char *pDesc)
    {
        id = pid;
        Name = NULL;
        desc = NULL;
        if(pName)
        {
            int Len = strlen(pName)+1;
            Name = new char[Len];
            strcpy_s(Name,Len,pName);
        }
        if(pDesc)
        {
            int Len = strlen(pDesc)+1;
            desc = new char[Len];
            strcpy_s(desc,Len,pDesc);
        }
        IsSelected = 0;
    }
    ~ProductStore()
    {
        if(Name!=NULL)
        {
            delete []Name;
            Name = NULL;
        }
        if(desc!=NULL)
        {
            delete []desc;
            desc = NULL;
        }
    }
    int id;
    char *Name;
    char *desc;
    QList<FeatureStore*> *Features;
    int IsSelected;
};


class ClientStore
{
public:
    ClientStore( int pid, char *pName, char *pPhone, char *pEmail)
    {
        Id = pid;
        Name = NULL;
        Phone = NULL;
        Email = NULL;
        IsSelected = 0;
        if(pName)
        {
            int Len = strlen(pName)+1;
            Name = new char[Len];
            strcpy_s(Name,Len,pName);
        }
        if(pPhone)
        {
            int Len = strlen(pPhone)+1;
            Phone = new char[Len];
            strcpy_s(Phone,Len,pPhone);
        }
        if(pEmail)
        {
            int Len = strlen(pEmail)+1;
            Email = new char[Len];
            strcpy_s(Email,Len,pEmail);
        }
    }
    ~ClientStore()
    {
        if(Name!=NULL)
        {
            delete []Name;
            Name = NULL;
        }
        if(Phone!=NULL)
        {
            delete []Phone;
            Phone = NULL;
        }
        if(Email!=NULL)
        {
            delete []Email;
            Email = NULL;
        }
    }
    void Update(const char *pName, const char *pPhone, const char *pEmail);
    void DeleteFromDb();
    int Id;
    char *Name;
    char *Phone;
    char *Email;
    int IsSelected;
};

class ClientSeedStore
{
public:
    ClientSeedStore( int id, int cid, char *pSeed, int pSeedSize)
    {
        Id = id;
        ClientId = cid;
        Seed = NULL;
        SeedSize = 0;
        if(pSeed)
        {
            SeedSize = pSeedSize;
            Seed = new char[SeedSize];
            memcpy(Seed,pSeed,SeedSize);
        }
    }
    ClientSeedStore(ClientSeedStore *cpy)
    {
        ClientSeedStore(cpy->Id,cpy->ClientId,cpy->Seed,cpy->SeedSize);
    }
    ~ClientSeedStore()
    {
        if(Seed!=NULL)
        {
            delete []Seed;
            Seed = NULL;
        }
    }
    int     Id;
    int     ClientId;
    char    *Seed;
    int     SeedSize;
};

class SQLConnection
{
public:
    static void ConnectSQLite();
    static void DisConnectSQLite();
    static void CreateTablesIfNotExist();
    //getters
    static QList<ProductStore*>     *GetProductList();
    static QList<ClientStore*>      *GetClientList();
    static QList<ClientSeedStore*>  *GetClientSeedList(int ClientId);
    //setters
    static int InsertClient(ClientStore *cs);
    static int InsertClientSeed(int ClientId, char *Seed, int Size);
};


#endif // MAINWINDOWSQL_H
