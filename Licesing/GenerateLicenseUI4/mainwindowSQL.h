#ifndef MAINWINDOWSQL_H
#define MAINWINDOWSQL_H

class FeatureStore
{
public:
    FeatureStore( int pid, char *pName, char *pDesc,int pProductId, char *pActivationKey,int pHiddenInUI,int pSelectedInUI,int pForcedAddToLicense,int pFixedValue,int pNotEncrypted, char *pCustomActivationKey=NULL)
    {
        id = pid;
        ProductId = pProductId;
        HiddenInUI = pHiddenInUI;
        SelectedInUI = pSelectedInUI;
        ForcedAddToLicense = pForcedAddToLicense;
        FixedValue = pFixedValue;
        NotEncrypted = pNotEncrypted;
        Name = NULL;
        desc = NULL;
        ActivationKey = NULL;
        CustomActivationKey = NULL;
        if(pName)
        {
            size_t Len = strlen(pName)+1;
            Name = new char[Len];
            strcpy_s(Name,Len,pName);
        }
        if(pDesc)
        {
            size_t Len = strlen(pDesc)+1;
            desc = new char[Len];
            strcpy_s(desc,Len,pDesc);
        }
        if(pActivationKey)
        {
            size_t Len = strlen(pActivationKey)+1;
            ActivationKey = new char[Len];
            strcpy_s(ActivationKey,Len,pActivationKey);
        }
        if(pCustomActivationKey)
        {
            size_t Len = strlen(pCustomActivationKey)+1;
            CustomActivationKey = new char[Len];
            strcpy_s(CustomActivationKey,Len,pCustomActivationKey);
        }
        IsSelected = pSelectedInUI;
    }
    FeatureStore(FeatureStore *cpy)
    {
        FeatureStore(cpy->id,cpy->Name,cpy->desc,cpy->ProductId,cpy->ActivationKey,cpy->HiddenInUI,cpy->SelectedInUI,cpy->ForcedAddToLicense,cpy->FixedValue,cpy->NotEncrypted);
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
        if(CustomActivationKey!=NULL)
        {
            delete []CustomActivationKey;
            CustomActivationKey = NULL;
        }
    }
    void SetCustomActivationKey(const char *pCustomActivationKey)
    {
        if(CustomActivationKey!=NULL)
        {
            delete []CustomActivationKey;
            CustomActivationKey = NULL;
        }
        if(strcmp(pCustomActivationKey,ActivationKey)==0)
            return;
        if(pCustomActivationKey)
        {
            size_t Len = strlen(pCustomActivationKey)+1;
            CustomActivationKey = new char[Len];
            strcpy_s(CustomActivationKey,Len,pCustomActivationKey);
        }
    }
    char *GetActivationKey()
    {
        if(CustomActivationKey!=NULL)
            return CustomActivationKey;
        else
            return ActivationKey;
    }
    bool HasCustomValue()
    {
        if(CustomActivationKey!=NULL)
            return 1;
        return 0;
    }

    int     id;
    int     ProductId;
    char    *Name;
    char    *desc;
    char    *ActivationKey;
    int     HiddenInUI;
    int     SelectedInUI;
    int     ForcedAddToLicense;
    int     FixedValue;
    int     NotEncrypted;
    char    *CustomActivationKey;
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
            size_t Len = strlen(pName)+1;
            Name = new char[Len];
            strcpy_s(Name,Len,pName);
        }
        if(pDesc)
        {
            size_t Len = strlen(pDesc)+1;
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


class CustomerStore
{
public:
    CustomerStore( int pid, const char *pName, const char *pPhone, const char *pEmail)
    {
        Id = pid;
        Name = NULL;
        Phone = NULL;
        Email = NULL;
        IsSelected = 0;
        if(pName)
        {
            size_t Len = strlen(pName)+1;
            Name = new char[Len];
            strcpy_s(Name,Len,pName);
        }
        if(pPhone)
        {
            size_t Len = strlen(pPhone)+1;
            Phone = new char[Len];
            strcpy_s(Phone,Len,pPhone);
        }
        if(pEmail)
        {
            size_t Len = strlen(pEmail)+1;
            Email = new char[Len];
            strcpy_s(Email,Len,pEmail);
        }
    }
    ~CustomerStore()
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
    void    Update(const char *pName, const char *pPhone, const char *pEmail);
    void    DeleteFromDb();
    int     Id;
    char    *Name;
    char    *Phone;
    char    *Email;
    int     IsSelected;
};

class CustomerSeedStore
{
public:
    CustomerSeedStore( int id, int cid, char *pSeed, int pSeedSize)
    {
        Id = id;
        CustomerId = cid;
        Seed = NULL;
        SeedSize = 0;
        if(pSeed)
        {
            SeedSize = pSeedSize;
            Seed = new char[SeedSize];
            memcpy(Seed,pSeed,SeedSize);
        }
    }
    CustomerSeedStore(CustomerSeedStore *cpy)
    {
        CustomerSeedStore(cpy->Id,cpy->CustomerId,cpy->Seed,cpy->SeedSize);
    }
    ~CustomerSeedStore()
    {
        if(Seed!=NULL)
        {
            delete []Seed;
            Seed = NULL;
        }
    }
    int     Id;
    int     CustomerId;
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
    static QList<CustomerStore*>      *GetCustomerList();
    static QList<CustomerSeedStore*>  *GetCustomerSeedList(int CustomerId);
    static int                      GetProductFeatureName(int ProductId, int FeatureId, char **ProductName, char **FeatureName);
    //setters
    static int InsertCustomer(CustomerStore *cs);
    static int InsertCustomerSeed(int CustomerId, char *Seed, int Size);
    static int InsertCustomerLicense(int CustomerId, char *License, int Size );
    static int GetCustomerLastLicenseId(int CustomerId, int *LicenseId);
    static int SetCustomerLicenseComputerSelectStatus(int CustomerId, int LicenseId, int SeedId);
    static int GetCustomerLicenseComputerSelectStatus(int CustomerId, int LicenseId, int SeedId, int *IsSelected);
    static int SetCustomerLicenseProductFeatureStatus(int CustomerId, int LicenseId, int ProductId, int FeatureId, char *Value);
    static int GetCustomerLicenseProductFeatureStatus(int CustomerId, int LicenseId, int ProductId, int FeatureId, char **Value);
    static int SetCustomerLicenseOptions(int CustomerId, int LicenseId, int StartDate, int EndDate, int GraceDuration, int Perpetual);
    static int GetCustomerLicenseOptions(int CustomerId, int LicenseId, int *StartDate, int *EndDate, int *GraceDuration, int *Perpetual);
};

#endif // MAINWINDOWSQL_H
