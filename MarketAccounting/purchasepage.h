#pragma once
#ifndef PURCHASEPAGE_H
#define PURCHASEPAGE_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QVector>
#include <QMap>

class StyledLineEdit;
class StyledComboBox;

struct PurchaseItem {
    int productId = -1;
    QString code;
    QString name;
    double price = 0.0;
    int quantity = 1;
    double lineTotal = 0.0;
};

struct PurchaseHeader {
    int id = -1;
    int supplierId = 0;
    QString supplierName;
    QString reference;
    QDate date;
    QString paymentType = "cash";
    int accountId = 0;
    double discount = 0.0;
    double tax = 0.0;
    QString notes;
};

class PurchasePage : public QWidget
{
    Q_OBJECT
public:
    explicit PurchasePage(QWidget* parent = nullptr);
    ~PurchasePage();

private slots:
    void onSupplierChanged(int index);
    void onPaymentChanged(int index);
    void onSearchTextChanged(const QString& text);
    void onProductSelected(const QModelIndex& index);
    void onAddItemClicked();
    void onDeleteItemClicked();
    void onEditItemClicked();
    void onSaveClicked();
    void onClearClicked();
    void onTableClicked(const QModelIndex& index);
    void calculateTotals();
    void onRecentTableClicked(const QModelIndex& index);
    void onNewPurchaseClicked();

private:
    void setupUI();
    void loadSuppliers();
    void loadAccounts();
    void loadProducts(const QString& filter = QString());
    void loadRecentPurchases();
    void refreshCartTable();
    void clearForm();
    bool validateForm();
    bool savePurchase();

    // Header data
    PurchaseHeader m_header;
    QVector<PurchaseItem> m_items;
    int m_nextRef = 1000;

    // Left widgets
    StyledComboBox* m_supplierCombo = nullptr;
    StyledLineEdit* m_refEdit = nullptr;
    QDateEdit* m_dateEdit = nullptr;
    StyledComboBox* m_paymentCombo = nullptr;
    StyledComboBox* m_accountCombo = nullptr;
    StyledLineEdit* m_discountEdit = nullptr;
    StyledLineEdit* m_taxEdit = nullptr;
    StyledLineEdit* m_notesEdit = nullptr;

    // Item entry
    StyledLineEdit* m_searchEdit = nullptr;
    QTableView* m_productTable = nullptr;
    QStandardItemModel* m_productModel = nullptr;
    QSpinBox* m_qtySpin = nullptr;
    QDoubleSpinBox* m_priceSpin = nullptr;
    QPushButton* m_addItemBtn = nullptr;

    // Cart
    QTableView* m_cartTable = nullptr;
    QStandardItemModel* m_cartModel = nullptr;
    QPushButton* m_deleteItemBtn = nullptr;
    QPushButton* m_editItemBtn = nullptr;

    // Totals
    QLabel* m_totalLabel = nullptr;
    QLabel* m_netLabel = nullptr;

    // Buttons
    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_clearBtn = nullptr;
    QPushButton* m_newBtn = nullptr;

    // Recent purchases
    QTableView* m_recentTable = nullptr;
    QStandardItemModel* m_recentModel = nullptr;
};

#endif