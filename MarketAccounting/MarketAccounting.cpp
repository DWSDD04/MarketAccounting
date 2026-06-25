#include "MarketAccounting.h"
#include "ui_MarketAccounting.h"
#include "supplierspage.h"
#include "productspage.h"
#include "debtspage.h"
#include "dashboardpage.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

MarketAccounting::MarketAccounting(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MarketAccountingClass)
{
    ui->setupUi(this);

    setWindowTitle(tr("Market Accounting System"));
    resize(1400, 900);

    // Central widget with light blue Windows-style background
    QWidget* central = new QWidget(this);
    central->setStyleSheet("background-color: #E8F0F8;");
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // Top header bar
    QWidget* header = new QWidget(central);
    header->setFixedHeight(50);
    header->setStyleSheet(
        "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #D6E6F0, stop:1 #B8D4E8);"
        "border: 1px solid #9DB9D2;"
        "border-radius: 4px;"
    );
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(15, 0, 15, 0);
    QLabel* title = new QLabel("Market Accounting System");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #1a3a5c;");
    headerLayout->addWidget(title);
    mainLayout->addWidget(header);

    // Tab widget — matches the tabs in your images
    QTabWidget* tabWidget = new QTabWidget(central);
    tabWidget->setStyleSheet(
        "QTabWidget::pane { "
        "  border: 1px solid #9DB9D2; "
        "  background-color: #E8F0F8; "
        "  border-top: none; "
        "}"
        "QTabBar::tab { "
        "  background-color: #D6E6F0; "
        "  border: 1px solid #9DB9D2; "
        "  padding: 8px 24px; "
        "  margin-right: 3px; "
        "  border-top-left-radius: 4px; "
        "  border-top-right-radius: 4px; "
        "  color: #1a3a5c; "
        "}"
        "QTabBar::tab:selected { "
        "  background-color: #E8F0F8; "
        "  border-bottom: 1px solid #E8F0F8; "
        "  font-weight: bold; "
        "}"
        "QTabBar::tab:hover { background-color: #C8DDE8; }"
    );

    DashboardPage* dashPage = new DashboardPage();
    SuppliersPage* suppPage = new SuppliersPage();
    ProductsPage* prodPage = new ProductsPage();
    DebtsPage* debtPage = new DebtsPage();

    tabWidget->addTab(dashPage, tr("المعلومات الرئيسية"));
    tabWidget->addTab(suppPage, tr("الحسابات"));
    tabWidget->addTab(prodPage, tr("المنتوجين"));
    tabWidget->addTab(debtPage, tr("الرصيد"));

    mainLayout->addWidget(tabWidget, 1);
    setCentralWidget(central);
}

MarketAccounting::~MarketAccounting()
{
    delete ui;
}