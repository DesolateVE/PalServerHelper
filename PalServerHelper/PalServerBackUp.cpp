#include "PalServerBackUp.h"
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>

PalServerBackUp::PalServerBackUp(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PalServerBackUpClass())
    , m_buttonState(false)
{
    ui->setupUi(this);

    // 获取 SaveBackUp 目录下文明夹数量
    QDir dir("./SaveBackUp");
    if (!dir.exists())
    {
        dir.mkdir(dir.absolutePath());
        m_saveCount = 0;
    }
    else
    {
        dir.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        m_saveCount = dir.count();
        ui->listWidget->addItems(dir.entryList());
    }

    ui->label_saveCount->setText(QString::number(m_saveCount));
    ui->label_curState->setStyleSheet("color:red;");
    ui->label_curState->setText("OFF");

    // 获取 Pal\Saved\SaveGames\0 目录下文件夹名称
    QDir dir2("../Pal/Saved/SaveGames/0");
    dir2.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QString uuid = dir2.entryList().first();
    m_rootPath = "../Pal/Saved/SaveGames/0/" + uuid;

    connect(ui->pushButton_saveImmediately, &QPushButton::clicked, this, &PalServerBackUp::pushButton_saveImmediately_clicked);
    connect(ui->pushButton_autoSave, &QPushButton::clicked, this, &PalServerBackUp::pushButton_autoSave_clicked);
    connect(&m_timer, &QTimer::timeout, this, &PalServerBackUp::timer_timeout);
}

PalServerBackUp::~PalServerBackUp()
{
    delete ui;
}

void PalServerBackUp::pushButton_saveImmediately_clicked()
{
    if (SaveBackUp())
    {
        QMessageBox::information(this, "提示", "备份成功");
    }
    else
    {
        QMessageBox::information(this, "提示", "备份失败");
    }
}

void PalServerBackUp::pushButton_autoSave_clicked()
{
    m_buttonState = !m_buttonState;

    if (m_buttonState)
    {
        ui->pushButton_autoSave->setText("关闭自动备份");
        // 字体颜色修改为绿色
        ui->label_curState->setStyleSheet("color:green;");
        ui->label_curState->setText("ON");

        int min = ui->spinBox_interval->value();
        m_timer.start(1000 * 60 * min);

        ui->spinBox_count->setEnabled(false);
        ui->spinBox_interval->setEnabled(false);
        ui->pushButton_saveImmediately->setEnabled(false);
    }
    else
    {
        ui->pushButton_autoSave->setText("开启自动备份");
        // 字体颜色修改为红色
        ui->label_curState->setStyleSheet("color:red;");
        ui->label_curState->setText("OFF");

        m_timer.stop();

        ui->spinBox_count->setEnabled(true);
        ui->spinBox_interval->setEnabled(true);
        ui->pushButton_saveImmediately->setEnabled(true);
    }
}

void PalServerBackUp::timer_timeout()
{
    SaveBackUp();
    while (m_saveCount > ui->spinBox_count->value())
    {
        // 删除最早的备份
        QDir dir("./SaveBackUp");
        dir.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        dir.cd(dir.entryList().first());
        dir.removeRecursively();
        m_saveCount--;
        ui->label_saveCount->setText(QString::number(m_saveCount));
        delete ui->listWidget->takeItem(0);
    }
}

bool PalServerBackUp::SaveBackUp()
{
    // 递归拷贝 m_rootPath 目录下所有文件到 SaveBackUp 目录下以当前时间命名的文件夹
    auto time = QDateTime::currentDateTime().toString("yyyy.MM.dd-hh.mm.ss");
    auto path = "./SaveBackUp/" + time;

    if (copyRecursively(m_rootPath, path, true))
    {
        m_saveCount++;
        ui->label_saveCount->setText(QString::number(m_saveCount));
        ui->listWidget->addItem(time);
        return true;
    }
    else
    {
        return false;
    }
}

bool PalServerBackUp::copyRecursively(QString sourceFolder, QString destFolder, bool bCoverIfFileExists)
{
    QDir formDir_ = sourceFolder;
    QDir toDir_ = destFolder;

    if (!toDir_.exists())
    {
        if (!toDir_.mkdir(toDir_.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = formDir_.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList)
    {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        //拷贝子目录
        if (fileInfo.isDir())
        {
            //递归调用拷贝
            if (!copyRecursively(fileInfo.filePath(), toDir_.filePath(fileInfo.fileName()), true))
                return false;
        }
        //拷贝子文件
        else
        {
            if (bCoverIfFileExists && toDir_.exists(fileInfo.fileName()))
            {
                toDir_.remove(fileInfo.fileName());
            }
            if (!QFile::copy(fileInfo.filePath(), toDir_.filePath(fileInfo.fileName())))
            {
                return false;
            }
        }
    }
    return true;
}
