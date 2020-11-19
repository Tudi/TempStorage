#include "frameprojectdetailslistview.h"
#include "ui_frameprojectdetailslistview.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

FrameProjectDetailsListView::FrameProjectDetailsListView(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FrameProjectDetailsListView)
{
    ui->setupUi(this);
    this->setMinimumWidth(this->geometry().width());
    this->setMinimumHeight(this->geometry().height());
    this->setMaximumWidth(this->geometry().width());
    this->setMaximumHeight(this->geometry().height());

    this->setStyleSheet("");
    this->setStyleSheet("background-color: rgb(255,255,255)");
}

FrameProjectDetailsListView::~FrameProjectDetailsListView()
{
    delete ui;
}

void FrameProjectDetailsListView::SetName(QString name)
{
    ui->l_ProjectName->setText(name);
}

void FrameProjectDetailsListView::SetIcon(QString name)
{
    if(name.length()==0)
    {
        ui->l_Icon->setText(ui->l_ProjectName->text());
        this->ui->l_Icon->setStyleSheet("QLabel { background-color : red; color : white; qproperty-alignment: AlignCenter;}");
        return;
    }
    QNetworkAccessManager *m_netwManager = new QNetworkAccessManager(this);
    connect(m_netwManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_netwManagerFinished(QNetworkReply*)));

    QUrl url(name);
    QNetworkRequest request(url);
    m_netwManager->get(request);
}

void FrameProjectDetailsListView::slot_netwManagerFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error in" << reply->url() << ":" << reply->errorString();
        return;
    }

    QByteArray jpegData = reply->readAll();
    QPixmap pixmap;
    pixmap.loadFromData(jpegData);
    ui->l_Icon->setPixmap(pixmap); // or whatever your labels name is
    ui->l_Icon->setScaledContents(true);
}

void FrameProjectDetailsListView::SetActive(bool pActive)
{
    if(pActive)
    {
        this->ui->l_Active->setText("Active");
        this->ui->l_Active->setStyleSheet("QLabel { background-color : white; color : lightgreen; }");
    }
    else
    {
        this->ui->l_Active->setText("Inactive");
        this->ui->l_Active->setStyleSheet("QLabel { background-color : white; color : lightgray; }");
    }

}

void FrameProjectDetailsListView::SetTimeWeek(int Seconds)
{
    int seconds = Seconds % 60;
    int minutes = Seconds / 60 % 60;
    int hours = Seconds / 60 / 60 % 60;
    char FormattedContent[50];
    sprintf_s(FormattedContent,sizeof(FormattedContent),"%02d:%02d:%02d",hours,minutes,seconds);
    this->ui->l_TimeWeek->setText(FormattedContent);
}

void FrameProjectDetailsListView::SetTimeMonth(int Seconds)
{
    int seconds = Seconds % 60;
    int minutes = Seconds / 60 % 60;
    int hours = Seconds / 60 / 60 % 60;
    char FormattedContent[50];
    sprintf_s(FormattedContent,sizeof(FormattedContent),"%02d:%02d:%02d",hours,minutes,seconds);
    this->ui->l_TimeMonth->setText(FormattedContent);
}

void FrameProjectDetailsListView::SetTimeTotal(int Seconds)
{
    int seconds = Seconds % 60;
    int minutes = Seconds / 60 % 60;
    int hours = Seconds / 60 / 60 % 60;
    char FormattedContent[50];
    sprintf_s(FormattedContent,sizeof(FormattedContent),"%02d:%02d:%02d",hours,minutes,seconds);
    this->ui->l_TimeTotal->setText(FormattedContent);

}
