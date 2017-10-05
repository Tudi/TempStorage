#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QListWidgetItem>
#include <time.h>

#define LIBRARY_API __declspec(dllimport)
#include "../LicenseDLL/License_API.h"
#include "../LicenseDLL/License.h"

#ifdef _DEBUG
    #pragma comment(lib, "../x64/Debug/LicenseDLL.lib")
#else
    #pragma comment(lib, "../x64/Release/LicenseDLL.lib")
#endif

#include "mainwindowSQL.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //first connect. Maybe some UI will initialize from DB
    SQLConnection::ConnectSQLite();

    connect(ui->pb_BrowseLicense, SIGNAL(clicked()), this, SLOT(OnBrowseForLicenseFile()));
}

MainWindow::~MainWindow()
{
    SQLConnection::DisConnectSQLite();
    delete ui;
}

void MainWindow::OnBrowseForLicenseFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select license file"), QDir::currentPath(), "*.dat");
    if(path.length()==0)
        return;
    std::string Path = path.toStdString();

     License *TestLicense = new License;

     if (TestLicense == NULL)
     {
         QMessageBox Msgbox;
         Msgbox.setText("Could not create license object. Unexpected error");
         Msgbox.exec();
         return;
     }
     const char *PathC = Path.c_str();
     int er = TestLicense->LoadFromFile(PathC);
//     printf("trying to load license from path %s\n",PathC);
     //maybe we are running it in a local directory ?
#ifdef SIEMS_SIDE_BUILD
     if (er==ERROR_NO_DECODABLE_LICENSE_FOUND)
     {
         QString SeedPath = QFileDialog::getOpenFileName(this, tr("Select seed file"), QDir::currentPath(), "*.dat");
         std::string SeedPath2 = SeedPath.toStdString();

         er = TestLicense->LoadFromFile(Path.c_str(), SeedPath2.c_str());
     }
#endif
     if (er != 0)
     {
         QMessageBox Msgbox;
         Msgbox.setText("Could not load a valid license from the file " + QString(PathC) + ". Error " + QString().number(er));
         Msgbox.exec();
         delete TestLicense;
         return;
     }

     time_t StartStamp;
     TestLicense->GetStartStamp(&StartStamp);

     time_t EndStamp;
     TestLicense->GetEndStamp(&EndStamp);

     time_t GracePeriodRemaining;
     char   IsGraceTriggered;
     TestLicense->IsGracePeriodTriggered(&GracePeriodRemaining,&IsGraceTriggered);

     QDateTime Start;
     Start.setTime_t(StartStamp);
     ui->te_ValidFrom->setText( Start.toString(Qt::SystemLocaleShortDate) );

     if(EndStamp==0)
     {
         ui->te_ValidFrom->setText( "Permanent license" );
     }
     else
     {
        QDateTime End;
        End.setTime_t(EndStamp);
        ui->te_ValidUntil->setText( End.toString(Qt::SystemLocaleShortDate) );
     }

     //grace remaining time only has a value if it has been triggered
     if(IsGraceTriggered)
         ui->te_GraceDuration->setDisabled(false);
     else
         ui->te_GraceDuration->setDisabled(true);

     ui->te_GraceDuration->setText( QString().number( (int)GracePeriodRemaining ) );

#define MAX_USED_SIEMENS_PROJECT_IDS 30
#define MAX_USED_SIEMENS_FEATURE_IDS 30
     ui->li_keys->clear();
     //iterate through all possible licenses and get their activation key
     for (int ProductID = 0; ProductID < MAX_USED_SIEMENS_PROJECT_IDS; ProductID++)
         for (int FeatureId = 0; FeatureId < MAX_USED_SIEMENS_FEATURE_IDS; FeatureId++)
         {
             char ActivationKeyBuffer[200];
             int GetKeyRes = TestLicense->GetActivationKey(ProductID, FeatureId, ActivationKeyBuffer, sizeof(ActivationKeyBuffer));
             if(GetKeyRes != 0 && GetKeyRes != WARNING_NO_LICENSE_FOUND)
             {
                 QMessageBox Msgbox;
                 Msgbox.setText("Could not get activation key. Error " + QString().number(er));
                 Msgbox.exec();
                 delete TestLicense;
                 return;
             }
             if(GetKeyRes!=0)
                 continue;
             char *ProductName=NULL;
             char *FeatureName=NULL;
             SQLConnection::GetProductFeatureName(ProductID,FeatureId,&ProductName,&FeatureName);
             QListWidgetItem *item = new QListWidgetItem();
             if(ProductName!=NULL && FeatureName!=NULL )
                 item->setText( QString(ProductName) + "-" + QString(FeatureName));
             else
                 item->setText( "Unknown product - feature " + QString().number(ProductID) + " " + QString().number(FeatureId));
            ui->li_keys->addItem( item );
         }
#if 0
#endif
}
