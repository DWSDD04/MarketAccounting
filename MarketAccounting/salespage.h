#pragma once
#ifndef SALESPAGE_H
#define SALESPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QStandardItemModel>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QListWidget>
#include <QGroupBox>
#include <QStackedWidget>

class StyledLineEdit;
class StyledComboBox;
class CardWidget;

struct SaleItem {
    int productId = -1;
    QString productCode;
    QString productName;
    double quantity = 0.0;
    double unitPrice = 0.0;
    double sum = 0.0;
};

struct SaleSession {
    int saleCode = -1;
    QList<SaleItem> items;
    QString paymentType; // "cash" or "debt"
    int accountId = -1;
    QString accountName;
    bool isSaved = false;
};

class PriceEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit PriceEditDialog(int productId, const QString& productName, double currentPrice, QWidget* parent = nullptr);
    double newPrice() const;

private:
    QDoubleSpinBox* m_priceSpin;
    double m_newPrice = 0.0;
};

class SalesPage : public QWidget {
    Q_OBJECT
public:
    explicit SalesPage(QWidget* parent = nullptr);
    ~SalesPage();

private slots:
    void onProductTypeClicked(QListWidgetItem* item);
    void onAddProductClicked();
    void onDeleteRowClicked();
    void onTableCellChanged(int row, int col);
    void onTableItemSelectionChanged();
    void onPaymentTypeChanged(QAbstractButton* btn);
    void onAccountChanged(int index);
    void onSearchProductChanged(const QString& text);
    void onSearchTableChanged(const QString& text);
    void onPrevSaleClicked();
    void onNextSaleClicked();
    void onEditByCodeClicked();
    void onSaleSlotClicked(int slotIndex);
    void onChangePriceClicked();
    void onSaveSaleClicked();
    void onNewSaleClicked();
    void onProductSearchSelected(const QModelIndex& index);

private:
    void setupUI();
    void loadProductTypes();
    void loadProductsByType(int typeId);
    void loadAccounts();
    void loadAllProducts();
    void refreshProductListWidget();
    void refreshTable();
    void updateTotals();
    void updateProductHistory(int productId);
    void switchToSession(int index);
    void generateSaleCode();
    double getExchangeRate() const; // LBP per 1 USD
    int getNextAvailableSaleCode();

    // Sessions
    QList<SaleSession> m_sessions;
    int m_currentSessionIndex = 0;
    int m_nextSaleCode = 1000;

    // UI Components
    // Top: Sale slots
    QPushButton* m_slotBtns[3];
    QLabel* m_slotLabels[3];

    // Navigation
    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;
    QPushButton* m_editCodeBtn;
    StyledLineEdit* m_codeEdit;

    // Payment & Account
    QButtonGroup* m_paymentGroup;
    QRadioButton* m_cashRadio;
    QRadioButton* m_debtRadio;
    StyledComboBox* m_accountCombo;

    // Product Type List
    QListWidget* m_typeList;
    int m_currentTypeId = -1;

    // Product Search & List
    StyledLineEdit* m_productSearch;
    QListWidget* m_productList;
    QList<QVariantMap> m_allProducts;
    QList<QVariantMap> m_filteredProducts;

    // Table
    QTableWidget* m_table;
    QStringList m_tableHeaders;

    // Table Search
    StyledLineEdit* m_tableSearch;

    // Totals
    QLabel* m_totalLbpLabel;
    QLabel* m_totalUsdLabel;

    // Product History
    QLabel* m_historyProductLabel;
    QLabel* m_historyTotalSoldLabel;
    QLabel* m_historyRemainingLabel;
    QLabel* m_historyAvgPriceLabel;

    // Buttons
    QPushButton* m_addBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_changePriceBtn;
    QPushButton* m_saveSaleBtn;
    QPushButton* m_newSaleBtn;

    // Currently selected product in table
    int m_selectedProductId = -1;

    // Exchange rate (LBP per USD) - can be fetched from DB or hardcoded
    double m_exchangeRate = 89500.0;
};

#endif