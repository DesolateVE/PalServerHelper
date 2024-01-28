#pragma once

#include <QWidget>
#include <QTimer>
#include <windows.h>
#include <thread>
#include "ui_PalServerMonitor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PalServerMonitorClass; };
QT_END_NAMESPACE

class PalServerMonitor : public QWidget
{
    Q_OBJECT

public:
    PalServerMonitor(QWidget* parent = nullptr);
    ~PalServerMonitor();

private:
    HANDLE FindMonitorProcess();
    HANDLE CreateMonitorProcess();
    void ButtonStateChange(bool state);

public Q_SLOTS:
    void pushButton_moinitor_clicked();
    void timingTimer_timeout();
    void thresholdTimer_timeout();
    void process_crash_slot();

Q_SIGNALS:
    void process_crash_signal();

private:
    Ui::PalServerMonitorClass* ui;
    uint64_t m_currentTime;
    bool m_buttonState;
    QTimer m_timingTimer;
    QTimer m_thresholdTimer;
    HANDLE m_processHandle;
    uint64_t m_systemMemorySize;
    std::thread m_monitorThread;
    HANDLE m_event;
};
