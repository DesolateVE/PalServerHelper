#pragma once

#include <QtWidgets/QWidget>
#include "ui_PalServerHelper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PalServerHelperClass; };
QT_END_NAMESPACE

class PalServerHelper : public QWidget
{
    Q_OBJECT

public:
    PalServerHelper(QWidget *parent = nullptr);
    ~PalServerHelper();

private:
    Ui::PalServerHelperClass *ui;
};
