#include "productspage.h"
#include "styledlineedit.h"
#include "styledcombobox.h"
#include "cardwidget.h"
#include "product.h"
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
#include <QDebug>
#include <QLineEdit>

ProductsPage::ProductsPage(QWidget* parent)
    : QWidget(parent)
    , m_codeEdit(nullptr)
    , m_nameEdit(nullptr)
    , m_categoryCombo(nullptr)
    , m_supplierCombo(nullptr)
    , m_purchasePriceEdit(nullptr)
    , m_salePriceEdit(nullptr)
    , m_quantityEdit(nullptr)
    , m_minStockEdit(nullptr)
    , m_unitCombo(nullptr)
    , m_barcodeEdit(nullptr)
    , m_activeCheck(nullptr)
    , m_tableView(nullptr)
    , m_tableModel(nullptr)
    , m_newBtn(nullptr)
    , m_saveBtn(nullptr)
    , m_clearBtn(nullptr)
    , m_deleteBtn(nullptr)
    , m_filterCategoryCombo(nullptr)
    , m_currentId(-1)
{
    setupUI();
    loadCategories();
    loadSuppliers();
    loadProducts();
}

ProductsPage::~ProductsPage() {}

void ProductsPage::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    setStyleSheet("background-color: #F8F9FA;");

    QLabel* title = new QLabel(tr("Product Management"));
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

    m_codeEdit = new StyledLineEdit(this);
    m_codeEdit->setPlaceholderText(tr("Product Code"));
    addLabel(tr("Code:"), 0, 0);
    formLayout->addWidget(m_codeEdit, 0, 1);

    m_nameEdit = new StyledLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("Product Name"));
    addLabel(tr("Name:"), 0, 2);
    formLayout->addWidget(m_nameEdit, 0, 3);

    m_categoryCombo = new StyledComboBox(this);
    addLabel(tr("Category:"), 1, 0);
    formLayout->addWidget(m_categoryCombo, 1, 1);

    m_supplierCombo = new StyledComboBox(this);
    addLabel(tr("Supplier:"), 1, 2);
    formLayout->addWidget(m_supplierCombo, 1, 3);

    m_purchasePriceEdit = new StyledLineEdit(this);
    m_purchasePriceEdit->setPlaceholderText("0.00");
    addLabel(tr("Purchase Price:"), 2, 0);
    formLayout->addWidget(m_purchasePriceEdit, 2, 1);

    m_salePriceEdit = new StyledLineEdit(this);
    m_salePriceEdit->setPlaceholderText("0.00");
    addLabel(tr("Sale Price:"), 2, 2);
    formLayout->addWidget(m_salePriceEdit, 2, 3);

    m_quantityEdit = new StyledLineEdit(this);
    m_quantityEdit->setPlaceholderText("0");
    addLabel(tr("Quantity:"), 3, 0);
    formLayout->addWidget(m_quantityEdit, 3, 1);

    m_minStockEdit = new StyledLineEdit(this);
    m_minStockEdit->setPlaceholderText("10");
    addLabel(tr("Min Stock:"), 3, 2);
    formLayout->addWidget(m_minStockEdit, 3, 3);

    m_unitCombo = new StyledComboBox(this);
    m_unitCombo->addItems({ tr("piece"), tr("kg"), tr("gram"), tr("liter"), tr("ml"), tr("box"), tr("pack"), tr("meter"), tr("dozen") });
    addLabel(tr("Unit:"), 4, 0);
    formLayout->addWidget(m_unitCombo, 4, 1);

    m_barcodeEdit = new StyledLineEdit(this);
    m_barcodeEdit->setPlaceholderText(tr("Barcode"));
    addLabel(tr("Barcode:"), 4, 2);
    formLayout->addWidget(m_barcodeEdit, 4, 3);

    m_activeCheck = new QPushButton(tr("Active"));
    m_activeCheck->setCheckable(true);
    m_activeCheck->setChecked(true);
    m_activeCheck->setStyleSheet(
        "QPushButton {"
        "  background-color: #28a745; color: white;"
        "  padding: 8px 24px; border-radius: 8px;"
        "  font-weight: 600; border: none; font-size: 13px;"
        "}"
        "QPushButton:checked { background-color: #28a745; }"
        "QPushButton:!checked { background-color: #dc3545; }"
    );
    addLabel(tr("Status:"), 5, 0);
    formLayout->addWidget(m_activeCheck, 5, 1);

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

    CardWidget* tableCard = new CardWidget(this);
    QVBoxLayout* tableLayout = new QVBoxLayout(tableCard);

    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* filterLabel = new QLabel(tr("Filter by Category:"));
    filterLabel->setStyleSheet("font-weight: 600; color: #495057; font-size: 13px;");
    filterLayout->addWidget(filterLabel);

    m_filterCategoryCombo = new StyledComboBox(this);
    m_filterCategoryCombo->addItem(tr("All Categories"), -1);
    connect(m_filterCategoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProductsPage::onCategoryFilterChanged);
    filterLayout->addWidget(m_filterCategoryCombo);
    filterLayout->addStretch();

    StyledLineEdit* searchEdit = new StyledLineEdit(this);
    searchEdit->setPlaceholderText(tr("Search product by name or code..."));
    filterLayout->addWidget(searchEdit);
    tableLayout->addLayout(filterLayout);

    QLabel* tableTitle = new QLabel(tr("Products List"));
    tableTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #1a1a2e;");
    tableLayout->addWidget(tableTitle);

    m_tableView = new QTableView(this);
    m_tableModel = new QStandardItemModel(this);
    m_tableModel->setHorizontalHeaderLabels({
        tr("ID"), tr("Code"), tr("Name"), tr("Category"),
        tr("Supplier"), tr("Purchase"), tr("Sale"), tr("Qty"),
        tr("Min Stock"), tr("Unit"), tr("Status")
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

    connect(m_saveBtn, &QPushButton::clicked, this, &ProductsPage::onSaveClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &ProductsPage::onClearClicked);
    connect(m_newBtn, &QPushButton::clicked, this, &ProductsPage::onNewClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &ProductsPage::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &ProductsPage::onSearchTextChanged);
    connect(m_tableView, &QTableView::clicked, this, &ProductsPage::onTableClicked);
}

void ProductsPage::loadCategories() {
    m_categoryCombo->clear();
    m_filterCategoryCombo->clear();
    m_filterCategoryCombo->addItem(tr("All Categories"), -1);

    QSqlQuery query("SELECT id, name FROM product_categories ORDER BY name");
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        m_categoryCombo->addItem(name, id);
        m_filterCategoryCombo->addItem(name, id);
    }
}

void ProductsPage::loadSuppliers() {
    m_supplierCombo->clear();
    QSqlQuery query("SELECT id, name FROM suppliers WHERE is_active = 1 ORDER BY name");
    while (query.next()) {
        m_supplierCombo->addItem(query.value("name").toString(), query.value("id"));
    }
}

void ProductsPage::loadProducts() {
    m_tableModel->removeRows(0, m_tableModel->rowCount());

    QSqlQuery query(
        "SELECT p.id, p.code, p.name, c.name as category_name, "
        "s.name as supplier_name, p.purchase_price, p.sale_price, "
        "p.quantity, p.min_stock, p.unit, p.is_active "
        "FROM products p "
        "LEFT JOIN product_categories c ON p.category_id = c.id "
        "LEFT JOIN suppliers s ON p.supplier_id = s.id "
        "ORDER BY p.name"
    );

    int row = 0;
    while (query.next()) {
        m_tableModel->insertRow(row);
        m_tableModel->setItem(row, 0, new QStandardItem(query.value("id").toString()));
        m_tableModel->setItem(row, 1, new QStandardItem(query.value("code").toString()));
        m_tableModel->setItem(row, 2, new QStandardItem(query.value("name").toString()));
        m_tableModel->setItem(row, 3, new QStandardItem(query.value("category_name").toString()));
        m_tableModel->setItem(row, 4, new QStandardItem(query.value("supplier_name").toString()));
        m_tableModel->setItem(row, 5, new QStandardItem(query.value("purchase_price").toString()));
        m_tableModel->setItem(row, 6, new QStandardItem(query.value("sale_price").toString()));
        m_tableModel->setItem(row, 7, new QStandardItem(query.value("quantity").toString()));
        m_tableModel->setItem(row, 8, new QStandardItem(query.value("min_stock").toString()));
        m_tableModel->setItem(row, 9, new QStandardItem(query.value("unit").toString()));
        m_tableModel->setItem(row, 10, new QStandardItem(
            query.value("is_active").toBool() ? tr("Active") : tr("Inactive")
        ));
        row++;
    }

    m_tableView->hideColumn(0);
    m_tableView->resizeColumnsToContents();
}

void ProductsPage::onSaveClicked() {
    if (!validateForm()) return;

    Product p = collectFormData();

    QSqlQuery query;
    if (m_currentId == -1) {
        query.prepare(
            "INSERT INTO products (code, name, category_id, supplier_id, "
            "purchase_price, sale_price, quantity, min_stock, unit, barcode, is_active) "
            "VALUES (:code, :name, :cat_id, :sup_id, :purchase, :sale, "
            ":qty, :min_stock, :unit, :barcode, :active)"
        );
    }
    else {
        query.prepare(
            "UPDATE products SET code=:code, name=:name, category_id=:cat_id, "
            "supplier_id=:sup_id, purchase_price=:purchase, sale_price=:sale, "
            "quantity=:qty, min_stock=:min_stock, unit=:unit, barcode=:barcode, "
            "is_active=:active WHERE id=:id"
        );
        query.bindValue(":id", m_currentId);
    }

    query.bindValue(":code", p.code);
    query.bindValue(":name", p.name);
    query.bindValue(":cat_id", p.categoryId);
    query.bindValue(":sup_id", p.supplierId);
    query.bindValue(":purchase", p.purchasePrice);
    query.bindValue(":sale", p.salePrice);
    query.bindValue(":qty", p.quantity);
    query.bindValue(":min_stock", p.minStock);
    query.bindValue(":unit", p.unit);
    query.bindValue(":barcode", p.barcode);
    query.bindValue(":active", p.isActive);

    if (query.exec()) {
        QMessageBox::information(this, tr("Success"), tr("Product saved successfully!"));
        clearForm();
        loadProducts();
    }
    else {
        QMessageBox::critical(this, tr("Error"), query.lastError().text());
    }
}

void ProductsPage::onClearClicked() {
    clearForm();
}

void ProductsPage::onNewClicked() {
    clearForm();
    m_codeEdit->setFocus();
}

void ProductsPage::onDeleteClicked() {
    if (m_currentId == -1) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a product to delete"));
        return;
    }

    auto reply = QMessageBox::question(this, tr("Confirm"),
        tr("Are you sure you want to delete this product?"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM products WHERE id = :id");
        query.bindValue(":id", m_currentId);
        if (query.exec()) {
            QMessageBox::information(this, tr("Success"), tr("Deleted successfully"));
            clearForm();
            loadProducts();
        }
        else {
            QMessageBox::critical(this, tr("Error"), query.lastError().text());
        }
    }
}

void ProductsPage::onSearchTextChanged(const QString& text) {
    for (int i = 0; i < m_tableModel->rowCount(); ++i) {
        bool match = false;
        for (int j = 1; j < m_tableModel->columnCount(); ++j) {
            if (m_tableModel->item(i, j) && m_tableModel->item(i, j)->text().contains(text, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }
        m_tableView->setRowHidden(i, !match);
    }
}

void ProductsPage::onTableClicked(const QModelIndex& index) {
    int row = index.row();
    int id = m_tableModel->item(row, 0)->text().toInt();

    QSqlQuery query;
    query.prepare("SELECT * FROM products WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        Product p;
        p.id = query.value("id").toInt();
        p.code = query.value("code").toString();
        p.name = query.value("name").toString();
        p.categoryId = query.value("category_id").toInt();
        p.supplierId = query.value("supplier_id").toInt();
        p.purchasePrice = query.value("purchase_price").toDouble();
        p.salePrice = query.value("sale_price").toDouble();
        p.quantity = query.value("quantity").toInt();
        p.minStock = query.value("min_stock").toInt();
        p.unit = query.value("unit").toString();
        p.barcode = query.value("barcode").toString();
        p.isActive = query.value("is_active").toBool();

        populateForm(p);
        m_currentId = id;
    }
}

void ProductsPage::onCategoryFilterChanged(int index) {
    int catId = m_filterCategoryCombo->itemData(index).toInt();

    for (int i = 0; i < m_tableModel->rowCount(); ++i) {
        if (catId == -1) {
            m_tableView->setRowHidden(i, false);
        }
        else {
            int productId = m_tableModel->item(i, 0)->text().toInt();
            QSqlQuery q;
            q.prepare("SELECT category_id FROM products WHERE id = :id");
            q.bindValue(":id", productId);
            bool hidden = true;
            if (q.exec() && q.next()) {
                hidden = (q.value("category_id").toInt() != catId);
            }
            m_tableView->setRowHidden(i, hidden);
        }
    }
}

void ProductsPage::clearForm() {
    m_currentId = -1;
    m_codeEdit->clear();
    m_nameEdit->clear();
    m_categoryCombo->setCurrentIndex(0);
    m_supplierCombo->setCurrentIndex(0);
    m_purchasePriceEdit->clear();
    m_salePriceEdit->clear();
    m_quantityEdit->clear();
    m_minStockEdit->clear();
    m_unitCombo->setCurrentIndex(0);
    m_barcodeEdit->clear();
    m_activeCheck->setChecked(true);
}

void ProductsPage::populateForm(const Product& p) {
    m_codeEdit->setText(p.code);
    m_nameEdit->setText(p.name);

    int catIndex = m_categoryCombo->findData(p.categoryId);
    if (catIndex >= 0) m_categoryCombo->setCurrentIndex(catIndex);

    int suppIndex = m_supplierCombo->findData(p.supplierId);
    if (suppIndex >= 0) m_supplierCombo->setCurrentIndex(suppIndex);

    m_purchasePriceEdit->setText(QString::number(p.purchasePrice));
    m_salePriceEdit->setText(QString::number(p.salePrice));
    m_quantityEdit->setText(QString::number(p.quantity));
    m_minStockEdit->setText(QString::number(p.minStock));
    m_unitCombo->setCurrentText(p.unit);
    m_barcodeEdit->setText(p.barcode);
    m_activeCheck->setChecked(p.isActive);
}

Product ProductsPage::collectFormData() const {
    Product p;
    p.id = m_currentId;
    p.code = m_codeEdit->text().trimmed();
    p.name = m_nameEdit->text().trimmed();
    p.categoryId = m_categoryCombo->currentData().toInt();
    p.supplierId = m_supplierCombo->currentData().toInt();
    p.purchasePrice = m_purchasePriceEdit->text().toDouble();
    p.salePrice = m_salePriceEdit->text().toDouble();
    p.quantity = m_quantityEdit->text().toInt();
    p.minStock = m_minStockEdit->text().toInt();
    p.unit = m_unitCombo->currentText();
    p.barcode = m_barcodeEdit->text().trimmed();
    p.isActive = m_activeCheck->isChecked();
    return p;
}

bool ProductsPage::validateForm() const {
    if (m_codeEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<ProductsPage*>(this),
            tr("Warning"), tr("Product code is required!"));
        return false;
    }
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<ProductsPage*>(this),
            tr("Warning"), tr("Product name is required!"));
        return false;
    }
    return true;
}