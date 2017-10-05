#include <stdlib.h>
#include <string.h>
#include <QList>
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
}

void SQLConnection::DisConnectSQLite()
{
    if(db)
    {
        sqlite3_close(db);
        db = NULL;
    }
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
        fprintf(stderr, "ClientListRowFetch expecting 2 columns, got %d\n", argc);
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
