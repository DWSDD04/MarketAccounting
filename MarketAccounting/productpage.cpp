#include "productspage.h"
#include "styledlineedit.h"
#include "styledcombobox.h"
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
#include <QGroupBox>
#include <QFileDialog>
#include <QTextStream>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>

ProductsPage::ProductsPage(QWidget* parent) : QWidget(parent), m_currentId(-1) {
    setupUI();
    loadCategories();
    loadSuppliers();
    loadProducts();
}

ProductsPage::~ProductsPage() {}

void ProductsPage::setupUI() {
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ========= LEFT SIDE: Form =========
    QGroupBox* formBox = new QGroupBox(tr("معلومات المنتج"), this);
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

    // Row 0: Code | Name
    m_codeEdit = new StyledLineEdit(this);
    m_codeEdit->setPlaceholderText(tr("Product Code"));
    addLabel(tr("الكود :"), 0, 2);
    formLayout->addWidget(m_codeEdit, 0, 1);

    m_nameEdit = new StyledLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("Product Name"));
    addLabel(tr("الاسم :"), 0, 0);
    formLayout->addWidget(m_nameEdit, 0, 3);

    // Row 1: Category | Supplier
    m_categoryCombo = new StyledComboBox(this);
    addLabel(tr("الفئة :"), 1, 2);
    formLayout->addWidget(m_categoryCombo, 1, 1);

    m_supplierCombo = new StyledComboBox(this);
    addLabel(tr("المورد :"), 1, 0);
    formLayout->addWidget(m_supplierCombo, 1, 3);

    // Row 2: Purchase Price | Sale Price
    m_purchasePriceEdit = new StyledLineEdit(this);
    m_purchasePriceEdit->setPlaceholderText("0.00");
    addLabel(tr("سعر الشراء :"), 2, 2);
    formLayout->addWidget(m_purchasePriceEdit, 2, 1);

    m_salePriceEdit = new StyledLineEdit(this);
    m_salePriceEdit->setPlaceholderText("0.00");
    addLabel(tr("سعر البيع :"), 2, 0);
    formLayout->addWidget(m_salePriceEdit, 2, 3);

    // Row 3: Quantity | Min Stock
    m_quantityEdit = new StyledLineEdit(this);
    m_quantityEdit->setPlaceholderText("0");
    addLabel(tr("الكمية :"), 3, 2);
    formLayout->addWidget(m_quantityEdit, 3, 1);

    m_minStockEdit = new StyledLineEdit(this);
    m_minStockEdit->setPlaceholderText("10");
    addLabel(tr("الحد الأدنى :"), 3, 0);
    formLayout->addWidget(m_minStockEdit, 3, 3);

    // Row 4: Unit | Barcode
    m_unitCombo = new StyledComboBox(this);
    m_unitCombo->addItems({ tr("piece"), tr("kg"), tr("liter"), tr("box"), tr("carton") });
    addLabel(tr("الوحدة :"), 4, 2);
    formLayout->addWidget(m_unitCombo, 4, 1);

    m_barcodeEdit = new StyledLineEdit(this);
    m_barcodeEdit->setPlaceholderText(tr("Barcode"));
    addLabel(tr("الباركود :"), 4, 0);
    formLayout->addWidget(m_barcodeEdit, 4, 3);

    // Row 5: Active checkbox
    m_activeCheck = new QPushButton(tr("نشط"));
    m_activeCheck->setCheckable(true);
    m_activeCheck->setChecked(true);
    m_activeCheck->setStyleSheet(
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
    addLabel(tr("الحالة :"), 5, 2);
    formLayout->addWidget(m_activeCheck, 5, 1);

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

    formLayout->addLayout(btnLayout, 6, 0, 1, 4);

    mainLayout->addWidget(formBox, 1);

    // ========= RIGHT SIDE: Table + Filter + Print/Export =========
    QGroupBox* tableBox = new QGroupBox(tr("قائمة المنتجات"), this);
    tableBox->setStyleSheet(formBox->styleSheet());
    QVBoxLayout* tableLayout = new QVBoxLayout(tableBox);

    // Filter + Print/Export row
    QHBoxLayout* topBarLayout = new QHBoxLayout();

    QLabel* filterLbl = new QLabel(tr("فئة :"));
    filterLbl->setStyleSheet("color: #1a3a5c; font-weight: 600;");
    topBarLayout->addWidget(filterLbl);

    m_filterCategoryCombo = new StyledComboBox(this);
    m_filterCategoryCombo->addItem(tr("الكل"), 0);
    topBarLayout->addWidget(m_filterCategoryCombo);
    topBarLayout->addStretch();

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
        tr("ID"), tr("Code"), tr("Name"), tr("Category"),
        tr("Supplier"), tr("Sale Price"), tr("Qty"), tr("Status")
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
    connect(m_saveBtn, &QPushButton::clicked, this, &ProductsPage::onSaveClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &ProductsPage::onClearClicked);
    connect(m_newBtn, &QPushButton::clicked, this, &ProductsPage::onNewClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &ProductsPage::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &ProductsPage::onSearchTextChanged);
    connect(m_tableView, &QTableView::clicked, this, &ProductsPage::onTableClicked);
    connect(m_filterCategoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ProductsPage::onCategoryFilterChanged);
    connect(printBtn, &QPushButton::clicked, this, &ProductsPage::onPrintClicked);
    connect(exportBtn, &QPushButton::clicked, this, &ProductsPage::onExportClicked);
}

void ProductsPage::loadCategories() {
    m_categoryCombo->clear();
    m_filterCategoryCombo->clear();
    m_filterCategoryCombo->addItem(tr("الكل"), 0);

    QSqlQuery query("SELECT id, name FROM categories ORDER BY name");
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        m_categoryCombo->addItem(name, id);
        m_filterCategoryCombo->addItem(name, id);
    }
}

void ProductsPage::loadSuppliers() {
    m_supplierCombo->clear();
    QSqlQuery query("SELECT id, name FROM suppliers ORDER BY name");
    while (query.next()) {
        m_supplierCombo->addItem(query.value("name").toString(), query.value("id"));
    }
}

void ProductsPage::loadProducts() {
    m_tableModel->removeRows(0, m_tableModel->rowCount());

    int catFilter = m_filterCategoryCombo->currentData().toInt();
    QString sql =
        "SELECT p.id, p.code, p.name, c.name as category_name, "
        "s.name as supplier_name, p.sale_price, p.quantity, p.is_active "
        "FROM products p "
        "LEFT JOIN categories c ON p.category_id = c.id "
        "LEFT JOIN suppliers s ON p.supplier_id = s.id ";

    if (catFilter > 0) {
        sql += "WHERE p.category_id = :cat_id ";
    }
    sql += "ORDER BY p.name";

    QSqlQuery query;
    query.prepare(sql);
    if (catFilter > 0) query.bindValue(":cat_id", catFilter);
    query.exec();

    int row = 0;
    while (query.next()) {
        m_tableModel->insertRow(row);
        m_tableModel->setItem(row, 0, new QStandardItem(query.value("id").toString()));
        m_tableModel->setItem(row, 1, new QStandardItem(query.value("code").toString()));
        m_tableModel->setItem(row, 2, new QStandardItem(query.value("name").toString()));
        m_tableModel->setItem(row, 3, new QStandardItem(query.value("category_name").toString()));
        m_tableModel->setItem(row, 4, new QStandardItem(query.value("supplier_name").toString()));
        m_tableModel->setItem(row, 5, new QStandardItem(query.value("sale_price").toString()));
        m_tableModel->setItem(row, 6, new QStandardItem(query.value("quantity").toString()));
        m_tableModel->setItem(row, 7, new QStandardItem(
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
            "INSERT INTO products (code, name, category_id, supplier_id, purchase_price, "
            "sale_price, quantity, min_stock, unit, barcode, is_active) "
            "VALUES (:code, :name, :category, :supplier, :purchase, :sale, "
            ":quantity, :min_stock, :unit, :barcode, :active)"
        );
    }
    else {
        query.prepare(
            "UPDATE products SET code=:code, name=:name, category_id=:category, "
            "supplier_id=:supplier, purchase_price=:purchase, sale_price=:sale, "
            "quantity=:quantity, min_stock=:min_stock, unit=:unit, barcode=:barcode, "
            "is_active=:active WHERE id=:id"
        );
        query.bindValue(":id", m_currentId);
    }

    query.bindValue(":code", p.code);
    query.bindValue(":name", p.name);
    query.bindValue(":category", p.categoryId);
    query.bindValue(":supplier", p.supplierId);
    query.bindValue(":purchase", p.purchasePrice);
    query.bindValue(":sale", p.salePrice);
    query.bindValue(":quantity", p.quantity);
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
    }
}

void ProductsPage::onSearchTextChanged(const QString& text) {
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
    Q_UNUSED(index)
        loadProducts();
}

void ProductsPage::onPrintClicked() {
    QMessageBox::information(this, tr("Print"), tr("Print functionality requires Qt PrintSupport module.\nPlease add 'printsupport' to Qt Modules in project settings."));
}

void ProductsPage::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export CSV"), "products.csv", tr("CSV Files (*.csv)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file for writing"));
        return;
    }

    QTextStream stream(&file);
    stream << QString("\uFEFF");

    stream << "\"" << tr("Code") << "\","
        << "\"" << tr("Name") << "\","
        << "\"" << tr("Category") << "\","
        << "\"" << tr("Supplier") << "\","
        << "\"" << tr("Price") << "\","
        << "\"" << tr("Qty") << "\","
        << "\"" << tr("Status") << "\"\n";

    for (int r = 0; r < m_tableModel->rowCount(); ++r) {
        for (int c = 1; c <= 7; ++c) {
            stream << "\"" << m_tableModel->item(r, c)->text() << "\"";
            if (c < 7) stream << ",";
        }
        stream << "\n";
    }

    file.close();
    QMessageBox::information(this, tr("Success"), tr("Exported successfully to ") + fileName);
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

    int unitIndex = m_unitCombo->findText(p.unit);
    if (unitIndex >= 0) m_unitCombo->setCurrentIndex(unitIndex);

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
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<ProductsPage*>(this),
            tr("Warning"), tr("Product name is required!"));
        return false;
    }
    if (m_codeEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<ProductsPage*>(this),
            tr("Warning"), tr("Product code is required!"));
        return false;
    }
    return true;
}