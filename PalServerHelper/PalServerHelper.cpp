#include "PalServerHelper.h"
#include "PalServerSetting.h"
#include "PalServerBackUp.h"
#include "PalServerMonitor.h"

PalServerHelper::PalServerHelper(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PalServerHelperClass())
{
    ui->setupUi(this);

    this->setWindowTitle("帕鲁服务器小助手");
    ui->tabWidget->addTab(new PalServerSetting(), "服务器参数设置");
    ui->tabWidget->addTab(new PalServerBackUp(), "服务器备份和恢复");
    ui->tabWidget->addTab(new PalServerMonitor(), "服务器监控");
}

PalServerHelper::~PalServerHelper()
{
    delete ui;
}
