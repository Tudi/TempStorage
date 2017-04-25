#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QListWidgetItem>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include "mainwindowSQL.h"
#include "mainwindowlic.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //first connect. Maybe some UI will initialize from DB
    SQLConnection::ConnectSQLite();

    ui->setupUi(this);

    //initialize date pickers to current time
    QDate TimeNow = QDate::currentDate();
    ui->dt_StartDate->setDate( TimeNow );

    //initialize date pickers to next month
    QDate NextMonth = QDate::currentDate();
    NextMonth = NextMonth.addMonths(3);
    ui->dt_EndDate->setDate( NextMonth );

    //browse will point to current directory bty default
    ui->te_OutputPath->setPlainText(QDir::currentPath());

    //populate product dropdown
    Products = SQLConnection::GetProductList();
    RefreshProductFeatureDropdownContent(0);
    connect(ui->CB_ProductIdPicker, SIGNAL(currentIndexChanged(int)), this, SLOT(OnProductDropdownSelected(int)));

    //make features list 1 click selectable
    connect(ui->li_Features, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(OnProductFeatureClicked(QListWidgetItem*)));

    //populate client list
    Clients = NULL;
    RefreshClientDropdownContent(0);
    connect(ui->cb_Clients, SIGNAL(currentIndexChanged(int)), this, SLOT(OnClientDropdownSelected(int)));
    connect(ui->b_UpdateClient, SIGNAL(clicked()), this, SLOT(OnClientUpdateCliecked()));
    connect(ui->b_DeleteClient, SIGNAL(clicked()), this, SLOT(OnClientDeleteCliecked()));

    //popup browse for browse button
    connect(ui->b_BrowseClientSeed, SIGNAL(clicked()), this, SLOT(OnBrowseForSeedFiles()));

    connect(ui->b_SelectAllHosts, SIGNAL(clicked()), this, SLOT(OnSelectAllHostsClicked()));

    connect(ui->b_BrowseOutputPath, SIGNAL(clicked()), this, SLOT(OnBrowseForOutputPathClicked()));

    connect(ui->b_GenerateLicense, SIGNAL(clicked()), this, SLOT(OnGenerateLicensesClicked()));
    connect(ui->checkb_PermanentLicense, SIGNAL(clicked()), this, SLOT(OnPermanentLicenseClicked()));
}

void MainWindow::OnPermanentLicenseClicked()
{
    if(ui->checkb_PermanentLicense->isChecked())
        ui->dt_EndDate->setEnabled(false);
    else
        ui->dt_EndDate->setEnabled(true);
}

void MainWindow::OnBrowseForOutputPathClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select destination folder"), ui->te_OutputPath->toPlainText());
    if(path.length()>0)
        ui->te_OutputPath->setPlainText( path );
}

void MainWindow::OnSelectAllHostsClicked()
{
    int Count = ui->li_ClientPCs->count();
    for(int i=0;i<Count;i++)
    {
        QListWidgetItem *item = ui->li_ClientPCs->item(i);
        if(item)
            item->setCheckState(Qt::Checked);
    }
}

void MainWindow::RefreshClientHostList()
{
    //get rid of the old content
    ui->li_ClientPCs->clear();

    ClientStore *SelectedClient = GetSelectedClient();
    if(SelectedClient==NULL || SelectedClient->Id<=0)
    {
        QMessageBox Msgbox;
        Msgbox.setText("Please select a client first");
        Msgbox.exec();
        return;
    }
    QList<ClientSeedStore*> *ClientSeeds = SQLConnection::GetClientSeedList(SelectedClient->Id);
    for(QList<ClientSeedStore*>::Iterator itr = ClientSeeds->begin();itr!=ClientSeeds->end();itr++)
    {
        ClientSeedStore *cs = *itr;
        char *Host;
        char *Role;
        LicenseAPI::GetSeedInfo(cs->Seed,cs->SeedSize,&Host,&Role);
        if(Host != NULL )
        {
            QListWidgetItemHost *item = new QListWidgetItemHost(cs);
            item->setData( Qt::DisplayRole, "text" );
            item->setData( Qt::CheckStateRole, Qt::Unchecked ); //checked
            item->setText( QString(Host) + "-" + QString(Role) );
            ui->li_ClientPCs->addItem( item );

            LicenseAPI::FreeLicDup( Host );
            LicenseAPI::FreeLicDup( Role );
        }
    }
    ClientSeeds->clear();
    delete ClientSeeds;
}

void MainWindow::OnBrowseForSeedFiles()
{
    ClientStore *SelectedClient = GetSelectedClient();
    if(SelectedClient==NULL || SelectedClient->Id<=0)
    {
        QMessageBox Msgbox;
        Msgbox.setText("Please select a client first");
        Msgbox.exec();
        return;
    }
    QFileDialog dialog(this);
    dialog.setDirectory(QDir::currentPath());
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(trUtf8("*.dat"));
    if (dialog.exec())
    {
        QStringList fileNames;
        fileNames = dialog.selectedFiles();
        for(QStringList::Iterator itr = fileNames.begin(); itr != fileNames.end();itr++)
        {
             std::string Path = (*itr).toStdString();
             char *data;
             int Len;
             LicenseAPI::GetSeedContentFromFile( Path.c_str(), &data, &Len );
             if(Len > 0)
             {
                SQLConnection::InsertClientSeed(SelectedClient->Id,data,Len);
                LicenseAPI::FreeLicDup( data );
                data = NULL;
             }
        }
    }
    RefreshClientHostList();
}

ClientStore *MainWindow::GetSelectedClient()
{
    for(QList<ClientStore*>::Iterator itr = Clients->begin();itr!=Clients->end();itr++)
    {
        ClientStore *ps = *itr;
        if( ps->IsSelected != 0)
            return ps;
    }
    return NULL;
}

void MainWindow::OnClientUpdateCliecked()
{
    ClientStore *SelectedClient = GetSelectedClient();
    //bugs :(
    if(SelectedClient == NULL)
    {
        fprintf(stderr,"Could not edit a non selected client");
        return;
    }
    //fetch new data form text fields
    std::string Name = ui->te_ClientName->toPlainText().toStdString();
    std::string Phone = ui->te_ClientPhone->toPlainText().toStdString().c_str();
    std::string Email = ui->te_ClientEmail->toPlainText().toStdString().c_str();

    SelectedClient->Update( Name.c_str(), Phone.c_str(), Email.c_str() );

    //refresh dropdown list content
    //TODO:auto select last selection
    ui->cb_Clients->clear();
}

void MainWindow::OnClientDeleteCliecked()
{
    ClientStore *SelectedClient = GetSelectedClient();
    //bugs :(
    if(SelectedClient == NULL)
    {
        fprintf(stderr,"Could not edit a non selected client");
        return;
    }
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "", "Delete client?",QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;
    SelectedClient->DeleteFromDb();
    ui->cb_Clients->clear();
}

void MainWindow::RefreshClientDropdownContent(int SelectedProductIndex)
{
    //init the list if we have not done so
    if(Clients)
        delete Clients;
    Clients = SQLConnection::GetClientList();
    {
        ClientStore *cs = new ClientStore(0,"New","","");
        Clients->push_back(cs);
    }

    int HasItems = ui->cb_Clients->count();
    int DropdownIndex = 0;
    for(QList<ClientStore*>::Iterator itr = Clients->begin();itr!=Clients->end();itr++)
    {
        ClientStore *ps = *itr;
        if(SelectedProductIndex>=0)
            ps->IsSelected = 0;
        if(DropdownIndex == SelectedProductIndex)
        {
            ui->te_ClientName->setText( QString(ps->Name));
            ui->te_ClientPhone->setText( QString(ps->Phone));
            ui->te_ClientEmail->setText( QString(ps->Email));
            ps->IsSelected = 1;
        }
        if(HasItems==0)
            ui->cb_Clients->addItem( QString(ps->Name) );
        DropdownIndex++;
    }

    //chain refresh the hosts list
    RefreshClientHostList();
}

void MainWindow::OnClientDropdownSelected(int SelectedProductIndex)
{
    RefreshClientDropdownContent(SelectedProductIndex);
}

void MainWindow::OnProductFeatureClicked(QListWidgetItem *item)
{
    FeatureStore *fs = dynamic_cast<QListWidgetItemFeature*>(item)->fs;
    if( item->checkState() == Qt::Checked)
    {
        item->setCheckState(Qt::Unchecked);
        fs->IsSelected = 0;
    }
    else
    {
        item->setCheckState(Qt::Checked);
        fs->IsSelected = 1;
    }
}

void MainWindow::RefreshProductFeatureDropdownContent(int SelectedProductIndex)
{
    if(Products==NULL)
        return;

    //get rid of old contet
    int HasContent = ui->CB_ProductIdPicker->count();

    //populate the dropdown with new content
    int DropdownIndex=0;
    for(QList<ProductStore*>::Iterator itr = Products->begin();itr!=Products->end();itr++)
    {
        ProductStore *ps = *itr;
        //clear this selectection
        ps->IsSelected = 0;

        //populate checkboxes
        if(DropdownIndex == SelectedProductIndex)
        {
            //mark it as a selected item
            ps->IsSelected = 1;
            //clear the checkbox list
            ui->li_Features->clear();
            //repopulate the checkbox list
            if( ps->Features != NULL )
            {
                for(QList<FeatureStore*>::Iterator itr2 = ps->Features->begin();itr2!=ps->Features->end();itr2++)
                {
                    FeatureStore *fs = *itr2;
                    QListWidgetItemFeature *item = new QListWidgetItemFeature(fs);
                    item->setData( Qt::DisplayRole, "text" );
                    if(fs->IsSelected != 0)
                        item->setData( Qt::CheckStateRole, Qt::Checked ); //checked
                    else
                        item->setData( Qt::CheckStateRole, Qt::Unchecked ); //checked
                    item->setText(fs->Name);
                    ui->li_Features->addItem( dynamic_cast<QListWidgetItem*>(item) );
                }
            }
        }
        DropdownIndex++;

        if(HasContent==0)
            ui->CB_ProductIdPicker->addItem( QString(ps->Name) );
//        printf("%d-%s\n",ps->id, ps->Name);
    }
}

void MainWindow::OnProductDropdownSelected(int index)
{
    //refresh the feature list based on the newly selected product
//    disconnect(ui->CB_ProductIdPicker, SIGNAL(currentIndexChanged(int)), this, SLOT(OnProductDropdownSelected(int)));
    RefreshProductFeatureDropdownContent(index);
//    connect(ui->CB_ProductIdPicker, SIGNAL(currentIndexChanged(int)), this, SLOT(OnProductDropdownSelected(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
    if(Products!=NULL)
    {
        delete Products;
        Products = NULL;
    }
    //maybe some UI will want to save their state before they destruct
    SQLConnection::DisConnectSQLite();
}
