#include "reportspage.h"
#include "dbmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

ReportsPage::ReportsPage(QWidget* parent) : QWidget(parent)
{
    setupUI();
    loadCustomers();
}

ReportsPage::~ReportsPage() {}

void ReportsPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ─── Shared Styles ───
    QString btnStyle =
        "QPushButton { "
        " background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        " stop:0 #F0F5FA, stop:1 #D6E6F0); "
        " border: 1px solid #9DB9D2; "
        " border-radius: 3px; "
        " padding: 8px 24px; "
        " color: #1a3a5c; "
        " font-weight: 600; "
        "}"
        "QPushButton:hover { background-color: #C8DDE8; }"
        "QPushButton:pressed { background-color: #B8D4E8; }";

    QString tableStyle =
        "QTableView { "
        " background-color: #FFFFFF; "
        " border: 1px solid #9DB9D2; "
        " gridline-color: #D6E6F0; "
        " selection-background-color: #B8D4E8; "
        " selection-color: #000000; "
        "}"
        "QHeaderView::section { "
        " background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        " stop:0 #F0F5FA, stop:1 #D6E6F0); "
        " padding: 8px; "
        " border: 1px solid #9DB9D2; "
        " border-left: none; "
        " font-weight: 600; "
        " color: #1a3a5c; "
        "}"
        "QTableView::item { padding: 6px; }"
        "QTableView::item:alternate { background-color: #F5F9FC; }";

    // ─── Title ───
    QLabel* title = new QLabel(tr("Reports & Analytics"));
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #1a3a5c; margin-bottom: 8px;");
    mainLayout->addWidget(title);

    // ─── Filter Group ───
    QGroupBox* filterBox = new QGroupBox(tr("Report Filters"), this);
    filterBox->setStyleSheet(
        "QGroupBox { "
        " background-color: #E8F0F8; "
        " border: 1px solid #9DB9D2; "
        " border-radius: 4px; "
        " margin-top: 12px; "
        " font-weight: bold; "
        " color: #1a3a5c; "
        "}"
        "QGroupBox::title { "
        " subcontrol-origin: margin; "
        " left: 10px; "
        " padding: 0 5px; "
        "}"
    );
    QHBoxLayout* filterLayout = new QHBoxLayout(filterBox);
    filterLayout->setSpacing(12);
    filterLayout->setContentsMargins(15, 20, 15, 15);

    // Report type
    QLabel* typeLbl = new QLabel(tr("Report Type:"));
    typeLbl->setStyleSheet("color: #1a3a5c; font-weight: 600;");
    filterLayout->addWidget(typeLbl);

    m_reportTypeCombo = new QComboBox(this);
    m_reportTypeCombo->setStyleSheet(
        "QComboBox { background-color: white; border: 1px solid #9DB9D2; "
        "border-radius: 3px; padding: 6px; color: #1a3a5c; }"
        "QComboBox::drop-down { border: none; width: 24px; }"
    );
    m_reportTypeCombo->addItem(tr("Monthly Sales Summary"), 0);
    m_reportTypeCombo->addItem(tr("Customer Sales History"), 1);
    m_reportTypeCombo->addItem(tr("Top Selling Products"), 2);
    filterLayout->addWidget(m_reportTypeCombo);

    // Customer filter (hidden by default)
    m_customerLabel = new QLabel(tr("Customer:"));
    m_customerLabel->setStyleSheet("color: #1a3a5c; font-weight: 600;");
    m_customerLabel->setVisible(false);
    filterLayout->addWidget(m_customerLabel);

    m_customerCombo = new QComboBox(this);
    m_customerCombo->setStyleSheet(m_reportTypeCombo->styleSheet());
    m_customerCombo->setVisible(false);
    filterLayout->addWidget(m_customerCombo);

    // Date range
    m_fromLabel = new QLabel(tr("From:"));
    m_fromLabel->setStyleSheet("color: #1a3a5c; font-weight: 600;");
    filterLayout->addWidget(m_fromLabel);

    m_fromDate = new QDateEdit(this);
    m_fromDate->setCalendarPopup(true);
    m_fromDate->setDate(QDate::currentDate().addMonths(-1));
    m_fromDate->setStyleSheet(
        "QDateEdit { background-color: white; border: 1px solid #9DB9D2; "
        "border-radius: 3px; padding: 6px; color: #1a3a5c; }"
    );
    filterLayout->addWidget(m_fromDate);

    m_toLabel = new QLabel(tr("To:"));
    m_toLabel->setStyleSheet("color: #1a3a5c; font-weight: 600;");
    filterLayout->addWidget(m_toLabel);

    m_toDate = new QDateEdit(this);
    m_toDate->setCalendarPopup(true);
    m_toDate->setDate(QDate::currentDate());
    m_toDate->setStyleSheet(m_fromDate->styleSheet());
    filterLayout->addWidget(m_toDate);

    filterLayout->addStretch();

    m_generateBtn = new QPushButton(tr("Generate Report"));
    m_generateBtn->setStyleSheet(btnStyle);
    filterLayout->addWidget(m_generateBtn);

    m_exportBtn = new QPushButton(tr("Export CSV"));
    m_exportBtn->setStyleSheet(btnStyle);
    filterLayout->addWidget(m_exportBtn);

    mainLayout->addWidget(filterBox);

    // ─── Summary Cards ───
    QHBoxLayout* summaryLayout = new QHBoxLayout();
    summaryLayout->setSpacing(15);

    auto makeCard = [&](const QString& label) -> QLabel* {
        QLabel* card = new QLabel(label);
        card->setStyleSheet(
            "QLabel { "
            " background-color: #FFFFFF; "
            " border: 1px solid #9DB9D2; "
            " border-radius: 4px; "
            " padding: 12px 20px; "
            " color: #1a3a5c; "
            " font-size: 14px; "
            " font-weight: 600; "
            "}"
        );
        card->setMinimumWidth(180);
        card->setAlignment(Qt::AlignCenter);
        return card;
        };

    m_totalLabel = makeCard(tr("Total: 0.00"));
    m_countLabel = makeCard(tr("Count: 0"));
    m_avgLabel = makeCard(tr("Average: 0.00"));

    summaryLayout->addWidget(m_totalLabel);
    summaryLayout->addWidget(m_countLabel);
    summaryLayout->addWidget(m_avgLabel);
    summaryLayout->addStretch();
    mainLayout->addLayout(summaryLayout);

    // ─── Results Table ───
    QGroupBox* tableBox = new QGroupBox(tr("Report Results"), this);
    tableBox->setStyleSheet(filterBox->styleSheet());
    QVBoxLayout* tableLayout = new QVBoxLayout(tableBox);

    m_tableView = new QTableView(this);
    m_tableModel = new QStandardItemModel(this);
    m_tableView->setModel(m_tableModel);
    m_tableView->setStyleSheet(tableStyle);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableLayout->addWidget(m_tableView);
    mainLayout->addWidget(tableBox, 1);

    // ─── Connections ───
    connect(m_reportTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ReportsPage::onReportTypeChanged);
    connect(m_generateBtn, &QPushButton::clicked, this, &ReportsPage::onGenerateClicked);
    connect(m_exportBtn, &QPushButton::clicked, this, &ReportsPage::onExportClicked);

    // Generate initial report
    onGenerateClicked();
}

void ReportsPage::loadCustomers()
{
    m_customerCombo->clear();
    m_customerCombo->addItem(tr("All Customers"), 0);

    QSqlQuery query("SELECT id, name FROM customers WHERE is_active = 1 ORDER BY name");
    while (query.next()) {
        m_customerCombo->addItem(query.value("name").toString(), query.value("id"));
    }
}

void ReportsPage::onReportTypeChanged(int index)
{
    m_currentReportType = index;

    bool isCustomerReport = (index == 1);
    m_customerLabel->setVisible(isCustomerReport);
    m_customerCombo->setVisible(isCustomerReport);

    // For top products, date range is still useful
    bool showDates = true;
    m_fromLabel->setVisible(showDates);
    m_fromDate->setVisible(showDates);
    m_toLabel->setVisible(showDates);
    m_toDate->setVisible(showDates);

    onGenerateClicked();
}

void ReportsPage::onGenerateClicked()
{
    m_tableModel->removeRows(0, m_tableModel->rowCount());

    switch (m_currentReportType) {
    case 0: generateMonthlyReport(); break;
    case 1: generateCustomerReport(); break;
    case 2: generateTopProductsReport(); break;
    }
}

void ReportsPage::generateMonthlyReport()
{
    m_tableModel->setHorizontalHeaderLabels({
        tr("Period"), tr("Transactions"), tr("Gross Sales"), tr("Discounts"), tr("Net Sales")
        });

    QString from = m_fromDate->date().toString("yyyy-MM-dd");
    QString to = m_toDate->date().toString("yyyy-MM-dd");

    QSqlQuery query;
    query.prepare(
        "SELECT DATE_FORMAT(created_at, '%Y-%m') as period, "
        "COUNT(*) as txn_count, "
        "SUM(total_amount) as gross, "
        "SUM(discount) as discount_total, "
        "SUM(net_amount) as net "
        "FROM transactions "
        "WHERE type = 'sale' AND DATE(created_at) BETWEEN :from_date AND :to_date "
        "GROUP BY period "
        "ORDER BY period DESC"
    );
    query.bindValue(":from_date", from);
    query.bindValue(":to_date", to);

    if (!query.exec()) {
        QMessageBox::critical(this, tr("Error"), query.lastError().text());
        return;
    }

    double grandTotal = 0.0;
    int grandCount = 0;
    int row = 0;

    while (query.next()) {
        m_tableModel->insertRow(row);
        m_tableModel->setItem(row, 0, new QStandardItem(query.value("period").toString()));
        m_tableModel->setItem(row, 1, new QStandardItem(query.value("txn_count").toString()));
        m_tableModel->setItem(row, 2, new QStandardItem(QString::number(query.value("gross").toDouble(), 'f', 2)));
        m_tableModel->setItem(row, 3, new QStandardItem(QString::number(query.value("discount_total").toDouble(), 'f', 2)));
        m_tableModel->setItem(row, 4, new QStandardItem(QString::number(query.value("net").toDouble(), 'f', 2)));

        grandTotal += query.value("net").toDouble();
        grandCount += query.value("txn_count").toInt();
        row++;
    }

    m_tableView->resizeColumnsToContents();
    updateSummary(grandTotal, grandCount);
}

void ReportsPage::generateCustomerReport()
{
    m_tableModel->setHorizontalHeaderLabels({
        tr("ID"), tr("Reference"), tr("Customer"), tr("Total"), tr("Discount"),
        tr("Tax"), tr("Net"), tr("Payment"), tr("Date")
        });

    QString from = m_fromDate->date().toString("yyyy-MM-dd");
    QString to = m_toDate->date().toString("yyyy-MM-dd");
    int customerId = m_customerCombo->currentData().toInt();

    QSqlQuery query;
    if (customerId > 0) {
        query.prepare(
            "SELECT t.id, t.reference_number, c.name as customer_name, "
            "t.total_amount, t.discount, t.tax_amount, t.net_amount, "
            "t.payment_method, t.created_at "
            "FROM transactions t "
            "LEFT JOIN customers c ON t.party_id = c.id "
            "WHERE t.type = 'sale' AND t.party_id = :customer_id "
            "AND DATE(t.created_at) BETWEEN :from_date AND :to_date "
            "ORDER BY t.created_at DESC"
        );
        query.bindValue(":customer_id", customerId);
    }
    else {
        query.prepare(
            "SELECT t.id, t.reference_number, c.name as customer_name, "
            "t.total_amount, t.discount, t.tax_amount, t.net_amount, "
            "t.payment_method, t.created_at "
            "FROM transactions t "
            "LEFT JOIN customers c ON t.party_id = c.id "
            "WHERE t.type = 'sale' "
            "AND DATE(t.created_at) BETWEEN :from_date AND :to_date "
            "ORDER BY t.created_at DESC"
        );
    }
    query.bindValue(":from_date", from);
    query.bindValue(":to_date", to);

    if (!query.exec()) {
        QMessageBox::critical(this, tr("Error"), query.lastError().text());
        return;
    }

    double grandTotal = 0.0;
    int grandCount = 0;
    int row = 0;

    while (query.next()) {
        m_tableModel->insertRow(row);
        m_tableModel->setItem(row, 0, new QStandardItem(query.value("id").toString()));
        m_tableModel->setItem(row, 1, new QStandardItem(query.value("reference_number").toString()));
        m_tableModel->setItem(row, 2, new QStandardItem(query.value("customer_name").toString()));
        m_tableModel->setItem(row, 3, new QStandardItem(QString::number(query.value("total_amount").toDouble(), 'f', 2)));
        m_tableModel->setItem(row, 4, new QStandardItem(QString::number(query.value("discount").toDouble(), 'f', 2)));
        m_tableModel->setItem(row, 5, new QStandardItem(QString::number(query.value("tax_amount").toDouble(), 'f', 2)));
        m_tableModel->setItem(row, 6, new QStandardItem(QString::number(query.value("net_amount").toDouble(), 'f', 2)));
        m_tableModel->setItem(row, 7, new QStandardItem(query.value("payment_method").toString()));
        m_tableModel->setItem(row, 8, new QStandardItem(query.value("created_at").toDateTime().toString("yyyy-MM-dd hh:mm")));

        grandTotal += query.value("net_amount").toDouble();
        grandCount++;
        row++;
    }

    m_tableView->hideColumn(0);
    m_tableView->resizeColumnsToContents();
    updateSummary(grandTotal, grandCount);
}

void ReportsPage::generateTopProductsReport()
{
    // NOTE: This assumes a transaction_items table exists.
    // If your schema uses a different table name (e.g., sale_items),
    // adjust the query accordingly.
    m_tableModel->setHorizontalHeaderLabels({
        tr("Rank"), tr("Product Code"), tr("Product Name"), tr("Units Sold"),
        tr("Revenue"), tr("Current Stock"), tr("Status")
        });

    QString from = m_fromDate->date().toString("yyyy-MM-dd");
    QString to = m_toDate->date().toString("yyyy-MM-dd");

    QSqlQuery query;
    query.prepare(
        "SELECT p.id, p.code, p.name, p.quantity as current_stock, p.is_active, "
        "COALESCE(SUM(ti.quantity), 0) as units_sold, "
        "COALESCE(SUM(ti.quantity * ti.unit_price), 0) as revenue "
        "FROM products p "
        "LEFT JOIN transaction_items ti ON p.id = ti.product_id "
        "LEFT JOIN transactions t ON ti.transaction_id = t.id AND t.type = 'sale' "
        "AND DATE(t.created_at) BETWEEN :from_date AND :to_date "
        "GROUP BY p.id, p.code, p.name, p.quantity, p.is_active "
        "ORDER BY units_sold DESC, revenue DESC "
        "LIMIT 50"
    );
    query.bindValue(":from_date", from);
    query.bindValue(":to_date", to);

    if (!query.exec()) {
        // Fallback: if transaction_items doesn't exist, show product stock report
        m_tableModel->setHorizontalHeaderLabels({
            tr("ID"), tr("Code"), tr("Name"), tr("Category"), tr("Sale Price"),
            tr("Stock"), tr("Min Stock"), tr("Status")
            });

        QSqlQuery fallback;
        fallback.prepare(
            "SELECT p.id, p.code, p.name, c.name as category_name, "
            "p.sale_price, p.quantity, p.min_stock, p.is_active "
            "FROM products p "
            "LEFT JOIN categories c ON p.category_id = c.id "
            "ORDER BY p.quantity DESC "
            "LIMIT 50"
        );
        if (!fallback.exec()) {
            QMessageBox::critical(this, tr("Error"), fallback.lastError().text());
            return;
        }

        double grandTotal = 0.0;
        int row = 0;
        while (fallback.next()) {
            m_tableModel->insertRow(row);
            m_tableModel->setItem(row, 0, new QStandardItem(fallback.value("id").toString()));
            m_tableModel->setItem(row, 1, new QStandardItem(fallback.value("code").toString()));
            m_tableModel->setItem(row, 2, new QStandardItem(fallback.value("name").toString()));
            m_tableModel->setItem(row, 3, new QStandardItem(fallback.value("category_name").toString()));
            m_tableModel->setItem(row, 4, new QStandardItem(QString::number(fallback.value("sale_price").toDouble(), 'f', 2)));
            m_tableModel->setItem(row, 5, new QStandardItem(fallback.value("quantity").toString()));
            m_tableModel->setItem(row, 6, new QStandardItem(fallback.value("min_stock").toString()));
            m_tableModel->setItem(row, 7, new QStandardItem(
                fallback.value("is_active").toBool() ? tr("Active") : tr("Inactive")
            ));
            grandTotal += fallback.value("sale_price").toDouble() * fallback.value("quantity").toInt();
            row++;
        }

        m_tableView->hideColumn(0);
        m_tableView->resizeColumnsToContents();
        updateSummary(grandTotal, row);
        return;
    }

    double grandTotal = 0.0;
    int row = 0;
    while (query.next()) {
        m_tableModel->insertRow(row);
        m_tableModel->setItem(row, 0, new QStandardItem(QString::number(row + 1)));
        m_tableModel->setItem(row, 1, new QStandardItem(query.value("code").toString()));
        m_tableModel->setItem(row, 2, new QStandardItem(query.value("name").toString()));
        m_tableModel->setItem(row, 3, new QStandardItem(query.value("units_sold").toString()));
        m_tableModel->setItem(row, 4, new QStandardItem(QString::number(query.value("revenue").toDouble(), 'f', 2)));
        m_tableModel->setItem(row, 5, new QStandardItem(query.value("current_stock").toString()));
        m_tableModel->setItem(row, 6, new QStandardItem(
            query.value("is_active").toBool() ? tr("Active") : tr("Inactive")
        ));

        grandTotal += query.value("revenue").toDouble();
        row++;
    }

    m_tableView->resizeColumnsToContents();
    updateSummary(grandTotal, row);
}

void ReportsPage::updateSummary(double total, int count)
{
    m_totalLabel->setText(tr("Total: %1").arg(QString::number(total, 'f', 2)));
    m_countLabel->setText(tr("Count: %1").arg(count));
    double avg = count > 0 ? total / count : 0.0;
    m_avgLabel->setText(tr("Average: %1").arg(QString::number(avg, 'f', 2)));
}

void ReportsPage::onExportClicked()
{
    QString defaultName;
    switch (m_currentReportType) {
    case 0: defaultName = "monthly_sales.csv"; break;
    case 1: defaultName = "customer_sales.csv"; break;
    case 2: defaultName = "top_products.csv"; break;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export CSV"), defaultName, tr("CSV Files (*.csv)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file for writing"));
        return;
    }

    QTextStream stream(&file);
    stream << QString("\uFEFF");

    // Headers
    QStringList headers;
    for (int c = 0; c < m_tableModel->columnCount(); ++c) {
        headers << "\"" + m_tableModel->horizontalHeaderItem(c)->text() + "\"";
    }
    stream << headers.join(",") << "\n";

    // Data
    for (int r = 0; r < m_tableModel->rowCount(); ++r) {
        QStringList row;
        for (int c = 0; c < m_tableModel->columnCount(); ++c) {
            row << "\"" + m_tableModel->item(r, c)->text() + "\"";
        }
        stream << row.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, tr("Success"), tr("Exported successfully to ") + fileName);
}