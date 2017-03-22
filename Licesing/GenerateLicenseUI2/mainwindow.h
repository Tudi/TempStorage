#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    void RefreshProductFeatureDropdownContent(int SelectedProductIndex);
private slots:
    void BrowseForSeedFile();
    void OnProductDropdownSelected(int index);
    void ProductFeatureAddToLicense();
    void OnRemoveSelectedLicenseClicked();
    void OnGenerateLicenseClicked();
};

#endif // MAINWINDOW_H
