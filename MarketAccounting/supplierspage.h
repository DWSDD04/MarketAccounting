#ifndef SUPPLIERSPAGE_H
#define SUPPLIERSPAGE_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>

// Forward declarations (use struct to match supplier.h)
struct Supplier;

class StyledLineEdit;
class StyledComboBox;
class CardWidget;

class SuppliersPage : public QWidget
{
    Q_OBJECT
public:
    explicit SuppliersPage(QWidget* parent = nullptr);
    ~SuppliersPage();

private slots:
    void onSaveClicked();
    void onClearClicked();
    void onSearchTextChanged(const QString& text);
    void onTableClicked(const QModelIndex& index);
    void onDeleteClicked();
    void onNewClicked();

private:
    void setupUI();
    void loadSuppliers();
    void loadGroups();
    void clearForm();
    void populateForm(const Supplier& supplier);
    Supplier collectFormData() const;
    bool validateForm() const;

    StyledLineEdit* m_nameEdit = nullptr;
    StyledLineEdit* m_addressEdit = nullptr;
    StyledLineEdit* m_phoneEdit = nullptr;
    StyledLineEdit* m_faxEdit = nullptr;
    StyledLineEdit* m_accountEdit = nullptr;
    StyledLineEdit* m_vatEdit = nullptr;
    StyledComboBox* m_currencyCombo = nullptr;
    StyledLineEdit* m_creditLimitEdit = nullptr;
    StyledComboBox* m_groupCombo = nullptr;
    StyledLineEdit* m_notesEdit = nullptr;

    QTableView* m_tableView = nullptr;
    QStandardItemModel* m_tableModel = nullptr;

    QPushButton* m_newBtn = nullptr;
    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_clearBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;

    int m_currentId = -1;
};

#endif