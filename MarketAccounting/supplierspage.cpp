#include "supplierspage.h"
#include "styledlineedit.h"
#include "styledcombobox.h"
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
#include <QGroupBox>
#include <QFileDialog>
#include <QTextStream>

SuppliersPage::SuppliersPage(QWidget* parent) : QWidget(parent), m_currentId(-1) {
    setupUI();
    loadGroups();
    loadSuppliers();
}

SuppliersPage::~SuppliersPage() {}

void SuppliersPage::setupUI() {
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ========= LEFT SIDE: Form =========
    QGroupBox* formBox = new QGroupBox(tr("معلومات المورد"), this);
    formBox->setStyleSheet(
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
        "}"
    );
    QGridLayout* formLayout = new QGridLayout(formBox);
    formLayout->setSpacing(10);
    formLayout->setContentsMargins(20, 20, 20, 20);

    auto addLabel = [&](const QString& text, int row, int col) {
        QLabel* lbl = new QLabel(text);
        lbl->setStyleSheet("color: #1a3a5c; font-size: 13px; font-weight: 600;");
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        formLayout->addWidget(lbl, row, col);
        };

    // Row 0: Name | Client
    m_nameEdit = new StyledLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("Supplier Name"));
    addLabel(tr("الاسم :"), 0, 2);
    formLayout->addWidget(m_nameEdit, 0, 1);

    m_phoneEdit = new StyledLineEdit(this);
    m_phoneEdit->setPlaceholderText("76505103");
    addLabel(tr("العميل :"), 0, 0);
    formLayout->addWidget(m_phoneEdit, 0, 3);

    // Row 1: Financial Number
    m_addressEdit = new StyledLineEdit(this);
    m_addressEdit->setPlaceholderText(tr("Financial Number"));
    addLabel(tr("الرقم المالي :"), 1, 2);
    formLayout->addWidget(m_addressEdit, 1, 1, 1, 3);

    // Row 2: Address (wide)
    m_notesEdit = new StyledLineEdit(this);
    m_notesEdit->setPlaceholderText(tr("Address"));
    addLabel(tr("العنوان :"), 2, 2);
    formLayout->addWidget(m_notesEdit, 2, 1, 1, 3);

    // Row 3: Account Basis | VAT
    m_accountEdit = new StyledLineEdit(this);
    m_accountEdit->setPlaceholderText("4011");
    addLabel(tr("اساس رقم الحساب :"), 3, 2);
    formLayout->addWidget(m_accountEdit, 3, 1);

    m_vatEdit = new StyledLineEdit(this);
    m_vatEdit->setPlaceholderText("442100207");
    addLabel(tr("VAT رقم حساب :"), 3, 0);
    formLayout->addWidget(m_vatEdit, 3, 3);

    // Row 4: Account Number | Currency
    m_faxEdit = new StyledLineEdit(this);
    m_faxEdit->setPlaceholderText("40110146-02");
    addLabel(tr("رقم الحساب :"), 4, 2);
    formLayout->addWidget(m_faxEdit, 4, 1);

    m_currencyCombo = new StyledComboBox(this);
    m_currencyCombo->addItems({ "دولار أميركي", "يورو", "ليرة لبنانية", "درهم إماراتي" });
    addLabel(tr("عملة المورد :"), 4, 0);
    formLayout->addWidget(m_currencyCombo, 4, 3);

    // Row 5: Region | Debt Ceiling
    m_groupCombo = new StyledComboBox(this);
    addLabel(tr("المنطقة :"), 5, 2);
    formLayout->addWidget(m_groupCombo, 5, 1);

    m_creditLimitEdit = new StyledLineEdit(this);
    m_creditLimitEdit->setPlaceholderText("0.00");
    addLabel(tr("سقف الدين :"), 5, 0);
    formLayout->addWidget(m_creditLimitEdit, 5, 3);

    // Row 6: Category | Customer (visual only)
    StyledLineEdit* categoryEdit = new StyledLineEdit(this);
    categoryEdit->setPlaceholderText("1");
    addLabel(tr("الفئة :"), 6, 2);
    formLayout->addWidget(categoryEdit, 6, 1);

    StyledComboBox* customerCombo = new StyledComboBox(this);
    addLabel(tr("الزبون :"), 6, 0);
    formLayout->addWidget(customerCombo, 6, 3);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

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

    m_newBtn = new QPushButton(tr("جديد"));
    m_newBtn->setStyleSheet(btnStyle);

    m_saveBtn = new QPushButton(tr("حفظ"));
    m_saveBtn->setStyleSheet(btnStyle);

    m_clearBtn = new QPushButton(tr("الغاء"));
    m_clearBtn->setStyleSheet(btnStyle);

    m_deleteBtn = new QPushButton(tr("حذف"));
    m_deleteBtn->setStyleSheet(
        "QPushButton { "
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #F0F5FA, stop:1 #D6E6F0); "
        "  border: 1px solid #9DB9D2; "
        "  border-radius: 3px; "
        "  padding: 8px 24px; "
        "  color: #dc3545; "
        "  font-weight: 600; "
        "}"
        "QPushButton:hover { background-color: #C8DDE8; }"
    );

    btnLayout->addWidget(m_newBtn);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addStretch();

    formLayout->addLayout(btnLayout, 7, 0, 1, 4);

    mainLayout->addWidget(formBox, 1);

    // ========= RIGHT SIDE: Table + Print/Export =========
    QGroupBox* tableBox = new QGroupBox(tr("قائمة الموردين"), this);
    tableBox->setStyleSheet(formBox->styleSheet());
    QVBoxLayout* tableLayout = new QVBoxLayout(tableBox);

    // Search + Print/Export row
    QHBoxLayout* topBarLayout = new QHBoxLayout();

    StyledLineEdit* searchEdit = new StyledLineEdit(this);
    searchEdit->setPlaceholderText(tr("بحث..."));
    topBarLayout->addWidget(searchEdit, 1);

    QPushButton* printBtn = new QPushButton(tr("طباعة"));
    printBtn->setStyleSheet(btnStyle);

    QPushButton* exportBtn = new QPushButton(tr("تصدير CSV"));
    exportBtn->setStyleSheet(btnStyle);

    topBarLayout->addWidget(printBtn);
    topBarLayout->addWidget(exportBtn);

    tableLayout->addLayout(topBarLayout);

    m_tableView = new QTableView(this);
    m_tableModel = new QStandardItemModel(this);
    m_tableModel->setHorizontalHeaderLabels({
        tr("ID"), tr("Name"), tr("Phone"), tr("Account"),
        tr("Currency"), tr("Credit"), tr("Status")
        });

    m_tableView->setModel(m_tableModel);
    m_tableView->setStyleSheet(
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
        "QTableView::item:alternate { background-color: #F5F9FC; }"
    );
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableLayout->addWidget(m_tableView);
    mainLayout->addWidget(tableBox, 1);

    // Connections
    connect(m_saveBtn, &QPushButton::clicked, this, &SuppliersPage::onSaveClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &SuppliersPage::onClearClicked);
    connect(m_newBtn, &QPushButton::clicked, this, &SuppliersPage::onNewClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &SuppliersPage::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &SuppliersPage::onSearchTextChanged);
    connect(m_tableView, &QTableView::clicked, this, &SuppliersPage::onTableClicked);
    connect(printBtn, &QPushButton::clicked, this, &SuppliersPage::onPrintClicked);
    connect(exportBtn, &QPushButton::clicked, this, &SuppliersPage::onExportClicked);
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

void SuppliersPage::onPrintClicked() {
    QMessageBox::information(this, tr("Print"), tr("Print functionality requires Qt PrintSupport module.\nPlease add 'printsupport' to Qt Modules in project settings."));
}

void SuppliersPage::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export CSV"), "suppliers.csv", tr("CSV Files (*.csv)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file for writing"));
        return;
    }

    QTextStream stream(&file);

    // UTF-8 BOM for Excel (Qt 6 compatible)
    stream << QString("\uFEFF");

    stream << "\"" << tr("Name") << "\","
        << "\"" << tr("Phone") << "\","
        << "\"" << tr("Account") << "\","
        << "\"" << tr("Currency") << "\","
        << "\"" << tr("Credit") << "\","
        << "\"" << tr("Status") << "\"\n";

    for (int r = 0; r < m_tableModel->rowCount(); ++r) {
        for (int c = 1; c <= 6; ++c) {
            stream << "\"" << m_tableModel->item(r, c)->text() << "\"";
            if (c < 6) stream << ",";
        }
        stream << "\n";
    }

    file.close();
    QMessageBox::information(this, tr("Success"), tr("Exported successfully to ") + fileName);
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