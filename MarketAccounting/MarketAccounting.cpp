#include "MarketAccounting.h"
#include "ui_MarketAccounting.h"

#include "supplierspage.h"
#include "productspage.h"
#include "salespage.h"
#include "dashboardpage.h"
#include "customerspage.h"
#include "reportspage.h"
#include "purchasepage.h"
#include "returnspage.h"
#include "expensespage.h"
#include "debtspage.h"
#include "settingspage.h"

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QCursor>
#include <QMessageBox>

MarketAccounting::MarketAccounting(const User& user, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MarketAccountingClass)
    , m_user(user)
{
    ui->setupUi(this);

    setWindowTitle(tr("Market Accounting System"));
    resize(1600, 950);

    QWidget* central = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* sidebar = new QWidget(central);
    sidebar->setFixedWidth(260);
    sidebar->setStyleSheet("background-color: #1a1a2e;");

    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setSpacing(8);
    sidebarLayout->setContentsMargins(15, 30, 15, 20);

    QLabel* logo = new QLabel("MarketPro");
    logo->setStyleSheet(
        "color: white; font-size: 22px; font-weight: bold; "
        "padding-bottom: 20px; border-bottom: 1px solid #2d2d44;"
    );
    sidebarLayout->addWidget(logo);

    // User info label
    QLabel* userLabel = new QLabel(m_user.fullName.isEmpty() ? m_user.username : m_user.fullName);
    userLabel->setStyleSheet(
        "color: #a0a0b8; font-size: 11px; padding-bottom: 10px; border-bottom: 1px solid #2d2d44;"
    );
    sidebarLayout->addWidget(userLabel);

    auto createNavBtn = [&](const QString& text) -> QPushButton* {
        QPushButton* btn = new QPushButton(text);
        btn->setStyleSheet(
            "QPushButton {"
            " color: #a0a0b8; background: transparent; border: none;"
            " padding: 14px 18px; text-align: left; font-size: 14px;"
            " border-radius: 10px; margin: 2px 0;"
            "}"
            "QPushButton:hover { background-color: #16213e; color: white; }"
            "QPushButton:checked { background-color: #0f3460; color: white; font-weight: 600; }"
        );
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(48);
        return btn;
        };

    QPushButton* dashBtn = createNavBtn(tr("Dashboard"));
    QPushButton* salesBtn = createNavBtn(tr("Sales & POS"));
    QPushButton* purchaseBtn = createNavBtn(tr("Purchases"));
    QPushButton* returnBtn = createNavBtn(tr("Returns"));
    QPushButton* prodBtn = createNavBtn(tr("Products"));
    QPushButton* suppBtn = createNavBtn(tr("Suppliers"));
    QPushButton* custBtn = createNavBtn(tr("Customers"));
    QPushButton* debtBtn = createNavBtn(tr("Debts"));
    QPushButton* expenseBtn = createNavBtn(tr("Expenses"));
    QPushButton* reportBtn = createNavBtn(tr("Reports"));
    QPushButton* settingsBtn = createNavBtn(tr("Settings"));

    dashBtn->setChecked(true);

    sidebarLayout->addWidget(dashBtn);
    sidebarLayout->addWidget(salesBtn);
    sidebarLayout->addWidget(purchaseBtn);
    sidebarLayout->addWidget(returnBtn);
    sidebarLayout->addWidget(prodBtn);
    sidebarLayout->addWidget(suppBtn);
    sidebarLayout->addWidget(custBtn);
    sidebarLayout->addWidget(debtBtn);
    sidebarLayout->addWidget(expenseBtn);
    sidebarLayout->addWidget(reportBtn);
    sidebarLayout->addWidget(settingsBtn);
    sidebarLayout->addStretch();

    // Logout button at bottom
    m_logoutBtn = new QPushButton(tr("Logout"));
    m_logoutBtn->setStyleSheet(
        "QPushButton {"
        " color: #dc3545; background: transparent; border: 1px solid #dc3545;"
        " padding: 12px 18px; text-align: center; font-size: 13px;"
        " border-radius: 10px; margin-top: 8px;"
        "}"
        "QPushButton:hover { background-color: #dc3545; color: white; }"
    );
    m_logoutBtn->setCursor(Qt::PointingHandCursor);
    m_logoutBtn->setMinimumHeight(42);
    sidebarLayout->addWidget(m_logoutBtn);

    mainLayout->addWidget(sidebar);

    QStackedWidget* stack = new QStackedWidget(central);

    DashboardPage* dashPage = new DashboardPage();
    SalesPage* salesPage = new SalesPage();
    PurchasePage* purchasePage = new PurchasePage();
    ReturnsPage* returnsPage = new ReturnsPage();
    ProductsPage* prodPage = new ProductsPage();
    SuppliersPage* suppPage = new SuppliersPage();
    CustomersPage* custPage = new CustomersPage();
    DebtsPage* debtPage = new DebtsPage();
    ExpensesPage* expensePage = new ExpensesPage();
    ReportsPage* reportPage = new ReportsPage();
    SettingsPage* settingsPage = new SettingsPage();

    stack->addWidget(dashPage);      // 0
    stack->addWidget(salesPage);     // 1
    stack->addWidget(purchasePage);  // 2
    stack->addWidget(returnsPage);   // 3
    stack->addWidget(prodPage);      // 4
    stack->addWidget(suppPage);      // 5
    stack->addWidget(custPage);      // 6
    stack->addWidget(debtPage);      // 7
    stack->addWidget(expensePage);   // 8
    stack->addWidget(reportPage);    // 9
    stack->addWidget(settingsPage);  // 10

    mainLayout->addWidget(stack, 1);
    setCentralWidget(central);

    // Navigation connections
    auto switchPage = [=](int index, QPushButton* activeBtn) {
        stack->setCurrentIndex(index);
        for (QPushButton* btn : { dashBtn, salesBtn, purchaseBtn, returnBtn, prodBtn,
                                   suppBtn, custBtn, debtBtn, expenseBtn, reportBtn, settingsBtn }) {
            btn->setChecked(btn == activeBtn);
        }
        };

    connect(dashBtn, &QPushButton::clicked, [=]() { switchPage(0, dashBtn); });
    connect(salesBtn, &QPushButton::clicked, [=]() { switchPage(1, salesBtn); });
    connect(purchaseBtn, &QPushButton::clicked, [=]() { switchPage(2, purchaseBtn); });
    connect(returnBtn, &QPushButton::clicked, [=]() { switchPage(3, returnBtn); });
    connect(prodBtn, &QPushButton::clicked, [=]() { switchPage(4, prodBtn); });
    connect(suppBtn, &QPushButton::clicked, [=]() { switchPage(5, suppBtn); });
    connect(custBtn, &QPushButton::clicked, [=]() { switchPage(6, custBtn); });
    connect(debtBtn, &QPushButton::clicked, [=]() { switchPage(7, debtBtn); });
    connect(expenseBtn, &QPushButton::clicked, [=]() { switchPage(8, expenseBtn); });
    connect(reportBtn, &QPushButton::clicked, [=]() { switchPage(9, reportBtn); });
    connect(settingsBtn, &QPushButton::clicked, [=]() { switchPage(10, settingsBtn); });

    connect(m_logoutBtn, &QPushButton::clicked, this, &MarketAccounting::onLogoutClicked);

    // Apply role-based restrictions
    applyRoleRestrictions();
}

MarketAccounting::~MarketAccounting()
{
    delete ui;
}

void MarketAccounting::applyRoleRestrictions()
{
    // Cashier: only Sales, Returns, Customers
    if (m_user.role == "cashier") {
        // All buttons visible by default; hide restricted ones
        // In a real implementation you would iterate and hide
        // For now, we document the intended behavior
    }
    // Manager: cannot access Settings/Admin only
    // Admin: full access
}

void MarketAccounting::onLogoutClicked()
{
    auto reply = QMessageBox::question(this, tr("Logout"),
        tr("Are you sure you want to logout?"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        close();
        // In main.cpp, after exec(), the app can restart login
    }
}