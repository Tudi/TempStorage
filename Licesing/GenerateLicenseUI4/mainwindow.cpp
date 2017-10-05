#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QListWidgetItem>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QStyledItemDelegate>
#include <QPainter>
#include "mainwindowSQL.h"
#include "mainwindowlic.h"
#include <QTreeWidgetItem>

#define TREE_ENTRY_VALUE_BG_COLOR           (qRgb(255,255,255))
#define TREE_ENTRY_CUSTOM_VALUE_BG_COLOR    (qRgb(0,255,0))

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //first connect. Maybe some UI will initialize from DB
    SQLConnection::ConnectSQLite();

    ui->setupUi(this);

    Customers = NULL;

    //browse will point to current directory bty default
    ui->te_OutputPath->setPlainText(QDir::currentPath());

    //populate product dropdown
    Products = SQLConnection::GetProductList();

    RefreshCustomerDropdownContent(0);

    //make features list 1 click selectable
    connect(ui->tv_products_features, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(OnProductFeatureClicked(QTreeWidgetItem*)));
    connect(ui->tv_products_features, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(OnProductFeatureValueEdit(QTreeWidgetItem*)));

    //populate Customer list
    connect(ui->cb_Customers, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCustomerDropdownSelected(int)));
    connect(ui->b_UpdateCustomer, SIGNAL(clicked()), this, SLOT(OnCustomerUpdateCliecked()));
    connect(ui->b_DeleteCustomer, SIGNAL(clicked()), this, SLOT(OnCustomerDeleteCliecked()));

    //popup browse for browse button
    connect(ui->b_BrowseCustomerSeed, SIGNAL(clicked()), this, SLOT(OnBrowseForSeedFiles()));

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
    int Count = ui->li_CustomerPCs->count();
    for(int i=0;i<Count;i++)
    {
        QListWidgetItem *item = ui->li_CustomerPCs->item(i);
        if(item)
            item->setCheckState(Qt::Checked);
    }
}

void MainWindow::RefreshCustomerHostList()
{
    //get rid of the old content
    ui->li_CustomerPCs->clear();

    CustomerStore *SelectedCustomer = GetSelectedCustomer();
    if(SelectedCustomer==NULL || SelectedCustomer->Id<=0)
    {
 /*       QMessageBox Msgbox;
        Msgbox.setText("Please select a Customer first");
        Msgbox.exec();*/
        return;
    }
    //the the last License ID we generated and automatically select hosts
    int LastInsertedLicenseId;
    SQLConnection::GetCustomerLastLicenseId(SelectedCustomer->Id,&LastInsertedLicenseId);

    QList<CustomerSeedStore*> *CustomerSeeds = SQLConnection::GetCustomerSeedList(SelectedCustomer->Id);
    for(QList<CustomerSeedStore*>::Iterator itr = CustomerSeeds->begin();itr!=CustomerSeeds->end();itr++)
    {       
        CustomerSeedStore *cs = *itr;

        int IsSelected;
        SQLConnection::GetCustomerLicenseComputerSelectStatus(SelectedCustomer->Id,LastInsertedLicenseId,cs->Id,&IsSelected);

        char *Host;
        char *Role;
        LicenseAPI::GetSeedInfo(cs->Seed,cs->SeedSize,&Host,&Role);
        if(Host != NULL )
        {
            QListWidgetItemHost *item = new QListWidgetItemHost(cs);

            item->setData( Qt::DisplayRole, "text" );

            if(IsSelected==1)
                item->setData( Qt::CheckStateRole, Qt::Checked ); //checked
            else
                item->setData( Qt::CheckStateRole, Qt::Unchecked ); //not checked

            item->setText( QString(Host) + "-" + QString(Role) );

            ui->li_CustomerPCs->addItem( item );

            LicenseAPI::FreeLicDup( Host );
            LicenseAPI::FreeLicDup( Role );
        }
    }
    CustomerSeeds->clear();
    delete CustomerSeeds;
}

void MainWindow::OnBrowseForSeedFiles()
{
    CustomerStore *SelectedCustomer = GetSelectedCustomer();
    if(SelectedCustomer==NULL || SelectedCustomer->Id<=0)
    {
        QMessageBox Msgbox;
        Msgbox.setText("Please select a Customer first");
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
                SQLConnection::InsertCustomerSeed(SelectedCustomer->Id,data,Len);
                LicenseAPI::FreeLicDup( data );
                data = NULL;
             }
        }
    }
    RefreshCustomerHostList();
}

CustomerStore *MainWindow::GetSelectedCustomer()
{
    for(QList<CustomerStore*>::Iterator itr = Customers->begin();itr!=Customers->end();itr++)
    {
        CustomerStore *ps = *itr;
        if( ps->IsSelected != 0)
            return ps;
    }
    return NULL;
}

void MainWindow::OnCustomerUpdateCliecked()
{
    CustomerStore *SelectedCustomer = GetSelectedCustomer();
    //bugs :(
    if(SelectedCustomer == NULL)
    {
        fprintf(stderr,"Could not edit a non selected Customer");
        return;
    }
    //fetch new data form text fields
    std::string Name = ui->te_CustomerName->toPlainText().toStdString();
    std::string Phone = ui->te_CustomerPhone->toPlainText().toStdString().c_str();
    std::string Email = ui->te_CustomerEmail->toPlainText().toStdString().c_str();

    SelectedCustomer->Update( Name.c_str(), Phone.c_str(), Email.c_str() );

    //refresh dropdown list content
    //TODO:auto select last selection
    ui->cb_Customers->clear();
}

void MainWindow::OnCustomerDeleteCliecked()
{
    CustomerStore *SelectedCustomer = GetSelectedCustomer();
    //bugs :(
    if(SelectedCustomer == NULL)
    {
        fprintf(stderr,"Could not edit a non selected Customer");
        return;
    }
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "", "Delete Customer?",QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;
    SelectedCustomer->DeleteFromDb();
    ui->cb_Customers->clear();
}

void MainWindow::RefreshCustomerDropdownContent(int SelectedProductIndex)
{
    //init the list if we have not done so
    if(Customers)
        delete Customers;
    Customers = SQLConnection::GetCustomerList();
    {
        CustomerStore *cs = new CustomerStore(0,"New","","");
        Customers->push_back(cs);
    }

    int ListHadItems = ui->cb_Customers->count();
    int DropdownIndex = 0;
    int FoundSelectedItem = 0;
    for(QList<CustomerStore*>::Iterator itr = Customers->begin();itr!=Customers->end();itr++)
    {
        CustomerStore *ps = *itr;
        if(SelectedProductIndex>=0)
            ps->IsSelected = 0;
        if(DropdownIndex == SelectedProductIndex)
        {
            ui->te_CustomerName->setText( QString(ps->Name));
            ui->te_CustomerPhone->setText( QString(ps->Phone));
            ui->te_CustomerEmail->setText( QString(ps->Email));
            ps->IsSelected = 1;
            FoundSelectedItem = 1;
        }
        if(ListHadItems==0)
            ui->cb_Customers->addItem( QString(ps->Name) );
        DropdownIndex++;
    }

    //chain refresh the hosts list
    if(FoundSelectedItem == 1)
    {
        RefreshProductFeatureDropdownContent();
        RefreshCustomerHostList();
    }
    RefreshCustomerLastLicenseOptions();
}

void MainWindow::OnCustomerDropdownSelected(int SelectedProductIndex)
{
    RefreshCustomerDropdownContent(SelectedProductIndex);
}

void MainWindow::OnProductFeatureClicked(QTreeWidgetItem *item)
{
    FeatureStore *fs = dynamic_cast<QWidgetItemFeature*>(item)->fs;

    //could be a product and not a feature
    if( fs == NULL )
        return;

    if(fs->ForcedAddToLicense!=0 && item->checkState(0) == Qt::Checked)
    {
        QMessageBox Msgbox;
        Msgbox.setText("You need to include this value in all licenses");
        Msgbox.exec();
    }
    else if( item->checkState(0) == Qt::Checked)
    {
        item->setCheckState(0,Qt::Unchecked);
        fs->IsSelected = 0;
    }
    else
    {
        item->setCheckState(0,Qt::Checked);
        fs->IsSelected = 1;
    }
}

class CMyDelegate : public QStyledItemDelegate
{
public:
    CMyDelegate(QObject* parent) : QStyledItemDelegate(parent) {}
    void paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
};

void CMyDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QStyleOptionViewItemV4 itemOption(option);
    initStyleOption(&itemOption, index);
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &itemOption, painter, nullptr);
}

void MainWindow::OnProductFeatureValueEdit(QTreeWidgetItem *item)
{
    FeatureStore *fs = dynamic_cast<QWidgetItemFeature*>(item)->fs;

    //could be a product and not a feature
    if( fs == NULL )
        return;

    if(fs->FixedValue!=0)
    {
        QMessageBox Msgbox;
        Msgbox.setText("You can not change this value");
        Msgbox.exec();
    }
    else
    {
        bool ok;
        QString text = QInputDialog::getText(this, tr("New value"), fs->Name, QLineEdit::Normal, fs->GetActivationKey(), &ok);
        if (ok && !text.isEmpty())
        {
            fs->SetCustomActivationKey(text.toStdString().c_str());
            if(fs->HasCustomValue())
            {

                item->setBackgroundColor(0, QColor(TREE_ENTRY_CUSTOM_VALUE_BG_COLOR));
            }
            else
            {
                item->setBackgroundColor(0, QColor(TREE_ENTRY_VALUE_BG_COLOR));
            }
            CMyDelegate* delegate = new CMyDelegate(item->treeWidget());
            item->treeWidget()->setItemDelegate(delegate);
        }
    }
}

void MainWindow::RefreshProductFeatureDropdownContent()
{
    if(Products==NULL)
        return;

    //try to restore last generated license details
    int SelectedCustomerId = -1;
    int LastInsertedLicenseId = -1;
    CustomerStore *SelectedCustomer = GetSelectedCustomer();
    if(SelectedCustomer != NULL)
    {
        SelectedCustomerId = SelectedCustomer->Id;
        SQLConnection::GetCustomerLastLicenseId(SelectedCustomer->Id,&LastInsertedLicenseId);
    }

    //clear old content
    ui->tv_products_features->clear();

    //populate the dropdown with new content
    for(QList<ProductStore*>::Iterator itr = Products->begin();itr!=Products->end();itr++)
    {
        ProductStore *ps = *itr;

        QTreeWidgetItem *ProductDropdown = new QWidgetItemFeature( NULL );
        ProductDropdown->setText( 0, ps->Name );
        ui->tv_products_features->addTopLevelItem(ProductDropdown);

        //repopulate the checkbox list
        if( ps->Features != NULL )
        {
            for(QList<FeatureStore*>::Iterator itr2 = ps->Features->begin();itr2!=ps->Features->end();itr2++)
            {
                FeatureStore *fs = *itr2;

                char *SavedValue;
                SQLConnection::GetCustomerLicenseProductFeatureStatus(SelectedCustomerId,LastInsertedLicenseId,fs->ProductId,fs->id,&SavedValue);
                if(SavedValue!=NULL)
                {
                    fs->IsSelected = 1;
                    fs->SetCustomActivationKey(SavedValue);
                    free(SavedValue);
                }
                else
                {
                    fs->IsSelected = 0;
                    fs->SetCustomActivationKey(NULL);
                }

                if( fs->HiddenInUI !=0 )
                    continue;

                QTreeWidgetItem *it2 = new QWidgetItemFeature(fs);
                it2->setText( 0 ,fs->Name);
                it2->setCheckState(0, Qt::Unchecked);

                if(fs->IsSelected != 0)
                    it2->setData( 0, Qt::CheckStateRole, Qt::Checked ); //checked
                else
                    it2->setData( 0, Qt::CheckStateRole, Qt::Unchecked ); //checked

                if(fs->HasCustomValue())
                    it2->setBackgroundColor(0, QColor(TREE_ENTRY_CUSTOM_VALUE_BG_COLOR));

                //at the end append to our tree
                ProductDropdown->addChild(it2);
            }
        }
//        printf("%d-%s\n",ps->id, ps->Name);
    }
}

void MainWindow::RefreshCustomerLastLicenseOptions()
{
    int SelectedCustomerId = -1;
    int LastInsertedLicenseId = -1;
    CustomerStore *SelectedCustomer = GetSelectedCustomer();
    if(SelectedCustomer != NULL)
    {
        SelectedCustomerId = SelectedCustomer->Id;
        SQLConnection::GetCustomerLastLicenseId(SelectedCustomer->Id,&LastInsertedLicenseId);
    }
    if(LastInsertedLicenseId!=-1)
    {
        int StartDate;
        int EndDate;
        int GraceDur;
        int Perpetual;
        int QueryRet = SQLConnection::GetCustomerLicenseOptions(SelectedCustomerId,LastInsertedLicenseId,&StartDate,&EndDate,&GraceDur,&Perpetual);
        QDateTime qdt;
        qdt.setTime_t(StartDate);
        ui->dt_StartDate->setDateTime(qdt);
        qdt.setTime_t(EndDate);
        ui->dt_EndDate->setDateTime(qdt);
        GraceDur = GraceDur / (24 * 60 * 60);
        ui->t_GraceDuration->setText(QString().number(GraceDur));
        if(Perpetual)
            ui->checkb_PermanentLicense->setChecked(true);
        else
            ui->checkb_PermanentLicense->setChecked(false);
        OnPermanentLicenseClicked();
    }
    else
    {

        //initialize date pickers to current time
        QDate TimeNow = QDate::currentDate();
        ui->dt_StartDate->setDate( TimeNow );

        //initialize date pickers to next month
        QDate NextMonth = QDate::currentDate();
        NextMonth = NextMonth.addMonths(3);
        ui->dt_EndDate->setDate( NextMonth );
    }
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
