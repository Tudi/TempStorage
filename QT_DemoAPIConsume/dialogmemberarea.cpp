#include "dialogmemberarea.h"
#include "ui_dialogmemberarea.h"
#include "QVBoxLayout"
#include "QHBoxLayout"
#include "frameprojectdetailslistview.h"
#include "CurlInterface.h"
#include "QJsonParseError"
#include "SessionStore.h"
#include "QJsonObject.h"
#include "qjsonarray.h"

struct ProjectDetail
{
    int Id;
    QString Name;
    QString ImgLoc;
    int IsActive;
    int TimeWeek;
    int TimeMonth;
    int TimeTotal;
    int Position;
};

//should do an API call and get the real project list for an active token
QString GetJsonProjectList()
{

    //    char POSTFIELD[8000];
    //    sprintf_s(POSTFIELD, sizeof(POSTFIELD),"projects-manage/index",temail,tpassw);
        char UrlField[5000];
        sprintf_s(UrlField,sizeof(UrlField),"projects/?token=%s&id=305&uid=test&email=vitaliibondtest@gmail.com&password=vitaliibondtest",GetSession()->GetSessionToken().toStdString().c_str());

        //check server reply for this login
        MemoryStruct *resp = GetAPIReply(UrlField, UrlField, CF_IS_GET);
        qDebug() << "received response : " << QString(resp->memory);

     return QString(R"({
    "projects": [
        {
         "id": 9,
         "name": "Dasd",
         "uid": "Dasd",
         "logo_url": "https://api.quwi.com/files/projects/584-thumb.png?v=15",
         "position": 1,
         "is_active": 1,
         "is_owner_watcher": 1,
         "spent_time_week": 0,
         "spent_time_month": 0,
         "spent_time_all": 0,
         "users": []
        },
      {
        "id": 9,
        "name": "Mitrich project",
        "uid": "mitrich-project",
        "logo_url": "http://api.quwi.loc/files/projects/9-thumb.png?v=1",
        "position": 1,
        "is_active": 1,
        "is_owner_watcher": 1,
        "spent_time_week": 0,
        "spent_time_month": 0,
        "spent_time_all": 0,
        "users": [
          {
            "id": 10,
            "id_company": 2,
            "role": "coder",
            "name": "Demian Venture",
            "avatar_url": null,
            "is_online": 0,
            "dta_activity": "2018-08-13 08:31:05",
            "is_timer_online": 0,
            "dta_timer_activity": null,
            "timezone": null,
            "timer_last": null
          },
          {
            "id": 96,
            "id_company": 2,
            "role": "coder",
            "name": "test",
            "avatar_url": null,
            "is_online": 0,
            "dta_activity": null,
            "is_timer_online": 0,
            "dta_timer_activity": null,
            "timezone": null,
            "timer_last": null
          }
        ]
      },
      {
        "id": 13,
        "name": "My test",
        "uid": "my-test",
        "logo_url": null,
        "position": 2,
        "is_active": 1,
        "is_owner_watcher": 1,
        "spent_time_week": 0,
        "spent_time_month": 0,
        "spent_time_all": 0,
        "users": [
          {
            "id": 10,
            "id_company": 2,
            "role": "coder",
            "name": "Demian Venture",
            "avatar_url": null,
            "is_online": 0,
            "dta_activity": "2018-08-13 08:31:05",
            "is_timer_online": 0,
            "dta_timer_activity": null,
            "timezone": null,
            "timer_last": null
          },
          {
            "id": 96,
            "id_company": 2,
            "role": "coder",
            "name": "test",
            "avatar_url": null,
            "is_online": 0,
            "dta_activity": null,
            "is_timer_online": 0,
            "dta_timer_activity": null,
            "timezone": null,
            "timer_last": null
          }
        ]
      },
      {
        "id": 135,
        "name": "Project X",
        "uid": "project-x",
        "logo_url": null,
        "position": 3,
        "is_active": 1,
        "is_owner_watcher": 1,
        "spent_time_week": 0,
        "spent_time_month": 0,
        "spent_time_all": 0,
        "users": []
      },
      {
        "id": 136,
        "name": "Project X",
        "uid": "project-x-2",
        "logo_url": null,
        "position": 4,
        "is_active": 1,
        "is_owner_watcher": 1,
        "spent_time_week": 0,
        "spent_time_month": 0,
        "spent_time_all": 0,
        "users": []
      },
      {
        "id": 137,
        "name": "Project y",
        "uid": "project-x-3",
        "logo_url": null,
        "position": 5,
        "is_active": 1,
        "is_owner_watcher": 1,
        "spent_time_week": 0,
        "spent_time_month": 0,
        "spent_time_all": 0,
        "users": []
      }
    ]
  })");
}

std::list<ProjectDetail*> *GetProjectList()
{
    std::list<ProjectDetail*> *ret = new std::list<ProjectDetail*>();
    ProjectDetail *p;
    QString jsonreply = GetJsonProjectList();

    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonreply.toLocal8Bit(),&jsonError);
    if (jsonError.error != QJsonParseError::NoError)
          qDebug() << jsonError.errorString();
    QJsonValue ProjectList = jsonResponse["projects"];
//    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray jsonArray = ProjectList.toArray();
    foreach (const QJsonValue & value, jsonArray)
    {
        QJsonObject obj = value.toObject();
        p = new ProjectDetail();
        p->Id = obj["id"].toInt();
        p->Name = obj["name"].toString();
        p->ImgLoc = obj["logo_url"].toString();
        p->IsActive = obj["is_active"].toInt();
        p->Position = obj["position"].toInt();
        p->TimeWeek = obj["spent_time_week"].toInt();
        p->TimeMonth = obj["spent_time_month"].toInt();
        p->TimeTotal = obj["spent_time_all"].toInt();
        ret->push_back(p);
    }
    return ret;
}

/*
std::list<ProjectDetail*> *GenDummyProjects()
{
    std::list<ProjectDetail*> *ret = new std::list<ProjectDetail*>();
    ProjectDetail *p;

    p = new ProjectDetail();
    p->Name = "Dasd";
    p->ImgLoc = "https://api.quwi.com/files/projects/584-thumb.png?v=15";
    p->IsActive = 1;
    p->TimeWeek = 11+60*22+60*60*33;
    p->TimeMonth = 11+60*12+60*60*13;
    p->TimeTotal = 11+60*21+60*60*31;
    ret->push_back(p);

    p = new ProjectDetail();
    p->Name = "My TestProject 3";
    p->ImgLoc = "https://api.quwi.com/files/projects/1097-thumb.png?v=2";
    p->IsActive = 0;
    p->TimeWeek = 0;
    p->TimeMonth = 0;
    p->TimeTotal = 0;
    ret->push_back(p);

    p = new ProjectDetail();
    p->Name = "MissingPic";
    p->ImgLoc = "";
    p->IsActive = 1;
    p->TimeWeek = 0;
    p->TimeMonth = 0;
    p->TimeTotal = 0;
    ret->push_back(p);

    return ret;
}*/

DialogMemberArea::DialogMemberArea(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMemberArea)
{
    ui->setupUi(this);

    QVBoxLayout *layout = new QVBoxLayout();
    scrollAreaContent = new QWidget();
    scrollAreaContent->setLayout(layout);
    this->ui->scrollArea->setWidget( scrollAreaContent );

    this->ui->scrollArea->setWidgetResizable(true);
    this->ui->scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    this->ui->scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    std::list<ProjectDetail*> *Projects = GetProjectList();

    for(auto itr = Projects->begin(); itr != Projects->end(); itr++)
    {
        ProjectDetail *p = (*itr);
        QWidget *p1 = new QWidget;
        QHBoxLayout *hl = new QHBoxLayout(p1);
        FrameProjectDetailsListView *fplw = new FrameProjectDetailsListView();
        fplw->SetName(p->Name);
        fplw->SetIcon(p->ImgLoc);
        fplw->SetActive(p->IsActive);
        fplw->SetTimeWeek(p->TimeWeek);
        fplw->SetTimeMonth(p->TimeMonth);
        fplw->SetTimeTotal(p->TimeTotal);

//        p1->setContentsMargins(0, -100, 0, -100);

        hl->addWidget(fplw, Qt::AlignCenter);
        layout->addWidget(p1, Qt::AlignCenter);

        //no longer need the temp store obtained
        delete p;
    }
    delete Projects;
}

DialogMemberArea::~DialogMemberArea()
{
    delete ui;
}

void DialogMemberArea::resizeEvent(QResizeEvent *event)
{
    QRect DialogSize = this->frameGeometry();
    QRect ScrollAreaSize = this->ui->scrollArea->frameGeometry();
    this->ui->scrollArea->resize(DialogSize.width(),DialogSize.height()-ScrollAreaSize.top()-39);
    //scrollAreaContent->resize(this->width(),scrollAreaContent->height());
}
