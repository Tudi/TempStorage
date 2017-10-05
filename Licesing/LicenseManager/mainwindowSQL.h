#ifndef MAINWINDOWSQL_H
#define MAINWINDOWSQL_H

class SQLConnection
{
public:
    static void ConnectSQLite();
    static void DisConnectSQLite();
    static int                      GetProductFeatureName(int ProductId, int FeatureId, char **ProductName, char **FeatureName);
};


#endif // MAINWINDOWSQL_H
