#pragma once

#include <QWidget>
#include <qtimer>
#include "ui_PalServerBackUp.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PalServerBackUpClass; };
QT_END_NAMESPACE

class PalServerBackUp : public QWidget
{
    Q_OBJECT

public:
    PalServerBackUp(QWidget* parent = nullptr);
    ~PalServerBackUp();

public Q_SLOTS:
    void pushButton_saveImmediately_clicked();
    void pushButton_autoSave_clicked();
    void timer_timeout();

private:
    bool SaveBackUp();
    bool PalServerBackUp::copyRecursively(QString sourceFolder, QString destFolder, bool bCoverIfFileExists);

private:
    Ui::PalServerBackUpClass* ui;
    int m_saveCount;
    QTimer m_timer;
    QString m_rootPath;
    bool m_buttonState;
};
