#pragma once
#ifndef EXPENSESPAGE_H
#define EXPENSESPAGE_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>

class StyledLineEdit;
class StyledComboBox;

class ExpensesPage : public QWidget
{
    Q_OBJECT
public:
    explicit ExpensesPage(QWidget* parent = nullptr);
    ~ExpensesPage();

private slots:
    void onSaveClicked();
    void onClearClicked();
    void onDeleteClicked();
    void onSearchTextChanged(const QString& text);
    void onTableClicked(const QModelIndex& index);
    void onCategoryFilterChanged(int index);
    void onExportClicked();
    void loadExpenses();

private:
    void setupUI();
    void loadCategories();
    void clearForm();
    void populateForm(int id);
    bool validateForm();

    int m_currentId = -1;

    // Form
    QDateEdit* m_dateEdit = nullptr;
    StyledComboBox* m_categoryCombo = nullptr;
    StyledLineEdit* m_amountEdit = nullptr;
    StyledComboBox* m_paymentCombo = nullptr;
    StyledComboBox* m_accountCombo = nullptr;
    StyledLineEdit* m_descriptionEdit = nullptr;
    StyledLineEdit* m_receiptEdit = nullptr;

    // Buttons
    QPushButton* m_newBtn = nullptr;
    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_clearBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
    QPushButton* m_exportBtn = nullptr;

    // Table
    QTableView* m_tableView = nullptr;
    QStandardItemModel* m_tableModel = nullptr;

    // Filter
    StyledComboBox* m_filterCategoryCombo = nullptr;
    StyledLineEdit* m_searchEdit = nullptr;

    // Summary
    QLabel* m_totalLabel = nullptr;
    QLabel* m_countLabel = nullptr;
};

#endif // EXPENSESPAGE_H