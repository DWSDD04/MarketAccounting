#include "MarketAccounting.h"
#include "ui_MarketAccounting.h"

#include "supplierspage.h"
#include "productspage.h"
#include "salespage.h"
#include "dashboardpage.h"
#include "customerspage.h"
#include "reportspage.h"

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QCursor>

MarketAccounting::MarketAccounting(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MarketAccountingClass)
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
    QPushButton* suppBtn = createNavBtn(tr("Suppliers"));
    QPushButton* prodBtn = createNavBtn(tr("Products"));
    QPushButton* salesBtn = createNavBtn(tr("Sales & Accounting"));
    QPushButton* custBtn = createNavBtn(tr("Customers"));
    QPushButton* reportBtn = createNavBtn(tr("Reports"));

    dashBtn->setChecked(true);

    sidebarLayout->addWidget(dashBtn);
    sidebarLayout->addWidget(suppBtn);
    sidebarLayout->addWidget(prodBtn);
    sidebarLayout->addWidget(salesBtn);
    sidebarLayout->addWidget(custBtn);
    sidebarLayout->addWidget(reportBtn);
    sidebarLayout->addStretch();

    mainLayout->addWidget(sidebar);

    QStackedWidget* stack = new QStackedWidget(central);

    DashboardPage* dashPage = new DashboardPage();
    SuppliersPage* suppPage = new SuppliersPage();
    ProductsPage* prodPage = new ProductsPage();
    SalesPage* salesPage = new SalesPage();
    CustomersPage* custPage = new CustomersPage();
    ReportsPage* reportPage = new ReportsPage();

    stack->addWidget(dashPage);
    stack->addWidget(suppPage);
    stack->addWidget(prodPage);
    stack->addWidget(salesPage);
    stack->addWidget(custPage);
    stack->addWidget(reportPage);

    mainLayout->addWidget(stack, 1);
    setCentralWidget(central);

    connect(dashBtn, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(0);
        dashBtn->setChecked(true);
        suppBtn->setChecked(false);
        prodBtn->setChecked(false);
        salesBtn->setChecked(false);
        custBtn->setChecked(false);
        reportBtn->setChecked(false);
        });
    connect(suppBtn, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(1);
        suppBtn->setChecked(true);
        dashBtn->setChecked(false);
        prodBtn->setChecked(false);
        salesBtn->setChecked(false);
        custBtn->setChecked(false);
        reportBtn->setChecked(false);
        });
    connect(prodBtn, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(2);
        prodBtn->setChecked(true);
        dashBtn->setChecked(false);
        suppBtn->setChecked(false);
        salesBtn->setChecked(false);
        custBtn->setChecked(false);
        reportBtn->setChecked(false);
        });
    connect(salesBtn, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(3);
        salesBtn->setChecked(true);
        dashBtn->setChecked(false);
        suppBtn->setChecked(false);
        prodBtn->setChecked(false);
        custBtn->setChecked(false);
        reportBtn->setChecked(false);
        });
    connect(custBtn, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(4);
        custBtn->setChecked(true);
        dashBtn->setChecked(false);
        suppBtn->setChecked(false);
        prodBtn->setChecked(false);
        salesBtn->setChecked(false);
        reportBtn->setChecked(false);
        });
    connect(reportBtn, &QPushButton::clicked, [=]() {
        stack->setCurrentIndex(5);
        reportBtn->setChecked(true);
        dashBtn->setChecked(false);
        suppBtn->setChecked(false);
        prodBtn->setChecked(false);
        salesBtn->setChecked(false);
        custBtn->setChecked(false);
        });
}

MarketAccounting::~MarketAccounting()
{
    delete ui;
}