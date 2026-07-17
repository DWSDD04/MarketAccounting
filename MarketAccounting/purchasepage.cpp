#include "purchasepage.h"
#include "styledlineedit.h"
#include "styledcombobox.h"
#include "dbmanager.h"
#include "product.h"

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
#include <QInputDialog>

PurchasePage::PurchasePage(QWidget* parent) : QWidget(parent)
{
    setupUI();
    loadSuppliers();
    loadAccounts();
    loadProducts();
    loadRecentPurchases();
    clearForm();
}

PurchasePage::~PurchasePage() {}

void PurchasePage::setupUI()
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

    // ========= LEFT SIDE =========
    QGroupBox* leftBox = new QGroupBox(tr("معلومات المشتريات"), this);
    leftBox->setStyleSheet(groupBoxStyle);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftBox);
    leftLayout->setSpacing(10);
    leftLayout->setContentsMargins(20, 20, 20, 20);

    // Header grid
    QGridLayout* headerGrid = new QGridLayout();
    headerGrid->setSpacing(10);
    headerGrid->setColumnStretch(1, 1);
    headerGrid->setColumnStretch(3, 1);

    m_supplierCombo = new StyledComboBox(this);
    m_supplierCombo->setPlaceholderText(tr("Select Supplier"));
    addLabel(headerGrid, tr("المورد :"), 0, 2);
    headerGrid->addWidget(m_supplierCombo, 0, 1, 1, 3);

    m_refEdit = new StyledLineEdit(this);
    m_refEdit->setPlaceholderText(tr("Auto"));
    m_refEdit->setReadOnly(true);
    addLabel(headerGrid, tr("الرقم :"), 1, 2);
    headerGrid->addWidget(m_refEdit, 1, 1);

    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDate(QDate::currentDate());
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    m_dateEdit->setStyleSheet(
        "QDateEdit { background-color: #ffffff; border: 1px solid #9DB9D2; "
        "border-radius: 3px; padding: 6px; font-size: 13px; color: #1a3a5c; }"
    );
    addLabel(headerGrid, tr("التاريخ :"), 1, 0);
    headerGrid->addWidget(m_dateEdit, 1, 3);

    m_paymentCombo = new StyledComboBox(this);
    m_paymentCombo->addItems({ tr("Cash"), tr("Debt") });
    addLabel(headerGrid, tr("الدفع :"), 2, 2);
    headerGrid->addWidget(m_paymentCombo, 2, 1);

    m_accountCombo = new StyledComboBox(this);
    m_accountCombo->setPlaceholderText(tr("Select Account"));
    addLabel(headerGrid, tr("الحساب :"), 2, 0);
    headerGrid->addWidget(m_accountCombo, 2, 3);

    m_discountEdit = new StyledLineEdit(this);
    m_discountEdit->setPlaceholderText("0.00");
    addLabel(headerGrid, tr("الخصم :"), 3, 2);
    headerGrid->addWidget(m_discountEdit, 3, 1);

    m_taxEdit = new StyledLineEdit(this);
    m_taxEdit->setPlaceholderText("0.00");
    addLabel(headerGrid, tr("الضريبة :"), 3, 0);
    headerGrid->addWidget(m_taxEdit, 3, 3);

    m_notesEdit = new StyledLineEdit(this);
    m_notesEdit->setPlaceholderText(tr("Notes..."));
    addLabel(headerGrid, tr("ملاحظات :"), 4, 2);
    headerGrid->addWidget(m_notesEdit, 4, 1, 1, 3);

    leftLayout->addLayout(headerGrid);

    // Item entry section
    QLabel* itemsTitle = new QLabel(tr("اضافة منتج"));
    itemsTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1a3a5c; margin-top: 8px;");
    leftLayout->addWidget(itemsTitle);

    m_searchEdit = new StyledLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search product name or code..."));
    leftLayout->addWidget(m_searchEdit);

    m_productTable = new QTableView(this);
    m_productModel = new QStandardItemModel(this);
    m_productModel->setHorizontalHeaderLabels({ tr("ID"), tr("Code"), tr("Name"), tr("Stock") });
    m_productTable->setModel(m_productModel);
    m_productTable->setStyleSheet(tableStyle);
    m_productTable->horizontalHeader()->setStretchLastSection(true);
    m_productTable->setAlternatingRowColors(true);
    m_productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_productTable->setMaximumHeight(160);
    m_productTable->hideColumn(0);
    leftLayout->addWidget(m_productTable);

    QHBoxLayout* entryLayout = new QHBoxLayout();
    QLabel* qtyLbl = new QLabel(tr("الكمية:"));
    qtyLbl->setStyleSheet("color: #1a3a5c; font-size: 13px; font-weight: 600;");
    entryLayout->addWidget(qtyLbl);

    m_qtySpin = new QSpinBox(this);
    m_qtySpin->setRange(1, 99999);
    m_qtySpin->setValue(1);
    m_qtySpin->setFixedWidth(80);
    m_qtySpin->setStyleSheet(
        "QSpinBox { background-color: white; border: 1px solid #9DB9D2; border-radius: 3px; "
        "padding: 6px; font-size: 13px; color: #1a3a5c; }"
    );
    entryLayout->addWidget(m_qtySpin);

    QLabel* priceLbl = new QLabel(tr("السعر:"));
    priceLbl->setStyleSheet("color: #1a3a5c; font-size: 13px; font-weight: 600;");
    entryLayout->addWidget(priceLbl);

    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setRange(0.01, 999999.99);
    m_priceSpin->setDecimals(2);
    m_priceSpin->setValue(0.0);
    m_priceSpin->setFixedWidth(100);
    m_priceSpin->setStyleSheet(
        "QDoubleSpinBox { background-color: white; border: 1px solid #9DB9D2; border-radius: 3px; "
        "padding: 6px; font-size: 13px; color: #1a3a5c; }"
    );
    entryLayout->addWidget(m_priceSpin);

    m_addItemBtn = new QPushButton(tr("Add to Purchase"));
    m_addItemBtn->setStyleSheet(btnStyle);
    entryLayout->addWidget(m_addItemBtn);
    entryLayout->addStretch();
    leftLayout->addLayout(entryLayout);

    // Action buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_newBtn = new QPushButton(tr("New Purchase"));
    m_newBtn->setStyleSheet(btnStyle);
    m_saveBtn = new QPushButton(tr("Save"));
    m_saveBtn->setStyleSheet(btnStyle);
    m_clearBtn = new QPushButton(tr("Clear"));
    m_clearBtn->setStyleSheet(btnStyle);

    btnLayout->addWidget(m_newBtn);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addStretch();
    leftLayout->addLayout(btnLayout);
    leftLayout->addStretch();

    mainLayout->addWidget(leftBox, 1);

    // ========= RIGHT SIDE =========
    QGroupBox* rightBox = new QGroupBox(tr("فاتورة المشتريات"), this);
    rightBox->setStyleSheet(groupBoxStyle);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightBox);
    rightLayout->setSpacing(10);
    rightLayout->setContentsMargins(20, 20, 20, 20);

    // Cart table
    m_cartTable = new QTableView(this);
    m_cartModel = new QStandardItemModel(this);
    m_cartModel->setHorizontalHeaderLabels({
        tr("ID"), tr("Code"), tr("Product"), tr("Qty"), tr("Price"), tr("Total")
        });
    m_cartTable->setModel(m_cartModel);
    m_cartTable->setStyleSheet(tableStyle);
    m_cartTable->horizontalHeader()->setStretchLastSection(true);
    m_cartTable->setAlternatingRowColors(true);
    m_cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_cartTable->hideColumn(0);
    rightLayout->addWidget(m_cartTable, 2);

    QHBoxLayout* cartBtnLayout = new QHBoxLayout();
    m_deleteItemBtn = new QPushButton(tr("Delete Item"));
    m_deleteItemBtn->setStyleSheet(deleteBtnStyle);
    m_editItemBtn = new QPushButton(tr("Edit Qty"));
    m_editItemBtn->setStyleSheet(btnStyle);
    cartBtnLayout->addWidget(m_deleteItemBtn);
    cartBtnLayout->addWidget(m_editItemBtn);
    cartBtnLayout->addStretch();
    rightLayout->addLayout(cartBtnLayout);

    // Totals
    QHBoxLayout* totalLayout = new QHBoxLayout();
    totalLayout->addStretch();
    m_totalLabel = new QLabel(tr("Total: $0.00"));
    m_totalLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #1a3a5c; "
        "padding: 10px 16px; background-color: #e9ecef; border-radius: 6px;"
    );
    m_netLabel = new QLabel(tr("Net: $0.00"));
    m_netLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: white; "
        "padding: 10px 16px; background-color: #1a3a5c; border-radius: 6px;"
    );
    totalLayout->addWidget(m_totalLabel);
    totalLayout->addWidget(m_netLabel);
    rightLayout->addLayout(totalLayout);

    // Recent purchases
    QLabel* recentTitle = new QLabel(tr("Recent Purchases"));
    recentTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1a3a5c; margin-top: 8px;");
    rightLayout->addWidget(recentTitle);

    m_recentTable = new QTableView(this);
    m_recentModel = new QStandardItemModel(this);
    m_recentModel->setHorizontalHeaderLabels({
        tr("ID"), tr("Ref"), tr("Supplier"), tr("Total"), tr("Date")
        });
    m_recentTable->setModel(m_recentModel);
    m_recentTable->setStyleSheet(tableStyle);
    m_recentTable->horizontalHeader()->setStretchLastSection(true);
    m_recentTable->setAlternatingRowColors(true);
    m_recentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recentTable->hideColumn(0);
    m_recentTable->setMaximumHeight(160);
    rightLayout->addWidget(m_recentTable, 1);

    mainLayout->addWidget(rightBox, 1);

    // Connections
    connect(m_supplierCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &PurchasePage::onSupplierChanged);
    connect(m_paymentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &PurchasePage::onPaymentChanged);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &PurchasePage::onSearchTextChanged);
    connect(m_productTable, &QTableView::clicked, this, &PurchasePage::onProductSelected);
    connect(m_addItemBtn, &QPushButton::clicked, this, &PurchasePage::onAddItemClicked);
    connect(m_deleteItemBtn, &QPushButton::clicked, this, &PurchasePage::onDeleteItemClicked);
    connect(m_editItemBtn, &QPushButton::clicked, this, &PurchasePage::onEditItemClicked);
    connect(m_saveBtn, &QPushButton::clicked, this, &PurchasePage::onSaveClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &PurchasePage::onClearClicked);
    connect(m_newBtn, &QPushButton::clicked, this, &PurchasePage::onNewPurchaseClicked);
    connect(m_cartTable, &QTableView::clicked, this, &PurchasePage::onTableClicked);
    connect(m_recentTable, &QTableView::clicked, this, &PurchasePage::onRecentTableClicked);
    connect(m_discountEdit, &QLineEdit::textChanged, this, &PurchasePage::calculateTotals);
    connect(m_taxEdit, &QLineEdit::textChanged, this, &PurchasePage::calculateTotals);
}

void PurchasePage::loadSuppliers()
{
    m_supplierCombo->clear();
    m_supplierCombo->addItem(tr("-- Select Supplier --"), 0);

    QSqlQuery query("SELECT id, name FROM suppliers WHERE is_active = 1 ORDER BY name");
    while (query.next()) {
        m_supplierCombo->addItem(query.value("name").toString(), query.value("id"));
    }
}

void PurchasePage::loadAccounts()
{
    m_accountCombo->clear();
    m_accountCombo->addItem(tr("-- Select Account --"), 0);

    QSqlQuery query("SELECT id, name, type FROM accounts WHERE is_active = 1 ORDER BY name");
    while (query.next()) {
        QString name = query.value("name").toString();
        QString type = query.value("type").toString();
        m_accountCombo->addItem(name + " (" + type + ")", query.value("id"));
    }
}

void PurchasePage::loadProducts(const QString& filter)
{
    m_productModel->removeRows(0, m_productModel->rowCount());

    QSqlQuery query;
    if (filter.trimmed().isEmpty()) {
        query.prepare("SELECT id, code, name, quantity FROM products WHERE is_active = 1 ORDER BY name LIMIT 50");
    }
    else {
        query.prepare("SELECT id, code, name, quantity FROM products "
            "WHERE is_active = 1 AND (code LIKE :f OR name LIKE :f) ORDER BY name LIMIT 50");
        query.bindValue(":f", "%" + filter + "%");
    }

    if (!query.exec()) return;

    int row = 0;
    while (query.next()) {
        int id = query.value("id").toInt();
        m_productModel->insertRow(row);
        QStandardItem* idItem = new QStandardItem(QString::number(id));
        idItem->setData(id, Qt::UserRole);
        m_productModel->setItem(row, 0, idItem);
        m_productModel->setItem(row, 1, new QStandardItem(query.value("code").toString()));
        m_productModel->setItem(row, 2, new QStandardItem(query.value("name").toString()));
        m_productModel->setItem(row, 3, new QStandardItem(query.value("quantity").toString()));
        row++;
    }
    m_productTable->resizeColumnsToContents();
}

void PurchasePage::loadRecentPurchases()
{
    m_recentModel->removeRows(0, m_recentModel->rowCount());

    QSqlQuery query(
        "SELECT t.id, t.reference_number, s.name as supplier_name, "
        "t.net_amount, t.created_at "
        "FROM transactions t "
        "LEFT JOIN suppliers s ON t.party_id = s.id "
        "WHERE t.type = 'purchase' "
        "ORDER BY t.created_at DESC LIMIT 20"
    );

    int row = 0;
    while (query.next()) {
        m_recentModel->insertRow(row);
        m_recentModel->setItem(row, 0, new QStandardItem(query.value("id").toString()));
        m_recentModel->setItem(row, 1, new QStandardItem(query.value("reference_number").toString()));
        m_recentModel->setItem(row, 2, new QStandardItem(query.value("supplier_name").toString()));
        m_recentModel->setItem(row, 3, new QStandardItem(QString::number(query.value("net_amount").toDouble(), 'f', 2)));
        m_recentModel->setItem(row, 4, new QStandardItem(query.value("created_at").toDateTime().toString("yyyy-MM-dd")));
        row++;
    }
    m_recentTable->resizeColumnsToContents();
}

void PurchasePage::onSupplierChanged(int index)
{
    if (index > 0) {
        m_header.supplierId = m_supplierCombo->itemData(index).toInt();
        m_header.supplierName = m_supplierCombo->itemText(index);
    }
    else {
        m_header.supplierId = 0;
        m_header.supplierName.clear();
    }
}

void PurchasePage::onPaymentChanged(int index)
{
    m_header.paymentType = (index == 0) ? "cash" : "debt";
}

void PurchasePage::onSearchTextChanged(const QString& text)
{
    loadProducts(text.trimmed());
}

void PurchasePage::onProductSelected(const QModelIndex& index)
{
    int row = index.row();
    if (row < 0) return;

    int productId = m_productModel->item(row, 0)->data(Qt::UserRole).toInt();

    QSqlQuery query;
    query.prepare("SELECT purchase_price FROM products WHERE id = :id");
    query.bindValue(":id", productId);
    if (query.exec() && query.next()) {
        m_priceSpin->setValue(query.value("purchase_price").toDouble());
    }
}

void PurchasePage::onAddItemClicked()
{
    QModelIndexList selected = m_productTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a product first."));
        return;
    }

    int row = selected.first().row();
    int productId = m_productModel->item(row, 0)->data(Qt::UserRole).toInt();
    QString code = m_productModel->item(row, 1)->text();
    QString name = m_productModel->item(row, 2)->text();
    int qty = m_qtySpin->value();
    double price = m_priceSpin->value();

    if (price <= 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Price must be greater than zero."));
        return;
    }

    // Update existing item if same product
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].productId == productId) {
            m_items[i].quantity += qty;
            m_items[i].lineTotal = m_items[i].quantity * m_items[i].price;
            refreshCartTable();
            calculateTotals();
            return;
        }
    }

    PurchaseItem item;
    item.productId = productId;
    item.code = code;
    item.name = name;
    item.price = price;
    item.quantity = qty;
    item.lineTotal = price * qty;
    m_items.append(item);

    refreshCartTable();
    calculateTotals();
    m_qtySpin->setValue(1);
}

void PurchasePage::onDeleteItemClicked()
{
    QModelIndexList selected = m_cartTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an item to delete."));
        return;
    }

    int row = selected.first().row();
    if (row >= 0 && row < m_items.size()) {
        m_items.removeAt(row);
        refreshCartTable();
        calculateTotals();
    }
}

void PurchasePage::onEditItemClicked()
{
    QModelIndexList selected = m_cartTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an item to edit."));
        return;
    }

    int row = selected.first().row();
    if (row < 0 || row >= m_items.size()) return;

    bool ok;
    int newQty = QInputDialog::getInt(this, tr("Edit Quantity"),
        tr("Enter new quantity:"), m_items[row].quantity, 1, 99999, 1, &ok);
    if (ok) {
        m_items[row].quantity = newQty;
        m_items[row].lineTotal = newQty * m_items[row].price;
        refreshCartTable();
        calculateTotals();
    }
}

void PurchasePage::refreshCartTable()
{
    m_cartModel->removeRows(0, m_cartModel->rowCount());

    for (int i = 0; i < m_items.size(); ++i) {
        const PurchaseItem& item = m_items[i];
        m_cartModel->insertRow(i);
        m_cartModel->setItem(i, 0, new QStandardItem(QString::number(item.productId)));
        m_cartModel->setItem(i, 1, new QStandardItem(item.code));
        m_cartModel->setItem(i, 2, new QStandardItem(item.name));
        m_cartModel->setItem(i, 3, new QStandardItem(QString::number(item.quantity)));
        m_cartModel->setItem(i, 4, new QStandardItem(QString::number(item.price, 'f', 2)));
        m_cartModel->setItem(i, 5, new QStandardItem(QString::number(item.lineTotal, 'f', 2)));
    }
    m_cartTable->resizeColumnsToContents();
}

void PurchasePage::calculateTotals()
{
    double total = 0.0;
    for (const auto& item : m_items) {
        total += item.lineTotal;
    }
    double discount = m_discountEdit->text().toDouble();
    double tax = m_taxEdit->text().toDouble();
    double net = total - discount + tax;

    m_header.discount = discount;
    m_header.tax = tax;

    m_totalLabel->setText(tr("Total: $%1").arg(QString::number(total, 'f', 2)));
    m_netLabel->setText(tr("Net: $%1").arg(QString::number(net, 'f', 2)));
}

bool PurchasePage::validateForm()
{
    if (m_header.supplierId <= 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a supplier."));
        return false;
    }
    if (m_items.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please add at least one item."));
        return false;
    }
    if (m_accountCombo->currentIndex() <= 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an account."));
        return false;
    }
    return true;
}

bool PurchasePage::savePurchase()
{
    double total = 0.0;
    for (const auto& item : m_items) total += item.lineTotal;
    double net = total - m_header.discount + m_header.tax;

    int accountId = m_accountCombo->currentData().toInt();
    QString ref = m_refEdit->text();
    if (ref.isEmpty()) {
        ref = QString("PUR-%1").arg(m_nextRef++);
    }

    DbManager* db = DbManager::instance();
    if (!db->transaction()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to start transaction."));
        return false;
    }

    // Insert transaction
    QSqlQuery transQuery;
    transQuery.prepare(
        "INSERT INTO transactions (type, reference_number, party_id, total_amount, "
        "discount, tax_amount, net_amount, payment_method, notes, created_at) "
        "VALUES ('purchase', :ref, :party, :total, :discount, :tax, :net, "
        ":payment, :notes, :created)"
    );
    transQuery.bindValue(":ref", ref);
    transQuery.bindValue(":party", m_header.supplierId);
    transQuery.bindValue(":total", total);
    transQuery.bindValue(":discount", m_header.discount);
    transQuery.bindValue(":tax", m_header.tax);
    transQuery.bindValue(":net", net);
    transQuery.bindValue(":payment", m_header.paymentType);
    transQuery.bindValue(":notes", m_notesEdit->text().trimmed());
    transQuery.bindValue(":created", QDateTime::currentDateTime());

    if (!transQuery.exec()) {
        db->rollback();
        QMessageBox::critical(this, tr("Error"), transQuery.lastError().text());
        return false;
    }

    int transactionId = transQuery.lastInsertId().toInt();

    // Insert items and update stock
    for (const auto& item : m_items) {
        QSqlQuery itemQuery;
        itemQuery.prepare(
            "INSERT INTO transaction_items (transaction_id, product_id, quantity, unit_price, total_price) "
            "VALUES (:tid, :pid, :qty, :price, :total)"
        );
        itemQuery.bindValue(":tid", transactionId);
        itemQuery.bindValue(":pid", item.productId);
        itemQuery.bindValue(":qty", item.quantity);
        itemQuery.bindValue(":price", item.price);
        itemQuery.bindValue(":total", item.lineTotal);

        if (!itemQuery.exec()) {
            db->rollback();
            QMessageBox::critical(this, tr("Error"), itemQuery.lastError().text());
            return false;
        }

        // Update stock
        QSqlQuery stockQuery;
        stockQuery.prepare("UPDATE products SET quantity = quantity + :qty WHERE id = :pid");
        stockQuery.bindValue(":qty", item.quantity);
        stockQuery.bindValue(":pid", item.productId);
        if (!stockQuery.exec()) {
            db->rollback();
            QMessageBox::critical(this, tr("Error"), stockQuery.lastError().text());
            return false;
        }
    }

    // If debt, record in debts table
    if (m_header.paymentType == "debt") {
        QSqlQuery debtQuery;
        debtQuery.prepare(
            "INSERT INTO debts (party_type, party_id, amount, debt_type, due_date, description, is_paid) "
            "VALUES ('supplier', :pid, :amount, 'owe', DATE_ADD(CURDATE(), INTERVAL 30 DAY), :desc, 0)"
        );
        debtQuery.bindValue(":pid", m_header.supplierId);
        debtQuery.bindValue(":amount", net);
        debtQuery.bindValue(":desc", tr("Purchase invoice %1").arg(ref));
        debtQuery.exec(); // Non-fatal
    }

    db->commit();
    return true;
}

void PurchasePage::onSaveClicked()
{
    if (!validateForm()) return;

    if (savePurchase()) {
        QMessageBox::information(this, tr("Success"), tr("Purchase saved successfully!"));
        clearForm();
        loadRecentPurchases();
    }
}

void PurchasePage::onClearClicked()
{
    clearForm();
}

void PurchasePage::onNewPurchaseClicked()
{
    clearForm();
}

void PurchasePage::onTableClicked(const QModelIndex& index)
{
    Q_UNUSED(index)
}

void PurchasePage::onRecentTableClicked(const QModelIndex& index)
{
    Q_UNUSED(index)
        // Could load the purchase for viewing in future
}

void PurchasePage::clearForm()
{
    m_header = PurchaseHeader();
    m_items.clear();
    m_header.date = QDate::currentDate();

    m_supplierCombo->setCurrentIndex(0);
    m_refEdit->setText(QString("PUR-%1").arg(m_nextRef));
    m_dateEdit->setDate(QDate::currentDate());
    m_paymentCombo->setCurrentIndex(0);
    m_accountCombo->setCurrentIndex(0);
    m_discountEdit->clear();
    m_taxEdit->clear();
    m_notesEdit->clear();
    m_searchEdit->clear();
    m_qtySpin->setValue(1);
    m_priceSpin->setValue(0.0);

    refreshCartTable();
    calculateTotals();
    loadProducts();
}