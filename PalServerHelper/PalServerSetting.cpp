#include "PalServerSetting.h"
#include "misc.h"
#include <qfile>
#include <qmessagebox>
#include <qcombobox>
#include <qspinbox>
#include <qdoublespinbox>


PalServerSetting::PalServerSetting(QWidget* parent)
    : QWidget(parent), ui(new Ui::PalServerSettingClass())
{
    ui->setupUi(this);

    RefreshTableData();

    connect(ui->pushButton_recoverDefault, &QPushButton::clicked, this, &PalServerSetting::pushButton_recoverDefault_clicked);
    connect(ui->pushButton_update, &QPushButton::clicked, this, &PalServerSetting::pushButton_update_clicked);
}

PalServerSetting::~PalServerSetting()
{
    delete ui;
}

std::optional<QStringList> PalServerSetting::ReadCurrentIniSetting()
{
    // 打开当前目录下名为 DefaultPalWorldSettings.ini 的文件
    QFile file("..\\Pal\\Saved\\Config\\WindowsServer\\PalWorldSettings.ini");

    // 如果文件大小小于 200 字节，说明文件内容为空，删除文件
    if (file.size() < 200)
    {
        file.remove();
    }

    // 将 .\\DefaultPalWorldSettings.ini 内容拷贝过去
    if (!file.exists())
    {
        if (!QFile::copy("..\\DefaultPalWorldSettings.ini", "..\\Pal\\Saved\\Config\\WindowsServer\\PalWorldSettings.ini"))
        {
            return std::nullopt;
        }
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return std::nullopt;
    }

    // 截取文本 '(' ')' 之间的内容
    QString text = file.readAll();
    int start = text.indexOf('(');
    int end = text.indexOf(')');
    text = text.mid(start + 1, end - start - 1);

    // 用 ',' 切割文本
    QStringList list = text.split(',', Qt::SkipEmptyParts, Qt::CaseInsensitive);
    QStringList result;

    // 取得切割后的键值对
    for (auto& str : list)
    {
        int index = str.indexOf('=');
        QString value = str.mid(index + 1);
        result.append(value);
    }

    file.close();
    return result;
}

void PalServerSetting::RefreshTableData()
{
    auto valueList = ReadCurrentIniSetting();
    if (!valueList.has_value() || valueList->count() != TableInfo_List.count())
    {
        QMessageBox::critical(this, "Error", "配置文件读取失败，数值不匹配或文件被占用！");
        ::exit(-1);
        return;
    }

    auto tableWidget = ui->tableWidget_serverSetting;

    m_typeList.clear();
    tableWidget->clear();

    tableWidget->setColumnCount(3);
    tableWidget->setRowCount(valueList->count());
    tableWidget->setHorizontalHeaderLabels({ "参数","翻译","值" });
    tableWidget->setColumnWidth(0, 250);
    tableWidget->setColumnWidth(1, 300);

    for (size_t i = 0; i < valueList->count(); i++)
    {
        QTableWidgetItem* item;
        QComboBox* boxItem;
        QDoubleSpinBox* doubleItem;
        QSpinBox* spinItem;
        eValueType curType;

        switch (static_cast<eSettingType>(i))
        {
        case eSettingType::Difficulty:
            curType = eValueType::Difficulty;
            break;
        case eSettingType::DeathPenalty:
            curType = eValueType::DeathPenalty;
            break;
        case eSettingType::bEnableInvaderEnemy:
        case eSettingType::bEnableAimAssistPad:
        case eSettingType::bIsMultiplay:
        case eSettingType::bEnableNonLoginPenalty:
        case eSettingType::bEnableFastTravel:
        case eSettingType::bIsStartLocationSelectByMap:
        case eSettingType::bUseAuth:
        case eSettingType::bEnablePlayerToPlayerDamage:
        case eSettingType::bEnableFriendlyFire:
        case eSettingType::bActiveUNKO:
        case eSettingType::bEnableAimAssistKeyboard:
        case eSettingType::bAutoResetGuildNoOnlinePlayers:
        case eSettingType::bIsPvP:
        case eSettingType::bCanPickupOtherGuildDeathPenaltyDrop:
        case eSettingType::bExistPlayerAfterLogout:
        case eSettingType::bEnableDefenseOtherGuildPlayer:
        case eSettingType::RCONEnabled:
            curType = eValueType::Bool;
            break;
        case eSettingType::ServerName:
        case eSettingType::ServerDescription:
        case eSettingType::AdminPassword:
        case eSettingType::ServerPassword:
        case eSettingType::PublicIP:
        case eSettingType::Region:
        case eSettingType::BanListURL:
            curType = eValueType::Text;
            break;
        case eSettingType::DayTimeSpeedRate:
        case eSettingType::NightTimeSpeedRate:
        case eSettingType::ExpRate:
        case eSettingType::PalCaptureRate:
        case eSettingType::PalSpawnNumRate:
        case eSettingType::PalDamageRateAttack:
        case eSettingType::PalDamageRateDefense:
        case eSettingType::PlayerDamageRateAttack:
        case eSettingType::PlayerDamageRateDefense:
        case eSettingType::PlayerStomachDecreaceRate:
        case eSettingType::PlayerStaminaDecreaceRate:
        case eSettingType::PlayerAutoHPRegeneRate:
        case eSettingType::PlayerAutoHpRegeneRateInSleep:
        case eSettingType::PalStomachDecreaceRate:
        case eSettingType::PalStaminaDecreaceRate:
        case eSettingType::PalAutoHPRegeneRate:
        case eSettingType::PalAutoHpRegeneRateInSleep:
        case eSettingType::BuildObjectDamageRate:
        case eSettingType::BuildObjectDeteriorationDamageRate:
        case eSettingType::CollectionDropRate:
        case eSettingType::CollectionObjectHpRate:
        case eSettingType::CollectionObjectRespawnSpeedRate:
        case eSettingType::EnemyDropItemRate:
        case eSettingType::DropItemAliveMaxHours:
        case eSettingType::AutoResetGuildTimeNoOnlinePlayers:
        case eSettingType::PalEggDefaultHatchingTime:
        case eSettingType::WorkSpeedRate:
            curType = eValueType::Double;
            break;
        default:
            curType = eValueType::Integer;
            break;
        }

        item = new QTableWidgetItem(TableInfo_List[i].first);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        tableWidget->setItem(i, 0, item);
        item = new QTableWidgetItem(TableInfo_List[i].second);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        tableWidget->setItem(i, 1, item);

        switch (curType)
        {
        case PalServerSetting::Integer:
            spinItem = new QSpinBox();
            spinItem->setRange(0, 100000);
            spinItem->setSingleStep(1);
            spinItem->setValue(valueList->at(i).toInt());
            tableWidget->setCellWidget(i, 2, spinItem);
            break;
        case PalServerSetting::Double:
            doubleItem = new QDoubleSpinBox();
            doubleItem->setRange(0, 1000);
            doubleItem->setSingleStep(1);
            doubleItem->setDecimals(6);
            doubleItem->setValue(valueList->at(i).toDouble());
            tableWidget->setCellWidget(i, 2, doubleItem);
            break;
        case PalServerSetting::Text:
            item = new QTableWidgetItem(valueList->at(i).mid(1, valueList->at(i).length() - 2));
            tableWidget->setItem(i, 2, item);
            break;
        case PalServerSetting::Bool:
            boxItem = new QComboBox();
            boxItem->addItems({ "True", "False" });
            boxItem->setCurrentText(valueList->at(i));
            tableWidget->setCellWidget(i, 2, boxItem);
            break;
        case PalServerSetting::Difficulty:
            boxItem = new QComboBox();
            boxItem->addItems({ "None", "1","2","3" });
            boxItem->setCurrentText(valueList->at(i));
            tableWidget->setCellWidget(i, 2, boxItem);
            break;
        case PalServerSetting::DeathPenalty:
            boxItem = new QComboBox();
            boxItem->addItems({ "None", "Item", "ItemAndEquipment", "All" });
            boxItem->setCurrentText(valueList->at(i));
            tableWidget->setCellWidget(i, 2, boxItem);
            break;
        }

        m_typeList << curType;
    }
}

void PalServerSetting::pushButton_update_clicked()
{
    auto tableWidget = ui->tableWidget_serverSetting;
    QString newText =
        "[/Script/Pal.PalGameWorldSettings]\r\n"
        "OptionSettings=(";

    for (size_t i = 0; i < tableWidget->rowCount(); i++)
    {
        newText += tableWidget->item(i, 0)->text() + "=";
        switch (m_typeList[i])
        {
        case PalServerSetting::Integer:
            newText += static_cast<QSpinBox*>(tableWidget->cellWidget(i, 2))->cleanText();
            break;
        case PalServerSetting::Bool:
        case PalServerSetting::DeathPenalty:
        case PalServerSetting::Difficulty:
            newText += static_cast<QComboBox*>(tableWidget->cellWidget(i, 2))->currentText();
            break;
        case PalServerSetting::Double:
            newText += static_cast<QDoubleSpinBox*>(tableWidget->cellWidget(i, 2))->cleanText();
            break;
        case PalServerSetting::Text:
            newText += "\"" + tableWidget->item(i, 2)->text() + "\"";
            break;
        }

        if (i != tableWidget->rowCount() - 1)
        {
            newText += ",";
        }
        else
        {
            newText += ")\r\n";
        }
    }

    QFile file("..\\Pal\\Saved\\Config\\WindowsServer\\PalWorldSettings.ini");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Error", "配置文件读取失败，文件被占用！");
        return;
    }

    // newText 写入 file
    file.write(newText.toUtf8());
    file.close();

    QMessageBox::information(this, "Success", "配置文件更新成功！");
}

void PalServerSetting::pushButton_recoverDefault_clicked()
{
    QFile::remove("..\\Pal\\Saved\\Config\\WindowsServer\\PalWorldSettings.ini");
    if (QFile::copy("..\\DefaultPalWorldSettings.ini", "..\\Pal\\Saved\\Config\\WindowsServer\\PalWorldSettings.ini"))
    {
        RefreshTableData();
        QMessageBox::information(this, "Success", "配置文件恢复成功！");
    }
    else
    {
        QMessageBox::critical(this, "Error", "配置文件恢复失败！");
    }
}