#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QDateTime>
#include <QDate>
#include <QMessageBox>
#include <QComboBox>
#include <QFileInfo>

#define LIBRARY_API __declspec(dllimport)
#include "../LicenseDLL/SimpleList.h"
#include "../LicenseDLL/ProjectFeatureKeys.h"
#include "../LicenseDLL/License.h"

#ifdef _DEBUG
    #pragma comment(lib, "../LicenseDLL/Debug/LicenseDLL.lib")
#else
    #pragma comment(lib, "../LicenseDLL/Release/LicenseDLL.lib")
#endif

#define LICENSE_SEED_CONSTANT_NAME "LicenseSeed.dat"
#define LICENSE_FILE_CONSTANT_NAME "License.dat"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //initialize date pickers to current time
    QDate TimeNow = QDate::currentDate();
    ui->dt_StartDate->setDate( TimeNow );

    //initialize date pickers to next month
    QDate NextMonth = QDate::currentDate();
    NextMonth = NextMonth.addMonths(1);
    ui->dt_EndDate->setDate( NextMonth );

    //populate dropdown lists
    RefreshProductFeatureDropdownContent(0);

    //link events to functions
    connect(ui->b_BrowseClientSeed, SIGNAL(clicked()), this, SLOT(BrowseForSeedFile()));
    connect(ui->CB_ProductIdPicker, SIGNAL(currentIndexChanged(int)), this, SLOT(OnProductDropdownSelected(int)));
    connect(ui->b_AddToLicense, SIGNAL(clicked()), this, SLOT(ProductFeatureAddToLicense()));
    connect(ui->b_RemoveSelectedFromLicense, SIGNAL(clicked()), this, SLOT(OnRemoveSelectedLicenseClicked()));
    connect(ui->b_GenerateLicense, SIGNAL(clicked()), this, SLOT(OnGenerateLicenseClicked()));

    //init table header for license content
    ui->table_LicenseContent->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->table_LicenseContent->setSelectionBehavior(QAbstractItemView::SelectRows);
//    ui->table_LicenseContent->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->table_LicenseContent->setColumnCount(2);
    QTableWidgetItem *item1=new QTableWidgetItem ("Product");
    QTableWidgetItem *item2=new QTableWidgetItem ("Feature");
    ui->table_LicenseContent->setHorizontalHeaderItem(0,item1);
    ui->table_LicenseContent->setHorizontalHeaderItem(1,item2);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::BrowseForSeedFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select license seed"), QDir::currentPath(), LICENSE_SEED_CONSTANT_NAME);
    ui->t_LicenseSeedPath->setPlainText( path );

}

void MainWindow::RefreshProductFeatureDropdownContent(int SelectedProductIndex)
{
    int DropdownIndex = 0;
    //populate the dropdown with product names
    ProjectFeatureKeyDB *ProjectFeatureDropdownList;
    ProjectFeatureDropdownList = new ProjectFeatureKeyDB;
    if(ProjectFeatureDropdownList!=NULL)
    {
        //do we need to clear the current content
        int HasContent = ui->CB_ProductIdPicker->count();
        //add new content
        int SelectedProductId = -1;
        for (ProjectFeatureKeyStore *itr = ProjectFeatureDropdownList->ListStart(); itr != NULL; itr = ProjectFeatureDropdownList->ListNext())
        {
            if(DropdownIndex == SelectedProductIndex)
                SelectedProductId = itr->ProjectID;
            DropdownIndex++;
            if(HasContent == 0)
            {
                QString itm( itr->ProjectName );
                ui->CB_ProductIdPicker->addItem( itm );
            }
        }

        //populate feature list based on first product
        ui->CB_FeatureIdPicker->clear();
        if(SelectedProductId!=-1)
        {
            for (ProjectFeatureKeyStore *itr = ProjectFeatureDropdownList->ListStart(); itr != NULL; itr = ProjectFeatureDropdownList->ListNext())
                if(itr->ProjectID == SelectedProductId)
                {
                    QString itm( itr->FeatureName );
                    ui->CB_FeatureIdPicker->addItem( itm );
                }
        }
    }
}

void MainWindow::OnProductDropdownSelected(int index)
{
    //refresh the feature list based on the newly selected product
    RefreshProductFeatureDropdownContent(index);
}

void MainWindow::ProductFeatureAddToLicense()
{
    //get the string of dropdown box
    QString Product = ui->CB_ProductIdPicker->currentText();
    QString Feature = ui->CB_FeatureIdPicker->currentText();

    //check if this row already exits
    int RowCount = ui->table_LicenseContent->rowCount();
    for( int i=0;i<RowCount;i++)
    {
        QTableWidgetItem *CurFeature = ui->table_LicenseContent->item(i,1);
        if(CurFeature->text().compare(Feature)==0)
            return;
    }
    //add it as a new row
    ui->table_LicenseContent->insertRow(0);

    QTableWidgetItem *item1=new QTableWidgetItem (Product);
    QTableWidgetItem *item2=new QTableWidgetItem (Feature);

    ui->table_LicenseContent->setItem(0,0,item1);
    ui->table_LicenseContent->setItem(0,1,item2);

    //sort items based on product
    ui->table_LicenseContent->sortItems(0);
}

void MainWindow::OnRemoveSelectedLicenseClicked()
{
    QModelIndexList indexes =  ui->table_LicenseContent->selectionModel()->selectedRows();
    int countRow = indexes.count();

    //always remove from high to low index to not change indexing while remove rows 1 by 1
    qSort( indexes );

    //remove rows 1 by 1
    for( int i = countRow; i > 0; i--)
           ui->table_LicenseContent->removeRow( indexes.at(i-1).row() );
}

bool fileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    if (check_file.exists() && check_file.isFile()) {
        return true;
    } else {
        return false;
    }
}

void MainWindow::OnGenerateLicenseClicked()
{
    QDate StartDate = ui->dt_StartDate->date();
    QDate EndDate = ui->dt_EndDate->date();

    //is date format valid ?
    if(EndDate<=StartDate)
    {
        QMessageBox Msgbox;
        Msgbox.setText("License duration is too small");
        Msgbox.exec();
        return;
    }

    //does fingerprint file exists ?
    QString FingerprintFilePath = ui->t_LicenseSeedPath->toPlainText();
    if(fileExists(FingerprintFilePath)== false)
    {
        QMessageBox Msgbox;
        Msgbox.setText("Could not find license seed file");
        Msgbox.exec();
    }

    //calculate start and duration
    QDateTime StartDateTime(StartDate);
    StartDateTime.setTimeSpec(Qt::UTC);
    QDateTime EndDateTime(StartDate);
    EndDateTime.setTimeSpec(Qt::UTC);

    int LicenseDuration = EndDateTime.toTime_t() - StartDateTime.toTime_t();
    time_t LicenseStart = StartDateTime.toTime_t();

    int er, ers = 0;
    //create a license
    License *TestLicense = new License;
    er = TestLicense->SetDuration(LicenseStart, LicenseDuration);
    ers += er;

    //iterate through the table and add rows 1 by 1
    int RowCount = ui->table_LicenseContent->rowCount();
    for( int row = 0; row<RowCount; row++)
    {
        QString ProductName = ui->table_LicenseContent->item(row,0)->text();
        QString FeatureName = ui->table_LicenseContent->item(row,1)->text();
        TestLicense->AddProjectFeature( ProductName.toStdString().c_str(), FeatureName.toStdString().c_str());
    }

    //where to save the output
    QString OutputPath(QDir(FingerprintFilePath).currentPath());
    OutputPath += "/";
    OutputPath += LICENSE_FILE_CONSTANT_NAME;

    er = TestLicense->SaveToFile(OutputPath.toStdString().c_str(),FingerprintFilePath.toStdString().c_str());
    ers += er;

    delete TestLicense;

    if( ers == 0 )
    {
        QMessageBox Msgbox;
        Msgbox.setText("License successfully generated");
        Msgbox.exec();
    }
    else
    {
        QMessageBox Msgbox;
        Msgbox.setText("There was an error generating the license");
        Msgbox.exec();
    }
}
