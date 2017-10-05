#include <stdlib.h>
#include <string.h>
#include <QList>
#include <time.h>
#include "mainwindowSQL.h"
#include "../SQLite/sqlite3.h"
sqlite3 *db = NULL; //global connection. One is enough / application

void SQLConnection::ConnectSQLite()
{
    int rc = sqlite3_open("CustomerLicense.db", &db);
    if( rc )
    {
       fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
       return;
    }

    //try to initialize SQL
    CreateTablesIfNotExist();
}

void SQLConnection::DisConnectSQLite()
{
    if(db)
    {
        sqlite3_close(db);
        db = NULL;
    }
}

void SQLConnection::CreateTablesIfNotExist()
{
    const char *sql;
    char *zErrMsg = 0;
    int rc;
    sql = "CREATE TABLE IF NOT EXISTS `products` ( "\
            "`id` INTEGER PRIMARY KEY, "\
            "`Name` varchar(50) DEFAULT NULL, "\
            "`Description` varchar(200) DEFAULT NULL, "\
            "`DeletedEntry` int(11) DEFAULT NULL "\
          ")";

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sql = "CREATE TABLE IF NOT EXISTS `features` ( "\
            "`id` INTEGER PRIMARY KEY, "\
            "`ProductId` int(11) DEFAULT NULL, "\
            "`Name` varchar(50) DEFAULT NULL, "\
            "`Description` varchar(200) DEFAULT NULL, "\
            "`ActivationKey` varchar(50) DEFAULT NULL, "\
            "`HiddenInUI` varchar(50) DEFAULT NULL, "\
            "`SelectedInUI` varchar(50) DEFAULT NULL, "\
            "`ForcedAddToLicense` varchar(50) DEFAULT NULL, "\
            "`FixedValue` varchar(50) DEFAULT NULL, "\
            "`NotEncrypted` varchar(50) DEFAULT NULL, "\
            "`DeletedEntry` int(11) DEFAULT NULL "\
          ")";

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sql = "CREATE TABLE IF NOT EXISTS `Customers` ( "\
            "`id` INTEGER PRIMARY KEY, "\
            "`Name` varchar(50) DEFAULT NULL, "\
            "`Phone` varchar(200) DEFAULT NULL, "\
            "`Email` varchar(200) DEFAULT NULL, "\
            "`Address` varchar(200) DEFAULT NULL, "\
            "`Notes` varchar(200) DEFAULT NULL, "\
            "`DeletedEntry` int(11) DEFAULT NULL "\
          ")";

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sql = "CREATE TABLE IF NOT EXISTS `CustomerSeeds` ( "\
            "`id` INTEGER PRIMARY KEY, "\
            "`CustomerId` int(11) DEFAULT NULL, "\
            "`Seed` blob, "\
            "`SelectedInUI` int(11) DEFAULT NULL, "\
            "`DeletedEntry` int(11) DEFAULT NULL "\
          ")";

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sql = "CREATE TABLE IF NOT EXISTS `CustomerLicenses` ( "\
            "`LicenseRowId` INTEGER PRIMARY KEY, "\
            "`CustomerId` int(11) DEFAULT NULL, "\
            "`CreateDate` int(11) DEFAULT NULL, "\
            "`License` blob "\
          ")";

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sql = "CREATE TABLE IF NOT EXISTS `CustomerLicenseValues` ( "\
            "`CustomerId` INTEGER, "\
            "`LicenseRowId` INTEGER, "\
            "`ProductId` int(11) DEFAULT NULL, "\
            "`FeatureId` int(11) DEFAULT NULL, "\
            "`value` varchar(200) DEFAULT NULL "\
          ")";

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sql = "CREATE TABLE IF NOT EXISTS `CustomerLicenseOptions` ( "\
            "`CustomerId` INTEGER, "\
            "`LicenseRowId` INTEGER, "\
            "`StartDate` int(11) DEFAULT NULL, "\
            "`EndDate` int(11) DEFAULT NULL, "\
            "`GraceDuration` int(11) DEFAULT NULL, "\
            "`Perpetual` int(11) DEFAULT NULL "\
          ")";

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sql = "CREATE TABLE IF NOT EXISTS `CustomerLicenseComputers` ( "\
            "`CustomerId` INTEGER, "\
            "`LicenseRowId` INTEGER, "\
            "`SeedId` int(11) DEFAULT NULL "\
          ")";

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

int atoi2(const char *str)
{
    if(str==NULL)
        return 0;
    return atoi(str);
}

static int FeatureListRowFetch(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=10)
    {
        fprintf(stderr, "ProductListRowFetch expecting 10 columns, got %d\n", argc);
        return 1;
    }
    QList<FeatureStore*> *Store = (QList<FeatureStore*> *)pStore;
    if(argv[1]!=NULL)
    {
        FeatureStore *ps = new FeatureStore(atoi(argv[0]),argv[1],argv[2], atoi(argv[3]), argv[4], atoi2(argv[5]), atoi2(argv[6]), atoi2(argv[7]), atoi2(argv[8]), atoi2(argv[9]));
        Store->push_back(ps);
    }
    return 0;
}

static int ProductListRowFetch(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=3)
    {
        fprintf(stderr, "ProductListRowFetch expecting 3 columns, got %d\n", argc);
        return 1;
    }
    if(argv[0]==NULL || argv[1]==NULL)
        return 1;

    QList<ProductStore*> *Store = (QList<ProductStore*> *)pStore;
    ProductStore *ps = new ProductStore(atoi(argv[0]),argv[1],argv[2]);

    //now load features for this product ( are we allowed to do query in query ? )
    ps->Features = new QList<FeatureStore*>;
    char sql[500];
    char *zErrMsg = 0;
    int rc;
    sprintf_s( sql, sizeof(sql), "select id,Name,Description,ProductId,ActivationKey,HiddenInUI,SelectedInUI,ForcedAddToLicense,FixedValue,NotEncrypted from features where ProductId='%d'", atoi(argv[0]));
    rc = sqlite3_exec(db, sql, FeatureListRowFetch, ps->Features, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    Store->push_back(ps);

    return 0;
}

QList<ProductStore*> *SQLConnection::GetProductList()
{
    const char *sql;
    char *zErrMsg = 0;
    int rc;
    QList<ProductStore*> *ret = new QList<ProductStore*>;
    sql = "select id,Name,Description from products";
    rc = sqlite3_exec(db, sql, ProductListRowFetch, ret, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    return ret;
}

static int CustomerListRowFetch(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=4)
    {
        fprintf(stderr, "CustomerListRowFetch expecting 4 columns, got %d\n", argc);
        return 1;
    }
    if(argv[0]==NULL || argv[1]==NULL)
        return 1;

    CustomerStore *cs = new CustomerStore(atoi(argv[0]),argv[1],argv[2],argv[3]);
    QList<CustomerStore*> *Store = (QList<CustomerStore*> *)pStore;
    Store->push_back(cs);

    return 0;
}

QList<CustomerStore*> *SQLConnection::GetCustomerList()
{
    const char *sql;
    char *zErrMsg = 0;
    int rc;
    QList<CustomerStore*> *ret = new QList<CustomerStore*>;
    sql = "select id,Name,phone,email from Customers";
    rc = sqlite3_exec(db, sql, CustomerListRowFetch, ret, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return ret;
}

static int CustomerSeedListRowFetch(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=4)
    {
        fprintf(stderr, "CustomerListRowFetch expecting 4 columns, got %d\n", argc);
        return 1;
    }
    if(argv[0]==NULL || argv[1]==NULL)
        return 1;

    CustomerSeedStore *cs = new CustomerSeedStore(atoi(argv[0]),atoi(argv[1]),argv[3], atoi(argv[2]));
    QList<CustomerSeedStore*> *Store = (QList<CustomerSeedStore*> *)pStore;
    Store->push_back(cs);

    return 0;
}

QList<CustomerSeedStore*> *SQLConnection::GetCustomerSeedList(int CustomerId)
{
    char *zErrMsg = 0;
    int rc;
    char Query[500];
    sprintf_s(Query,sizeof(Query),"select id,CustomerId,LENGTH(HEX(Seed))/2,Seed from CustomerSeeds where CustomerId=%d",CustomerId);
    QList<CustomerSeedStore*> *ret = new QList<CustomerSeedStore*>;
    rc = sqlite3_exec(db, Query, CustomerSeedListRowFetch, ret, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return ret;
}

int SQLConnection::InsertCustomerSeed(int CustomerId, char *Seed, int Size)
{
    sqlite3_stmt *pStmt;
    const char *sql = "INSERT INTO CustomerSeeds(CustomerId,seed) VALUES(?,?)";
    int rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(pStmt, 1, CustomerId);
    sqlite3_bind_blob(pStmt, 2, Seed, Size, SQLITE_STATIC);

    rc = sqlite3_step(pStmt);

    if (rc != SQLITE_DONE)
    {
        printf("execution failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(pStmt);

    return 0;
}

int SQLConnection::InsertCustomerLicense(int CustomerId, char *License, int Size)
{
    sqlite3_stmt *pStmt;
    const char *sql = "INSERT INTO CustomerLicenses(CustomerId,CreateDate,License) VALUES(?,?,?)";
    int rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(pStmt, 1, CustomerId);
    sqlite3_bind_int(pStmt, 2, time(NULL));
    sqlite3_bind_blob(pStmt, 3, License, Size, SQLITE_STATIC);

    rc = sqlite3_step(pStmt);

    if (rc != SQLITE_DONE)
    {
        printf("execution failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(pStmt);

    return 0;
}

void CustomerStore::Update(const char *pName, const char *pPhone, const char *pEmail)
{
    //avoid useless spamming ? Is it a useless check to check for useless update ?
    int DataChanged = 0;
    if(strcmp(Name,pName)!=0)
        DataChanged = 1;
    else if(strcmp(Phone,pPhone)!=0)
            DataChanged = 1;
    else if(strcmp(Email,pEmail)!=0)
            DataChanged = 1;
    if(DataChanged == 0)
        return;
//printf("update 2 %s , %s , %s =\n",Name,Phone,Email);

    //do the update
    if(pName!=NULL)
    {
        if(Name)
            delete Name;
        size_t Len = strlen(pName)+1;
        Name = new char[Len];
        strcpy_s(Name,Len,pName);
    }
    if(pPhone!=NULL)
    {
        if(Phone)
            delete Phone;
        size_t Len = strlen(pPhone)+1;
        Phone = new char[Len];
        strcpy_s(Phone,Len,pPhone);
    }
    if(pEmail!=NULL)
    {
        if(Email)
            delete Email;
        size_t Len = strlen(pEmail)+1;
        Email = new char[Len];
        strcpy_s(Email,Len,pEmail);
    }
//printf("update 3 %s , %s , %s =\n",Name,Phone,Email);

    //update or insert DB side
    SQLConnection::InsertCustomer(this);
}

int SQLConnection::InsertCustomer(CustomerStore *cs)
{
    //this is an insert
    if(cs->Id<=0)
    {
        sqlite3_stmt *pStmt;
        const char *sql = "INSERT INTO Customers(name,phone,email) VALUES(?,?,?)";
        int rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
            return 1;
        }
        sqlite3_bind_text(pStmt, 1, cs->Name, (int)strlen(cs->Name),SQLITE_STATIC);
        sqlite3_bind_text(pStmt, 2, cs->Phone,(int) strlen(cs->Phone), SQLITE_STATIC);
        sqlite3_bind_text(pStmt, 3, cs->Email, (int)strlen(cs->Email), SQLITE_STATIC);

        rc = sqlite3_step(pStmt);

        if (rc != SQLITE_DONE)
        {
            printf("execution failed: %s", sqlite3_errmsg(db));
        }

        sqlite3_finalize(pStmt);
    }
    //this is an update
    else
    {
        printf("we are making an update!\n");
        sqlite3_stmt *pStmt;
        const char *sql = "update Customers set name=?,phone=?,email=? where id=?";
        int rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
            return 1;
        }
        sqlite3_bind_text(pStmt, 1, cs->Name, (int)strlen(cs->Name),SQLITE_STATIC);
        sqlite3_bind_text(pStmt, 2, cs->Phone, (int)strlen(cs->Phone), SQLITE_STATIC);
        sqlite3_bind_text(pStmt, 3, cs->Email, (int)strlen(cs->Email), SQLITE_STATIC);
        sqlite3_bind_int(pStmt, 4, cs->Id);

        rc = sqlite3_step(pStmt);

        if (rc != SQLITE_DONE)
            printf("execution failed: %s", sqlite3_errmsg(db));

        sqlite3_finalize(pStmt);
    }
    return 0;
}

void CustomerStore::DeleteFromDb()
{
    sqlite3_stmt *pStmt;
    const char *sql = "delete from Customers where id=?";
    int rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_int(pStmt, 1, Id);
    rc = sqlite3_step(pStmt);

    if (rc != SQLITE_DONE)
        printf("execution failed: %s", sqlite3_errmsg(db));

    sqlite3_finalize(pStmt);

    //delete seeds also
    sql = "delete from CustomerSeeds where CustomerId=?";
    rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_int(pStmt, 1, Id);
    rc = sqlite3_step(pStmt);

    if (rc != SQLITE_DONE)
        printf("execution failed: %s", sqlite3_errmsg(db));

    sqlite3_finalize(pStmt);
}

struct ProductFeatureNameStore
{
    char *ProductName;
    char *FeatureName;
};

static int GetProductsFeatureName(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=2)
    {
        fprintf(stderr, "CustomerListRowFetch expecting 2 columns, got %d\n", argc);
        return 1;
    }
    if(argv[0]==NULL || argv[1]==NULL)
        return 1;

    ProductFeatureNameStore *ns = (ProductFeatureNameStore *)pStore;
    ns->ProductName = _strdup(argv[0]);
    ns->FeatureName = _strdup(argv[1]);

    return 0;
}

int SQLConnection::GetProductFeatureName(int ProductId, int FeatureId, char **ProductName, char **FeatureName)
{
    //always init return even if we do not return anything
    *ProductName = NULL;
    *FeatureName = NULL;
    char *zErrMsg = 0;
    int rc;
    char Query[500];
    sprintf_s(Query,sizeof(Query),"select products.Name,features.Name from products,features where products.id=features.ProductId and products.id=%d and features.id=%d",ProductId,FeatureId);
    ProductFeatureNameStore Names;
    memset(&Names,0,sizeof(ProductFeatureNameStore));
    rc = sqlite3_exec(db, Query, GetProductsFeatureName, &Names, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    *ProductName = Names.ProductName;
    *FeatureName = Names.FeatureName;
    return rc;
}

static int GetCustomerLastLicenseId_(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=1)
    {
        fprintf(stderr, "GetCustomerLastLicenseId expecting 1 columns, got %d\n", argc);
        return 1;
    }
    if(argv[0]==NULL)
        return 1;

    *(int*)pStore = atoi(argv[0]);

    return 0;
}

int SQLConnection::GetCustomerLastLicenseId(int CustomerId, int *LicenseId)
{
    //always init return even if we do not return anything
    char *zErrMsg = 0;
    int rc;
    char Query[500];
    sprintf_s(Query,sizeof(Query),"select LicenseRowId from CustomerLicenses where CustomerId=%d order by LicenseRowId desc limit 0,1",CustomerId);
    *LicenseId = -1;
    rc = sqlite3_exec(db, Query, GetCustomerLastLicenseId_, LicenseId, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return rc;
}

int SQLConnection::SetCustomerLicenseComputerSelectStatus(int CustomerId, int LicenseId, int ComputerId)
{
    sqlite3_stmt *pStmt;
    const char *sql = "replace into CustomerLicenseComputers VALUES(?,?,?)";
    int rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(pStmt, 1, CustomerId);
    sqlite3_bind_int(pStmt, 2, LicenseId);
    sqlite3_bind_int(pStmt, 3, ComputerId);

    rc = sqlite3_step(pStmt);

    if (rc != SQLITE_DONE)
    {
        printf("execution failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(pStmt);

    return 0;
}

static int GetCustomerLicenseComputerSelectStatus_(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=1)
    {
        fprintf(stderr, "GetCustomerLicenseComputerSelectStatus_ expecting 1 columns, got %d\n", argc);
        return 1;
    }
    if(argv[0]==NULL)
        return 1;

    *(int*)pStore = atoi(argv[0]);

    return 0;
}

int SQLConnection::GetCustomerLicenseComputerSelectStatus(int CustomerId, int LicenseId, int SeedId, int *IsSelected)
{
    //always init return even if we do not return anything
    char *zErrMsg = 0;
    int rc;
    char Query[500];
    sprintf_s(Query,sizeof(Query),"select count(*) from CustomerLicenseComputers where CustomerId=%d and LicenseRowId=%d and SeedId=%d",CustomerId,LicenseId,SeedId);
    *IsSelected = -1;
    rc = sqlite3_exec(db, Query, GetCustomerLicenseComputerSelectStatus_, IsSelected, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return rc;
}

int SQLConnection::SetCustomerLicenseProductFeatureStatus(int CustomerId, int LicenseId, int ProductId, int FeatureId, char *Value)
{
    sqlite3_stmt *pStmt;
    const char *sql = "replace into CustomerLicenseValues VALUES(?,?,?,?,?)";
    int rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(pStmt, 1, CustomerId);
    sqlite3_bind_int(pStmt, 2, LicenseId);
    sqlite3_bind_int(pStmt, 3, ProductId);
    sqlite3_bind_int(pStmt, 4, FeatureId);
    sqlite3_bind_text(pStmt, 5, Value, (int)strlen(Value), SQLITE_STATIC);

    rc = sqlite3_step(pStmt);

    if (rc != SQLITE_DONE)
    {
        printf("execution failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(pStmt);

    return 0;
}

static int GetCustomerLicenseProductFeatureStatus_(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=1)
    {
        fprintf(stderr, "GetCustomerLicenseProductFeatureStatus_ expecting 1 columns, got %d\n", argc);
        return 1;
    }
    if(argv[0]==NULL)
        return 1;

    *(char**)pStore = strdup( argv[0] );

    return 0;
}

int SQLConnection::GetCustomerLicenseProductFeatureStatus(int CustomerId, int LicenseId, int ProductId, int FeatureId, char **Value)
{
    //always init return even if we do not return anything
    char *zErrMsg = 0;
    int rc;
    char Query[500];
    sprintf_s(Query,sizeof(Query),"select value from CustomerLicenseValues where CustomerId=%d and LicenseRowId=%d and ProductId=%d and FeatureId=%d",CustomerId,LicenseId,ProductId,FeatureId);
    *Value = NULL;
    rc = sqlite3_exec(db, Query, GetCustomerLicenseProductFeatureStatus_, Value, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return rc;
}

int SQLConnection::SetCustomerLicenseOptions(int CustomerId, int LicenseId, int StartDate, int EndDate, int GraceDuration, int Perpetual)
{
    sqlite3_stmt *pStmt;
    const char *sql = "replace into CustomerLicenseOptions VALUES(?,?,?,?,?,?)";
    int rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(pStmt, 1, CustomerId);
    sqlite3_bind_int(pStmt, 2, LicenseId);
    sqlite3_bind_int(pStmt, 3, StartDate);
    sqlite3_bind_int(pStmt, 4, EndDate);
    sqlite3_bind_int(pStmt, 5, GraceDuration);
    sqlite3_bind_int(pStmt, 6, Perpetual);

    rc = sqlite3_step(pStmt);

    if (rc != SQLITE_DONE)
    {
        printf("execution failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(pStmt);

    return 0;
}

struct GetCustomerLicenseOptions_Store
{
    int StartDate;
    int EndDate;
    int GraceDuration;
    int Perpetual;
};

static int GetCustomerLicenseOptions_(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=4)
    {
        fprintf(stderr, "GetCustomerLicenseProductFeatureStatus_ expecting 1 columns, got %d\n", argc);
        return 1;
    }
    if(argv[0]==NULL)
        return 1;

    ((GetCustomerLicenseOptions_Store*)pStore)->StartDate = atoi( argv[0] );
    ((GetCustomerLicenseOptions_Store*)pStore)->EndDate = atoi( argv[1] );
    ((GetCustomerLicenseOptions_Store*)pStore)->GraceDuration = atoi( argv[2] );
    ((GetCustomerLicenseOptions_Store*)pStore)->Perpetual = atoi( argv[3] );

    return 0;
}

int SQLConnection::GetCustomerLicenseOptions(int CustomerId, int LicenseId, int *StartDate, int *EndDate, int *GraceDuration, int *Perpetual)
{
    //always init return even if we do not return anything
    char *zErrMsg = 0;
    int rc;
    char Query[500];
    sprintf_s(Query,sizeof(Query),"select StartDate,EndDate,GraceDuration,Perpetual from CustomerLicenseOptions where CustomerId=%d and LicenseRowId=%d",CustomerId,LicenseId);
    GetCustomerLicenseOptions_Store t;
    memset(&t,0,sizeof(t));
    rc = sqlite3_exec(db, Query, GetCustomerLicenseOptions_, &t, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    *StartDate = t.StartDate;
    *EndDate = t.EndDate;
    *GraceDuration = t.GraceDuration;
    *Perpetual = t.Perpetual;
    return rc;
}
