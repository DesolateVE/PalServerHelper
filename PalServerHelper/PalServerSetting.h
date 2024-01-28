#pragma once

#include <QWidget>
#include <optional>
#include "ui_PalServerSetting.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PalServerSettingClass; };
QT_END_NAMESPACE

class PalServerSetting : public QWidget
{
    Q_OBJECT

private:
    enum eValueType
    {
        Integer,
        Double,
        Text,
        Bool,
        Difficulty,
        DeathPenalty
    };

public:
    PalServerSetting(QWidget* parent = nullptr);
    ~PalServerSetting();

private:
    std::optional<QStringList> ReadCurrentIniSetting();
    void RefreshTableData();

public Q_SLOTS:
    void pushButton_recoverDefault_clicked();
    void pushButton_update_clicked();

private:
    Ui::PalServerSettingClass* ui;
    QList<eValueType> m_typeList;
};
