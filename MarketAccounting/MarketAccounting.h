#ifndef MARKETACCOUNTING_H
#define MARKETACCOUNTING_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MarketAccountingClass; }
QT_END_NAMESPACE

class MarketAccounting : public QMainWindow
{
    Q_OBJECT

public:
    MarketAccounting(QWidget* parent = nullptr);
    ~MarketAccounting();

private:
    Ui::MarketAccountingClass* ui;
};

#endif