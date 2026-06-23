#include "supplierspage.h"
#include "styledlineedit.h"
#include "styledcombobox.h"
#include "cardwidget.h"
#include "supplier.h"
#include "dbmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QLineEdit>

SuppliersPage::SuppliersPage(QWidget* parent) : QWidget(parent), m_currentId(-1) {
    setupUI();
    loadGroups();
    loadSuppliers();
}

SuppliersPage::~SuppliersPage() {}

void SuppliersPage::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    setStyleSheet("background-color: #F8F9FA;");

    QLabel* title = new QLabel(tr("Supplier Management"));
    title->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: #1a1a2e; "
        "padding-bottom: 10px; border-bottom: 3px solid #16213e;"
    );
    mainLayout->addWidget(title);

    CardWidget* formCard = new CardWidget(this);
    QGridLayout* formLayout = new QGridLayout(formCard);
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(25, 25, 25, 25);

    auto addLabel = [&](const QString& text, int row, int col) {
        QLabel* lbl = new QLabel(text);
        lbl->setStyleSheet("font-weight: 600; color: #495057; font-size: 13px;");
        formLayout->addWidget(lbl, row, col, Qt::AlignRight | Qt::AlignVCenter);
        };

    // Row 0: Name | Address
    m_nameEdit = new StyledLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("Supplier Name"));
    addLabel(tr("Name:"), 0, 2);
    formLayout->addWidget(m_nameEdit, 0, 1);

    m_addressEdit = new StyledLineEdit(this);
    m_addressEdit->setPlaceholderText(tr("Address"));
    addLabel(tr("Address:"), 0, 0);
    formLayout->addWidget(m_addressEdit, 0, 3);

    // Row 1: Phone | Fax
    m_phoneEdit = new StyledLineEdit(this);
    m_phoneEdit->setPlaceholderText("76505103");
    addLabel(tr("Phone:"), 1, 2);
    formLayout->addWidget(m_phoneEdit, 1, 1);

    m_faxEdit = new StyledLineEdit(this);
    addLabel(tr("Fax:"), 1, 0);
    formLayout->addWidget(m_faxEdit, 1, 3);

    // Row 2: Account | VAT
    m_accountEdit = new StyledLineEdit(this);
    m_accountEdit->setPlaceholderText("40110146-02");
    addLabel(tr("Account #:"), 2, 2);
    formLayout->addWidget(m_accountEdit, 2, 1);

    m_vatEdit = new StyledLineEdit(this);
    m_vatEdit->setPlaceholderText("442100207");
    addLabel(tr("VAT #:"), 2, 0);
    formLayout->addWidget(m_vatEdit, 2, 3);

    // Row 3: Currency | Credit Limit
    m_currencyCombo = new StyledComboBox(this);
    m_currencyCombo->addItems({ "USD", "EUR", "SAR", "AED", "IQD", "EGP" });
    addLabel(tr("Currency:"), 3, 2);
    formLayout->addWidget(m_currencyCombo, 3, 1);

    m_creditLimitEdit = new StyledLineEdit(this);
    m_creditLimitEdit->setPlaceholderText("0.00");
    addLabel(tr("Credit Limit:"), 3, 0);
    formLayout->addWidget(m_creditLimitEdit, 3, 3);

    // Row 4: Group
    m_groupCombo = new StyledComboBox(this);
    addLabel(tr("Group:"), 4, 2);
    formLayout->addWidget(m_groupCombo, 4, 1);

    // Row 5: Notes
    m_notesEdit = new StyledLineEdit(this);
    m_notesEdit->setPlaceholderText(tr("Notes..."));
    addLabel(tr("Notes:"), 5, 2);
    formLayout->addWidget(m_notesEdit, 5, 1, 1, 3);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_newBtn = new QPushButton(tr("New"));
    m_newBtn->setStyleSheet("QPushButton { background-color: #6c757d; color: white; padding: 10px 24px; border-radius: 8px; font-weight: 600; border: none; }"
        "QPushButton:hover { background-color: #5a6268; }");

    m_saveBtn = new QPushButton(tr("Save"));
    m_saveBtn->setStyleSheet("QPushButton { background-color: #16213e; color: white; padding: 10px 24px; border-radius: 8px; font-weight: 600; border: none; }"
        "QPushButton:hover { background-color: #0f3460; }");

    m_clearBtn = new QPushButton(tr("Cancel"));
    m_clearBtn->setStyleSheet("QPushButton { background-color: #e9ecef; color: #495057; padding: 10px 24px; border-radius: 8px; font-weight: 600; border: 1px solid #dee2e6; }"
        "QPushButton:hover { background-color: #dee2e6; }");

    m_deleteBtn = new QPushButton(tr("Delete"));
    m_deleteBtn->setStyleSheet("QPushButton { background-color: #dc3545; color: white; padding: 10px 24px; border-radius: 8px; font-weight: 600; border: none; }"
        "QPushButton:hover { background-color: #c82333; }");

    btnLayout->addStretch();
    btnLayout->addWidget(m_newBtn);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addWidget(m_deleteBtn);
    formLayout->addLayout(btnLayout, 6, 0, 1, 4);

    mainLayout->addWidget(formCard);

    // Table Card
    CardWidget* tableCard = new CardWidget(this);
    QVBoxLayout* tableLayout = new QVBoxLayout(tableCard);

    QLabel* tableTitle = new QLabel(tr("Suppliers List"));
    tableTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #1a1a2e;");
    tableLayout->addWidget(tableTitle);

    StyledLineEdit* searchEdit = new StyledLineEdit(this);
    searchEdit->setPlaceholderText(tr("Search supplier..."));
    tableLayout->addWidget(searchEdit);

    m_tableView = new QTableView(this);
    m_tableModel = new QStandardItemModel(this);
    m_tableModel->setHorizontalHeaderLabels({
        tr("ID"), tr("Name"), tr("Phone"), tr("Account"),
        tr("Currency"), tr("Credit"), tr("Status")
        });

    m_tableView->setModel(m_tableModel);
    m_tableView->setStyleSheet(
        "QTableView { background-color: white; border: 1px solid #e9ecef; border-radius: 8px; gridline-color: #f1f3f5; selection-background-color: #16213e; selection-color: white; }"
        "QHeaderView::section { background-color: #f8f9fa; padding: 10px; border: none; border-bottom: 2px solid #16213e; font-weight: 600; color: #1a1a2e; }"
        "QTableView::item { padding: 8px; }"
    );
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableLayout->addWidget(m_tableView);
    mainLayout->addWidget(tableCard, 1);

    // Connections
    connect(m_saveBtn, &QPushButton::clicked, this, &SuppliersPage::onSaveClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &SuppliersPage::onClearClicked);
    connect(m_newBtn, &QPushButton::clicked, this, &SuppliersPage::onNewClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &SuppliersPage::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &SuppliersPage::onSearchTextChanged);
    connect(m_tableView, &QTableView::clicked, this, &SuppliersPage::onTableClicked);
}

void SuppliersPage::loadGroups() {
    m_groupCombo->clear();
    QSqlQuery query("SELECT id, name FROM supplier_groups ORDER BY name");
    while (query.next()) {
        m_groupCombo->addItem(query.value("name").toString(), query.value("id"));
    }
}

void SuppliersPage::loadSuppliers() {
    m_tableModel->removeRows(0, m_tableModel->rowCount());

    QSqlQuery query(
        "SELECT s.id, s.name, s.phone, s.account_number, "
        "s.currency, s.credit_limit, s.is_active "
        "FROM suppliers s ORDER BY s.name"
    );

    int row = 0;
    while (query.next()) {
        m_tableModel->insertRow(row);
        m_tableModel->setItem(row, 0, new QStandardItem(query.value("id").toString()));
        m_tableModel->setItem(row, 1, new QStandardItem(query.value("name").toString()));
        m_tableModel->setItem(row, 2, new QStandardItem(query.value("phone").toString()));
        m_tableModel->setItem(row, 3, new QStandardItem(query.value("account_number").toString()));
        m_tableModel->setItem(row, 4, new QStandardItem(query.value("currency").toString()));
        m_tableModel->setItem(row, 5, new QStandardItem(query.value("credit_limit").toString()));
        m_tableModel->setItem(row, 6, new QStandardItem(
            query.value("is_active").toBool() ? tr("Active") : tr("Inactive")
        ));
        row++;
    }

    m_tableView->hideColumn(0);
    m_tableView->resizeColumnsToContents();
}

void SuppliersPage::onSaveClicked() {
    if (!validateForm()) return;

    Supplier s = collectFormData();

    QSqlQuery query;
    if (m_currentId == -1) {
        query.prepare(
            "INSERT INTO suppliers (name, address, phone, fax, account_number, "
            "vat_number, currency, credit_limit, group_id, notes) "
            "VALUES (:name, :address, :phone, :fax, :account, :vat, "
            ":currency, :credit, :group, :notes)"
        );
    }
    else {
        query.prepare(
            "UPDATE suppliers SET name=:name, address=:address, phone=:phone, "
            "fax=:fax, account_number=:account, vat_number=:vat, "
            "currency=:currency, credit_limit=:credit, group_id=:group, "
            "notes=:notes WHERE id=:id"
        );
        query.bindValue(":id", m_currentId);
    }

    query.bindValue(":name", s.name);
    query.bindValue(":address", s.address);
    query.bindValue(":phone", s.phone);
    query.bindValue(":fax", s.fax);
    query.bindValue(":account", s.accountNumber);
    query.bindValue(":vat", s.vatNumber);
    query.bindValue(":currency", s.currency);
    query.bindValue(":credit", s.creditLimit);
    query.bindValue(":group", s.groupId);
    query.bindValue(":notes", s.notes);

    if (query.exec()) {
        QMessageBox::information(this, tr("Success"), tr("Supplier saved successfully!"));
        clearForm();
        loadSuppliers();
    }
    else {
        QMessageBox::critical(this, tr("Error"), query.lastError().text());
    }
}

void SuppliersPage::onClearClicked() {
    clearForm();
}

void SuppliersPage::onNewClicked() {
    clearForm();
    m_nameEdit->setFocus();
}

void SuppliersPage::onDeleteClicked() {
    if (m_currentId == -1) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a supplier to delete"));
        return;
    }

    auto reply = QMessageBox::question(this, tr("Confirm"),
        tr("Are you sure you want to delete this supplier?"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM suppliers WHERE id = :id");
        query.bindValue(":id", m_currentId);
        if (query.exec()) {
            QMessageBox::information(this, tr("Success"), tr("Deleted successfully"));
            clearForm();
            loadSuppliers();
        }
    }
}

void SuppliersPage::onSearchTextChanged(const QString& text) {
    for (int i = 0; i < m_tableModel->rowCount(); ++i) {
        bool match = false;
        for (int j = 1; j < m_tableModel->columnCount(); ++j) {
            if (m_tableModel->item(i, j)->text().contains(text, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }
        m_tableView->setRowHidden(i, !match);
    }
}

void SuppliersPage::onTableClicked(const QModelIndex& index) {
    int row = index.row();
    int id = m_tableModel->item(row, 0)->text().toInt();

    QSqlQuery query;
    query.prepare("SELECT * FROM suppliers WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        Supplier s;
        s.id = query.value("id").toInt();
        s.name = query.value("name").toString();
        s.address = query.value("address").toString();
        s.phone = query.value("phone").toString();
        s.fax = query.value("fax").toString();
        s.accountNumber = query.value("account_number").toString();
        s.vatNumber = query.value("vat_number").toString();
        s.currency = query.value("currency").toString();
        s.creditLimit = query.value("credit_limit").toDouble();
        s.groupId = query.value("group_id").toInt();
        s.notes = query.value("notes").toString();

        populateForm(s);
        m_currentId = id;
    }
}

void SuppliersPage::clearForm() {
    m_currentId = -1;
    m_nameEdit->clear();
    m_addressEdit->clear();
    m_phoneEdit->clear();
    m_faxEdit->clear();
    m_accountEdit->clear();
    m_vatEdit->clear();
    m_currencyCombo->setCurrentIndex(0);
    m_creditLimitEdit->clear();
    m_groupCombo->setCurrentIndex(0);
    m_notesEdit->clear();
}

void SuppliersPage::populateForm(const Supplier& s) {
    m_nameEdit->setText(s.name);
    m_addressEdit->setText(s.address);
    m_phoneEdit->setText(s.phone);
    m_faxEdit->setText(s.fax);
    m_accountEdit->setText(s.accountNumber);
    m_vatEdit->setText(s.vatNumber);
    m_currencyCombo->setCurrentText(s.currency);
    m_creditLimitEdit->setText(QString::number(s.creditLimit));

    int groupIndex = m_groupCombo->findData(s.groupId);
    if (groupIndex >= 0) m_groupCombo->setCurrentIndex(groupIndex);

    m_notesEdit->setText(s.notes);
}

Supplier SuppliersPage::collectFormData() const {
    Supplier s;
    s.id = m_currentId;
    s.name = m_nameEdit->text().trimmed();
    s.address = m_addressEdit->text().trimmed();
    s.phone = m_phoneEdit->text().trimmed();
    s.fax = m_faxEdit->text().trimmed();
    s.accountNumber = m_accountEdit->text().trimmed();
    s.vatNumber = m_vatEdit->text().trimmed();
    s.currency = m_currencyCombo->currentText();
    s.creditLimit = m_creditLimitEdit->text().toDouble();
    s.groupId = m_groupCombo->currentData().toInt();
    s.notes = m_notesEdit->text().trimmed();
    return s;
}

bool SuppliersPage::validateForm() const {
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<SuppliersPage*>(this),
            tr("Warning"), tr("Name is required!"));
        return false;
    }
    return true;
}