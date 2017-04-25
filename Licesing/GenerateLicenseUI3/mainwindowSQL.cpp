#include <stdlib.h>
#include <string.h>
#include <QList>
#include "mainwindowSQL.h"
#include "../SQLite/sqlite3.h"
#pragma comment(lib, "../SQLite/sqlite3.lib")
sqlite3 *db = NULL; //global connection. One is enough / application

void SQLConnection::ConnectSQLite()
{
    int rc = sqlite3_open("ClientLicense.db", &db);
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
    char *sql;
    char *zErrMsg = 0;
    int rc;
    sql = "CREATE TABLE IF NOT EXISTS `products` ( "\
            "`id` INTEGER PRIMARY KEY, "\
            "`Name` varchar(50) DEFAULT NULL, "\
            "`Description` varchar(200) DEFAULT NULL "\
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
            "`ActivationKey` varchar(50) DEFAULT NULL "\
          ")";

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sql = "CREATE TABLE IF NOT EXISTS `clients` ( "\
            "`id` INTEGER PRIMARY KEY, "\
            "`Name` varchar(50) DEFAULT NULL, "\
            "`Phone` varchar(200) DEFAULT NULL, "\
            "`Email` varchar(200) DEFAULT NULL "\
          ")";

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sql = "CREATE TABLE IF NOT EXISTS `client_seeds` ( "\
            "`id` INTEGER PRIMARY KEY, "\
            "`ClientId` int(11) DEFAULT NULL, "\
            "`Seed` blob "\
          ")";

    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

static int FeatureListRowFetch(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=5)
    {
        fprintf(stderr, "ProductListRowFetch expecting 3 columns, got %d\n", argc);
        return 1;
    }
    QList<FeatureStore*> *Store = (QList<FeatureStore*> *)pStore;
    if(argv[1]!=NULL)
    {
        FeatureStore *ps = new FeatureStore(atoi(argv[0]),argv[1],argv[2], atoi(argv[3]), argv[4]);
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
    sprintf_s( sql, sizeof(sql), "select id,Name,Description,ProductId,ActivationKey from features where ProductId='%d'", atoi(argv[0]));
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
    char *sql;
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

static int ClientListRowFetch(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=4)
    {
        fprintf(stderr, "ClientListRowFetch expecting 4 columns, got %d\n", argc);
        return 1;
    }
    if(argv[0]==NULL || argv[1]==NULL)
        return 1;

    ClientStore *cs = new ClientStore(atoi(argv[0]),argv[1],argv[2],argv[3]);
    QList<ClientStore*> *Store = (QList<ClientStore*> *)pStore;
    Store->push_back(cs);

    return 0;
}

QList<ClientStore*> *SQLConnection::GetClientList()
{
    char *sql;
    char *zErrMsg = 0;
    int rc;
    QList<ClientStore*> *ret = new QList<ClientStore*>;
    sql = "select id,Name,phone,email from clients";
    rc = sqlite3_exec(db, sql, ClientListRowFetch, ret, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return ret;
}

static int ClientSeedListRowFetch(void *pStore, int argc, char **argv, char **azColName)
{
    (void)azColName;
    if(argc!=4)
    {
        fprintf(stderr, "ClientListRowFetch expecting 4 columns, got %d\n", argc);
        return 1;
    }
    if(argv[0]==NULL || argv[1]==NULL)
        return 1;

    ClientSeedStore *cs = new ClientSeedStore(atoi(argv[0]),atoi(argv[1]),argv[3], atoi(argv[2]));
    QList<ClientSeedStore*> *Store = (QList<ClientSeedStore*> *)pStore;
    Store->push_back(cs);

    return 0;
}

QList<ClientSeedStore*> *SQLConnection::GetClientSeedList(int ClientId)
{
    char *zErrMsg = 0;
    int rc;
    char Query[500];
    sprintf_s(Query,sizeof(Query),"select id,ClientId,LENGTH(HEX(Seed))/2,Seed from client_seeds where ClientId=%d",ClientId);
    QList<ClientSeedStore*> *ret = new QList<ClientSeedStore*>;
    rc = sqlite3_exec(db, Query, ClientSeedListRowFetch, ret, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return ret;
}

int SQLConnection::InsertClientSeed(int ClientId, char *Seed, int Size)
{
    sqlite3_stmt *pStmt;
    char *sql = "INSERT INTO client_seeds(clientid,seed) VALUES(?,?)";
    int rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_int(pStmt, 1, ClientId);
    sqlite3_bind_blob(pStmt, 2, Seed, Size, SQLITE_STATIC);

    rc = sqlite3_step(pStmt);

    if (rc != SQLITE_DONE)
    {
        printf("execution failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(pStmt);

    return 0;
}

void ClientStore::Update(const char *pName, const char *pPhone, const char *pEmail)
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
        int Len = strlen(pName)+1;
        Name = new char[Len];
        strcpy_s(Name,Len,pName);
    }
    if(pPhone!=NULL)
    {
        if(Phone)
            delete Phone;
        int Len = strlen(pPhone)+1;
        Phone = new char[Len];
        strcpy_s(Phone,Len,pPhone);
    }
    if(pEmail!=NULL)
    {
        if(Email)
            delete Email;
        int Len = strlen(pEmail)+1;
        Email = new char[Len];
        strcpy_s(Email,Len,pEmail);
    }
//printf("update 3 %s , %s , %s =\n",Name,Phone,Email);

    //update or insert DB side
    SQLConnection::InsertClient(this);
}

int SQLConnection::InsertClient(ClientStore *cs)
{
    //this is an insert
    if(cs->Id<=0)
    {
        sqlite3_stmt *pStmt;
        char *sql = "INSERT INTO clients(name,phone,email) VALUES(?,?,?)";
        int rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
            return 1;
        }
        sqlite3_bind_text(pStmt, 1, cs->Name, strlen(cs->Name),SQLITE_STATIC);
        sqlite3_bind_text(pStmt, 2, cs->Phone, strlen(cs->Phone), SQLITE_STATIC);
        sqlite3_bind_text(pStmt, 3, cs->Email, strlen(cs->Email), SQLITE_STATIC);

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
        char *sql = "update clients set name=?,phone=?,email=? where id=?";
        int rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);

        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
            return 1;
        }
        sqlite3_bind_text(pStmt, 1, cs->Name, strlen(cs->Name),SQLITE_STATIC);
        sqlite3_bind_text(pStmt, 2, cs->Phone, strlen(cs->Phone), SQLITE_STATIC);
        sqlite3_bind_text(pStmt, 3, cs->Email, strlen(cs->Email), SQLITE_STATIC);
        sqlite3_bind_int(pStmt, 4, cs->Id);

        rc = sqlite3_step(pStmt);

        if (rc != SQLITE_DONE)
            printf("execution failed: %s", sqlite3_errmsg(db));

        sqlite3_finalize(pStmt);
    }
    return 0;
}

void ClientStore::DeleteFromDb()
{
    sqlite3_stmt *pStmt;
    char *sql = "delete from clients where id=?";
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
    sql = "delete from client_seeds where ClientId=?";
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
