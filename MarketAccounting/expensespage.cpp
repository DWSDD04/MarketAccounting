#include "expensespage.h"
#include "styledlineedit.h"
#include "styledcombobox.h"
#include "dbmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QGroupBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

ExpensesPage::ExpensesPage(QWidget* parent) : QWidget(parent)
{
    setupUI();
    loadCategories();
    loadExpenses();
    clearForm();
}

ExpensesPage::~ExpensesPage() {}

void ExpensesPage::setupUI()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    QString groupBoxStyle =
        "QGroupBox { "
        "  background-color: #E8F0F8; "
        "  border: 1px solid #9DB9D2; "
        "  border-radius: 4px; "
        "  margin-top: 12px; "
        "  font-weight: bold; "
        "  color: #1a3a5c; "
        "}"
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  left: 10px; "
        "  padding: 0 5px; "
        "}";

    QString btnStyle =
        "QPushButton { "
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #F0F5FA, stop:1 #D6E6F0); "
        "  border: 1px solid #9DB9D2; "
        "  border-radius: 3px; "
        "  padding: 8px 24px; "
        "  color: #1a3a5c; "
        "  font-weight: 600; "
        "}"
        "QPushButton:hover { background-color: #C8DDE8; }"
        "QPushButton:pressed { background-color: #B8D4E8; }";

    QString deleteBtnStyle =
        "QPushButton { "
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #F0F5FA, stop:1 #D6E6F0); "
        "  border: 1px solid #9DB9D2; "
        "  border-radius: 3px; "
        "  padding: 8px 24px; "
        "  color: #dc3545; "
        "  font-weight: 600; "
        "}"
        "QPushButton:hover { background-color: #C8DDE8; }";

    QString tableStyle =
        "QTableView { "
        "  background-color: #FFFFFF; "
        "  border: 1px solid #9DB9D2; "
        "  gridline-color: #D6E6F0; "
        "  selection-background-color: #B8D4E8; "
        "  selection-color: #000000; "
        "}"
        "QHeaderView::section { "
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #F0F5FA, stop:1 #D6E6F0); "
        "  padding: 8px; "
        "  border: 1px solid #9DB9D2; "
        "  border-left: none; "
        "  font-weight: 600; "
        "  color: #1a3a5c; "
        "}"
        "QTableView::item { padding: 6px; }"
        "QTableView::item:alternate { background-color: #F5F9FC; }";

    auto addLabel = [&](QGridLayout* layout, const QString& text, int row, int col) {
        QLabel* lbl = new QLabel(text);
        lbl->setStyleSheet("color: #1a3a5c; font-size: 13px; font-weight: 600;");
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        layout->addWidget(lbl, row, col);
        };

    QGroupBox* formBox = new QGroupBox(tr("Expense Entry"), this);
    formBox->setStyleSheet(groupBoxStyle);
    QVBoxLayout* formLayout = new QVBoxLayout(formBox);
    formLayout->setSpacing(10);
    formLayout->setContentsMargins(20, 20, 20, 20);

    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(10);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(3, 1);

    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDate(QDate::currentDate());
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    m_dateEdit->setStyleSheet(
        "QDateEdit { background-color: #ffffff; border: 1px solid #9DB9D2; "
        "border-radius: 3px; padding: 6px; font-size: 13px; color: #1a3a5c; }"
    );
    addLabel(grid, tr("Date :"), 0, 2);
    grid->addWidget(m_dateEdit, 0, 1);

    m_categoryCombo = new StyledComboBox(this);
    addLabel(grid, tr("Category :"), 0, 0);
    grid->addWidget(m_categoryCombo, 0, 3);

    m_amountEdit = new StyledLineEdit(this);
    m_amountEdit->setPlaceholderText("0.00");
    addLabel(grid, tr("Amount :"), 1, 2);
    grid->addWidget(m_amountEdit, 1, 1);

    m_paymentCombo = new StyledComboBox(this);
    m_paymentCombo->addItems({ tr("Cash"), tr("Bank Transfer"), tr("Check"), tr("Credit Card") });
    addLabel(grid, tr("Payment :"), 1, 0);
    grid->addWidget(m_paymentCombo, 1, 3);

    m_accountCombo = new StyledComboBox(this);
    m_accountCombo->setPlaceholderText(tr("Select Account"));
    addLabel(grid, tr("Account :"), 2, 2);
    grid->addWidget(m_accountCombo, 2, 1, 1, 3);

    m_descriptionEdit = new StyledLineEdit(this);
    m_descriptionEdit->setPlaceholderText(tr("Description / Notes..."));
    addLabel(grid, tr("Description :"), 3, 2);
    grid->addWidget(m_descriptionEdit, 3, 1, 1, 3);

    m_receiptEdit = new StyledLineEdit(this);
    m_receiptEdit->setPlaceholderText(tr("Receipt / Reference number..."));
    addLabel(grid, tr("Receipt No :"), 4, 2);
    grid->addWidget(m_receiptEdit, 4, 1, 1, 3);

    formLayout->addLayout(grid);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_newBtn = new QPushButton(tr("New"));
    m_saveBtn = new QPushButton(tr("Save"));
    m_clearBtn = new QPushButton(tr("Clear"));
    m_deleteBtn = new QPushButton(tr("Delete"));
    m_newBtn->setStyleSheet(btnStyle);
    m_saveBtn->setStyleSheet(btnStyle);
    m_clearBtn->setStyleSheet(btnStyle);
    m_deleteBtn->setStyleSheet(deleteBtnStyle);
    btnLayout->addWidget(m_newBtn);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addStretch();
    formLayout->addLayout(btnLayout);
    formLayout->addStretch();

    mainLayout->addWidget(formBox, 1);

    QGroupBox* tableBox = new QGroupBox(tr("Expense History"), this);
    tableBox->setStyleSheet(groupBoxStyle);
    QVBoxLayout* tableLayout = new QVBoxLayout(tableBox);
    tableLayout->setContentsMargins(16, 16, 16, 16);

    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* catFilterLbl = new QLabel(tr("Category :"));
    catFilterLbl->setStyleSheet("color: #1a3a5c; font-weight: 600;");
    filterLayout->addWidget(catFilterLbl);

    m_filterCategoryCombo = new StyledComboBox(this);
    m_filterCategoryCombo->addItem(tr("All"), 0);
    filterLayout->addWidget(m_filterCategoryCombo);

    m_searchEdit = new StyledLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search..."));
    filterLayout->addWidget(m_searchEdit, 1);

    m_exportBtn = new QPushButton(tr("Export CSV"));
    m_exportBtn->setStyleSheet(btnStyle);
    filterLayout->addWidget(m_exportBtn);
    tableLayout->addLayout(filterLayout);

    QHBoxLayout* summaryLayout = new QHBoxLayout();
    m_totalLabel = new QLabel(tr("Total: $0.00"));
    m_totalLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: #1a3a5c; padding: 8px 16px; "
        "background-color: #FFFFFF; border: 1px solid #9DB9D2; border-radius: 4px;"
    );
    m_countLabel = new QLabel(tr("Count: 0"));
    m_countLabel->setStyleSheet(m_totalLabel->styleSheet());
    summaryLayout->addWidget(m_totalLabel);
    summaryLayout->addWidget(m_countLabel);
    summaryLayout->addStretch();
    tableLayout->addLayout(summaryLayout);

    m_tableView = new QTableView(this);
    m_tableModel = new QStandardItemModel(this);
    m_tableModel->setHorizontalHeaderLabels({
        tr("ID"), tr("Date"), tr("Category"), tr("Amount"), tr("Payment"), tr("Account"), tr("Description")
        });
    m_tableView->setModel(m_tableModel);
    m_tableView->setStyleSheet(tableStyle);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->hideColumn(0);
    tableLayout->addWidget(m_tableView, 1);

    mainLayout->addWidget(tableBox, 1);

    connect(m_newBtn, &QPushButton::clicked, this, &ExpensesPage::onClearClicked);
    connect(m_saveBtn, &QPushButton::clicked, this, &ExpensesPage::onSaveClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &ExpensesPage::onClearClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &ExpensesPage::onDeleteClicked);
    connect(m_tableView, &QTableView::clicked, this, &ExpensesPage::onTableClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ExpensesPage::onSearchTextChanged);
    connect(m_filterCategoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ExpensesPage::onCategoryFilterChanged);
    connect(m_exportBtn, &QPushButton::clicked, this, &ExpensesPage::onExportClicked);
}

void ExpensesPage::loadCategories()
{
    m_categoryCombo->clear();
    m_filterCategoryCombo->clear();
    m_filterCategoryCombo->addItem(tr("All"), 0);

    QStringList defaults = {
        tr("Rent"), tr("Utilities"), tr("Salaries"), tr("Maintenance"),
        tr("Transport"), tr("Marketing"), tr("Office Supplies"), tr("Other")
    };

    QSqlQuery query("SELECT id, name FROM expense_categories ORDER BY name");
    if (!query.exec()) {
        for (const QString& cat : defaults) {
            m_categoryCombo->addItem(cat);
            m_filterCategoryCombo->addItem(cat);
        }
        return;
    }

    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        m_categoryCombo->addItem(name, id);
        m_filterCategoryCombo->addItem(name, id);
    }

    if (m_categoryCombo->count() == 0) {
        for (const QString& cat : defaults) {
            m_categoryCombo->addItem(cat);
            m_filterCategoryCombo->addItem(cat);
        }
    }

    m_accountCombo->clear();
    m_accountCombo->addItem(tr("-- Select Account --"), 0);
    QSqlQuery accQuery("SELECT id, name FROM accounts WHERE is_active = 1 ORDER BY name");
    while (accQuery.next()) {
        m_accountCombo->addItem(accQuery.value("name").toString(), accQuery.value("id"));
    }
}

void ExpensesPage::loadExpenses()
{
    m_tableModel->removeRows(0, m_tableModel->rowCount());

    int catFilter = m_filterCategoryCombo->currentData().toInt();
    QString sql =
        "SELECT e.id, e.expense_date, c.name as category_name, e.amount, "
        "e.payment_method, a.name as account_name, e.description "
        "FROM expenses e "
        "LEFT JOIN expense_categories c ON e.category_id = c.id "
        "LEFT JOIN accounts a ON e.account_id = a.id "
        "WHERE 1=1 ";

    if (catFilter > 0) {
        sql += "AND e.category_id = :cat_id ";
    }
    sql += "ORDER BY e.expense_date DESC LIMIT 100";

    QSqlQuery query;
    query.prepare(sql);
    if (catFilter > 0) query.bindValue(":cat_id", catFilter);

    double grandTotal = 0.0;
    int row = 0;
    if (query.exec()) {
        while (query.next()) {
            m_tableModel->insertRow(row);
            m_tableModel->setItem(row, 0, new QStandardItem(query.value("id").toString()));
            m_tableModel->setItem(row, 1, new QStandardItem(query.value("expense_date").toDate().toString("yyyy-MM-dd")));
            m_tableModel->setItem(row, 2, new QStandardItem(query.value("category_name").toString()));
            m_tableModel->setItem(row, 3, new QStandardItem(QString::number(query.value("amount").toDouble(), 'f', 2)));
            m_tableModel->setItem(row, 4, new QStandardItem(query.value("payment_method").toString()));
            m_tableModel->setItem(row, 5, new QStandardItem(query.value("account_name").toString()));
            m_tableModel->setItem(row, 6, new QStandardItem(query.value("description").toString()));
            grandTotal += query.value("amount").toDouble();
            row++;
        }
    }

    m_tableView->resizeColumnsToContents();
    m_totalLabel->setText(tr("Total: $%1").arg(QString::number(grandTotal, 'f', 2)));
    m_countLabel->setText(tr("Count: %1").arg(row));
}

bool ExpensesPage::validateForm()
{
    if (m_amountEdit->text().toDouble() <= 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Amount must be greater than zero."));
        return false;
    }
    if (m_descriptionEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Description is required."));
        return false;
    }
    if (m_accountCombo->currentIndex() <= 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an account."));
        return false;
    }
    return true;
}

void ExpensesPage::onSaveClicked()
{
    if (!validateForm()) return;

    double amount = m_amountEdit->text().toDouble();
    int categoryId = m_categoryCombo->currentData().toInt();
    int accountId = m_accountCombo->currentData().toInt();
    QString payment = m_paymentCombo->currentText();
    QString desc = m_descriptionEdit->text().trimmed();
    QString receipt = m_receiptEdit->text().trimmed();
    QDate date = m_dateEdit->date();

    QSqlQuery query;
    if (m_currentId == -1) {
        query.prepare(
            "INSERT INTO expenses (expense_date, category_id, amount, payment_method, "
            "account_id, description, receipt_number, created_at) "
            "VALUES (:date, :cat, :amount, :payment, :acc, :desc, :receipt, NOW())"
        );
    }
    else {
        query.prepare(
            "UPDATE expenses SET expense_date=:date, category_id=:cat, amount=:amount, "
            "payment_method=:payment, account_id=:acc, description=:desc, "
            "receipt_number=:receipt WHERE id=:id"
        );
        query.bindValue(":id", m_currentId);
    }

    query.bindValue(":date", date.toString("yyyy-MM-dd"));
    query.bindValue(":cat", categoryId > 0 ? categoryId : QVariant());
    query.bindValue(":amount", amount);
    query.bindValue(":payment", payment);
    query.bindValue(":acc", accountId);
    query.bindValue(":desc", desc);
    query.bindValue(":receipt", receipt.isEmpty() ? QVariant() : receipt);

    if (query.exec()) {
        QMessageBox::information(this, tr("Success"), tr("Expense saved successfully!"));
        clearForm();
        loadExpenses();
    }
    else {
        QMessageBox::critical(this, tr("Error"), query.lastError().text());
    }
}

void ExpensesPage::onClearClicked()
{
    clearForm();
}

void ExpensesPage::onDeleteClicked()
{
    if (m_currentId == -1) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an expense to delete."));
        return;
    }

    auto reply = QMessageBox::question(this, tr("Confirm"),
        tr("Are you sure you want to delete this expense?"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM expenses WHERE id = :id");
        query.bindValue(":id", m_currentId);
        if (query.exec()) {
            QMessageBox::information(this, tr("Success"), tr("Deleted successfully."));
            clearForm();
            loadExpenses();
        }
        else {
            QMessageBox::critical(this, tr("Error"), query.lastError().text());
        }
    }
}

void ExpensesPage::onSearchTextChanged(const QString& text)
{
    QString trimmed = text.trimmed().toLower();
    for (int i = 0; i < m_tableModel->rowCount(); ++i) {
        bool match = false;
        for (int j = 1; j < m_tableModel->columnCount(); ++j) {
            if (m_tableModel->item(i, j)->text().toLower().contains(trimmed)) {
                match = true;
                break;
            }
        }
        m_tableView->setRowHidden(i, !match);
    }
}

void ExpensesPage::onTableClicked(const QModelIndex& index)
{
    int row = index.row();
    int id = m_tableModel->item(row, 0)->text().toInt();
    populateForm(id);
}

void ExpensesPage::onCategoryFilterChanged(int index)
{
    Q_UNUSED(index)
        loadExpenses();
}

void ExpensesPage::populateForm(int id)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM expenses WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next()) return;

    m_currentId = id;
    m_dateEdit->setDate(query.value("expense_date").toDate());

    int catId = query.value("category_id").toInt();
    int catIndex = m_categoryCombo->findData(catId);
    if (catIndex >= 0) m_categoryCombo->setCurrentIndex(catIndex);

    m_amountEdit->setText(QString::number(query.value("amount").toDouble(), 'f', 2));
    m_paymentCombo->setCurrentText(query.value("payment_method").toString());

    int accId = query.value("account_id").toInt();
    int accIndex = m_accountCombo->findData(accId);
    if (accIndex >= 0) m_accountCombo->setCurrentIndex(accIndex);

    m_descriptionEdit->setText(query.value("description").toString());
    m_receiptEdit->setText(query.value("receipt_number").toString());
}

void ExpensesPage::clearForm()
{
    m_currentId = -1;
    m_dateEdit->setDate(QDate::currentDate());
    m_categoryCombo->setCurrentIndex(0);
    m_amountEdit->clear();
    m_paymentCombo->setCurrentIndex(0);
    m_accountCombo->setCurrentIndex(0);
    m_descriptionEdit->clear();
    m_receiptEdit->clear();
}

void ExpensesPage::onExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export CSV"), "expenses.csv", tr("CSV Files (*.csv)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file for writing."));
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << QString("﻿");

    QStringList headers;
    headers << "Date" << "Category" << "Amount" << "Payment" << "Account" << "Description";
    stream << """ << headers.join("","") << ""
        ";

        for (int r = 0; r < m_tableModel->rowCount(); ++r) {
            QStringList row;
            for (int c = 1; c <= 6; ++c) {
                row << m_tableModel->item(r, c)->text();
            }
            stream << """ << row.join("","") << ""
                ";
        }

    file.close();
    QMessageBox::information(this, tr("Success"), tr("Exported successfully to ") + fileName);
}