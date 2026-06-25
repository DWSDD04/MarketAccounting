#pragma once
#ifndef DEBTSPAGE_H
#define DEBTSPAGE_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QModelIndex>

class StyledLineEdit;
class StyledComboBox;
struct Debt;

class DebtsPage : public QWidget
{
    Q_OBJECT
public:
    explicit DebtsPage(QWidget* parent = nullptr);
    ~DebtsPage();

private slots:
    void onSaveClicked();
    void onClearClicked();
    void onNewClicked();
    void onDeleteClicked();
    void onSearchTextChanged(const QString& text);
    void onTableClicked(const QModelIndex& index);
    void onPartyTypeChanged(int index);
    void onPrintClicked();
    void onExportClicked();

private:
    void setupUI();
    void loadParties();
    void loadDebts();
    void clearForm();
    void populateForm(const Debt& debt);
    Debt collectFormData() const;
    bool validateForm() const;

    // Form fields
    StyledComboBox* m_partyTypeCombo;
    StyledComboBox* m_partyCombo;
    StyledLineEdit* m_amountEdit;
    StyledComboBox* m_debtTypeCombo;
    StyledLineEdit* m_dueDateEdit;
    StyledLineEdit* m_descriptionEdit;
    QPushButton* m_paidCheck;

    // Table
    QTableView* m_tableView;
    QStandardItemModel* m_tableModel;

    // Buttons
    QPushButton* m_newBtn;
    QPushButton* m_saveBtn;
    QPushButton* m_clearBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_printBtn;
    QPushButton* m_exportBtn;

    int m_currentId;
};

#endif