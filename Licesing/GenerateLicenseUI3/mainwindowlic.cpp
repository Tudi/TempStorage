#include <stdlib.h>
#include <stdio.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwindowlic.h"
#include "mainwindowSQL.h"
#include <QListWidgetItem>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>

#define LIBRARY_API __declspec(dllimport)
#include "../LicenseDLL/License_API.h"
#include "../LicenseDLL/SimpleList.h"
#include "../LicenseDLL/License.h"
#include "../LicenseDLL/src/ComputerFingerprint.h"
#include "../LicenseDLL/src/DataPacker.h"

#ifdef _DEBUG
    #pragma comment(lib, "../Debug/LicenseDLL.lib")
#else
    #pragma comment(lib, "../Release/LicenseDLL.lib")
#endif

#define LICENSE_FILE_CONSTANT_NAME "License.dat"
#define LICENSE_SEED_CONSTANT_NAME "LicenseSeed.dat"

int LicenseAPI::GetSeedInfo(char *Seed, int SeedLen, char **HostName, char **Role)
{
    //sanity initialization for early exits
    *HostName = NULL;
    *Role = NULL;

    ComputerFingerprint *cf = new ComputerFingerprint;

    if(cf == NULL)
        return 1;

    //load unencrypted seed content from buffer
    if( cf->LoadFingerprint(Seed,SeedLen) != 0 )
        return 1;

    //once the container is initialized, get the data of a specific field
    int Size;
    cf->DupField(HostName,&Size, DB_COMPUTER_NAME);
    cf->DupField(Role,&Size, DB_MACHINE_ROLE);

    return 0;
}

int LicenseAPI::GetSeedContentFromFile(const char *Path, char **Seed, int *Len)
{
    //sanity initialization for early exits
    *Seed = NULL;
    *Len = 0;

    //load and descrypt seed content
    ComputerFingerprint *cf = new ComputerFingerprint;
    if(cf!=NULL)
    {
        if( cf->LoadFingerprint(Path) == 0 )
            cf->DupContent(Seed,Len);
        delete cf;
    }
    return 0;
}

void LicenseAPI::FreeLicDup(void *p)
{
    FreeDup(p);
}


void MainWindow::OnGenerateLicensesClicked()
{
    ClientStore *SelectedClient = GetSelectedClient();
    if(SelectedClient==NULL || SelectedClient->Id<=0)
    {
        QMessageBox Msgbox;
        Msgbox.setText("Please select a client first");
        Msgbox.exec();
        return;
    }

    bool IsPermanentLicense = ui->checkb_PermanentLicense->isChecked();

    QDate StartDate = ui->dt_StartDate->date();
    QDate EndDate = ui->dt_EndDate->date();

    //is date format valid ?
    if(EndDate<=StartDate && IsPermanentLicense==false)
    {
        QMessageBox Msgbox;
        Msgbox.setText("License duration is too small");
        Msgbox.exec();
        return;
    }

    int GraceDuration = ui->t_GraceDuration->toPlainText().toInt();
    if(GraceDuration > 90)
    {
        QMessageBox Msgbox;
        Msgbox.setText("Max Grace duration is 90 days");
        Msgbox.exec();
        GraceDuration = 90; // this limitation comes from inside licenseDLL. Can be extended, but needs a rebuild
    }
    GraceDuration = GraceDuration * 24 * 60 * 60;

    //calculate start and duration
    QDateTime StartDateTime(StartDate);
    StartDateTime.setTimeSpec(Qt::UTC);
    QDateTime EndDateTime(EndDate);
    EndDateTime.setTimeSpec(Qt::UTC);

    unsigned int LicenseDuration = EndDateTime.toTime_t() - StartDateTime.toTime_t();
    if(IsPermanentLicense)
        LicenseDuration = ~0;
    time_t LicenseStart = StartDateTime.toTime_t();

    //make sure at least one host was selected to generate license for
    {
        int HostsSelectedCount = 0;
        int Count = ui->li_ClientPCs->count();
        for(int i=0;i<Count;i++)
        {
            QListWidgetItem *item = ui->li_ClientPCs->item(i);
            if(item && item->checkState() == Qt::Checked)
                HostsSelectedCount++;
        }
        if(HostsSelectedCount==0)
        {
            QMessageBox Msgbox;
            Msgbox.setText("Please select at least one host to generate license for");
            Msgbox.exec();
            return;
        }
    }

    //make sure at least one product - feature is selected for the license
    {
        int FeaturesSelectedCount = 0;
        for( QList<ProductStore*>::Iterator pitr = Products->begin();pitr!=Products->end();pitr++)
            for(QList<FeatureStore*>::Iterator fitr = (*pitr)->Features->begin();fitr!=(*pitr)->Features->end();fitr++)
                if((*fitr)->IsSelected)
                    FeaturesSelectedCount++;
        if(FeaturesSelectedCount==0)
        {
            QMessageBox Msgbox;
            Msgbox.setText("Please select at least one product-feature");
            Msgbox.exec();
            return;
        }
    }

    //assemble a license with the selected product-feature list
    int er, ers = 0;
    //create a license
    License *TestLicense = new License;
    er = TestLicense->SetDuration(LicenseStart, LicenseDuration, GraceDuration);
    if( er != 0 )
    {
        QMessageBox Msgbox;
        Msgbox.setText("There was an error setting license duration");
        Msgbox.exec();
    }
    ers += er;
    //add features to it
    for( QList<ProductStore*>::Iterator pitr = Products->begin();pitr!=Products->end();pitr++)
        for(QList<FeatureStore*>::Iterator fitr = (*pitr)->Features->begin();fitr!=(*pitr)->Features->end();fitr++)
            if((*fitr)->IsSelected)
            {
                TestLicense->AddProjectFeature((*fitr)->ProductId,(*fitr)->id,(*fitr)->ActivationKey);
                if( er != 0 )
                {
                    QMessageBox Msgbox;
                    Msgbox.setText("There was an error adding activation key to the license");
                    Msgbox.exec();
                }
            }

    //where to save the output
    QString FingerprintFilePath = ui->te_OutputPath->toPlainText();
    QString OutputPath(QDir(FingerprintFilePath).currentPath());
    OutputPath += "/";
    OutputPath += LICENSE_FILE_CONSTANT_NAME;
    std::string sOutputPath = OutputPath.toStdString();

    //encode the license for each host
    int HostCount = ui->li_ClientPCs->count();
    for(int i=0;i<HostCount;i++)
    {
        QListWidgetItemHost *item = dynamic_cast<QListWidgetItemHost*>(ui->li_ClientPCs->item(i));
        if(item && item->checkState() == Qt::Checked)
        {
            //generate a temporary seed file
            ComputerFingerprint CF;
            if (CF.LoadFingerprint(item->cs->Seed,item->cs->SeedSize) != 0)
            {
                fprintf(stderr, "client %d, size %d\n", item->cs->ClientId,item->cs->SeedSize);
                fprintf(stderr, "Could not initialize fingerprint store from buffer. Host %s. Seed size %d\n", item->text().toStdString().c_str(),item->cs->SeedSize);
                continue;
            }

            //load the encryption key from file
            char *EncryptKey;
            int KeyLen;
            if (CF.DupEncryptionKey(&EncryptKey, KeyLen) != 0)
            {
                fprintf(stderr, "Could not generate encryption key for Host %s\n", item->text().toStdString().c_str());
                continue;
            }

            //save the license to the file
            er = TestLicense->SaveToFile(sOutputPath.c_str(),EncryptKey, KeyLen, i!=0);
            if( er != 0 )
            {
                QMessageBox Msgbox;
                Msgbox.setText("There was an error saving the license");
                Msgbox.exec();
            }
            ers += er;

            FreeDup(EncryptKey);
        }
    }
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
