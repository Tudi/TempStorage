#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QListWidgetItem>

namespace Ui {
class MainWindow;
}

class ProductStore;
class FeatureStore;
class CustomerStore;
class CustomerSeedStore;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    void                    RefreshCustomerDropdownContent(int SelectedProductIndex);
    void                    RefreshProductFeatureDropdownContent();
    void                    RefreshCustomerHostList();
    void                    RefreshCustomerLastLicenseOptions();
    CustomerStore           *GetSelectedCustomer();
    Ui::MainWindow          *ui;
    QList<ProductStore*>    *Products;
    QList<CustomerStore*>   *Customers;
private slots:
//    void OnProductDropdownSelected(int index);
    void OnCustomerDropdownSelected(int index);
    void OnCustomerUpdateCliecked();
    void OnCustomerDeleteCliecked();
    void OnProductFeatureClicked(QTreeWidgetItem*);
    void OnBrowseForSeedFiles();
    void OnSelectAllHostsClicked();
    void OnBrowseForOutputPathClicked();
    void OnGenerateLicensesClicked();
    void OnPermanentLicenseClicked();
    void OnProductFeatureValueEdit(QTreeWidgetItem*);
};

class QWidgetItemFeature : public QTreeWidgetItem
{
public:
    QWidgetItemFeature(FeatureStore *pfs)
    {
        fs = pfs; // should be shared pointer !
    }
    FeatureStore *fs;
};

class QListWidgetItemHost : public QListWidgetItem
{
public:
    QListWidgetItemHost(CustomerSeedStore *pcs)
    {
       cs = pcs;
    }
    CustomerSeedStore *cs;
};
#endif // MAINWINDOW_H
