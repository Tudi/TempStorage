#include "dialoglogin.h"
#include "ui_dialoglogin.h"
#include "CurlInterface.h"
#include "QJsonParseError"
#include "SessionStore.h"
#include "dialogmemberarea.h"

DialogLogin::DialogLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLogin)
{
    ui->setupUi(this);
    ui->b_Login->setStyleSheet("");
    ui->b_Login->setStyleSheet("background-color: rgb(57,83,120);color: white;border-radius: 5px;");

    ui->le_Email->setStyleSheet("");
    ui->le_Email->setStyleSheet("border-radius: 2px;border: 1px solid lightgray");
    ui->le_Passw->setStyleSheet("");
    ui->le_Passw->setStyleSheet("border-radius: 2px;border: 1px solid lightgray");
#ifdef _DEBUG
    //this is just to speed up testing. Should be removed in release version
    ui->le_Email->setText(QString("vitaliibondtest@gmail.com"));
    ui->le_Passw->setText(QString("vitaliibondtest"));
#endif

}

DialogLogin::~DialogLogin()
{
    delete ui;
}

void DialogLogin::on_DialogLogin_destroyed()
{
    QApplication::exit();
}

void DialogLogin::on_b_Login_clicked()
{
    QString email=ui->le_Email->text();
    QString passw=ui->le_Passw->text();

    //nothing to do until data has been entered
    if(email.size() <=0 || passw.size() <= 0)
        return;

    //avoid hacks
    char *temail = EscapeString(email.toStdString().c_str());
    char *tpassw = EscapeString(passw.toStdString().c_str());

    //something went wrong
    if( temail == NULL || tpassw == NULL)
        return;

    //format the body for the login
    char POSTFIELD[8000];
    sprintf_s(POSTFIELD, sizeof(POSTFIELD),"email=%s&password=%s",temail,tpassw);

    //no longer need these
    free(temail);
    free(tpassw);
    temail = tpassw = NULL;

    //check server reply for this login
   MemoryStruct *resp = GetAPIReply("auth/login", POSTFIELD, CF_IS_LOGIN);

   //do not leak resources
   if( resp != NULL)
   {
       //qDebug() << "We got API reply : \n" << resp->memory;

       QJsonParseError jsonError;
       QJsonDocument flowerJson = QJsonDocument::fromJson(QByteArray(resp->memory),&jsonError);
       if (jsonError.error != QJsonParseError::NoError)
             qDebug() << jsonError.errorString();
       QString Token = flowerJson["token"].toString();
       //qDebug() << "user token : \n" << Token;
       GetSession()->SetSessionToken(Token);

        //cleanup unused resource
        if(resp->memory != NULL)
            free(resp->memory);
        free(resp);

        //we no longer need the login window
        this->close();

        //create a new main window
        DialogMemberArea *dma = new DialogMemberArea();
        dma->show();
   }
}
