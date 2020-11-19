#include "dialogmemberarea.h"
#include "ui_dialogmemberarea.h"
#include "QVBoxLayout"
#include "QHBoxLayout"
#include "frameprojectdetailslistview.h"
#include "CurlInterface.h"
#include "QJsonParseError"
#include "SessionStore.h"

struct ProjectDetail
{
    QString Name;
    QString ImgLoc;
    int IsActive;
    int TimeWeek;
    int TimeMonth;
    int TimeTotal;
};

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
}

DialogMemberArea::DialogMemberArea(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMemberArea)
{
    ui->setupUi(this);

//    char POSTFIELD[8000];
//    sprintf_s(POSTFIELD, sizeof(POSTFIELD),"projects-manage/index",temail,tpassw);
    char UrlField[5000];
    sprintf_s(UrlField,sizeof(UrlField),"projects/?token=%s&id=305&uid=test&email=vitaliibondtest@gmail.com&password=vitaliibondtest",GetSession()->GetSessionToken().toStdString().c_str());

    //check server reply for this login
    MemoryStruct *resp = GetAPIReply(UrlField, UrlField, CF_IS_GET);
    qDebug() << "received response : " << QString(resp->memory);


    QVBoxLayout *layout = new QVBoxLayout();
    scrollAreaContent = new QWidget();
    scrollAreaContent->setLayout(layout);
    this->ui->scrollArea->setWidget( scrollAreaContent );

    this->ui->scrollArea->setWidgetResizable(true);
    this->ui->scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    this->ui->scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    std::list<ProjectDetail*> *Projects = GenDummyProjects();

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

        p1->setContentsMargins(0, -100, 0, -100);

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
