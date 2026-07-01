#pragma once
#ifndef CUSTOMERSPAGE_H
#define CUSTOMERSPAGE_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class CustomersPage : public QWidget
{
    Q_OBJECT
public:
    explicit CustomersPage(QWidget* parent = nullptr);
    ~CustomersPage();

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onSearchTextChanged(const QString& text);
    void onTableClicked(const QModelIndex& index);

private:
    void setupUI();
    void refreshTable();
    void clearSelection();

    QTableView* m_table = nullptr;
    QStandardItemModel* m_model = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_editBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QLabel* m_countLabel = nullptr;

    int m_selectedId = -1;
    int m_selectedRow = -1;
};

#endif