#pragma once
#ifndef SALESPAGE_H
#define SALESPAGE_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QButtonGroup>
#include <QMap>
#include <QVector>
#include <QTabWidget>
#include <QDateEdit>
#include <QVariant>

class StyledLineEdit;
class StyledComboBox;
class CardWidget;

struct SaleItem {
    int productId = -1;
    QString code;
    QString name;
    double price = 0.0;
    int quantity = 1;
    double lineTotal = 0.0;
    QMap<QString, QVariant> customValues;
};

struct SaleSession {
    int id = -1;
    int code = 0;
    QString paymentType = "cash";
    int accountId = 0;
    QString accountName;
    int customerId = 0;
    QString customerName;
    QVector<SaleItem> items;
    double totalLBP = 0.0;
    double totalUSD = 0.0;
};

class SalesPage : public QWidget
{
    Q_OBJECT
public:
    explicit SalesPage(QWidget* parent = nullptr);
    ~SalesPage();

public slots:
    void scanProductCode(const QString& code);

private slots:
    void onCategoryClicked(int categoryId);
    void onProductSelected(const QModelIndex& index);
    void onAddProductClicked();
    void onDeleteItemClicked();
    void onEditItemClicked();
    void onPaymentTypeChanged(int index);
    void onAccountChanged(int index);
    void onSaleSlotClicked(int slot);
    void onPrevSaleClicked();
    void onNextSaleClicked();
    void onEditByCodeClicked();
    void onSearchTextChanged(const QString& text);
    void onSaleTableClicked(const QModelIndex& index);
    void onChangePriceClicked();
    void calculateNet();
    void onManageColumnsClicked();
    void onCustomerChanged(int index);

    // NEW: Save, Print, and Settings reload
    void onSaveClicked();
    void onPrintReceiptClicked();
    void loadSettings();

private:
    void setupUI();
    void loadCategories();
    void loadAccounts();
    void loadCustomers();
    void loadProductsByCategory(int categoryId);
    void refreshSaleTable();
    void refreshTotals();
    void refreshProductHistory(int productId);
    void switchToSlot(int slot);
    void clearCurrentSale();
    void loadSaleFromSession(int slot);
    int generateSaleCode();
    void updateSlotButtons();
    void enforcePaymentRules();

    void rebuildSaleModelHeaders();
    void addCustomColumn(const QString& name);
    void removeCustomColumn(int colIndex);
    void editCustomColumn(int colIndex, const QString& newName);

    void buildSaleInfoTab(QTabWidget* tabs);
    void buildItemsTab(QTabWidget* tabs);
    void buildAccountsTab(QTabWidget* tabs);

    QVector<SaleSession> m_sessions;
    int m_currentSlot = 0;
    int m_nextSaleCode = 1000;
    QMap<int, QString> m_categories;
    int m_currentCategoryId = -1;
    QMap<int, QString> m_customColumns;

    QButtonGroup* m_slotButtons = nullptr;
    QPushButton* m_slot1Btn = nullptr;
    QPushButton* m_slot2Btn = nullptr;
    QPushButton* m_slot3Btn = nullptr;

    QLabel* m_saleCodeLabel = nullptr;
    StyledComboBox* m_paymentCombo = nullptr;
    StyledComboBox* m_accountCombo = nullptr;
    StyledLineEdit* m_searchEdit = nullptr;

    QPushButton* m_prevBtn = nullptr;
    QPushButton* m_nextBtn = nullptr;
    QPushButton* m_editCodeBtn = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
    QPushButton* m_editBtn = nullptr;
    QPushButton* m_changePriceBtn = nullptr;

    QTableView* m_productTable = nullptr;
    QStandardItemModel* m_productModel = nullptr;

    QTableView* m_saleTable = nullptr;
    QStandardItemModel* m_saleModel = nullptr;

    QLabel* m_totalLBPLabel = nullptr;
    QLabel* m_totalUSDLabel = nullptr;

    QLabel* m_histTotalSold = nullptr;
    QLabel* m_histRemaining = nullptr;
    QLabel* m_histAvgPrice = nullptr;
    QLabel* m_histLastSale = nullptr;

    QSpinBox* m_quantitySpin = nullptr;

    StyledLineEdit* m_refEdit = nullptr;
    QDateEdit* m_dateEdit = nullptr;
    StyledComboBox* m_customerCombo = nullptr;
    StyledLineEdit* m_discountEdit = nullptr;
    StyledLineEdit* m_taxEdit = nullptr;
    StyledLineEdit* m_netEdit = nullptr;
    StyledLineEdit* m_notesEdit = nullptr;

    // NEW: Receipt printing & live exchange rate
    double m_usdToLbpRate = 89500.0;
    QString m_companyName;
    QString m_receiptHeader;
    QString m_receiptFooter;
    QPushButton* m_printReceiptBtn = nullptr;
};

#endif