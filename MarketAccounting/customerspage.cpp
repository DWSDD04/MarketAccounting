#include "customerspage.h"
#include "dbmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextEdit>

CustomersPage::CustomersPage(QWidget* parent) : QWidget(parent)
{
    setupUI();
    refreshTable();
}

CustomersPage::~CustomersPage() {}

void CustomersPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

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

    QLabel* title = new QLabel(tr("Customer Management"));
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #1a3a5c; margin-bottom: 8px;");
    mainLayout->addWidget(title);

    QHBoxLayout* topLayout = new QHBoxLayout();

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search by name or phone..."));
    m_searchEdit->setStyleSheet(
        "QLineEdit { background-color: white; border: 1px solid #9DB9D2; "
        "border-radius: 3px; padding: 8px 12px; font-size: 13px; color: #1a3a5c; }"
        "QLineEdit:focus { border: 2px solid #1a3a5c; }"
    );
    m_searchEdit->setMinimumHeight(32);
    topLayout->addWidget(m_searchEdit, 1);

    m_addBtn = new QPushButton(tr("+ Add Customer"));
    m_addBtn->setStyleSheet(btnStyle);
    m_editBtn = new QPushButton(tr("Edit"));
    m_editBtn->setStyleSheet(btnStyle);
    m_editBtn->setEnabled(false);
    m_deleteBtn = new QPushButton(tr("Delete"));
    m_deleteBtn->setStyleSheet(deleteBtnStyle);
    m_deleteBtn->setEnabled(false);

    topLayout->addWidget(m_addBtn);
    topLayout->addWidget(m_editBtn);
    topLayout->addWidget(m_deleteBtn);
    mainLayout->addLayout(topLayout);

    m_table = new QTableView(this);
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels({
        tr("ID"), tr("Name"), tr("Phone"), tr("Email"), tr("Address"), tr("Balance"), tr("Status")
        });
    m_table->setModel(m_model);
    m_table->setStyleSheet(tableStyle);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->hideColumn(0);
    mainLayout->addWidget(m_table, 1);

    m_countLabel = new QLabel(tr("Total: 0"));
    m_countLabel->setStyleSheet("font-size: 13px; color: #495057; padding: 4px;");
    mainLayout->addWidget(m_countLabel);

    connect(m_addBtn, &QPushButton::clicked, this, &CustomersPage::onAddClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &CustomersPage::onEditClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &CustomersPage::onDeleteClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &CustomersPage::onSearchTextChanged);
    connect(m_table, &QTableView::clicked, this, &CustomersPage::onTableClicked);
}

void CustomersPage::refreshTable()
{
    m_model->removeRows(0, m_model->rowCount());

    QSqlQuery query("SELECT id, name, phone, email, address, balance, is_active FROM customers "
        "WHERE is_active = 1 ORDER BY name");
    int row = 0;
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        QString phone = query.value("phone").toString();
        QString email = query.value("email").toString();
        QString address = query.value("address").toString();
        double balance = query.value("balance").toDouble();
        bool active = query.value("is_active").toBool();

        m_model->insertRow(row);
        QStandardItem* idItem = new QStandardItem(QString::number(id));
        idItem->setData(id, Qt::UserRole);
        m_model->setItem(row, 0, idItem);
        m_model->setItem(row, 1, new QStandardItem(name));
        m_model->setItem(row, 2, new QStandardItem(phone));
        m_model->setItem(row, 3, new QStandardItem(email));
        m_model->setItem(row, 4, new QStandardItem(address));
        m_model->setItem(row, 5, new QStandardItem(QString::number(balance, 'f', 2)));
        m_model->setItem(row, 6, new QStandardItem(active ? tr("Active") : tr("Inactive")));
        row++;
    }

    m_table->resizeColumnsToContents();
    m_countLabel->setText(tr("Total: %1").arg(row));
    clearSelection();
}

void CustomersPage::clearSelection()
{
    m_selectedId = -1;
    m_selectedRow = -1;
    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
}

void CustomersPage::onTableClicked(const QModelIndex& index)
{
    m_selectedRow = index.row();
    m_selectedId = m_model->item(m_selectedRow, 0)->data(Qt::UserRole).toInt();
    m_editBtn->setEnabled(true);
    m_deleteBtn->setEnabled(true);
}

void CustomersPage::onAddClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Add Customer"));
    dialog.setLayoutDirection(Qt::RightToLeft);
    QGridLayout* layout = new QGridLayout(&dialog);
    layout->setSpacing(10);

    auto addRow = [&](int row, const QString& label, QWidget* widget) {
        QLabel* lbl = new QLabel(label);
        lbl->setStyleSheet("color: #1a3a5c; font-weight: 600;");
        layout->addWidget(lbl, row, 0);
        layout->addWidget(widget, row, 1);
        };

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QLineEdit* phoneEdit = new QLineEdit(&dialog);
    QLineEdit* emailEdit = new QLineEdit(&dialog);
    QTextEdit* addressEdit = new QTextEdit(&dialog);
    addressEdit->setMaximumHeight(80);

    addRow(0, tr("Name:"), nameEdit);
    addRow(1, tr("Phone:"), phoneEdit);
    addRow(2, tr("Email:"), emailEdit);
    addRow(3, tr("Address:"), addressEdit);

    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(btns, 4, 0, 1, 2);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Name is required."));
            return;
        }

        QSqlQuery query;
        query.prepare(
            "INSERT INTO customers (name, phone, email, address) "
            "VALUES (:name, :phone, :email, :address)"
        );
        query.bindValue(":name", name);
        query.bindValue(":phone", phoneEdit->text().trimmed());
        query.bindValue(":email", emailEdit->text().trimmed());
        query.bindValue(":address", addressEdit->toPlainText().trimmed());

        if (!query.exec()) {
            QMessageBox::critical(this, tr("Database Error"), query.lastError().text());
        }
        else {
            refreshTable();
        }
    }
}

void CustomersPage::onEditClicked()
{
    if (m_selectedId < 0) return;

    QSqlQuery fetch;
    fetch.prepare("SELECT name, phone, email, address, balance FROM customers WHERE id = :id");
    fetch.bindValue(":id", m_selectedId);
    if (!fetch.exec() || !fetch.next()) {
        QMessageBox::warning(this, tr("Error"), tr("Customer not found."));
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Edit Customer"));
    dialog.setLayoutDirection(Qt::RightToLeft);
    QGridLayout* layout = new QGridLayout(&dialog);
    layout->setSpacing(10);

    auto addRow = [&](int row, const QString& label, QWidget* widget) {
        QLabel* lbl = new QLabel(label);
        lbl->setStyleSheet("color: #1a3a5c; font-weight: 600;");
        layout->addWidget(lbl, row, 0);
        layout->addWidget(widget, row, 1);
        };

    QLineEdit* nameEdit = new QLineEdit(fetch.value("name").toString(), &dialog);
    QLineEdit* phoneEdit = new QLineEdit(fetch.value("phone").toString(), &dialog);
    QLineEdit* emailEdit = new QLineEdit(fetch.value("email").toString(), &dialog);
    QTextEdit* addressEdit = new QTextEdit(&dialog);
    addressEdit->setText(fetch.value("address").toString());
    addressEdit->setMaximumHeight(80);

    addRow(0, tr("Name:"), nameEdit);
    addRow(1, tr("Phone:"), phoneEdit);
    addRow(2, tr("Email:"), emailEdit);
    addRow(3, tr("Address:"), addressEdit);

    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(btns, 4, 0, 1, 2);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Name is required."));
            return;
        }

        QSqlQuery query;
        query.prepare(
            "UPDATE customers SET name = :name, phone = :phone, email = :email, "
            "address = :address WHERE id = :id"
        );
        query.bindValue(":name", name);
        query.bindValue(":phone", phoneEdit->text().trimmed());
        query.bindValue(":email", emailEdit->text().trimmed());
        query.bindValue(":address", addressEdit->toPlainText().trimmed());
        query.bindValue(":id", m_selectedId);

        if (!query.exec()) {
            QMessageBox::critical(this, tr("Database Error"), query.lastError().text());
        }
        else {
            refreshTable();
        }
    }
}

void CustomersPage::onDeleteClicked()
{
    if (m_selectedId < 0) return;

    QString name = m_model->item(m_selectedRow, 1)->text();
    int ret = QMessageBox::question(this, tr("Confirm Delete"),
        tr("Are you sure you want to delete customer '%1'?").arg(name),
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("UPDATE customers SET is_active = 0 WHERE id = :id");
        query.bindValue(":id", m_selectedId);

        if (!query.exec()) {
            QMessageBox::critical(this, tr("Database Error"), query.lastError().text());
        }
        else {
            refreshTable();
        }
    }
}

void CustomersPage::onSearchTextChanged(const QString& text)
{
    QString trimmed = text.trimmed().toLower();
    for (int i = 0; i < m_model->rowCount(); ++i) {
        bool match = false;
        for (int j = 1; j < m_model->columnCount(); ++j) {
            if (m_model->item(i, j)->text().toLower().contains(trimmed)) {
                match = true;
                break;
            }
        }
        m_table->setRowHidden(i, !match);
    }
}