#include "debtspage.h"
#include "styledlineedit.h"
#include "styledcombobox.h"
#include "debt.h"
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

DebtsPage::DebtsPage(QWidget* parent) : QWidget(parent), m_currentId(-1) {
    setupUI();
    loadParties();
    loadDebts();
}

DebtsPage::~DebtsPage() {}

void DebtsPage::setupUI() {
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ========= LEFT SIDE: Form =========
    QGroupBox* formBox = new QGroupBox(tr("معلومات الدين"), this);
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

    // Row 0: Party Type | Party
    m_partyTypeCombo = new StyledComboBox(this);
    m_partyTypeCombo->addItems({ tr("مورد"), tr("زبون") });
    addLabel(tr("نوع الطرف :"), 0, 2);
    formLayout->addWidget(m_partyTypeCombo, 0, 1);

    m_partyCombo = new StyledComboBox(this);
    addLabel(tr("الطرف :"), 0, 0);
    formLayout->addWidget(m_partyCombo, 0, 3);

    // Row 1: Amount | Debt Type
    m_amountEdit = new StyledLineEdit(this);
    m_amountEdit->setPlaceholderText("0.00");
    addLabel(tr("المبلغ :"), 1, 2);
    formLayout->addWidget(m_amountEdit, 1, 1);

    m_debtTypeCombo = new StyledComboBox(this);
    m_debtTypeCombo->addItems({ tr("علينا"), tr("لنا") });
    addLabel(tr("نوع الدين :"), 1, 0);
    formLayout->addWidget(m_debtTypeCombo, 1, 3);

    // Row 2: Due Date
    m_dueDateEdit = new StyledLineEdit(this);
    m_dueDateEdit->setPlaceholderText("2026-06-25");
    addLabel(tr("تاريخ الاستحقاق :"), 2, 2);
    formLayout->addWidget(m_dueDateEdit, 2, 1);

    // Row 3: Description (wide)
    m_descriptionEdit = new StyledLineEdit(this);
    m_descriptionEdit->setPlaceholderText(tr("Description..."));
    addLabel(tr("الوصف :"), 3, 2);
    formLayout->addWidget(m_descriptionEdit, 3, 1, 1, 3);

    // Row 4: Paid checkbox
    m_paidCheck = new QPushButton(tr("مدفوع"));
    m_paidCheck->setCheckable(true);
    m_paidCheck->setChecked(false);
    m_paidCheck->setStyleSheet(
        "QPushButton { "
        "  background-color: #FFFFFF; "
        "  border: 1px solid #9DB9D2; "
        "  border-radius: 3px; "
        "  padding: 6px 20px; "
        "  color: #1a3a5c; "
        "}"
        "QPushButton:checked { "
        "  background-color: #16213e; "
        "  color: white; "
        "  font-weight: bold; "
        "}"
    );
    addLabel(tr("الحالة :"), 4, 2);
    formLayout->addWidget(m_paidCheck, 4, 1);

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

    formLayout->addLayout(btnLayout, 5, 0, 1, 4);

    mainLayout->addWidget(formBox, 1);

    // ========= RIGHT SIDE: Table + Print/Export =========
    QGroupBox* tableBox = new QGroupBox(tr("قائمة الديون"), this);
    tableBox->setStyleSheet(formBox->styleSheet());
    QVBoxLayout* tableLayout = new QVBoxLayout(tableBox);

    // Search + Print/Export row
    QHBoxLayout* topBarLayout = new QHBoxLayout();

    StyledLineEdit* searchEdit = new StyledLineEdit(this);
    searchEdit->setPlaceholderText(tr("بحث..."));
    topBarLayout->addWidget(searchEdit, 1);

    m_printBtn = new QPushButton(tr("طباعة"));
    m_printBtn->setStyleSheet(btnStyle);
    topBarLayout->addWidget(m_printBtn);

    m_exportBtn = new QPushButton(tr("تصدير CSV"));
    m_exportBtn->setStyleSheet(btnStyle);
    topBarLayout->addWidget(m_exportBtn);

    tableLayout->addLayout(topBarLayout);

    m_tableView = new QTableView(this);
    m_tableModel = new QStandardItemModel(this);
    m_tableModel->setHorizontalHeaderLabels({
        tr("ID"), tr("Party"), tr("Type"), tr("Amount"),
        tr("Debt Type"), tr("Due Date"), tr("Status")
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
    connect(m_saveBtn, &QPushButton::clicked, this, &DebtsPage::onSaveClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &DebtsPage::onClearClicked);
    connect(m_newBtn, &QPushButton::clicked, this, &DebtsPage::onNewClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &DebtsPage::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &DebtsPage::onSearchTextChanged);
    connect(m_tableView, &QTableView::clicked, this, &DebtsPage::onTableClicked);
    connect(m_partyTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &DebtsPage::onPartyTypeChanged);
    connect(m_printBtn, &QPushButton::clicked, this, &DebtsPage::onPrintClicked);
    connect(m_exportBtn, &QPushButton::clicked, this, &DebtsPage::onExportClicked);
}

void DebtsPage::loadParties() {
    m_partyCombo->clear();
    QString type = m_partyTypeCombo->currentIndex() == 0 ? "supplier" : "customer";

    QSqlQuery query;
    if (type == "supplier") {
        query.exec("SELECT id, name FROM suppliers ORDER BY name");
    }
    else {
        query.exec("SELECT id, name FROM customers ORDER BY name");
    }

    while (query.next()) {
        m_partyCombo->addItem(query.value("name").toString(), query.value("id"));
    }
}

void DebtsPage::loadDebts() {
    m_tableModel->removeRows(0, m_tableModel->rowCount());

    QSqlQuery query(
        "SELECT d.id, d.party_type, d.amount, d.debt_type, "
        "d.due_date, d.description, d.is_paid, "
        "COALESCE(s.name, c.name) as party_name "
        "FROM debts d "
        "LEFT JOIN suppliers s ON d.party_type='supplier' AND d.party_id=s.id "
        "LEFT JOIN customers c ON d.party_type='customer' AND d.party_id=c.id "
        "ORDER BY d.due_date"
    );

    int row = 0;
    while (query.next()) {
        m_tableModel->insertRow(row);
        m_tableModel->setItem(row, 0, new QStandardItem(query.value("id").toString()));
        m_tableModel->setItem(row, 1, new QStandardItem(query.value("party_name").toString()));
        m_tableModel->setItem(row, 2, new QStandardItem(query.value("party_type").toString()));
        m_tableModel->setItem(row, 3, new QStandardItem(query.value("amount").toString()));
        m_tableModel->setItem(row, 4, new QStandardItem(query.value("debt_type").toString()));
        m_tableModel->setItem(row, 5, new QStandardItem(query.value("due_date").toString()));
        m_tableModel->setItem(row, 6, new QStandardItem(
            query.value("is_paid").toBool() ? tr("Paid") : tr("Unpaid")
        ));
        row++;
    }

    m_tableView->hideColumn(0);
    m_tableView->resizeColumnsToContents();
}

void DebtsPage::onSaveClicked() {
    if (!validateForm()) return;

    Debt d = collectFormData();

    QSqlQuery query;
    if (m_currentId == -1) {
        query.prepare(
            "INSERT INTO debts (party_type, party_id, amount, debt_type, "
            "due_date, description, is_paid, paid_at) "
            "VALUES (:party_type, :party_id, :amount, :debt_type, "
            ":due_date, :description, :is_paid, :paid_at)"
        );
    }
    else {
        query.prepare(
            "UPDATE debts SET party_type=:party_type, party_id=:party_id, "
            "amount=:amount, debt_type=:debt_type, due_date=:due_date, "
            "description=:description, is_paid=:is_paid, paid_at=:paid_at "
            "WHERE id=:id"
        );
        query.bindValue(":id", m_currentId);
    }

    query.bindValue(":party_type", d.partyType);
    query.bindValue(":party_id", d.partyId);
    query.bindValue(":amount", d.amount);
    query.bindValue(":debt_type", d.debtType);
    query.bindValue(":due_date", d.dueDate.toString("yyyy-MM-dd"));
    query.bindValue(":description", d.description);
    query.bindValue(":is_paid", d.isPaid);
    query.bindValue(":paid_at", d.isPaid ? QDate::currentDate().toString("yyyy-MM-dd") : QVariant());

    if (query.exec()) {
        QMessageBox::information(this, tr("Success"), tr("Debt saved successfully!"));
        clearForm();
        loadDebts();
    }
    else {
        QMessageBox::critical(this, tr("Error"), query.lastError().text());
    }
}

void DebtsPage::onClearClicked() {
    clearForm();
}

void DebtsPage::onNewClicked() {
    clearForm();
    m_amountEdit->setFocus();
}

void DebtsPage::onDeleteClicked() {
    if (m_currentId == -1) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a debt to delete"));
        return;
    }

    auto reply = QMessageBox::question(this, tr("Confirm"),
        tr("Are you sure you want to delete this debt?"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM debts WHERE id = :id");
        query.bindValue(":id", m_currentId);
        if (query.exec()) {
            QMessageBox::information(this, tr("Success"), tr("Deleted successfully"));
            clearForm();
            loadDebts();
        }
    }
}

void DebtsPage::onSearchTextChanged(const QString& text) {
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

void DebtsPage::onTableClicked(const QModelIndex& index) {
    int row = index.row();
    int id = m_tableModel->item(row, 0)->text().toInt();

    QSqlQuery query;
    query.prepare("SELECT * FROM debts WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        Debt d;
        d.id = query.value("id").toInt();
        d.partyType = query.value("party_type").toString();
        d.partyId = query.value("party_id").toInt();
        d.amount = query.value("amount").toDouble();
        d.debtType = query.value("debt_type").toString();
        d.dueDate = query.value("due_date").toDate();
        d.description = query.value("description").toString();
        d.isPaid = query.value("is_paid").toBool();

        populateForm(d);
        m_currentId = id;
    }
}

void DebtsPage::onPartyTypeChanged(int index) {
    Q_UNUSED(index)
        loadParties();
}

void DebtsPage::onPrintClicked() {
    QMessageBox::information(this, tr("Print"), tr("Print functionality requires Qt PrintSupport module.\nPlease add 'printsupport' to Qt Modules in project settings."));
}

void DebtsPage::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export CSV"), "debts.csv", tr("CSV Files (*.csv)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file for writing"));
        return;
    }

    QTextStream stream(&file);
    stream << QString("\uFEFF");

    stream << "\"" << tr("Party") << "\","
        << "\"" << tr("Type") << "\","
        << "\"" << tr("Amount") << "\","
        << "\"" << tr("Debt Type") << "\","
        << "\"" << tr("Due Date") << "\","
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

void DebtsPage::clearForm() {
    m_currentId = -1;
    m_partyTypeCombo->setCurrentIndex(0);
    m_partyCombo->setCurrentIndex(0);
    m_amountEdit->clear();
    m_debtTypeCombo->setCurrentIndex(0);
    m_dueDateEdit->clear();
    m_descriptionEdit->clear();
    m_paidCheck->setChecked(false);
}

void DebtsPage::populateForm(const Debt& d) {
    int typeIndex = (d.partyType == "customer") ? 1 : 0;
    m_partyTypeCombo->setCurrentIndex(typeIndex);
    loadParties();

    int partyIndex = m_partyCombo->findData(d.partyId);
    if (partyIndex >= 0) m_partyCombo->setCurrentIndex(partyIndex);

    m_amountEdit->setText(QString::number(d.amount));
    int debtIndex = (d.debtType == "owed") ? 1 : 0;
    m_debtTypeCombo->setCurrentIndex(debtIndex);
    m_dueDateEdit->setText(d.dueDate.toString("yyyy-MM-dd"));
    m_descriptionEdit->setText(d.description);
    m_paidCheck->setChecked(d.isPaid);
}

Debt DebtsPage::collectFormData() const {
    Debt d;
    d.id = m_currentId;
    d.partyType = m_partyTypeCombo->currentIndex() == 0 ? "supplier" : "customer";
    d.partyId = m_partyCombo->currentData().toInt();
    d.amount = m_amountEdit->text().toDouble();
    d.debtType = m_debtTypeCombo->currentIndex() == 0 ? "owe" : "owed";
    d.dueDate = QDate::fromString(m_dueDateEdit->text(), "yyyy-MM-dd");
    d.description = m_descriptionEdit->text().trimmed();
    d.isPaid = m_paidCheck->isChecked();
    return d;
}

bool DebtsPage::validateForm() const {
    if (m_partyCombo->currentIndex() < 0) {
        QMessageBox::warning(const_cast<DebtsPage*>(this),
            tr("Warning"), tr("Please select a party!"));
        return false;
    }
    if (m_amountEdit->text().toDouble() <= 0) {
        QMessageBox::warning(const_cast<DebtsPage*>(this),
            tr("Warning"), tr("Amount must be greater than zero!"));
        return false;
    }
    return true;
}