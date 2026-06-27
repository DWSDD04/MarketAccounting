#pragma once
#ifndef PRODUCTSPAGE_H
#define PRODUCTSPAGE_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QModelIndex>

class StyledLineEdit;
class StyledComboBox;
struct Product;

class ProductsPage : public QWidget
{
    Q_OBJECT
public:
    explicit ProductsPage(QWidget* parent = nullptr);
    ~ProductsPage();

private slots:
    void onSaveClicked();
    void onClearClicked();
    void onNewClicked();
    void onDeleteClicked();
    void onSearchTextChanged(const QString& text);
    void onTableClicked(const QModelIndex& index);
    void onCategoryFilterChanged(int index);
    void onPrintClicked();
    void onExportClicked();

private:
    void setupUI();
    void loadCategories();
    void loadSuppliers();
    void loadProducts();
    void clearForm();
    void populateForm(const Product& product);
    Product collectFormData() const;
    bool validateForm() const;

    // Form fields
    StyledLineEdit* m_codeEdit;
    StyledLineEdit* m_nameEdit;
    StyledComboBox* m_categoryCombo;
    StyledComboBox* m_supplierCombo;
    StyledLineEdit* m_purchasePriceEdit;
    StyledLineEdit* m_salePriceEdit;
    StyledLineEdit* m_quantityEdit;
    StyledLineEdit* m_minStockEdit;
    StyledComboBox* m_unitCombo;
    StyledLineEdit* m_barcodeEdit;
    QPushButton* m_activeCheck;

    // Table
    QTableView* m_tableView;
    QStandardItemModel* m_tableModel;

    // Buttons
    QPushButton* m_newBtn;
    QPushButton* m_saveBtn;
    QPushButton* m_clearBtn;
    QPushButton* m_deleteBtn;

    // Filter
    StyledComboBox* m_filterCategoryCombo;

    int m_currentId;
};

#endif
