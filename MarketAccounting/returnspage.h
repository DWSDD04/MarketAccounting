#pragma once
#ifndef RETURNSPAGE_H
#define RETURNSPAGE_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QVector>

class StyledLineEdit;
class StyledComboBox;

struct ReturnItem {
    int originalItemId = -1;
    int productId = -1;
    QString code;
    QString name;
    double originalPrice = 0.0;
    int originalQty = 0;
    int returnQty = 0;
    double returnTotal = 0.0;
};

class ReturnsPage : public QWidget
{
    Q_OBJECT
public:
    explicit ReturnsPage(QWidget* parent = nullptr);
    ~ReturnsPage();

private slots:
    void onTypeChanged(int index);
    void onSearchOriginal();
    void onItemQtyChanged(const QModelIndex& index);
    void onSaveClicked();
    void onClearClicked();
    void onRecentTableClicked(const QModelIndex& index);
    void calculateReturnTotal();

private:
    void setupUI();
    void loadRecentReturns();
    void clearForm();
    bool validateForm();
    bool saveReturn();
    void loadOriginalItems(int transactionId);

    // Data
    int m_originalTransactionId = -1;
    QString m_originalRef;
    int m_partyId = 0;
    QVector<ReturnItem> m_items;

    // Form
    StyledComboBox* m_typeCombo = nullptr;
    StyledLineEdit* m_refEdit = nullptr;
    QPushButton* m_searchBtn = nullptr;
    QDateEdit* m_dateEdit = nullptr;
    StyledComboBox* m_refundMethodCombo = nullptr;
    StyledLineEdit* m_reasonEdit = nullptr;
    StyledLineEdit* m_discountEdit = nullptr;

    // Original items table
    QTableView* m_itemsTable = nullptr;
    QStandardItemModel* m_itemsModel = nullptr;

    // Totals
    QLabel* m_originalTotalLabel = nullptr;
    QLabel* m_returnTotalLabel = nullptr;
    QLabel* m_netRefundLabel = nullptr;

    // Buttons
    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_clearBtn = nullptr;

    // Recent returns
    QTableView* m_recentTable = nullptr;
    QStandardItemModel* m_recentModel = nullptr;
};

#endif