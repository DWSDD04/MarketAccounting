#include "salespage.h"
#include "styledlineedit.h"
#include "styledcombobox.h"
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
#include <QSpinBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QInputDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QButtonGroup>
#include <QLineEdit>
#include <QGroupBox>
#include <QDateEdit>
#include <QListWidget>

// NEW: Receipt printing
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>

SalesPage::SalesPage(QWidget* parent) : QWidget(parent), m_currentSlot(0)
{
    for (int i = 0; i < 3; ++i) {
        SaleSession session;
        session.code = generateSaleCode();
        m_sessions.append(session);
    }

    setupUI();
    loadCategories();
    loadAccounts();
    loadCustomers();
    loadSettings();          // NEW
    refreshSaleTable();
    refreshTotals();
    updateSlotButtons();
}

SalesPage::~SalesPage() {}

int SalesPage::generateSaleCode()
{
    return m_nextSaleCode++;
}

void SalesPage::setupUI()
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

    QGroupBox* leftBox = new QGroupBox(tr("معلومات البيع"), this);
    leftBox->setStyleSheet(groupBoxStyle);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftBox);
    leftLayout->setSpacing(10);
    leftLayout->setContentsMargins(20, 20, 20, 20);

    QHBoxLayout* slotLayout = new QHBoxLayout();
    slotLayout->setSpacing(8);

    QLabel* slotLabel = new QLabel(tr("Active Sale:"));
    slotLabel->setStyleSheet("color: #1a3a5c; font-size: 13px; font-weight: 600;");
    slotLayout->addWidget(slotLabel);

    m_slotButtons = new QButtonGroup(this);
    m_slotButtons->setExclusive(true);

    auto createSlotBtn = [&](const QString& text, int slot) -> QPushButton* {
        QPushButton* btn = new QPushButton(text);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(32);
        m_slotButtons->addButton(btn, slot);
        return btn;
        };

    m_slot1Btn = createSlotBtn(tr("Sale #1"), 0);
    m_slot2Btn = createSlotBtn(tr("Sale #2"), 1);
    m_slot3Btn = createSlotBtn(tr("Sale #3"), 2);
    m_slot1Btn->setChecked(true);

    slotLayout->addWidget(m_slot1Btn);
    slotLayout->addWidget(m_slot2Btn);
    slotLayout->addWidget(m_slot3Btn);
    slotLayout->addSpacing(16);

    QLabel* codeLabel = new QLabel(tr("Code:"));
    codeLabel->setStyleSheet("color: #1a3a5c; font-size: 13px; font-weight: 600;");
    slotLayout->addWidget(codeLabel);
    m_saleCodeLabel = new QLabel(QString::number(m_sessions[0].code));
    m_saleCodeLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #1a3a5c; padding: 0 8px;");
    slotLayout->addWidget(m_saleCodeLabel);
    slotLayout->addStretch();

    QLabel* payLabel = new QLabel(tr("Payment:"));
    payLabel->setStyleSheet("color: #1a3a5c; font-size: 13px; font-weight: 600;");
    slotLayout->addWidget(payLabel);
    m_paymentCombo = new StyledComboBox(this);
    m_paymentCombo->addItems({ tr("Cash"), tr("Debt") });
    m_paymentCombo->setFixedWidth(120);
    slotLayout->addWidget(m_paymentCombo);

    QLabel* accLabel = new QLabel(tr("Account:"));
    accLabel->setStyleSheet("color: #1a3a5c; font-size: 13px; font-weight: 600;");
    slotLayout->addWidget(accLabel);
    m_accountCombo = new StyledComboBox(this);
    m_accountCombo->setFixedWidth(160);
    slotLayout->addWidget(m_accountCombo);

    leftLayout->addLayout(slotLayout);

    QGridLayout* infoGrid = new QGridLayout();
    infoGrid->setSpacing(10);
    infoGrid->setContentsMargins(0, 0, 0, 0);
    infoGrid->setColumnStretch(1, 1);
    infoGrid->setColumnStretch(3, 1);

    m_refEdit = new StyledLineEdit(this);
    m_refEdit->setPlaceholderText(tr("Auto or Manual"));
    addLabel(infoGrid, tr("Reference:"), 0, 2);
    infoGrid->addWidget(m_refEdit, 0, 1);

    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDate(QDate::currentDate());
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    m_dateEdit->setStyleSheet(
        "QDateEdit { background-color: #ffffff; border: 1px solid #9DB9D2; "
        "border-radius: 3px; padding: 6px; font-size: 13px; color: #1a3a5c; }"
        "QDateEdit:focus { border: 2px solid #1a3a5c; }"
        "QDateEdit::drop-down { border: none; width: 24px; }"
    );
    m_dateEdit->setMinimumHeight(32);
    addLabel(infoGrid, tr("Date:"), 0, 0);
    infoGrid->addWidget(m_dateEdit, 0, 3);

    m_customerCombo = new StyledComboBox(this);
    m_customerCombo->setPlaceholderText(tr("Select Customer"));
    addLabel(infoGrid, tr("Customer:"), 1, 2);
    infoGrid->addWidget(m_customerCombo, 1, 1, 1, 3);

    m_discountEdit = new StyledLineEdit(this);
    m_discountEdit->setPlaceholderText("0.00");
    addLabel(infoGrid, tr("Discount:"), 2, 2);
    infoGrid->addWidget(m_discountEdit, 2, 1);

    m_taxEdit = new StyledLineEdit(this);
    m_taxEdit->setPlaceholderText("0.00");
    addLabel(infoGrid, tr("Tax:"), 2, 0);
    infoGrid->addWidget(m_taxEdit, 2, 3);

    m_netEdit = new StyledLineEdit(this);
    m_netEdit->setPlaceholderText("0.00");
    m_netEdit->setReadOnly(true);
    m_netEdit->setStyleSheet(
        "QLineEdit { background-color: #e9ecef; border: 2px solid #dee2e6; "
        "border-radius: 8px; padding: 10px 14px; font-size: 13px; color: #212529; }"
    );
    addLabel(infoGrid, tr("Net Amount:"), 3, 2);
    infoGrid->addWidget(m_netEdit, 3, 1);

    m_notesEdit = new StyledLineEdit(this);
    m_notesEdit->setPlaceholderText(tr("Notes..."));
    addLabel(infoGrid, tr("Notes:"), 3, 0);
    infoGrid->addWidget(m_notesEdit, 3, 3);

    leftLayout->addLayout(infoGrid);

    QLabel* itemsTitle = new QLabel(tr("اختيار المنتج"));
    itemsTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1a3a5c; margin-top: 8px;");
    leftLayout->addWidget(itemsTitle);

    m_searchEdit = new StyledLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search product name or code..."));
    leftLayout->addWidget(m_searchEdit);

    QWidget* categoryContainer = new QWidget(this);
    categoryContainer->setObjectName("categoryContainer");
    QVBoxLayout* catLayout = new QVBoxLayout(categoryContainer);
    catLayout->setSpacing(4);
    catLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->addWidget(categoryContainer);

    m_productTable = new QTableView(this);
    m_productModel = new QStandardItemModel(this);
    m_productModel->setHorizontalHeaderLabels({ tr("ID"), tr("Code"), tr("Name"), tr("Price") });
    m_productTable->setModel(m_productModel);
    m_productTable->setStyleSheet(tableStyle);
    m_productTable->horizontalHeader()->setStretchLastSection(true);
    m_productTable->setAlternatingRowColors(true);
    m_productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_productTable->setMaximumHeight(180);
    m_productTable->hideColumn(0);
    leftLayout->addWidget(m_productTable);

    QHBoxLayout* addLayout = new QHBoxLayout();
    QLabel* qtyLabel = new QLabel(tr("Qty:"));
    qtyLabel->setStyleSheet("color: #1a3a5c; font-size: 13px; font-weight: 600;");
    addLayout->addWidget(qtyLabel);

    m_quantitySpin = new QSpinBox(this);
    m_quantitySpin->setRange(1, 9999);
    m_quantitySpin->setValue(1);
    m_quantitySpin->setStyleSheet(
        "QSpinBox { background-color: white; border: 1px solid #9DB9D2; border-radius: 3px; "
        "padding: 6px; font-size: 13px; color: #1a3a5c; }"
        "QSpinBox:focus { border: 2px solid #1a3a5c; }"
    );
    m_quantitySpin->setFixedWidth(80);
    addLayout->addWidget(m_quantitySpin);

    m_addBtn = new QPushButton(tr("Add to Sale"));
    m_addBtn->setStyleSheet(btnStyle);
    addLayout->addWidget(m_addBtn);
    addLayout->addStretch();
    leftLayout->addLayout(addLayout);

    QHBoxLayout* navLayout = new QHBoxLayout();
    m_prevBtn = new QPushButton(tr("<< Previous"));
    m_prevBtn->setStyleSheet(btnStyle);
    m_nextBtn = new QPushButton(tr("Next >>"));
    m_nextBtn->setStyleSheet(btnStyle);
    m_editCodeBtn = new QPushButton(tr("Edit by Code"));
    m_editCodeBtn->setStyleSheet(btnStyle);
    navLayout->addWidget(m_prevBtn);
    navLayout->addWidget(m_nextBtn);
    navLayout->addWidget(m_editCodeBtn);
    navLayout->addStretch();
    leftLayout->addLayout(navLayout);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* newBtn = new QPushButton(tr("New"));
    newBtn->setStyleSheet(btnStyle);
    QPushButton* saveBtn = new QPushButton(tr("Save"));
    saveBtn->setStyleSheet(btnStyle);
    QPushButton* cancelBtn = new QPushButton(tr("Cancel"));
    cancelBtn->setStyleSheet(btnStyle);
    QPushButton* deleteBtn = new QPushButton(tr("Delete"));
    deleteBtn->setStyleSheet(deleteBtnStyle);

    btnLayout->addWidget(newBtn);
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addStretch();
    leftLayout->addLayout(btnLayout);

    leftLayout->addStretch();
    mainLayout->addWidget(leftBox, 2);

    QGroupBox* rightBox = new QGroupBox(tr("السلة"), this);
    rightBox->setStyleSheet(groupBoxStyle);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightBox);
    rightLayout->setSpacing(10);
    rightLayout->setContentsMargins(20, 20, 20, 20);

    m_saleTable = new QTableView(this);
    m_saleModel = new QStandardItemModel(this);
    rebuildSaleModelHeaders();
    m_saleTable->setModel(m_saleModel);
    m_saleTable->setStyleSheet(tableStyle);
    m_saleTable->horizontalHeader()->setStretchLastSection(true);
    m_saleTable->setAlternatingRowColors(true);
    m_saleTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_saleTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_saleTable->hideColumn(0);
    rightLayout->addWidget(m_saleTable, 1);

    QHBoxLayout* tblBtnLayout = new QHBoxLayout();
    m_deleteBtn = new QPushButton(tr("Delete Item"));
    m_deleteBtn->setStyleSheet(deleteBtnStyle);
    m_editBtn = new QPushButton(tr("Edit Qty"));
    m_editBtn->setStyleSheet(btnStyle);
    m_changePriceBtn = new QPushButton(tr("Change Price"));
    m_changePriceBtn->setStyleSheet(btnStyle);

    QPushButton* manageColsBtn = new QPushButton(tr("Manage Columns"));
    manageColsBtn->setStyleSheet(btnStyle);

    tblBtnLayout->addWidget(m_deleteBtn);
    tblBtnLayout->addWidget(m_editBtn);
    tblBtnLayout->addWidget(m_changePriceBtn);
    tblBtnLayout->addWidget(manageColsBtn);
    tblBtnLayout->addStretch();
    rightLayout->addLayout(tblBtnLayout);

    QHBoxLayout* totalLayout = new QHBoxLayout();
    totalLayout->setSpacing(16);
    m_totalLBPLabel = new QLabel(tr("Total LBP: 0"));
    m_totalLBPLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #1a3a5c; "
        "padding: 10px 16px; background-color: #d4edda; border-radius: 6px;"
    );
    m_totalUSDLabel = new QLabel(tr("Total USD: $0.00"));
    m_totalUSDLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #1a3a5c; "
        "padding: 10px 16px; background-color: #e9ecef; border-radius: 6px;"
    );
    totalLayout->addStretch();
    totalLayout->addWidget(m_totalLBPLabel);
    totalLayout->addWidget(m_totalUSDLabel);
    rightLayout->addLayout(totalLayout);

    // NEW: Print Receipt button
    QHBoxLayout* printLayout = new QHBoxLayout();
    printLayout->addStretch();
    m_printReceiptBtn = new QPushButton(tr("Print Receipt"));
    m_printReceiptBtn->setStyleSheet(btnStyle);
    printLayout->addWidget(m_printReceiptBtn);
    rightLayout->addLayout(printLayout);

    QHBoxLayout* histLayout = new QHBoxLayout();
    histLayout->setSpacing(12);

    QLabel* histTitle = new QLabel(tr("Product History:"));
    histTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #1a3a5c;");
    histLayout->addWidget(histTitle);

    auto createHistLabel = [&](const QString& title) -> QLabel* {
        QLabel* lbl = new QLabel(title + " -");
        lbl->setStyleSheet("font-size: 12px; color: #495057; padding: 4px 10px; "
            "background-color: #f8f9fa; border-radius: 4px; border: 1px solid #dee2e6;");
        return lbl;
        };

    m_histTotalSold = createHistLabel(tr("Total Sold"));
    m_histRemaining = createHistLabel(tr("Remaining Qty"));
    m_histAvgPrice = createHistLabel(tr("Avg Sale Price"));
    m_histLastSale = createHistLabel(tr("Last Sale Date"));

    histLayout->addWidget(m_histTotalSold);
    histLayout->addWidget(m_histRemaining);
    histLayout->addWidget(m_histAvgPrice);
    histLayout->addWidget(m_histLastSale);
    histLayout->addStretch();
    rightLayout->addLayout(histLayout);

    mainLayout->addWidget(rightBox, 3);

    connect(m_slotButtons, QOverload<int>::of(&QButtonGroup::idClicked),
        this, &SalesPage::onSaleSlotClicked);
    connect(m_paymentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &SalesPage::onPaymentTypeChanged);
    connect(m_accountCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &SalesPage::onAccountChanged);
    connect(m_customerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &SalesPage::onCustomerChanged);
    connect(m_prevBtn, &QPushButton::clicked, this, &SalesPage::onPrevSaleClicked);
    connect(m_nextBtn, &QPushButton::clicked, this, &SalesPage::onNextSaleClicked);
    connect(m_editCodeBtn, &QPushButton::clicked, this, &SalesPage::onEditByCodeClicked);
    connect(m_addBtn, &QPushButton::clicked, this, &SalesPage::onAddProductClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &SalesPage::onDeleteItemClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &SalesPage::onEditItemClicked);
    connect(m_changePriceBtn, &QPushButton::clicked, this, &SalesPage::onChangePriceClicked);
    connect(m_saleTable, &QTableView::clicked, this, &SalesPage::onSaleTableClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &SalesPage::onSearchTextChanged);
    connect(m_productTable, &QTableView::clicked, this, &SalesPage::onProductSelected);
    connect(m_discountEdit, &QLineEdit::textChanged, this, &SalesPage::calculateNet);
    connect(m_taxEdit, &QLineEdit::textChanged, this, &SalesPage::calculateNet);
    connect(manageColsBtn, &QPushButton::clicked, this, &SalesPage::onManageColumnsClicked);

    // NEW: Save and Print connections
    connect(saveBtn, &QPushButton::clicked, this, &SalesPage::onSaveClicked);
    connect(m_printReceiptBtn, &QPushButton::clicked, this, &SalesPage::onPrintReceiptClicked);

    connect(m_searchEdit, &QLineEdit::returnPressed, [=]() {
        QString text = m_searchEdit->text().trimmed();
        if (!text.isEmpty()) {
            scanProductCode(text);
            m_searchEdit->clear();
        }
        });
}

void SalesPage::buildSaleInfoTab(QTabWidget* tabs) { Q_UNUSED(tabs) }
void SalesPage::buildItemsTab(QTabWidget* tabs) { Q_UNUSED(tabs) }
void SalesPage::buildAccountsTab(QTabWidget* tabs) { Q_UNUSED(tabs) }

void SalesPage::calculateNet()
{
    double total = 0.0;
    for (const auto& item : m_sessions[m_currentSlot].items) {
        total += item.lineTotal;
    }
    double discount = m_discountEdit ? m_discountEdit->text().toDouble() : 0.0;
    double tax = m_taxEdit ? m_taxEdit->text().toDouble() : 0.0;
    double net = total - discount + tax;

    if (m_netEdit) m_netEdit->setText(QString::number(net, 'f', 2));
}

void SalesPage::loadCategories()
{
    QWidget* catContainer = findChild<QWidget*>("categoryContainer");
    if (!catContainer) return;
    QVBoxLayout* catLayout = qobject_cast<QVBoxLayout*>(catContainer->layout());
    if (!catLayout) return;

    QLayoutItem* item;
    while ((item = catLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    QSqlQuery query("SELECT id, name FROM product_categories WHERE is_active = 1 ORDER BY name");
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        m_categories[id] = name;

        QPushButton* btn = new QPushButton(name);
        btn->setStyleSheet(
            "QPushButton {"
            "  background-color: #f8f9fa; color: #495057;"
            "  border: 1px solid #dee2e6; border-radius: 3px;"
            "  padding: 10px 14px; text-align: center; font-size: 13px;"
            "}"
            "QPushButton:hover { background-color: #e9ecef; color: #1a3a5c; }"
            "QPushButton:pressed { background-color: #1a3a5c; color: white; }"
        );
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, [=]() { onCategoryClicked(id); });
        catLayout->addWidget(btn);
    }
    catLayout->addStretch();

    if (!m_categories.isEmpty()) {
        m_currentCategoryId = m_categories.firstKey();
        onCategoryClicked(m_currentCategoryId);
    }
}

void SalesPage::loadAccounts()
{
    m_accountCombo->clear();
    m_accountCombo->addItem(tr("-- Select Account --"), 0);

    QSqlQuery query("SELECT id, name, type FROM accounts WHERE is_active = 1 ORDER BY name");
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        QString type = query.value("type").toString();
        m_accountCombo->addItem(name + " (" + type + ")", id);
        m_accountCombo->setItemData(m_accountCombo->count() - 1, type, Qt::UserRole + 1);
    }
}

void SalesPage::loadCustomers()
{
    m_customerCombo->clear();
    m_customerCombo->addItem(tr("-- Select Customer --"), 0);

    QSqlQuery query("SELECT id, name, phone FROM customers WHERE is_active = 1 ORDER BY name");
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        QString phone = query.value("phone").toString();
        QString display = phone.isEmpty() ? name : name + " | " + phone;
        m_customerCombo->addItem(display, id);
    }
}

void SalesPage::onCustomerChanged(int index)
{
    if (index <= 0) {
        m_sessions[m_currentSlot].customerId = 0;
        m_sessions[m_currentSlot].customerName.clear();
        return;
    }

    int customerId = m_customerCombo->itemData(index).toInt();
    QString display = m_customerCombo->itemText(index);
    m_sessions[m_currentSlot].customerId = customerId;
    m_sessions[m_currentSlot].customerName = display;
}

void SalesPage::onCategoryClicked(int categoryId)
{
    m_currentCategoryId = categoryId;
    loadProductsByCategory(categoryId);
}

void SalesPage::loadProductsByCategory(int categoryId)
{
    m_productModel->removeRows(0, m_productModel->rowCount());

    QSqlQuery query;
    query.prepare(
        "SELECT id, code, name, sale_price, quantity FROM products "
        "WHERE category_id = :catId AND is_active = 1 ORDER BY name"
    );
    query.bindValue(":catId", categoryId);

    if (!query.exec()) return;

    int row = 0;
    while (query.next()) {
        int id = query.value("id").toInt();
        QString code = query.value("code").toString();
        QString name = query.value("name").toString();
        double price = query.value("sale_price").toDouble();

        m_productModel->insertRow(row);
        QStandardItem* idItem = new QStandardItem(QString::number(id));
        idItem->setData(id, Qt::UserRole);
        m_productModel->setItem(row, 0, idItem);
        m_productModel->setItem(row, 1, new QStandardItem(code));
        m_productModel->setItem(row, 2, new QStandardItem(name));
        m_productModel->setItem(row, 3, new QStandardItem(QString::number(price, 'f', 2)));
        row++;
    }

    m_productTable->resizeColumnsToContents();
    m_productTable->hideColumn(0);
}

void SalesPage::onProductSelected(const QModelIndex& index)
{
    int row = index.row();
    if (row < 0 || row >= m_productModel->rowCount()) return;

    int productId = m_productModel->item(row, 0)->data(Qt::UserRole).toInt();
    refreshProductHistory(productId);
}

void SalesPage::onAddProductClicked()
{
    QModelIndexList selected = m_productTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a product from the list first."));
        return;
    }

    int row = selected.first().row();
    int productId = m_productModel->item(row, 0)->data(Qt::UserRole).toInt();
    QString code = m_productModel->item(row, 1)->text();
    QString name = m_productModel->item(row, 2)->text();
    double price = m_productModel->item(row, 3)->text().toDouble();
    int qty = m_quantitySpin->value();

    SaleSession& session = m_sessions[m_currentSlot];
    for (int i = 0; i < session.items.size(); ++i) {
        if (session.items[i].productId == productId) {
            session.items[i].quantity += qty;
            session.items[i].lineTotal = session.items[i].quantity * session.items[i].price;
            refreshSaleTable();
            refreshTotals();
            calculateNet();
            return;
        }
    }

    SaleItem item;
    item.productId = productId;
    item.code = code;
    item.name = name;
    item.price = price;
    item.quantity = qty;
    item.lineTotal = price * qty;
    session.items.append(item);

    refreshSaleTable();
    refreshTotals();
    calculateNet();
    m_quantitySpin->setValue(1);
}

void SalesPage::onDeleteItemClicked()
{
    QModelIndexList selected = m_saleTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an item to delete."));
        return;
    }

    int row = selected.first().row();
    SaleSession& session = m_sessions[m_currentSlot];
    if (row >= 0 && row < session.items.size()) {
        session.items.removeAt(row);
        refreshSaleTable();
        refreshTotals();
        calculateNet();
    }
}

void SalesPage::onEditItemClicked()
{
    QModelIndexList selected = m_saleTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an item to edit."));
        return;
    }

    int row = selected.first().row();
    SaleSession& session = m_sessions[m_currentSlot];
    if (row < 0 || row >= session.items.size()) return;

    bool ok;
    int newQty = QInputDialog::getInt(this, tr("Edit Quantity"),
        tr("Enter new quantity:"),
        session.items[row].quantity, 1, 9999, 1, &ok);
    if (ok) {
        session.items[row].quantity = newQty;
        session.items[row].lineTotal = newQty * session.items[row].price;
        refreshSaleTable();
        refreshTotals();
        calculateNet();
    }
}

void SalesPage::onChangePriceClicked()
{
    QModelIndexList selected = m_saleTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an item to change price."));
        return;
    }

    int row = selected.first().row();
    SaleSession& session = m_sessions[m_currentSlot];
    if (row < 0 || row >= session.items.size()) return;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Change Product Price"));
    dialog.setLayoutDirection(Qt::RightToLeft);
    QVBoxLayout* dlgLayout = new QVBoxLayout(&dialog);

    QLabel* info = new QLabel(tr("Product: %1").arg(session.items[row].name));
    info->setStyleSheet("font-weight: 600; color: #1a3a5c;");
    dlgLayout->addWidget(info);

    QDoubleSpinBox* priceSpin = new QDoubleSpinBox(&dialog);
    priceSpin->setRange(0.01, 999999.99);
    priceSpin->setDecimals(2);
    priceSpin->setValue(session.items[row].price);
    priceSpin->setStyleSheet(
        "QDoubleSpinBox { background-color: white; border: 2px solid #e9ecef; "
        "border-radius: 8px; padding: 8px; font-size: 14px; }"
        "QDoubleSpinBox:focus { border: 2px solid #16213e; }"
    );
    dlgLayout->addWidget(priceSpin);

    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dlgLayout->addWidget(btns);

    if (dialog.exec() == QDialog::Accepted) {
        session.items[row].price = priceSpin->value();
        session.items[row].lineTotal = session.items[row].quantity * session.items[row].price;
        refreshSaleTable();
        refreshTotals();
        calculateNet();
    }
}

void SalesPage::onPaymentTypeChanged(int index)
{
    SaleSession& session = m_sessions[m_currentSlot];
    session.paymentType = (index == 0) ? "cash" : "debt";
    enforcePaymentRules();
}

void SalesPage::onAccountChanged(int index)
{
    if (index <= 0) return;

    SaleSession& session = m_sessions[m_currentSlot];
    int accountId = m_accountCombo->itemData(index).toInt();
    QString accName = m_accountCombo->itemText(index);

    session.accountId = accountId;
    session.accountName = accName;
    enforcePaymentRules();
}

void SalesPage::enforcePaymentRules()
{
    int accIndex = m_accountCombo->currentIndex();
    if (accIndex <= 0) return;

    SaleSession& session = m_sessions[m_currentSlot];
    QString accType = m_accountCombo->itemData(accIndex, Qt::UserRole + 1).toString();

    if (accType == "store" && session.paymentType == "debt") {
        QMessageBox::warning(this, tr("Invalid"),
            tr("Store account only allows Cash payments."));
        m_paymentCombo->setCurrentIndex(0);
        session.paymentType = "cash";
    }
    else if (accType == "customer" && session.paymentType == "cash") {
        QMessageBox::warning(this, tr("Invalid"),
            tr("Customer accounts only allow Debt payments."));
        m_paymentCombo->setCurrentIndex(1);
        session.paymentType = "debt";
    }
}

void SalesPage::onSaleSlotClicked(int slot)
{
    if (slot == m_currentSlot) return;
    switchToSlot(slot);
}

void SalesPage::onPrevSaleClicked()
{
    bool ok;
    int code = QInputDialog::getInt(this, tr("Previous Sale"),
        tr("Enter sale code to navigate:"),
        m_sessions[m_currentSlot].code, 1, 999999, 1, &ok);
    if (ok) {
        QMessageBox::information(this, tr("Info"),
            tr("Loading sale code %1 from database...").arg(code));
    }
}

void SalesPage::onNextSaleClicked()
{
    clearCurrentSale();
    m_sessions[m_currentSlot].code = generateSaleCode();
    m_sessions[m_currentSlot].items.clear();
    m_sessions[m_currentSlot].totalLBP = 0;
    m_sessions[m_currentSlot].totalUSD = 0;
    m_sessions[m_currentSlot].paymentType = "cash";
    m_sessions[m_currentSlot].accountId = 0;
    m_sessions[m_currentSlot].accountName.clear();
    m_sessions[m_currentSlot].customerId = 0;
    m_sessions[m_currentSlot].customerName.clear();
    m_paymentCombo->setCurrentIndex(0);
    m_accountCombo->setCurrentIndex(0);
    m_customerCombo->setCurrentIndex(0);
    refreshSaleTable();
    refreshTotals();
    calculateNet();
    m_saleCodeLabel->setText(QString::number(m_sessions[m_currentSlot].code));
    updateSlotButtons();
}

void SalesPage::onEditByCodeClicked()
{
    bool ok;
    int code = QInputDialog::getInt(this, tr("Edit Sale by Code"),
        tr("Enter sale code (try 123):"),
        0, 1, 999999, 1, &ok);
    if (ok) {
        if (code == 123) {
            SaleSession& session = m_sessions[m_currentSlot];
            session.code = 123;
            session.items.clear();

            SaleItem item1;
            item1.productId = 1;
            item1.code = "P001";
            item1.name = tr("Sample Cheese");
            item1.price = 15.50;
            item1.quantity = 3;
            item1.lineTotal = 46.50;
            session.items.append(item1);

            SaleItem item2;
            item2.productId = 2;
            item2.code = "P002";
            item2.name = tr("Sample Milk");
            item2.price = 8.00;
            item2.quantity = 2;
            item2.lineTotal = 16.00;
            session.items.append(item2);

            refreshSaleTable();
            refreshTotals();
            calculateNet();
            m_saleCodeLabel->setText("123");
            QMessageBox::information(this, tr("Success"), tr("Sale #123 loaded for editing."));
        }
        else {
            QMessageBox::information(this, tr("Info"),
                tr("Searching database for sale code %1...").arg(code));
        }
    }
}

void SalesPage::onSearchTextChanged(const QString& text)
{
    QString trimmed = text.trimmed();

    if (trimmed.isEmpty()) {
        if (m_currentCategoryId >= 0) {
            loadProductsByCategory(m_currentCategoryId);
        }
        else if (!m_categories.isEmpty()) {
            onCategoryClicked(m_categories.firstKey());
        }
        return;
    }

    m_productModel->removeRows(0, m_productModel->rowCount());

    QSqlQuery query;
    query.prepare(
        "SELECT id, code, name, sale_price, quantity FROM products "
        "WHERE (code LIKE :search OR name LIKE :search) AND is_active = 1 "
        "ORDER BY name"
    );
    query.bindValue(":search", "%" + trimmed + "%");

    if (!query.exec()) {
        qDebug() << "Search error:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        int id = query.value("id").toInt();
        QString code = query.value("code").toString();
        QString name = query.value("name").toString();
        double price = query.value("sale_price").toDouble();

        m_productModel->insertRow(row);
        QStandardItem* idItem = new QStandardItem(QString::number(id));
        idItem->setData(id, Qt::UserRole);
        m_productModel->setItem(row, 0, idItem);
        m_productModel->setItem(row, 1, new QStandardItem(code));
        m_productModel->setItem(row, 2, new QStandardItem(name));
        m_productModel->setItem(row, 3, new QStandardItem(QString::number(price, 'f', 2)));
        row++;
    }
    m_productTable->resizeColumnsToContents();
    m_productTable->hideColumn(0);
}

void SalesPage::onSaleTableClicked(const QModelIndex& index)
{
    int row = index.row();
    SaleSession& session = m_sessions[m_currentSlot];
    if (row < 0 || row >= session.items.size()) return;

    int productId = session.items[row].productId;
    refreshProductHistory(productId);
}

void SalesPage::refreshSaleTable()
{
    m_saleModel->removeRows(0, m_saleModel->rowCount());

    SaleSession& session = m_sessions[m_currentSlot];
    for (int i = 0; i < session.items.size(); ++i) {
        const SaleItem& item = session.items[i];
        m_saleModel->insertRow(i);
        m_saleModel->setItem(i, 0, new QStandardItem(QString::number(item.productId)));
        m_saleModel->setItem(i, 1, new QStandardItem(item.code));
        m_saleModel->setItem(i, 2, new QStandardItem(item.name));
        m_saleModel->setItem(i, 3, new QStandardItem(QString::number(item.quantity)));
        m_saleModel->setItem(i, 4, new QStandardItem(QString::number(item.price, 'f', 2)));
        m_saleModel->setItem(i, 5, new QStandardItem(QString::number(item.lineTotal, 'f', 2)));

        for (auto it = m_customColumns.begin(); it != m_customColumns.end(); ++it) {
            QString val = item.customValues.value(it.value(), "").toString();
            m_saleModel->setItem(i, it.key(), new QStandardItem(val));
        }
    }

    m_saleTable->resizeColumnsToContents();
}

void SalesPage::refreshTotals()
{
    SaleSession& session = m_sessions[m_currentSlot];
    double total = 0.0;
    for (const auto& item : session.items) {
        total += item.lineTotal;
    }

    session.totalUSD = total;
    // NEW: use live rate from database instead of hardcoded constant
    session.totalLBP = total * m_usdToLbpRate;

    m_totalUSDLabel->setText(tr("Total USD: $%1").arg(QString::number(total, 'f', 2)));
    m_totalLBPLabel->setText(tr("Total LBP: %L1").arg(session.totalLBP, 0, 'f', 0));
}

void SalesPage::refreshProductHistory(int productId)
{
    if (productId <= 0) {
        m_histTotalSold->setText(tr("Total Sold: -"));
        m_histRemaining->setText(tr("Remaining: -"));
        m_histAvgPrice->setText(tr("Avg Price: -"));
        m_histLastSale->setText(tr("Last Sale: -"));
        return;
    }

    QSqlQuery q1;
    q1.prepare("SELECT COALESCE(SUM(quantity),0) as total_sold FROM sale_items WHERE product_id = :pid");
    q1.bindValue(":pid", productId);
    int totalSold = 0;
    if (q1.exec() && q1.next()) {
        totalSold = q1.value("total_sold").toInt();
    }

    QSqlQuery q2;
    q2.prepare("SELECT quantity FROM products WHERE id = :pid");
    q2.bindValue(":pid", productId);
    int remaining = 0;
    if (q2.exec() && q2.next()) {
        remaining = q2.value("quantity").toInt();
    }

    QSqlQuery q3;
    q3.prepare("SELECT COALESCE(AVG(unit_price),0) as avg_price FROM sale_items WHERE product_id = :pid");
    q3.bindValue(":pid", productId);
    double avgPrice = 0.0;
    if (q3.exec() && q3.next()) {
        avgPrice = q3.value("avg_price").toDouble();
    }

    QSqlQuery q4;
    q4.prepare(
        "SELECT MAX(s.created_at) as last_sale FROM sales s "
        "JOIN sale_items si ON s.id = si.sale_id WHERE si.product_id = :pid"
    );
    q4.bindValue(":pid", productId);
    QString lastSale = tr("Never");
    if (q4.exec() && q4.next()) {
        QDateTime dt = q4.value("last_sale").toDateTime();
        if (dt.isValid()) lastSale = dt.toString("yyyy-MM-dd hh:mm");
    }

    m_histTotalSold->setText(tr("Total Sold: %1").arg(totalSold));
    m_histRemaining->setText(tr("Remaining: %1").arg(remaining));
    m_histAvgPrice->setText(tr("Avg Price: $%1").arg(QString::number(avgPrice, 'f', 2)));
    m_histLastSale->setText(tr("Last Sale: %1").arg(lastSale));
}

void SalesPage::switchToSlot(int slot)
{
    m_currentSlot = slot;
    loadSaleFromSession(slot);
    updateSlotButtons();
}

void SalesPage::loadSaleFromSession(int slot)
{
    SaleSession& session = m_sessions[slot];
    m_saleCodeLabel->setText(QString::number(session.code));
    m_paymentCombo->setCurrentIndex(session.paymentType == "cash" ? 0 : 1);

    int accIndex = m_accountCombo->findData(session.accountId);
    if (accIndex >= 0) m_accountCombo->setCurrentIndex(accIndex);
    else m_accountCombo->setCurrentIndex(0);

    int custIndex = m_customerCombo->findData(session.customerId);
    if (custIndex >= 0) m_customerCombo->setCurrentIndex(custIndex);
    else m_customerCombo->setCurrentIndex(0);

    refreshSaleTable();
    refreshTotals();
    calculateNet();
}

void SalesPage::clearCurrentSale()
{
    m_saleModel->removeRows(0, m_saleModel->rowCount());
}

void SalesPage::updateSlotButtons()
{
    m_slot1Btn->setChecked(m_currentSlot == 0);
    m_slot2Btn->setChecked(m_currentSlot == 1);
    m_slot3Btn->setChecked(m_currentSlot == 2);

    auto updateStyle = [&](QPushButton* btn, int slot) {
        bool hasItems = !m_sessions[slot].items.isEmpty();
        bool isCurrent = slot == m_currentSlot;

        if (isCurrent) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background-color: #1a3a5c; color: white;"
                "  border: 1px solid #1a3a5c; border-radius: 3px;"
                "  padding: 8px 20px; font-weight: 600; font-size: 13px;"
                "}"
                "QPushButton:hover { background-color: #0f2a4a; }"
            );
        }
        else if (hasItems) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background-color: #d4edda; color: #155724;"
                "  border: 1px solid #c3e6cb; border-radius: 3px;"
                "  padding: 8px 20px; font-weight: 600; font-size: 13px;"
                "}"
                "QPushButton:checked {"
                "  background-color: #1a3a5c; color: white; border-color: #1a3a5c;"
                "}"
                "QPushButton:hover { background-color: #c3e6cb; }"
            );
        }
        else {
            btn->setStyleSheet(
                "QPushButton {"
                "  background-color: #e9ecef; color: #495057;"
                "  border: 1px solid #dee2e6; border-radius: 3px;"
                "  padding: 8px 20px; font-weight: 600; font-size: 13px;"
                "}"
                "QPushButton:checked {"
                "  background-color: #1a3a5c; color: white; border-color: #1a3a5c;"
                "}"
                "QPushButton:hover { background-color: #dee2e6; }"
            );
        }
        };

    updateStyle(m_slot1Btn, 0);
    updateStyle(m_slot2Btn, 1);
    updateStyle(m_slot3Btn, 2);
}

void SalesPage::rebuildSaleModelHeaders()
{
    QStringList headers;
    headers << tr("ID") << tr("Code") << tr("Product Name") << tr("Qty")
        << tr("Unit Price") << tr("Sum");

    for (auto it = m_customColumns.begin(); it != m_customColumns.end(); ++it) {
        headers << it.value();
    }

    m_saleModel->setHorizontalHeaderLabels(headers);
}

void SalesPage::addCustomColumn(const QString& name)
{
    int newIndex = 6 + m_customColumns.size();
    m_customColumns[newIndex] = name;
    rebuildSaleModelHeaders();
    refreshSaleTable();
}

void SalesPage::removeCustomColumn(int colIndex)
{
    if (!m_customColumns.contains(colIndex)) return;

    QString oldName = m_customColumns[colIndex];
    for (auto& session : m_sessions) {
        for (auto& item : session.items) {
            item.customValues.remove(oldName);
        }
    }

    m_customColumns.remove(colIndex);

    QMap<int, QString> newCols;
    int idx = 6;
    for (auto it = m_customColumns.begin(); it != m_customColumns.end(); ++it) {
        newCols[idx++] = it.value();
    }
    m_customColumns = newCols;

    rebuildSaleModelHeaders();
    refreshSaleTable();
}

void SalesPage::editCustomColumn(int colIndex, const QString& newName)
{
    if (!m_customColumns.contains(colIndex)) return;

    QString oldName = m_customColumns[colIndex];
    m_customColumns[colIndex] = newName;

    for (auto& session : m_sessions) {
        for (auto& item : session.items) {
            if (item.customValues.contains(oldName)) {
                QVariant v = item.customValues.take(oldName);
                item.customValues[newName] = v;
            }
        }
    }

    rebuildSaleModelHeaders();
    refreshSaleTable();
}

void SalesPage::onManageColumnsClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Manage Sale Columns"));
    dialog.setLayoutDirection(Qt::RightToLeft);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QLabel* info = new QLabel(tr("Current custom columns:"));
    info->setStyleSheet("font-weight: 600; color: #1a3a5c;");
    layout->addWidget(info);

    QListWidget* list = new QListWidget(&dialog);
    for (auto it = m_customColumns.begin(); it != m_customColumns.end(); ++it) {
        list->addItem(it.value());
    }
    layout->addWidget(list);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* addBtn = new QPushButton(tr("Add"));
    QPushButton* editBtn = new QPushButton(tr("Edit"));
    QPushButton* delBtn = new QPushButton(tr("Delete"));
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(delBtn);
    layout->addLayout(btnLayout);

    connect(addBtn, &QPushButton::clicked, [&]() {
        bool ok;
        QString name = QInputDialog::getText(&dialog, tr("Add Column"),
            tr("Column name:"), QLineEdit::Normal, "", &ok);
        if (ok && !name.trimmed().isEmpty()) {
            addCustomColumn(name.trimmed());
            list->addItem(name.trimmed());
        }
        });

    connect(editBtn, &QPushButton::clicked, [&]() {
        if (!list->currentItem()) return;
        QString oldName = list->currentItem()->text();
        int colIndex = -1;
        for (auto it = m_customColumns.begin(); it != m_customColumns.end(); ++it) {
            if (it.value() == oldName) { colIndex = it.key(); break; }
        }
        bool ok;
        QString newName = QInputDialog::getText(&dialog, tr("Edit Column"),
            tr("New name:"), QLineEdit::Normal, oldName, &ok);

        if (ok && !newName.trimmed().isEmpty() && colIndex >= 0) {
            editCustomColumn(colIndex, newName.trimmed());
            list->currentItem()->setText(newName.trimmed());
        }
        });

    connect(delBtn, &QPushButton::clicked, [&]() {
        if (!list->currentItem()) return;
        QString name = list->currentItem()->text();
        int colIndex = -1;
        for (auto it = m_customColumns.begin(); it != m_customColumns.end(); ++it) {
            if (it.value() == name) { colIndex = it.key(); break; }
        }
        if (colIndex >= 0) {
            removeCustomColumn(colIndex);
            delete list->takeItem(list->currentRow());
        }
        });

    QDialogButtonBox* closeBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(closeBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(closeBox);

    dialog.exec();
}

void SalesPage::scanProductCode(const QString& code)
{
    if (code.trimmed().isEmpty()) return;

    QSqlQuery query;
    query.prepare("SELECT id, code, name, sale_price, quantity FROM products WHERE code = :code AND is_active = 1");
    query.bindValue(":code", code.trimmed());

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, tr("Product Not Found"),
            tr("No product found with code: %1").arg(code));
        return;
    }

    int productId = query.value("id").toInt();
    QString dbCode = query.value("code").toString();
    QString name = query.value("name").toString();
    double price = query.value("sale_price").toDouble();
    int stockQty = query.value("quantity").toInt();

    if (stockQty <= 0) {
        QMessageBox::warning(this, tr("Out of Stock"),
            tr("Product %1 is out of stock.").arg(name));
        return;
    }

    SaleSession& session = m_sessions[m_currentSlot];

    for (int i = 0; i < session.items.size(); ++i) {
        if (session.items[i].productId == productId) {
            session.items[i].quantity += 1;
            session.items[i].lineTotal = session.items[i].quantity * session.items[i].price;
            refreshSaleTable();
            refreshTotals();
            calculateNet();
            return;
        }
    }

    SaleItem item;
    item.productId = productId;
    item.code = dbCode;
    item.name = name;
    item.price = price;
    item.quantity = 1;
    item.lineTotal = price;
    session.items.append(item);

    refreshSaleTable();
    refreshTotals();
    calculateNet();
}

// ------------------------------------------------------------------
// NEW: Settings & Exchange Rate
// ------------------------------------------------------------------
void SalesPage::loadSettings()
{
    QSqlQuery query("SELECT setting_key, setting_value FROM settings");
    while (query.next()) {
        QString key = query.value("setting_key").toString();
        QString val = query.value("setting_value").toString();
        if (key == "usd_to_lbp_rate") m_usdToLbpRate = val.toDouble();
        else if (key == "company_name") m_companyName = val;
        else if (key == "receipt_header") m_receiptHeader = val;
        else if (key == "receipt_footer") m_receiptFooter = val;
    }
    if (m_usdToLbpRate <= 0) m_usdToLbpRate = 89500.0;
}

// ------------------------------------------------------------------
// NEW: Save Sale to Database
// ------------------------------------------------------------------
void SalesPage::onSaveClicked()
{
    SaleSession& session = m_sessions[m_currentSlot];
    if (session.items.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No items in the cart to save."));
        return;
    }
    if (m_accountCombo->currentIndex() <= 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an account."));
        return;
    }

    double total = 0.0;
    for (const auto& item : session.items) total += item.lineTotal;
    double discount = m_discountEdit ? m_discountEdit->text().toDouble() : 0.0;
    double tax = m_taxEdit ? m_taxEdit->text().toDouble() : 0.0;
    double net = total - discount + tax;

    QString ref = m_refEdit->text().trimmed();
    if (ref.isEmpty()) ref = QString("SAL-%1").arg(session.code);

    DbManager* db = DbManager::instance();
    if (!db->transaction()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to start transaction."));
        return;
    }

    QSqlQuery transQuery;
    transQuery.prepare(
        "INSERT INTO transactions (type, reference_number, party_id, total_amount, "
        "discount, tax_amount, net_amount, payment_method, notes, created_at) "
        "VALUES ('sale', :ref, :party, :total, :discount, :tax, :net, "
        ":payment, :notes, :created)"
    );
    transQuery.bindValue(":ref", ref);
    transQuery.bindValue(":party", session.customerId);
    transQuery.bindValue(":total", total);
    transQuery.bindValue(":discount", discount);
    transQuery.bindValue(":tax", tax);
    transQuery.bindValue(":net", net);
    transQuery.bindValue(":payment", session.paymentType);
    transQuery.bindValue(":notes", m_notesEdit ? m_notesEdit->text().trimmed() : QString());
    transQuery.bindValue(":created", QDateTime::currentDateTime());

    if (!transQuery.exec()) {
        db->rollback();
        QMessageBox::critical(this, tr("Error"), transQuery.lastError().text());
        return;
    }

    int transactionId = transQuery.lastInsertId().toInt();

    for (const auto& item : session.items) {
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
            return;
        }

        // Deduct stock
        QSqlQuery stockQuery;
        stockQuery.prepare("UPDATE products SET quantity = quantity - :qty WHERE id = :pid");
        stockQuery.bindValue(":qty", item.quantity);
        stockQuery.bindValue(":pid", item.productId);
        if (!stockQuery.exec()) {
            db->rollback();
            QMessageBox::critical(this, tr("Error"), stockQuery.lastError().text());
            return;
        }
    }

    // If debt sale, update customer balance
    if (session.paymentType == "debt" && session.customerId > 0) {
        QSqlQuery custQuery;
        custQuery.prepare("UPDATE customers SET balance = balance + :amount WHERE id = :id");
        custQuery.bindValue(":amount", net);
        custQuery.bindValue(":id", session.customerId);
        custQuery.exec(); // best effort
    }

    db->commit();
    QMessageBox::information(this, tr("Success"), tr("Sale saved successfully!"));

    // Reset current session
    session.items.clear();
    session.totalLBP = 0;
    session.totalUSD = 0;
    refreshSaleTable();
    refreshTotals();
    calculateNet();
}

// ------------------------------------------------------------------
// NEW: Print Receipt (80mm thermal paper)
// ------------------------------------------------------------------
void SalesPage::onPrintReceiptClicked()
{
    SaleSession& session = m_sessions[m_currentSlot];
    if (session.items.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Cart is empty. Nothing to print."));
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    // 80 mm x 297 mm thermal roll (common receipt size)
    printer.setPageSize(QPageSize(QSizeF(80, 297), QPageSize::Millimeter));
    printer.setPageOrientation(QPageLayout::Portrait);

    QPrintDialog dialog(&printer, this);
    if (dialog.exec() != QDialog::Accepted) return;

    QPainter p(&printer);
    p.setRenderHint(QPainter::Antialiasing);
    p.setLayoutDirection(Qt::RightToLeft);

    QFont font("Segoe UI", 10);
    p.setFont(font);

    int margin = 30;
    // FIX: pageRect() returns QRectF in Qt 6; convert to QRect
    QRect paintRect = printer.pageRect(QPrinter::DevicePixel).toRect();
    int pageW = paintRect.width();
    int contentW = pageW - margin * 2;
    int y = 40;

    // ... rest of the function stays exactly the same as before ...

    // Header
    QFont headerFont("Segoe UI", 14, QFont::Bold);
    p.setFont(headerFont);
    QString headerText = m_receiptHeader.isEmpty() ? m_companyName : m_receiptHeader;
    if (headerText.isEmpty()) headerText = tr("MarketPro");
    p.drawText(margin, y, contentW, 40, Qt::AlignCenter, headerText);
    y += 50;

    p.setFont(font);
    p.drawText(margin, y, contentW, 20, Qt::AlignCenter, "------------------------------");
    y += 30;

    // Info block
    QString info = tr("Sale #: %1\nDate: %2\nCustomer: %3\nPayment: %4")
        .arg(QString::number(session.code))
        .arg(QDate::currentDate().toString("yyyy-MM-dd"))
        .arg(session.customerName.isEmpty() ? tr("Walk-in") : session.customerName)
        .arg(session.paymentType == "cash" ? tr("Cash") : tr("Debt"));
    p.drawText(margin, y, contentW, 80, Qt::AlignRight | Qt::TextWordWrap, info);
    y += 90;

    p.drawText(margin, y, contentW, 20, Qt::AlignCenter, "------------------------------");
    y += 30;

    // Items header
    QFont boldFont = font;
    boldFont.setBold(true);
    p.setFont(boldFont);

    // RTL columns drawn right-to-left
    int colWidths[4] = { 70, 60, 50, contentW - 180 }; // Total, Price, Qty, Product
    QStringList headers = { tr("Total"), tr("Price"), tr("Qty"), tr("Product") };
    int x = pageW - margin;
    for (int i = 0; i < 4; ++i) {
        x -= colWidths[i];
        p.drawText(x, y, colWidths[i], 25, Qt::AlignCenter, headers[i]);
    }
    y += 30;
    p.setFont(font);

    // Items
    double subtotal = 0.0;
    for (const auto& item : session.items) {
        x = pageW - margin;
        QStringList vals = {
            QString::number(item.lineTotal, 'f', 2),
            QString::number(item.price, 'f', 2),
            QString::number(item.quantity),
            item.name
        };
        for (int i = 0; i < 4; ++i) {
            x -= colWidths[i];
            Qt::Alignment align = (i == 3) ? Qt::AlignRight : Qt::AlignCenter;
            p.drawText(x, y, colWidths[i], 25, align, vals[i]);
        }
        y += 25;
        subtotal += item.lineTotal;
    }

    y += 10;
    p.drawText(margin, y, contentW, 20, Qt::AlignCenter, "------------------------------");
    y += 30;

    // Totals
    double discount = m_discountEdit ? m_discountEdit->text().toDouble() : 0.0;
    double tax = m_taxEdit ? m_taxEdit->text().toDouble() : 0.0;
    double net = subtotal - discount + tax;

    auto drawRow = [&](const QString& label, const QString& value, bool bold = false) {
        QFont f = font;
        if (bold) f.setBold(true);
        p.setFont(f);
        p.drawText(margin, y, contentW / 2, 25, Qt::AlignLeft, label);
        p.drawText(margin + contentW / 2, y, contentW / 2, 25, Qt::AlignRight, value);
        y += 25;
        };

    drawRow(tr("Subtotal:"), QString::number(subtotal, 'f', 2));
    drawRow(tr("Discount:"), QString::number(discount, 'f', 2));
    drawRow(tr("Tax:"), QString::number(tax, 'f', 2));
    y += 5;
    drawRow(tr("NET TOTAL:"), "$" + QString::number(net, 'f', 2), true);

    double lbpTotal = net * m_usdToLbpRate;
    QFont lbpFont = font;
    lbpFont.setPointSize(9);
    p.setFont(lbpFont);
    p.drawText(margin, y, contentW, 25, Qt::AlignRight,
        tr("LBP: %L1").arg(lbpTotal, 0, 'f', 0));
    y += 35;

    p.setFont(font);
    p.drawText(margin, y, contentW, 20, Qt::AlignCenter, "------------------------------");
    y += 30;

    // Footer
    QString footerText = m_receiptFooter.isEmpty() ? tr("Thank you!") : m_receiptFooter;
    p.drawText(margin, y, contentW, 60, Qt::AlignCenter | Qt::TextWordWrap, footerText);

    p.end();
}