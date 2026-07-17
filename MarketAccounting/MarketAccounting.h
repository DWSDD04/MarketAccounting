#ifndef MARKETACCOUNTING_H
#define MARKETACCOUNTING_H

#include <QMainWindow>
#include <QPushButton>
#include "user.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MarketAccountingClass; }
QT_END_NAMESPACE

class MarketAccounting : public QMainWindow
{
    Q_OBJECT

public:
    explicit MarketAccounting(const User& user, QWidget* parent = nullptr);
    ~MarketAccounting();

private slots:
    void onLogoutClicked();

private:
    void setupNavigation();
    void applyRoleRestrictions();

    Ui::MarketAccountingClass* ui;
    User m_user;
    QPushButton* m_logoutBtn = nullptr;
};

#endif