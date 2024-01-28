#include "PalServerMonitor.h"
#include <QProcess>
#include <QMessageBox>
#include <psapi.h>
#include <TlHelp32.h>

PalServerMonitor::PalServerMonitor(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PalServerMonitorClass())
    , m_currentTime(0)
    , m_buttonState(false)
    , m_processHandle(NULL)
{
    ui->setupUi(this);

    m_event = CreateEvent(NULL, FALSE, FALSE, NULL);

    // 获取系统内存大小
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    m_systemMemorySize = statex.ullTotalPhys;

    connect(ui->pushButton_moinitor, SIGNAL(clicked()), this, SLOT(pushButton_moinitor_clicked()));
    connect(&m_timingTimer, SIGNAL(timeout()), this, SLOT(timingTimer_timeout()));
    connect(&m_thresholdTimer, SIGNAL(timeout()), this, SLOT(thresholdTimer_timeout()));
    connect(this, SIGNAL(process_crash_signal()), this, SLOT(process_crash_slot()));
}

PalServerMonitor::~PalServerMonitor()
{
    // 发出信号，通知线程关闭
    SetEvent(m_event);
    m_monitorThread.join();
    CloseHandle(m_event);

    if (m_processHandle)
    {
        CloseHandle(m_processHandle);
    }

    delete ui;
}

HANDLE PalServerMonitor::FindMonitorProcess()
{
    // Windows Api 判断 PalServer-Win64-Test-Cmd 进程是否存在
    DWORD pid = 0;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    BOOL bMore = Process32First(hProcessSnap, &pe32);
    while (bMore)
    {
        if (wcscmp(pe32.szExeFile, TEXT("PalServer-Win64-Test-Cmd.exe")) == 0)
        {
            pid = pe32.th32ProcessID;
            break;
        }
        bMore = Process32Next(hProcessSnap, &pe32);
    }
    CloseHandle(hProcessSnap);

    if (!pid)
    {
        return CreateMonitorProcess();
    }
    else
    {
        return OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, pid);
    }
}

HANDLE PalServerMonitor::CreateMonitorProcess()
{
    // CreateProcess 创建进程
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    wchar_t processPath[] = TEXT("../Pal/Binaries/Win64/PalServer-Win64-Test-Cmd.exe");
    if (!CreateProcess(NULL, processPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        return NULL;
    }
    else
    {
        CloseHandle(pi.hThread);
        return pi.hProcess;
    }
}

void PalServerMonitor::ButtonStateChange(bool state)
{
    if (state)
    {
        ui->pushButton_moinitor->setText("停止监控");
        ui->radioButton_timing->setEnabled(false);
        ui->radioButton_threshold->setEnabled(false);
        ui->radioButton_crash->setEnabled(false);
        ui->spinBox_timing->setEnabled(false);
        ui->spinBox_threshold->setEnabled(false);
    }
    else
    {
        ui->pushButton_moinitor->setText("开始监控");
        ui->radioButton_timing->setEnabled(true);
        ui->radioButton_threshold->setEnabled(true);
        ui->radioButton_crash->setEnabled(true);
        ui->spinBox_timing->setEnabled(true);
        ui->spinBox_threshold->setEnabled(true);
    }
}

void PalServerMonitor::timingTimer_timeout()
{
    m_currentTime++;
    int hour = m_currentTime / 3600;
    int min = m_currentTime % 3600 / 60;
    int sec = m_currentTime % 60;
    ui->label_timing->setText(QString("%1 时 %2 分 %3 秒").arg(hour, 2, 10, QChar('0')).arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')));

    if (m_currentTime > ui->spinBox_timing->value() * 3600)
    {
        // 计时器归零并关闭进程
        m_currentTime = 0;
        TerminateProcess(m_processHandle, 0);
        WaitForSingleObject(m_processHandle, INFINITE);
        CloseHandle(m_processHandle);

        // 重新创建进程
        m_processHandle = CreateMonitorProcess();
        if (m_processHandle == NULL)
        {
            m_timingTimer.stop();
            m_processHandle = NULL;
            m_buttonState = false;
            ButtonStateChange(m_buttonState);
            QMessageBox::warning(this, "警告", "PalServer-Win64-Test-Cmd 启动失败！");
        }
    }
}

void PalServerMonitor::thresholdTimer_timeout()
{
    // 获取当前进程内存占用情况
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(m_processHandle, &pmc, sizeof(pmc));
    SIZE_T memorySize = pmc.WorkingSetSize;
    int rate = (double)memorySize / (double)m_systemMemorySize * 100;
    ui->label_threshold->setText(QString("%1 %").arg(rate, 2, 10, QChar('0')));

    // 判断内存占用是否超过阈值
    if (rate > ui->spinBox_threshold->value())
    {
        // 关闭进程
        TerminateProcess(m_processHandle, 0);
        WaitForSingleObject(m_processHandle, INFINITE);
        CloseHandle(m_processHandle);

        // 重新创建进程
        m_processHandle = CreateMonitorProcess();
        if (m_processHandle == NULL)
        {
            m_thresholdTimer.stop();
            m_processHandle = NULL;
            m_buttonState = false;
            ButtonStateChange(m_buttonState);
            QMessageBox::warning(this, "警告", "PalServer-Win64-Test-Cmd 启动失败！");
        }
    }
}

void PalServerMonitor::process_crash_slot()
{
    // 获取当前时间
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    QString time = QString("%1-%2-%3 %4:%5:%6").arg(sys.wYear).arg(sys.wMonth).arg(sys.wDay).arg(sys.wHour).arg(sys.wMinute).arg(sys.wSecond);
    ui->label_crash->setText(time);
}

void PalServerMonitor::pushButton_moinitor_clicked()
{
    m_buttonState = !m_buttonState;

    m_processHandle = FindMonitorProcess();
    if (m_processHandle == NULL)
    {
        m_buttonState = false;
        ButtonStateChange(m_buttonState);
        QMessageBox::warning(this, "警告", "PalServer-Win64-Test-Cmd 启动失败！");
        return;
    }

    if (ui->radioButton_timing->isChecked())
    {
        if (m_buttonState)
        {
            m_currentTime = 0;
            m_timingTimer.start(1000);
        }
        else
        {
            CloseHandle(m_processHandle);
            m_processHandle = NULL;
            m_timingTimer.stop();
        }
    }
    else if (ui->radioButton_threshold->isChecked())
    {
        if (m_buttonState)
        {
            m_thresholdTimer.start(1000);
        }
        else
        {
            CloseHandle(m_processHandle);
            m_processHandle = NULL;
            m_thresholdTimer.stop();
        }
    }
    else if (ui->radioButton_crash->isChecked())
    {
        if (m_buttonState)
        {
            m_monitorThread = std::thread(
                [this]()
                {
                    while (true)
                    {
                        // 等待 m_event 或 m_processHandle
                        HANDLE handles[2] = { m_event, m_processHandle };
                        DWORD result = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

                        if (result == WAIT_OBJECT_0)
                        {
                            break;
                        }
                        else if (result == WAIT_OBJECT_0 + 1)
                        {
                            CloseHandle(m_processHandle);
                            emit process_crash_signal();
                            m_processHandle = CreateMonitorProcess();
                        }
                    }
                });
        }
        else
        {
            SetEvent(m_event);
            m_monitorThread.join();
            CloseHandle(m_processHandle);
            m_processHandle = NULL;
        }
    }

    ButtonStateChange(m_buttonState);
}