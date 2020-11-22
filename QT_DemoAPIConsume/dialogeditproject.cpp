#include "dialogeditproject.h"
#include "ui_dialogeditproject.h"
#include "frameprojectdetailslistview.h"

DialogEditProject::DialogEditProject(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditProject)
{
    ui->setupUi(this);

    ui->b_ChangeName->setStyleSheet("");
    ui->b_ChangeName->setStyleSheet("background-color: rgb(57,83,120);color: white;border-radius: 5px;");

    ProjectId = -1;
    owner = NULL;
}

DialogEditProject::~DialogEditProject()
{
    delete ui;
}

void DialogEditProject::on_b_ChangeName_clicked()
{
    //should update project name server side also
    //not doing it since the whole token authorization is messed up
    //update parent data
    if(owner)
        owner->SetName(ui->le_ProjectName->text());
}

void DialogEditProject::on_cb_Active_stateChanged(int arg1)
{
    //should update project name server side also
    //not doing it since the whole token authorization is messed up
    //update parent data
    if(owner)
        owner->SetActive(arg1);
}

void DialogEditProject::SetActive(int NewState)
{
    if(NewState)
        ui->cb_Active->setCheckState(Qt::CheckState::Checked);
    else
        ui->cb_Active->setCheckState(Qt::CheckState::Unchecked);
}

void DialogEditProject::SetProjectName(QString pName)
{
    ui->le_ProjectName->setText(pName);
}

void DialogEditProject::SetProjectId(int pProjectId)
{
    ProjectId = pProjectId;
}

void DialogEditProject::SetOwner(FrameProjectDetailsListView *pOwner)
{
    owner = pOwner;
}
