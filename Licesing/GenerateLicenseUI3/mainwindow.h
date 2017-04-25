#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

namespace Ui {
class MainWindow;
}

class ProductStore;
class FeatureStore;
class ClientStore;
class ClientSeedStore;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    void RefreshProductFeatureDropdownContent(int SelectedProductIndex);
    void RefreshClientDropdownContent(int SelectedProductIndex);
    void RefreshClientHostList();
    ClientStore *GetSelectedClient();
    Ui::MainWindow          *ui;
    QList<ProductStore*>    *Products;
    QList<ClientStore*>     *Clients;
private slots:
    void OnProductDropdownSelected(int index);
    void OnClientDropdownSelected(int index);
    void OnClientUpdateCliecked();
    void OnClientDeleteCliecked();
    void OnProductFeatureClicked(QListWidgetItem*);
    void OnBrowseForSeedFiles();
    void OnSelectAllHostsClicked();
    void OnBrowseForOutputPathClicked();
    void OnGenerateLicensesClicked();
    void OnPermanentLicenseClicked();
};

class QListWidgetItemFeature : public QListWidgetItem
{
public:
    QListWidgetItemFeature(FeatureStore *pfs)
    {
        fs = pfs; // should be shared pointer !
    }
    FeatureStore *fs;
};

class QListWidgetItemHost : public QListWidgetItem
{
public:
    QListWidgetItemHost(ClientSeedStore *pcs)
    {
       cs = pcs;
    }
    ClientSeedStore *cs;
};
#endif // MAINWINDOW_H
