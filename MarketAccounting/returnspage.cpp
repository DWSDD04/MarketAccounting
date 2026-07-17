#include "returnspage.h"
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
#include <QCheckBox>

ReturnsPage::ReturnsPage(QWidget* parent) : QWidget(parent)
{
    setupUI();
    loadRecentReturns();
    clearForm();
}

ReturnsPage::~ReturnsPage() {}

void ReturnsPage::setupUI()
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

    // ========= LEFT SIDE: Form =========
    QGroupBox* leftBox = new QGroupBox(tr("بيانات المرتجع"), this);
    leftBox->setStyleSheet(groupBoxStyle);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftBox);
    leftLayout->setSpacing(10);
    leftLayout->setContentsMargins(20, 20, 20, 20);

    QGridLayout* formGrid = new QGridLayout();
    formGrid->setSpacing(10);
    formGrid->setColumnStretch(1, 1);
    formGrid->setColumnStretch(3, 1);

    m_typeCombo = new StyledComboBox(this);
    m_typeCombo->addItems({ tr("Sales Return (from Customer)"), tr("Purchase Return (to Supplier)") });
    addLabel(formGrid, tr("نوع المرتجع :"), 0, 2);
    formGrid->addWidget(m_typeCombo, 0, 1, 1, 3);

    m_refEdit = new StyledLineEdit(this);
    m_refEdit->setPlaceholderText(tr("Original invoice reference..."));
    addLabel(formGrid, tr("رقم الفاتورة الأصلية :"), 1, 2);
    formGrid->addWidget(m_refEdit, 1, 1);

    m_searchBtn = new QPushButton(tr("بحث"));
    m_searchBtn->setStyleSheet(btnStyle);
    formGrid->addWidget(m_searchBtn, 1, 3);

    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDate(QDate::currentDate());
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    m_dateEdit->setStyleSheet(
        "QDateEdit { background-color: #ffffff; border: 1px solid #9DB9D2; "
        "border-radius: 3px; padding: 6px; font-size: 13px; color: #1a3a5c; }"
    );
    addLabel(formGrid, tr("تاريخ المرتجع :"), 2, 2);
    formGrid->addWidget(m_dateEdit, 2, 1);

    m_refundMethodCombo = new StyledComboBox(this);
    m_refundMethodCombo->addItems({ tr("Cash Refund"), tr("Store Credit"), tr("Reduce Debt") });
    addLabel(formGrid, tr("طريقة الاسترجاع :"), 2, 0);
    formGrid->addWidget(m_refundMethodCombo, 2, 3);

    m_reasonEdit = new StyledLineEdit(this);
    m_reasonEdit->setPlaceholderText(tr("Reason for return..."));
    addLabel(formGrid, tr("السبب :"), 3, 2);
    formGrid->addWidget(m_reasonEdit, 3, 1, 1, 3);

    m_discountEdit = new StyledLineEdit(this);
    m_discountEdit->setPlaceholderText("0.00");
    addLabel(formGrid, tr("خصم اضافي :"), 4, 2);
    formGrid->addWidget(m_discountEdit, 4, 1);

    leftLayout->addLayout(formGrid);

    // Original items section
    QLabel* itemsTitle = new QLabel(tr("اختيار الأصناف للإرجاع"));
    itemsTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1a3a5c; margin-top: 8px;");
    leftLayout->addWidget(itemsTitle);

    m_itemsTable = new QTableView(this);
    m_itemsModel = new QStandardItemModel(this);
    m_itemsModel->setHorizontalHeaderLabels({
        tr("Return?"), tr("Product"), tr("Orig Qty"), tr("Return Qty"), tr("Unit Price"), tr("Total")
        });
    m_itemsTable->setModel(m_itemsModel);
    m_itemsTable->setStyleSheet(tableStyle);
    m_itemsTable->horizontalHeader()->setStretchLastSection(true);
    m_itemsTable->setAlternatingRowColors(true);
    m_itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_itemsTable->setMaximumHeight(220);
    leftLayout->addWidget(m_itemsTable);

    // Totals
    QHBoxLayout* totalLayout = new QHBoxLayout();
    totalLayout->addStretch();
    m_originalTotalLabel = new QLabel(tr("Original: $0.00"));
    m_originalTotalLabel->setStyleSheet(
        "font-size: 13px; font-weight: 600; color: #1a3a5c; padding: 6px 12px; "
        "background-color: #e9ecef; border-radius: 4px;"
    );
    m_returnTotalLabel = new QLabel(tr("Return: $0.00"));
    m_returnTotalLabel->setStyleSheet(
        "font-size: 13px; font-weight: 600; color: #1a3a5c; padding: 6px 12px; "
        "background-color: #d4edda; border-radius: 4px;"
    );
    m_netRefundLabel = new QLabel(tr("Net Refund: $0.00"));
    m_netRefundLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: white; padding: 8px 16px; "
        "background-color: #1a3a5c; border-radius: 4px;"
    );
    totalLayout->addWidget(m_originalTotalLabel);
    totalLayout->addWidget(m_returnTotalLabel);
    totalLayout->addWidget(m_netRefundLabel);
    leftLayout->addLayout(totalLayout);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_saveBtn = new QPushButton(tr("Save Return"));
    m_saveBtn->setStyleSheet(btnStyle);
    m_clearBtn = new QPushButton(tr("Clear"));
    m_clearBtn->setStyleSheet(btnStyle);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addStretch();
    leftLayout->addLayout(btnLayout);
    leftLayout->addStretch();

    mainLayout->addWidget(leftBox, 1);

    // ========= RIGHT SIDE: Recent Returns =========
    QGroupBox* rightBox = new QGroupBox(tr("سجل المرتجعات"), this);
    rightBox->setStyleSheet(groupBoxStyle);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightBox);
    rightLayout->setContentsMargins(16, 16, 16, 16);

    m_recentTable = new QTableView(this);
    m_recentModel = new QStandardItemModel(this);
    m_recentModel->setHorizontalHeaderLabels({
        tr("ID"), tr("Type"), tr("Original Ref"), tr("Amount"), tr("Reason"), tr("Date")
        });
    m_recentTable->setModel(m_recentModel);
    m_recentTable->setStyleSheet(tableStyle);
    m_recentTable->horizontalHeader()->setStretchLastSection(true);
    m_recentTable->setAlternatingRowColors(true);
    m_recentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recentTable->hideColumn(0);
    rightLayout->addWidget(m_recentTable);

    mainLayout->addWidget(rightBox, 1);

    // Connections
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ReturnsPage::onTypeChanged);
    connect(m_searchBtn, &QPushButton::clicked, this, &ReturnsPage::onSearchOriginal);
    connect(m_saveBtn, &QPushButton::clicked, this, &ReturnsPage::onSaveClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &ReturnsPage::onClearClicked);
    connect(m_recentTable, &QTableView::clicked, this, &ReturnsPage::onRecentTableClicked);
    connect(m_discountEdit, &QLineEdit::textChanged, this, &ReturnsPage::calculateReturnTotal);
}

void ReturnsPage::onTypeChanged(int index)
{
    Q_UNUSED(index)
        clearForm();
}

void ReturnsPage::onSearchOriginal()
{
    QString ref = m_refEdit->text().trimmed();
    if (ref.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please enter the original invoice reference."));
        return;
    }

    QString type = (m_typeCombo->currentIndex() == 0) ? "sale" : "purchase";

    QSqlQuery query;
    query.prepare(
        "SELECT id, party_id, total_amount, net_amount FROM transactions "
        "WHERE type = :type AND reference_number = :ref LIMIT 1"
    );
    query.bindValue(":type", type);
    query.bindValue(":ref", ref);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, tr("Not Found"),
            tr("No %1 transaction found with reference: %2").arg(type, ref));
        return;
    }

    m_originalTransactionId = query.value("id").toInt();
    m_partyId = query.value("party_id").toInt();
    double origTotal = query.value("net_amount").toDouble();
    m_originalRef = ref;

    m_originalTotalLabel->setText(tr("Original: $%1").arg(QString::number(origTotal, 'f', 2)));
    loadOriginalItems(m_originalTransactionId);
}

void ReturnsPage::loadOriginalItems(int transactionId)
{
    m_itemsModel->removeRows(0, m_itemsModel->rowCount());
    m_items.clear();

    QSqlQuery query;
    query.prepare(
        "SELECT ti.id as item_id, ti.product_id, p.code, p.name, "
        "ti.quantity, ti.unit_price, ti.total_price "
        "FROM transaction_items ti "
        "JOIN products p ON ti.product_id = p.id "
        "WHERE ti.transaction_id = :tid"
    );
    query.bindValue(":tid", transactionId);

    if (!query.exec()) return;

    int row = 0;
    while (query.next()) {
        ReturnItem item;
        item.originalItemId = query.value("item_id").toInt();
        item.productId = query.value("product_id").toInt();
        item.code = query.value("code").toString();
        item.name = query.value("name").toString();
        item.originalQty = query.value("quantity").toInt();
        item.originalPrice = query.value("unit_price").toDouble();
        item.returnQty = 0;
        item.returnTotal = 0.0;
        m_items.append(item);

        m_itemsModel->insertRow(row);
        QStandardItem* checkItem = new QStandardItem();
        checkItem->setCheckable(true);
        checkItem->setCheckState(Qt::Unchecked);
        m_itemsModel->setItem(row, 0, checkItem);
        m_itemsModel->setItem(row, 1, new QStandardItem(item.name));
        m_itemsModel->setItem(row, 2, new QStandardItem(QString::number(item.originalQty)));
        m_itemsModel->setItem(row, 3, new QStandardItem("0"));
        m_itemsModel->setItem(row, 4, new QStandardItem(QString::number(item.originalPrice, 'f', 2)));
        m_itemsModel->setItem(row, 5, new QStandardItem("0.00"));
        row++;
    }

    m_itemsTable->resizeColumnsToContents();
}

void ReturnsPage::onItemQtyChanged(const QModelIndex& index)
{
    Q_UNUSED(index)
}

void ReturnsPage::calculateReturnTotal()
{
    double returnTotal = 0.0;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_itemsModel->item(i, 0)->checkState() == Qt::Checked) {
            returnTotal += m_items[i].returnTotal;
        }
    }
    double discount = m_discountEdit->text().toDouble();
    double net = returnTotal - discount;

    m_returnTotalLabel->setText(tr("Return: $%1").arg(QString::number(returnTotal, 'f', 2)));
    m_netRefundLabel->setText(tr("Net Refund: $%1").arg(QString::number(net, 'f', 2)));
}

bool ReturnsPage::validateForm()
{
    if (m_originalTransactionId < 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Please search and select an original invoice."));
        return false;
    }
    bool hasChecked = false;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_itemsModel->item(i, 0)->checkState() == Qt::Checked) {
            hasChecked = true;
            break;
        }
    }
    if (!hasChecked) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select at least one item to return."));
        return false;
    }
    return true;
}

bool ReturnsPage::saveReturn()
{
    QString type = (m_typeCombo->currentIndex() == 0) ? "return_sale" : "return_purchase";
    QString refundMethod = m_refundMethodCombo->currentText();
    QString reason = m_reasonEdit->text().trimmed();
    double discount = m_discountEdit->text().toDouble();

    double returnTotal = 0.0;
    QVector<ReturnItem> itemsToReturn;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_itemsModel->item(i, 0)->checkState() == Qt::Checked) {
            // Read return qty from column 3
            int retQty = m_itemsModel->item(i, 3)->text().toInt();
            if (retQty <= 0) retQty = 1;
            if (retQty > m_items[i].originalQty) retQty = m_items[i].originalQty;

            m_items[i].returnQty = retQty;
            m_items[i].returnTotal = retQty * m_items[i].originalPrice;
            itemsToReturn.append(m_items[i]);
            returnTotal += m_items[i].returnTotal;
        }
    }
    double net = returnTotal - discount;

    DbManager* db = DbManager::instance();
    if (!db->transaction()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to start transaction."));
        return false;
    }

    // Insert return transaction
    QSqlQuery transQuery;
    transQuery.prepare(
        "INSERT INTO transactions (type, reference_number, party_id, total_amount, "
        "discount, tax_amount, net_amount, payment_method, notes, created_at) "
        "VALUES (:type, :ref, :party, :total, :discount, 0, :net, "
        ":payment, :notes, :created)"
    );
    transQuery.bindValue(":type", type);
    transQuery.bindValue(":ref", m_originalRef + "-RET");
    transQuery.bindValue(":party", m_partyId);
    transQuery.bindValue(":total", returnTotal);
    transQuery.bindValue(":discount", discount);
    transQuery.bindValue(":net", net);
    transQuery.bindValue(":payment", refundMethod);
    transQuery.bindValue(":notes", reason);
    transQuery.bindValue(":created", QDateTime::currentDateTime());

    if (!transQuery.exec()) {
        db->rollback();
        QMessageBox::critical(this, tr("Error"), transQuery.lastError().text());
        return false;
    }

    int returnTransId = transQuery.lastInsertId().toInt();

    // Insert items and adjust stock
    for (const auto& item : itemsToReturn) {
        QSqlQuery itemQuery;
        itemQuery.prepare(
            "INSERT INTO transaction_items (transaction_id, product_id, quantity, unit_price, total_price) "
            "VALUES (:tid, :pid, :qty, :price, :total)"
        );
        itemQuery.bindValue(":tid", returnTransId);
        itemQuery.bindValue(":pid", item.productId);
        itemQuery.bindValue(":qty", item.returnQty);
        itemQuery.bindValue(":price", item.originalPrice);
        itemQuery.bindValue(":total", item.returnTotal);

        if (!itemQuery.exec()) {
            db->rollback();
            QMessageBox::critical(this, tr("Error"), itemQuery.lastError().text());
            return false;
        }

        // Adjust stock
        QSqlQuery stockQuery;
        if (m_typeCombo->currentIndex() == 0) {
            // Sales return: add stock back
            stockQuery.prepare("UPDATE products SET quantity = quantity + :qty WHERE id = :pid");
        }
        else {
            // Purchase return: remove stock
            stockQuery.prepare("UPDATE products SET quantity = quantity - :qty WHERE id = :pid");
        }
        stockQuery.bindValue(":qty", item.returnQty);
        stockQuery.bindValue(":pid", item.productId);
        if (!stockQuery.exec()) {
            db->rollback();
            QMessageBox::critical(this, tr("Error"), stockQuery.lastError().text());
            return false;
        }
    }

    // Update debt if applicable
    if (refundMethod == "Reduce Debt" && m_partyId > 0) {
        QString partyType = (m_typeCombo->currentIndex() == 0) ? "customer" : "supplier";
        QSqlQuery debtQuery;
        debtQuery.prepare(
            "INSERT INTO debts (party_type, party_id, amount, debt_type, due_date, description, is_paid) "
            "VALUES (:ptype, :pid, :amount, 'owed', CURDATE(), :desc, 1)"
        );
        debtQuery.bindValue(":ptype", partyType);
        debtQuery.bindValue(":pid", m_partyId);
        debtQuery.bindValue(":amount", net);
        debtQuery.bindValue(":desc", tr("Return %1 - %2").arg(m_originalRef, reason));
        debtQuery.exec(); // Best effort
    }

    db->commit();
    return true;
}

void ReturnsPage::onSaveClicked()
{
    if (!validateForm()) return;

    if (saveReturn()) {
        QMessageBox::information(this, tr("Success"), tr("Return processed successfully!"));
        clearForm();
        loadRecentReturns();
    }
}

void ReturnsPage::onClearClicked()
{
    clearForm();
}

void ReturnsPage::clearForm()
{
    m_originalTransactionId = -1;
    m_partyId = 0;
    m_originalRef.clear();
    m_items.clear();

    m_refEdit->clear();
    m_dateEdit->setDate(QDate::currentDate());
    m_refundMethodCombo->setCurrentIndex(0);
    m_reasonEdit->clear();
    m_discountEdit->clear();

    m_itemsModel->removeRows(0, m_itemsModel->rowCount());
    m_originalTotalLabel->setText(tr("Original: $0.00"));
    m_returnTotalLabel->setText(tr("Return: $0.00"));
    m_netRefundLabel->setText(tr("Net Refund: $0.00"));
}

void ReturnsPage::loadRecentReturns()
{
    m_recentModel->removeRows(0, m_recentModel->rowCount());

    QSqlQuery query(
        "SELECT id, type, reference_number, net_amount, notes, created_at "
        "FROM transactions "
        "WHERE type IN ('return_sale', 'return_purchase') "
        "ORDER BY created_at DESC LIMIT 30"
    );

    int row = 0;
    while (query.next()) {
        m_recentModel->insertRow(row);
        m_recentModel->setItem(row, 0, new QStandardItem(query.value("id").toString()));
        QString t = query.value("type").toString();
        m_recentModel->setItem(row, 1, new QStandardItem(t == "return_sale" ? tr("Sales Return") : tr("Purchase Return")));
        m_recentModel->setItem(row, 2, new QStandardItem(query.value("reference_number").toString()));
        m_recentModel->setItem(row, 3, new QStandardItem(QString::number(query.value("net_amount").toDouble(), 'f', 2)));
        m_recentModel->setItem(row, 4, new QStandardItem(query.value("notes").toString()));
        m_recentModel->setItem(row, 5, new QStandardItem(query.value("created_at").toDateTime().toString("yyyy-MM-dd")));
        row++;
    }
    m_recentTable->resizeColumnsToContents();
}

void ReturnsPage::onRecentTableClicked(const QModelIndex& index)
{
    Q_UNUSED(index)
}