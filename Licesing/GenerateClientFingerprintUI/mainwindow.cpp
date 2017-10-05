#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QDateTime>

#define LIBRARY_API __declspec(dllimport)
#include "../LicenseDLL/ComputerFingerprint.h"
#include "../LicenseDLL/VMTools.h"
#include "../LicenseDLL/Config/ConfigLoader.h"
#include "../LicenseDLL/SSLClient/RemoteInfo_API.h"

#define LICENSE_SEED_CONSTANT_NAME "LicenseSeed"

#ifdef _DEBUG
    #pragma comment(lib, "../x64/Debug/LicenseDLL.lib")
#else
    #pragma comment(lib, "../x64/Release/LicenseDLL.lib")
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->te_OutputPath->setPlainText(QDir::currentPath());

    connect(ui->pb_BrowsePath, SIGNAL(clicked()), this, SLOT(browse()));
    connect(ui->pb_Generate, SIGNAL(clicked()), this, SLOT(GenerateSeedFile()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::browse()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select destination folder"), QDir::currentPath());

    ui->te_OutputPath->setPlainText( path );
}

int MainWindow::GenerateSeedFile(QString path)
{

    ComputerFingerprint *ClientSeed;
    int er = 0;

    //create a new store
    ClientSeed = CreateComputerFingerprint();
    if (ClientSeed == NULL)
    {
        printf("Could not create ComputerFingerprint object\n");
        return 1;
    }

    //test generate API
    er = ClientSeed->GenerateFingerprint();
    if (er != 0)
    {
        printf("Could not generate ComputerFingerprint content\n");
        DestroyComputerFingerprint(&ClientSeed);
        return 1;
    }

    std::string Role = ui->co_Role->currentText().toStdString();
    std::string cName = ui->te_Name->toPlainText().toStdString();
    er = ClientSeed->AppendClientInfo( Role.c_str(), cName.c_str() );
    if (er != 0)
    {

        printf("Could not append client info\n");
        DestroyComputerFingerprint(&ClientSeed);
        return 1;
    }

    QDateTime TimeNow = QDateTime::currentDateTime();
    QString dateTimeString = TimeNow.toString("yyyy_MMM_dd_hh_mm");

    QString FullPath = path;
    FullPath += QString("\\LicenseSeed_");
    FullPath += QString(Role.c_str()).split("(")[0];
    FullPath += "_" + dateTimeString;
    FullPath += QString(".dat");

    //test save
    er = ClientSeed->SaveFingerprint( (char*)FullPath.toStdString().c_str() );
    if (er != 0)
    {
        printf("Could not save ComputerFingerprint content\n");
        DestroyComputerFingerprint(&ClientSeed);
        return 1;
    }

    //test destroy
    DestroyComputerFingerprint(&ClientSeed);

    return 0;
}

void MainWindow::GenerateSeedFile()
{
    //if we are inside a VM, we will need to connect to a fingerprinting service
    if( Detect_VM() != 0 )
    {
        //open config file
        char	ConfigIP[20];
        int		ConfigPort;
        if (GetStrConfig("config.txt", "QueryServiceIP", ConfigIP, sizeof(ConfigIP)))
        {
            QMessageBox Msgbox;
            Msgbox.setText("Could not load QueryServiceIP config value from config.txt");
            Msgbox.exec();
            return;
        }
        if (GetIntConfig("config.txt", "QueryServicePort", &ConfigPort))
        {
            QMessageBox Msgbox;
            Msgbox.setText("Could not load QueryServicePort config value from config.txt");
            Msgbox.exec();
            return;
        }
        //connect to the fingerprinting service
        char RemoteUUID[16];
        int err = GetRemoteUUID(ConfigIP, ConfigPort, RemoteUUID, sizeof(RemoteUUID));
        if( err != 0)
        {
            QMessageBox Msgbox;
            Msgbox.setText("There was issue " + QString().number(err) + " obtaining info from remote server " + QString(ConfigIP) + ":" + QString().number(ConfigPort) + ". Check QueryServiceIP and QueryServicePort values inside config.txt");
            Msgbox.exec();
            return;
        }
    }
    QString FullPath( ui->te_OutputPath->toPlainText() );
    QDir path = ui->te_OutputPath->toPlainText();

    //if does not exist, try to create it
    if(!path.exists())
        path.mkdir( "." );

    //if it still does not exist maybe it's a bug.
    if(!path.exists())
    {
        QMessageBox Msgbox;
        Msgbox.setText("Could not save file to the given path");
        Msgbox.exec();
        return;
    }

    //try to generate the seed file at the target location
    if(GenerateSeedFile(path.absolutePath())!=0)
    {
        QMessageBox Msgbox;
        Msgbox.setText("There was an error generating the output file");
        Msgbox.exec();
        return;
    }

    QString fname( LICENSE_SEED_CONSTANT_NAME );
    QMessageBox Msgbox;
    Msgbox.setText("Output file " + fname + " was successfuly generated at the target path");
    Msgbox.exec();
}
