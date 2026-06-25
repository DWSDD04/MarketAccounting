#include "salespage.h"
#include "styledlineedit.h"
#include "styledcombobox.h"
#include "cardwidget.h"
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
#include <QDebug>
#include <QLineEdit>
#include <QListWidget>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QKeyEvent>
#include <QDateTime>

// ============================================================
// PriceEditDialog
// ============================================================
PriceEditDialog::PriceEditDialog(int productId, const QString& productName, double currentPrice, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Change Product Price"));
    setMinimumWidth(350);
    setStyleSheet("QDialog { background-color: #F8F9FA; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    QLabel* title = new QLabel(tr("Edit Price for: %1").arg(productName));
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #1a1a2e;");
    mainLayout->addWidget(title);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);

    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setRange(0.01, 99999999.99);
    m_priceSpin->setDecimals(2);
    m_priceSpin->setValue(currentPrice);
    m_priceSpin->setStyleSheet(
        "QDoubleSpinBox {"
        "  background-color: #ffffff;"
        "  border: 2px solid #e9ecef;"
        "  border-radius: 8px;"
        "  padding: 10px 14px;"
        "  font-size: 13px;"
        "  color: #212529;"
        "  min-height: 38px;"
        "}"
        "QDoubleSpinBox:focus { border: 2px solid #16213e; }"
    );
    formLayout->addRow(tr("New Price (USD):"), m_priceSpin);
    mainLayout->addLayout(formLayout);

    QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    btnBox->setStyleSheet(
        "QPushButton {"
        "  background-color: #16213e; color: white;"
        "  padding: 10px 24px; border-radius: 8px;"
        "  font-weight: 600; border: none; min-width: 80px;"
        "}"
        "QPushButton:hover { background-color: #0f3460; }"
        "QPushButton[text=\"Cancel\"] {"
        "  background-color: #e9ecef; color: #495057;"
        "  border: 1px solid #dee2e6;"
        "}"
    );
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(btnBox, 0, Qt::AlignRight);
}

double PriceEditDialog::newPrice() const {
    return m_priceSpin->value();
}

// ============================================================
// SalesPage
// ============================================================
SalesPage::SalesPage(QWidget* parent) : QWidget(parent) {
    // Initialize 3 empty sessions
    for (int i = 0; i < 3; ++i) {
        SaleSession session;
        session.saleCode = -1;
        session.paymentType = "cash";
        m_sessions.append(session);
    }
    setupUI();
    loadProductTypes();
    loadAccounts();
    loadAllProducts();
    refreshProductListWidget();
    switchToSession(0);
}

SalesPage::~SalesPage() {}

void SalesPage::setupUI() {
    QHBoxLayout* outerLayout = new QHBoxLayout(this);
    outerLayout->setSpacing(0);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    setStyleSheet("background-color: #F2F2F2;");

    // ==================== LEFT PANEL: Product Types & Search ====================
    QWidget* leftPanel = new QWidget(this);
    leftPanel->setFixedWidth(280);
    leftPanel->setStyleSheet("background-color: #ffffff; border-right: 1px solid #e9ecef;");
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(15);
    leftLayout->setContentsMargins(15, 20, 15, 20);

    QLabel* typesTitle = new QLabel(tr("Product Types"));
    typesTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #1a1a2e; padding-bottom: 8px;");
    leftLayout->addWidget(typesTitle);

    m_typeList = new QListWidget(this);
    m_typeList->setStyleSheet(
        "QListWidget {"
        "  background-color: #f8f9fa;"
        "  border: 1px solid #e9ecef;"
        "  border-radius: 10px;"
        "  padding: 8px;"
        "  font-size: 13px;"
        "  color: #495057;"
        "}"
        "QListWidget::item {"
        "  padding: 10px 14px;"
        "  border-radius: 8px;"
        "  margin: 2px 0;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: #e9ecef;"
        "  color: #1a1a2e;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #16213e;"
        "  color: white;"
        "  font-weight: 600;"
        "}"
    );
    connect(m_typeList, &QListWidget::itemClicked, this, &SalesPage::onProductTypeClicked);
    leftLayout->addWidget(m_typeList);

    // Product search
    QLabel* searchTitle = new QLabel(tr("Search Products"));
    searchTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1a1a2e; margin-top: 10px;");
    leftLayout->addWidget(searchTitle);

    m_productSearch = new StyledLineEdit(this);
    m_productSearch->setPlaceholderText(tr("Name or Code..."));
    connect(m_productSearch, &QLineEdit::textChanged, this, &SalesPage::onSearchProductChanged);
    leftLayout->addWidget(m_productSearch);

    m_productList = new QListWidget(this);
    m_productList->setStyleSheet(
        "QListWidget {"
        "  background-color: #ffffff;"
        "  border: 1px solid #e9ecef;"
        "  border-radius: 10px;"
        "  padding: 8px;"
        "  font-size: 13px;"
        "}"
        "QListWidget::item {"
        "  padding: 10px 14px;"
        "  border-bottom: 1px solid #f1f3f5;"
        "  border-radius: 6px;"
        "}"
        "QListWidget::item:hover { background-color: #f1f3f5; }"
        "QListWidget::item:selected {"
        "  background-color: #0f3460;"
        "  color: white;"
        "}"
    );
    connect(m_productList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        int idx = m_productList->row(item);
        if (idx >= 0 && idx < m_filteredProducts.size()) {
            QVariantMap prod = m_filteredProducts[idx];
            // Add to current session
            SaleItem si;
            si.productId = prod["id"].toInt();
            si.productCode = prod["code"].toString();
            si.productName = prod["name"].toString();
            si.unitPrice = prod["sale_price"].toDouble();
            si.quantity = 1.0;
            si.sum = si.unitPrice * si.quantity;
            m_sessions[m_currentSessionIndex].items.append(si);
            refreshTable();
            updateTotals();
        }
        });
    leftLayout->addWidget(m_productList, 1);

    outerLayout->addWidget(leftPanel);

    // ==================== CENTER PANEL: Main Sale Area ====================
    QWidget* centerPanel = new QWidget(this);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setSpacing(15);
    centerLayout->setContentsMargins(25, 20, 25, 20);

    // --- Title ---
    QLabel* title = new QLabel(tr("New Sale / Accounting"));
    title->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: #1a1a2e; padding-bottom: 10px;"
    );
    centerLayout->addWidget(title);

    // --- Sale Slots (3 tabs) ---
    CardWidget* slotsCard = new CardWidget(this);
    QHBoxLayout* slotsLayout = new QHBoxLayout(slotsCard);
    slotsLayout->setSpacing(12);
    slotsLayout->setContentsMargins(15, 12, 15, 12);

    for (int i = 0; i < 3; ++i) {
        QVBoxLayout* slotVLayout = new QVBoxLayout();
        slotVLayout->setSpacing(4);
        slotVLayout->setContentsMargins(0, 0, 0, 0);

        m_slotBtns[i] = new QPushButton(tr("Sale %1").arg(i + 1));
        m_slotBtns[i]->setCheckable(true);
        m_slotBtns[i]->setMinimumHeight(44);
        m_slotBtns[i]->setStyleSheet(
            "QPushButton {"
            "  background-color: #e9ecef;"
            "  color: #495057;"
            "  border: none;"
            "  border-radius: 10px;"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "  padding: 10px 20px;"
            "}"
            "QPushButton:hover { background-color: #dee2e6; }"
            "QPushButton:checked {"
            "  background-color: #16213e;"
            "  color: white;"
            "}"
        );
        connect(m_slotBtns[i], &QPushButton::clicked, this, [this, i]() {
            onSaleSlotClicked(i);
            });
        slotVLayout->addWidget(m_slotBtns[i]);

        m_slotLabels[i] = new QLabel(tr("Empty"));
        m_slotLabels[i]->setAlignment(Qt::AlignCenter);
        m_slotLabels[i]->setStyleSheet("font-size: 11px; color: #6c757d;");
        slotVLayout->addWidget(m_slotLabels[i]);

        slotsLayout->addLayout(slotVLayout);
    }
    slotsLayout->addStretch();
    centerLayout->addWidget(slotsCard);

    // --- Navigation Bar (Prev / Next / Edit Code) ---
    CardWidget* navCard = new CardWidget(this);
    QHBoxLayout* navLayout = new QHBoxLayout(navCard);
    navLayout->setSpacing(12);
    navLayout->setContentsMargins(15, 10, 15, 10);

    m_prevBtn = new QPushButton(tr("Previous Sale"));
    m_prevBtn->setStyleSheet(
        "QPushButton { background-color: #6c757d; color: white; padding: 8px 18px;"
        "border-radius: 8px; font-weight: 600; border: none; font-size: 12px;}"
        "QPushButton:hover { background-color: #5a6268; }"
    );
    connect(m_prevBtn, &QPushButton::clicked, this, &SalesPage::onPrevSaleClicked);

    m_nextBtn = new QPushButton(tr("Next Sale"));
    m_nextBtn->setStyleSheet(
        "QPushButton { background-color: #6c757d; color: white; padding: 8px 18px;"
        "border-radius: 8px; font-weight: 600; border: none; font-size: 12px;}"
        "QPushButton:hover { background-color: #5a6268; }"
    );
    connect(m_nextBtn, &QPushButton::clicked, this, &SalesPage::onNextSaleClicked);

    QLabel* codeLabel = new QLabel(tr("Edit by Code:"));
    codeLabel->setStyleSheet("font-weight: 600; color: #495057; font-size: 13px;");

    m_codeEdit = new StyledLineEdit(this);
    m_codeEdit->setPlaceholderText(tr("Enter code..."));
    m_codeEdit->setFixedWidth(120);

    m_editCodeBtn = new QPushButton(tr("Unlock"));
    m_editCodeBtn->setStyleSheet(
        "QPushButton { background-color: #16213e; color: white; padding: 8px 18px;"
        "border-radius: 8px; font-weight: 600; border: none; font-size: 12px;}"
        "QPushButton:hover { background-color: #0f3460; }"
    );
    connect(m_editCodeBtn, &QPushButton::clicked, this, &SalesPage::onEditByCodeClicked);

    navLayout->addWidget(m_prevBtn);
    navLayout->addWidget(m_nextBtn);
    navLayout->addStretch();
    navLayout->addWidget(codeLabel);
    navLayout->addWidget(m_codeEdit);
    navLayout->addWidget(m_editCodeBtn);
    centerLayout->addWidget(navCard);

    // --- Payment & Account Row ---
    CardWidget* paymentCard = new CardWidget(this);
    QHBoxLayout* paymentLayout = new QHBoxLayout(paymentCard);
    paymentLayout->setSpacing(20);
    paymentLayout->setContentsMargins(15, 12, 15, 12);

    QLabel* payLabel = new QLabel(tr("Payment Type:"));
    payLabel->setStyleSheet("font-weight: 600; color: #495057; font-size: 13px;");
    paymentLayout->addWidget(payLabel);

    m_paymentGroup = new QButtonGroup(this);
    m_cashRadio = new QRadioButton(tr("Cash"));
    m_debtRadio = new QRadioButton(tr("Debt"));
    m_cashRadio->setChecked(true);
    m_paymentGroup->addButton(m_cashRadio);
    m_paymentGroup->addButton(m_debtRadio);

    QString radioStyle =
        "QRadioButton { font-size: 13px; color: #495057; spacing: 6px; }"
        "QRadioButton::indicator { width: 18px; height: 18px; }";
    m_cashRadio->setStyleSheet(radioStyle);
    m_debtRadio->setStyleSheet(radioStyle);

    paymentLayout->addWidget(m_cashRadio);
    paymentLayout->addWidget(m_debtRadio);
    paymentLayout->addSpacing(30);

    QLabel* accLabel = new QLabel(tr("Account:"));
    accLabel->setStyleSheet("font-weight: 600; color: #495057; font-size: 13px;");
    paymentLayout->addWidget(accLabel);

    m_accountCombo = new StyledComboBox(this);
    m_accountCombo->setMinimumWidth(200);
    connect(m_accountCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SalesPage::onAccountChanged);
    paymentLayout->addWidget(m_accountCombo);
    paymentLayout->addStretch();

    connect(m_paymentGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &SalesPage::onPaymentTypeChanged);
    centerLayout->addWidget(paymentCard);

    // --- Table Search ---
    QHBoxLayout* tableSearchLayout = new QHBoxLayout();
    m_tableSearch = new StyledLineEdit(this);
    m_tableSearch->setPlaceholderText(tr("Search in table by product name or code..."));
    tableSearchLayout->addWidget(m_tableSearch, 1);
    centerLayout->addLayout(tableSearchLayout);
    connect(m_tableSearch, &QLineEdit::textChanged, this, &SalesPage::onSearchTableChanged);

    // --- Sale Items Table ---
    CardWidget* tableCard = new CardWidget(this);
    QVBoxLayout* tableCardLayout = new QVBoxLayout(tableCard);
    tableCardLayout->setContentsMargins(15, 15, 15, 15);

    m_table = new QTableWidget(this);
    m_tableHeaders << tr("Product Code") << tr("Product Name") << tr("Quantity") << tr("Unit Price") << tr("Sum");
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels(m_tableHeaders);
    m_table->setStyleSheet(
        "QTableWidget {"
        "  background-color: white;"
        "  border: 1px solid #e9ecef;"
        "  border-radius: 10px;"
        "  gridline-color: #f1f3f5;"
        "  font-size: 13px;"
        "}"
        "QTableWidget::item { padding: 8px 12px; }"
        "QTableWidget::item:selected { background-color: #16213e; color: white; }"
        "QHeaderView::section {"
        "  background-color: #f8f9fa;"
        "  padding: 10px 12px;"
        "  border: none;"
        "  border-bottom: 2px solid #16213e;"
        "  font-weight: 600;"
        "  color: #1a1a2e;"
        "  font-size: 13px;"
        "}"
        "QTableWidget QLineEdit {"
        "  border: 1px solid #e9ecef;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "  background: white;"
        "}"
    );
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_table->verticalHeader()->setVisible(false);
    connect(m_table, &QTableWidget::cellChanged, this, &SalesPage::onTableCellChanged);
    connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SalesPage::onTableItemSelectionChanged);
    tableCardLayout->addWidget(m_table);
    centerLayout->addWidget(tableCard, 1);

    // --- Action Buttons Row ---
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(12);

    m_addBtn = new QPushButton(tr("+ Add Product"));
    m_addBtn->setStyleSheet(
        "QPushButton { background-color: #28a745; color: white; padding: 10px 22px;"
        "border-radius: 8px; font-weight: 600; border: none; font-size: 13px;}"
        "QPushButton:hover { background-color: #218838; }"
    );
    connect(m_addBtn, &QPushButton::clicked, this, &SalesPage::onAddProductClicked);

    m_deleteBtn = new QPushButton(tr("Delete Row"));
    m_deleteBtn->setStyleSheet(
        "QPushButton { background-color: #dc3545; color: white; padding: 10px 22px;"
        "border-radius: 8px; font-weight: 600; border: none; font-size: 13px;}"
        "QPushButton:hover { background-color: #c82333; }"
    );
    connect(m_deleteBtn, &QPushButton::clicked, this, &SalesPage::onDeleteRowClicked);

    m_changePriceBtn = new QPushButton(tr("Change Price"));
    m_changePriceBtn->setStyleSheet(
        "QPushButton { background-color: #ffc107; color: #1a1a2e; padding: 10px 22px;"
        "border-radius: 8px; font-weight: 600; border: none; font-size: 13px;}"
        "QPushButton:hover { background-color: #e0a800; }"
    );
    connect(m_changePriceBtn, &QPushButton::clicked, this, &SalesPage::onChangePriceClicked);

    m_newSaleBtn = new QPushButton(tr("New Sale"));
    m_newSaleBtn->setStyleSheet(
        "QPushButton { background-color: #6c757d; color: white; padding: 10px 22px;"
        "border-radius: 8px; font-weight: 600; border: none; font-size: 13px;}"
        "QPushButton:hover { background-color: #5a6268; }"
    );
    connect(m_newSaleBtn, &QPushButton::clicked, this, &SalesPage::onNewSaleClicked);

    m_saveSaleBtn = new QPushButton(tr("Save Sale"));
    m_saveSaleBtn->setStyleSheet(
        "QPushButton { background-color: #16213e; color: white; padding: 10px 28px;"
        "border-radius: 8px; font-weight: 600; border: none; font-size: 13px;}"
        "QPushButton:hover { background-color: #0f3460; }"
    );
    connect(m_saveSaleBtn, &QPushButton::clicked, this, &SalesPage::onSaveSaleClicked);

    actionLayout->addWidget(m_addBtn);
    actionLayout->addWidget(m_deleteBtn);
    actionLayout->addWidget(m_changePriceBtn);
    actionLayout->addStretch();
    actionLayout->addWidget(m_newSaleBtn);
    actionLayout->addWidget(m_saveSaleBtn);
    centerLayout->addLayout(actionLayout);

    // --- Totals ---
    CardWidget* totalsCard = new CardWidget(this);
    QHBoxLayout* totalsLayout = new QHBoxLayout(totalsCard);
    totalsLayout->setSpacing(30);
    totalsLayout->setContentsMargins(20, 15, 20, 15);

    m_totalLbpLabel = new QLabel(tr("Total (LBP): 0"));
    m_totalLbpLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #1a1a2e;");

    m_totalUsdLabel = new QLabel(tr("Total (USD): $0.00"));
    m_totalUsdLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #28a745;");

    totalsLayout->addWidget(m_totalLbpLabel);
    totalsLayout->addWidget(m_totalUsdLabel);
    totalsLayout->addStretch();
    centerLayout->addWidget(totalsCard);

    // --- Product History Panel ---
    CardWidget* historyCard = new CardWidget(this);
    QVBoxLayout* historyLayout = new QVBoxLayout(historyCard);
    historyLayout->setSpacing(10);
    historyLayout->setContentsMargins(15, 15, 15, 15);

    QLabel* historyTitle = new QLabel(tr("Product Sale History"));
    historyTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1a1a2e; padding-bottom: 6px; border-bottom: 1px solid #e9ecef;");
    historyLayout->addWidget(historyTitle);

    QGridLayout* historyGrid = new QGridLayout();
    historyGrid->setSpacing(10);

    auto addHistoryLabel = [&](const QString& text, int row, int col) {
        QLabel* lbl = new QLabel(text);
        lbl->setStyleSheet("font-weight: 600; color: #495057; font-size: 12px;");
        historyGrid->addWidget(lbl, row, col);
        };

    addHistoryLabel(tr("Product:"), 0, 0);
    m_historyProductLabel = new QLabel(tr("--"));
    m_historyProductLabel->setStyleSheet("font-size: 12px; color: #212529;");
    historyGrid->addWidget(m_historyProductLabel, 0, 1);

    addHistoryLabel(tr("Total Sold:"), 0, 2);
    m_historyTotalSoldLabel = new QLabel(tr("0"));
    m_historyTotalSoldLabel->setStyleSheet("font-size: 12px; color: #212529;");
    historyGrid->addWidget(m_historyTotalSoldLabel, 0, 3);

    addHistoryLabel(tr("Remaining Qty:"), 1, 0);
    m_historyRemainingLabel = new QLabel(tr("0"));
    m_historyRemainingLabel->setStyleSheet("font-size: 12px; color: #212529;");
    historyGrid->addWidget(m_historyRemainingLabel, 1, 1);

    addHistoryLabel(tr("Avg Sale Price:"), 1, 2);
    m_historyAvgPriceLabel = new QLabel(tr("$0.00"));
    m_historyAvgPriceLabel->setStyleSheet("font-size: 12px; color: #212529;");
    historyGrid->addWidget(m_historyAvgPriceLabel, 1, 3);

    historyLayout->addLayout(historyGrid);
    centerLayout->addWidget(historyCard);

    outerLayout->addWidget(centerPanel, 1);
}

// ============================================================
// Data Loading
// ============================================================
void SalesPage::loadProductTypes() {
    m_typeList->clear();
    QListWidgetItem* allItem = new QListWidgetItem(tr("All Products"));
    allItem->setData(Qt::UserRole, -1);
    m_typeList->addItem(allItem);

    QSqlQuery query("SELECT id, name FROM product_categories ORDER BY name");
    while (query.next()) {
        QListWidgetItem* item = new QListWidgetItem(query.value("name").toString());
        item->setData(Qt::UserRole, query.value("id"));
        m_typeList->addItem(item);
    }
    m_typeList->setCurrentRow(0);
}

void SalesPage::loadProductsByType(int typeId) {
    m_allProducts.clear();
    QString sql;
    if (typeId == -1) {
        sql = "SELECT id, code, name, sale_price, quantity, unit FROM products WHERE is_active = 1 ORDER BY name";
    }
    else {
        sql = QString("SELECT id, code, name, sale_price, quantity, unit FROM products "
            "WHERE category_id = %1 AND is_active = 1 ORDER BY name").arg(typeId);
    }
    QSqlQuery query(sql);
    while (query.next()) {
        QVariantMap prod;
        prod["id"] = query.value("id").toInt();
        prod["code"] = query.value("code").toString();
        prod["name"] = query.value("name").toString();
        prod["sale_price"] = query.value("sale_price").toDouble();
        prod["quantity"] = query.value("quantity").toInt();
        prod["unit"] = query.value("unit").toString();
        m_allProducts.append(prod);
    }
    m_filteredProducts = m_allProducts;
    refreshProductListWidget();
}

void SalesPage::loadAccounts() {
    m_accountCombo->clear();
    // Store account (id = 1 typically)
    m_accountCombo->addItem(tr("Store Account"), 1);
    // Customer accounts
    QSqlQuery query("SELECT id, name FROM customers WHERE is_active = 1 ORDER BY name");
    while (query.next()) {
        m_accountCombo->addItem(query.value("name").toString(), query.value("id"));
    }
    // Also add supplier accounts if needed
    QSqlQuery suppQuery("SELECT id, name FROM suppliers WHERE is_active = 1 ORDER BY name");
    while (suppQuery.next()) {
        m_accountCombo->addItem(tr("Supplier: %1").arg(suppQuery.value("name").toString()), suppQuery.value("id"));
    }
    onAccountChanged(0);
}

void SalesPage::loadAllProducts() {
    loadProductsByType(-1);
}

void SalesPage::refreshProductListWidget() {
    m_productList->clear();
    for (const auto& prod : m_filteredProducts) {
        QString text = QString("%1 | %2 | $%3 | Qty: %4 %5")
            .arg(prod["code"].toString())
            .arg(prod["name"].toString())
            .arg(prod["sale_price"].toDouble(), 0, 'f', 2)
            .arg(prod["quantity"].toInt())
            .arg(prod["unit"].toString());
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, prod["id"]);
        m_productList->addItem(item);
    }
}

// ============================================================
// Table Management
// ============================================================
void SalesPage::refreshTable() {
    m_table->blockSignals(true);
    m_table->clearContents();
    const SaleSession& session = m_sessions[m_currentSessionIndex];
    m_table->setRowCount(session.items.size());

    for (int i = 0; i < session.items.size(); ++i) {
        const SaleItem& item = session.items[i];
        m_table->setItem(i, 0, new QTableWidgetItem(item.productCode));
        m_table->setItem(i, 1, new QTableWidgetItem(item.productName));
        m_table->setItem(i, 2, new QTableWidgetItem(QString::number(item.quantity)));
        m_table->setItem(i, 3, new QTableWidgetItem(QString::number(item.unitPrice, 'f', 2)));
        m_table->setItem(i, 4, new QTableWidgetItem(QString::number(item.sum, 'f', 2)));
    }

    m_table->resizeColumnsToContents();
    m_table->blockSignals(false);
}

void SalesPage::updateTotals() {
    double totalUsd = 0.0;
    const SaleSession& session = m_sessions[m_currentSessionIndex];
    for (const auto& item : session.items) {
        totalUsd += item.sum;
    }
    double totalLbp = totalUsd * m_exchangeRate;

    m_totalLbpLabel->setText(tr("Total (LBP): %1").arg(QString::number(totalLbp, 'f', 0)));
    m_totalUsdLabel->setText(tr("Total (USD): $%1").arg(QString::number(totalUsd, 'f', 2)));
}

// ============================================================
// Product History
// ============================================================
void SalesPage::updateProductHistory(int productId) {
    if (productId <= 0) {
        m_historyProductLabel->setText(tr("--"));
        m_historyTotalSoldLabel->setText(tr("0"));
        m_historyRemainingLabel->setText(tr("0"));
        m_historyAvgPriceLabel->setText(tr("$0.00"));
        return;
    }

    QSqlQuery prodQuery;
    prodQuery.prepare("SELECT name, quantity FROM products WHERE id = :id");
    prodQuery.bindValue(":id", productId);
    QString productName = tr("Unknown");
    int remainingQty = 0;
    if (prodQuery.exec() && prodQuery.next()) {
        productName = prodQuery.value("name").toString();
        remainingQty = prodQuery.value("quantity").toInt();
    }

    QSqlQuery soldQuery;
    soldQuery.prepare(
        "SELECT COALESCE(SUM(si.quantity), 0) as total_sold, "
        "COALESCE(AVG(si.unit_price), 0) as avg_price "
        "FROM sale_items si "
        "JOIN sales s ON si.sale_id = s.id "
        "WHERE si.product_id = :pid"
    );
    soldQuery.bindValue(":pid", productId);
    double totalSold = 0;
    double avgPrice = 0;
    if (soldQuery.exec() && soldQuery.next()) {
        totalSold = soldQuery.value("total_sold").toDouble();
        avgPrice = soldQuery.value("avg_price").toDouble();
    }

    m_historyProductLabel->setText(productName);
    m_historyTotalSoldLabel->setText(QString::number(totalSold, 'f', 2));
    m_historyRemainingLabel->setText(QString::number(remainingQty));
    m_historyAvgPriceLabel->setText(tr("$%1").arg(QString::number(avgPrice, 'f', 2)));
}

// ============================================================
// Session Management
// ============================================================
void SalesPage::switchToSession(int index) {
    if (index < 0 || index >= 3) return;

    m_currentSessionIndex = index;
    for (int i = 0; i < 3; ++i) {
        m_slotBtns[i]->setChecked(i == index);
        if (m_sessions[i].saleCode > 0) {
            m_slotLabels[i]->setText(tr("Code: %1").arg(m_sessions[i].saleCode));
        }
        else {
            m_slotLabels[i]->setText(tr("Empty"));
        }
    }

    SaleSession& session = m_sessions[index];

    // Restore payment type
    if (session.paymentType == "cash") {
        m_cashRadio->setChecked(true);
    }
    else {
        m_debtRadio->setChecked(true);
    }

    // Restore account
    int accIdx = m_accountCombo->findData(session.accountId);
    if (accIdx >= 0) {
        m_accountCombo->setCurrentIndex(accIdx);
    }

    refreshTable();
    updateTotals();
    m_selectedProductId = -1;
    updateProductHistory(-1);
}

void SalesPage::generateSaleCode() {
    SaleSession& session = m_sessions[m_currentSessionIndex];
    if (session.saleCode <= 0) {
        session.saleCode = getNextAvailableSaleCode();
    }
}

int SalesPage::getNextAvailableSaleCode() {
    QSqlQuery query("SELECT COALESCE(MAX(code), 999) as max_code FROM sales");
    int maxCode = 999;
    if (query.exec() && query.next()) {
        maxCode = query.value("max_code").toInt();
    }
    // Also check in-memory sessions
    for (const auto& s : m_sessions) {
        if (s.saleCode > maxCode) {
            maxCode = s.saleCode;
        }
    }
    return maxCode + 1;
}

// ============================================================
// Slots
// ============================================================
void SalesPage::onProductTypeClicked(QListWidgetItem* item) {
    if (!item) return;
    int typeId = item->data(Qt::UserRole).toInt();
    m_currentTypeId = typeId;
    loadProductsByType(typeId);
}

void SalesPage::onAddProductClicked() {
    // Open a dialog to search and add product
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Add Product to Sale"));
    dialog.setMinimumWidth(450);
    dialog.setStyleSheet("QDialog { background-color: #F8F9FA; }");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 20, 20, 20);

    StyledLineEdit* search = new StyledLineEdit(&dialog);
    search->setPlaceholderText(tr("Type product name or code..."));
    layout->addWidget(search);

    QListWidget* list = new QListWidget(&dialog);
    list->setStyleSheet(
        "QListWidget { background-color: white; border: 1px solid #e9ecef; border-radius: 8px; padding: 8px; }"
        "QListWidget::item { padding: 10px; border-bottom: 1px solid #f1f3f5; }"
        "QListWidget::item:selected { background-color: #16213e; color: white; }"
    );
    layout->addWidget(list, 1);

    // Populate list
    for (const auto& prod : m_allProducts) {
        QString text = QString("%1 | %2 | $%3 | Stock: %4")
            .arg(prod["code"].toString())
            .arg(prod["name"].toString())
            .arg(prod["sale_price"].toDouble(), 0, 'f', 2)
            .arg(prod["quantity"].toInt());
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, prod["id"]);
        list->addItem(item);
    }

    auto filterList = [&](const QString& text) {
        list->clear();
        for (const auto& prod : m_allProducts) {
            if (prod["name"].toString().contains(text, Qt::CaseInsensitive) ||
                prod["code"].toString().contains(text, Qt::CaseInsensitive)) {
                QString itemText = QString("%1 | %2 | $%3 | Stock: %4")
                    .arg(prod["code"].toString())
                    .arg(prod["name"].toString())
                    .arg(prod["sale_price"].toDouble(), 0, 'f', 2)
                    .arg(prod["quantity"].toInt());
                QListWidgetItem* item = new QListWidgetItem(itemText);
                item->setData(Qt::UserRole, prod["id"]);
                list->addItem(item);
            }
        }
        };
    connect(search, &QLineEdit::textChanged, filterList);

    QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    btnBox->setStyleSheet(
        "QPushButton { background-color: #16213e; color: white; padding: 10px 24px;"
        "border-radius: 8px; font-weight: 600; border: none; }"
        "QPushButton:hover { background-color: #0f3460; }"
    );
    connect(btnBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(btnBox);

    if (dialog.exec() == QDialog::Accepted && list->currentItem()) {
        int pid = list->currentItem()->data(Qt::UserRole).toInt();
        for (const auto& prod : m_allProducts) {
            if (prod["id"].toInt() == pid) {
                SaleItem si;
                si.productId = pid;
                si.productCode = prod["code"].toString();
                si.productName = prod["name"].toString();
                si.unitPrice = prod["sale_price"].toDouble();
                si.quantity = 1.0;
                si.sum = si.unitPrice;
                m_sessions[m_currentSessionIndex].items.append(si);
                refreshTable();
                updateTotals();
                break;
            }
        }
    }
}

void SalesPage::onDeleteRowClicked() {
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a row to delete"));
        return;
    }
    auto reply = QMessageBox::question(this, tr("Confirm"), tr("Delete this item?"));
    if (reply == QMessageBox::Yes) {
        m_sessions[m_currentSessionIndex].items.removeAt(row);
        refreshTable();
        updateTotals();
    }
}

void SalesPage::onTableCellChanged(int row, int col) {
    if (row < 0 || row >= m_sessions[m_currentSessionIndex].items.size()) return;
    if (col != 2 && col != 3) return; // Only quantity and price columns are editable

    SaleItem& item = m_sessions[m_currentSessionIndex].items[row];
    QTableWidgetItem* cell = m_table->item(row, col);
    if (!cell) return;

    bool ok;
    double val = cell->text().toDouble(&ok);
    if (!ok || val < 0) {
        refreshTable(); // Revert
        return;
    }

    if (col == 2) {
        item.quantity = val;
    }
    else if (col == 3) {
        item.unitPrice = val;
    }
    item.sum = item.quantity * item.unitPrice;

    m_table->blockSignals(true);
    m_table->item(row, 4)->setText(QString::number(item.sum, 'f', 2));
    m_table->blockSignals(false);
    updateTotals();
}

void SalesPage::onTableItemSelectionChanged() {
    int row = m_table->currentRow();
    if (row >= 0 && row < m_sessions[m_currentSessionIndex].items.size()) {
        m_selectedProductId = m_sessions[m_currentSessionIndex].items[row].productId;
        updateProductHistory(m_selectedProductId);
    }
}

void SalesPage::onPaymentTypeChanged(QAbstractButton* btn) {
    SaleSession& session = m_sessions[m_currentSessionIndex];
    if (btn == m_cashRadio) {
        session.paymentType = "cash";
    }
    else {
        session.paymentType = "debt";
    }

    // Enforce rules based on account
    int accountId = m_accountCombo->currentData().toInt();
    bool isStoreAccount = (m_accountCombo->currentIndex() == 0); // Store Account is first

    if (isStoreAccount) {
        // Store account: only cash allowed
        m_cashRadio->setChecked(true);
        m_debtRadio->setEnabled(false);
        session.paymentType = "cash";
    }
    else {
        // Customer/Supplier accounts: only debt allowed
        m_debtRadio->setChecked(true);
        m_cashRadio->setEnabled(false);
        m_debtRadio->setEnabled(true);
        session.paymentType = "debt";
    }
}

void SalesPage::onAccountChanged(int index) {
    if (index < 0) return;
    int accountId = m_accountCombo->itemData(index).toInt();
    m_sessions[m_currentSessionIndex].accountId = accountId;
    m_sessions[m_currentSessionIndex].accountName = m_accountCombo->currentText();

    bool isStoreAccount = (index == 0);

    if (isStoreAccount) {
        // Store account: only cash
        m_cashRadio->setChecked(true);
        m_cashRadio->setEnabled(true);
        m_debtRadio->setEnabled(false);
        m_sessions[m_currentSessionIndex].paymentType = "cash";
    }
    else {
        // Customer/Supplier: only debt
        m_debtRadio->setChecked(true);
        m_cashRadio->setEnabled(false);
        m_debtRadio->setEnabled(true);
        m_sessions[m_currentSessionIndex].paymentType = "debt";
    }
}

void SalesPage::onSearchProductChanged(const QString& text) {
    m_filteredProducts.clear();
    if (text.isEmpty()) {
        m_filteredProducts = m_allProducts;
    }
    else {
        for (const auto& prod : m_allProducts) {
            if (prod["name"].toString().contains(text, Qt::CaseInsensitive) ||
                prod["code"].toString().contains(text, Qt::CaseInsensitive)) {
                m_filteredProducts.append(prod);
            }
        }
    }
    refreshProductListWidget();
}

void SalesPage::onSearchTableChanged(const QString& text) {
    for (int i = 0; i < m_table->rowCount(); ++i) {
        bool match = false;
        for (int j = 0; j < m_table->columnCount(); ++j) {
            QTableWidgetItem* item = m_table->item(i, j);
            if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }
        m_table->setRowHidden(i, !match);
    }
}

void SalesPage::onPrevSaleClicked() {
    int prev = m_currentSessionIndex - 1;
    if (prev < 0) prev = 2;
    switchToSession(prev);
}

void SalesPage::onNextSaleClicked() {
    int next = m_currentSessionIndex + 1;
    if (next > 2) next = 0;
    switchToSession(next);
}

void SalesPage::onEditByCodeClicked() {
    QString code = m_codeEdit->text().trimmed();
    if (code.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please enter a sale code"));
        return;
    }

    // Hardcoded unlock code for now
    if (code == "123") {
        QMessageBox::information(this, tr("Success"), tr("Edit mode unlocked!"));
        // In a real app, this would load a saved sale from DB by code
        return;
    }

    // Try to find the sale code in DB
    QSqlQuery query;
    query.prepare("SELECT id, code, payment_type, account_id FROM sales WHERE code = :code");
    query.bindValue(":code", code.toInt());
    if (query.exec() && query.next()) {
        // Load sale into current session
        int saleId = query.value("id").toInt();
        SaleSession& session = m_sessions[m_currentSessionIndex];
        session.saleCode = query.value("code").toInt();
        session.paymentType = query.value("payment_type").toString();
        session.accountId = query.value("account_id").toInt();
        session.items.clear();

        QSqlQuery itemQuery;
        itemQuery.prepare("SELECT product_id, product_code, product_name, quantity, unit_price, total FROM sale_items WHERE sale_id = :sid");
        itemQuery.bindValue(":sid", saleId);
        if (itemQuery.exec()) {
            while (itemQuery.next()) {
                SaleItem si;
                si.productId = itemQuery.value("product_id").toInt();
                si.productCode = itemQuery.value("product_code").toString();
                si.productName = itemQuery.value("product_name").toString();
                si.quantity = itemQuery.value("quantity").toDouble();
                si.unitPrice = itemQuery.value("unit_price").toDouble();
                si.sum = itemQuery.value("total").toDouble();
                session.items.append(si);
            }
        }

        // Restore account combo
        int accIdx = m_accountCombo->findData(session.accountId);
        if (accIdx >= 0) m_accountCombo->setCurrentIndex(accIdx);

        // Restore payment
        if (session.paymentType == "cash") {
            m_cashRadio->setChecked(true);
        }
        else {
            m_debtRadio->setChecked(true);
        }

        switchToSession(m_currentSessionIndex);
        QMessageBox::information(this, tr("Success"), tr("Sale #%1 loaded for editing").arg(session.saleCode));
    }
    else {
        QMessageBox::warning(this, tr("Not Found"), tr("Sale code not found"));
    }
}

void SalesPage::onSaleSlotClicked(int slotIndex) {
    switchToSession(slotIndex);
}

void SalesPage::onChangePriceClicked() {
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a product row first"));
        return;
    }

    SaleItem& item = m_sessions[m_currentSessionIndex].items[row];
    PriceEditDialog dialog(item.productId, item.productName, item.unitPrice, this);
    if (dialog.exec() == QDialog::Accepted) {
        double newPrice = dialog.newPrice();
        item.unitPrice = newPrice;
        item.sum = item.quantity * newPrice;

        // Update DB
        QSqlQuery query;
        query.prepare("UPDATE products SET sale_price = :price WHERE id = :id");
        query.bindValue(":price", newPrice);
        query.bindValue(":id", item.productId);
        if (!query.exec()) {
            QMessageBox::critical(this, tr("Error"), query.lastError().text());
        }

        refreshTable();
        updateTotals();
        QMessageBox::information(this, tr("Success"), tr("Price updated to $%1").arg(newPrice, 0, 'f', 2));
    }
}

void SalesPage::onSaveSaleClicked() {
    SaleSession& session = m_sessions[m_currentSessionIndex];
    if (session.items.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No items to save"));
        return;
    }

    generateSaleCode();

    double totalUsd = 0.0;
    for (const auto& item : session.items) {
        totalUsd += item.sum;
    }

    if (!DbManager::instance()->transaction()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to start transaction"));
        return;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO sales (code, payment_type, account_id, account_name, total_amount, "
        "discount, tax_amount, net_amount, created_at) "
        "VALUES (:code, :payment, :acc_id, :acc_name, :total, 0, 0, :net, NOW())"
    );
    query.bindValue(":code", session.saleCode);
    query.bindValue(":payment", session.paymentType);
    query.bindValue(":acc_id", session.accountId);
    query.bindValue(":acc_name", session.accountName);
    query.bindValue(":total", totalUsd);
    query.bindValue(":net", totalUsd);

    if (!query.exec()) {
        DbManager::instance()->rollback();
        QMessageBox::critical(this, tr("Error"), query.lastError().text());
        return;
    }

    int saleId = query.lastInsertId().toInt();

    for (const auto& item : session.items) {
        QSqlQuery itemQuery;
        itemQuery.prepare(
            "INSERT INTO sale_items (sale_id, product_id, product_code, product_name, "
            "quantity, unit_price, total) "
            "VALUES (:sid, :pid, :code, :name, :qty, :price, :total)"
        );
        itemQuery.bindValue(":sid", saleId);
        itemQuery.bindValue(":pid", item.productId);
        itemQuery.bindValue(":code", item.productCode);
        itemQuery.bindValue(":name", item.productName);
        itemQuery.bindValue(":qty", item.quantity);
        itemQuery.bindValue(":price", item.unitPrice);
        itemQuery.bindValue(":total", item.sum);

        if (!itemQuery.exec()) {
            DbManager::instance()->rollback();
            QMessageBox::critical(this, tr("Error"), itemQuery.lastError().text());
            return;
        }

        // Update product stock
        QSqlQuery stockQuery;
        stockQuery.prepare("UPDATE products SET quantity = quantity - :qty WHERE id = :id");
        stockQuery.bindValue(":qty", item.quantity);
        stockQuery.bindValue(":id", item.productId);
        stockQuery.exec();
    }

    // Create debt record if debt payment
    if (session.paymentType == "debt") {
        QSqlQuery debtQuery;
        debtQuery.prepare(
            "INSERT INTO debts (party_type, party_id, amount, debt_type, description, due_date, is_paid) "
            "VALUES (:party_type, :party_id, :amount, 'owed', :desc, DATE_ADD(NOW(), INTERVAL 30 DAY), 0)"
        );
        QString partyType = (session.accountName.startsWith(tr("Supplier"))) ? "supplier" : "customer";
        debtQuery.bindValue(":party_type", partyType);
        debtQuery.bindValue(":party_id", session.accountId);
        debtQuery.bindValue(":amount", totalUsd);
        debtQuery.bindValue(":desc", tr("Sale #%1").arg(session.saleCode));
        debtQuery.exec();
    }

    if (!DbManager::instance()->commit()) {
        DbManager::instance()->rollback();
        QMessageBox::critical(this, tr("Error"), tr("Failed to commit transaction"));
        return;
    }

    session.isSaved = true;
    m_slotLabels[m_currentSessionIndex]->setText(tr("Code: %1 (Saved)").arg(session.saleCode));

    QMessageBox::information(this, tr("Success"),
        tr("Sale #%1 saved successfully!\nTotal: $%2")
        .arg(session.saleCode)
        .arg(totalUsd, 0, 'f', 2));
}

void SalesPage::onNewSaleClicked() {
    auto reply = QMessageBox::question(this, tr("Confirm"), tr("Start a new sale? Current data will be cleared."));
    if (reply == QMessageBox::Yes) {
        SaleSession& session = m_sessions[m_currentSessionIndex];
        session.items.clear();
        session.saleCode = -1;
        session.isSaved = false;
        session.paymentType = "cash";
        session.accountId = -1;
        m_accountCombo->setCurrentIndex(0);
        onAccountChanged(0);
        refreshTable();
        updateTotals();
        m_slotLabels[m_currentSessionIndex]->setText(tr("Empty"));
    }
}

void SalesPage::onProductSearchSelected(const QModelIndex& index) {
    // Not used directly; product list click handles it
    Q_UNUSED(index)
}

double SalesPage::getExchangeRate() const {
    return m_exchangeRate;
}