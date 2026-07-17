#pragma once
#ifndef REPORTSPAGE_H
#define REPORTSPAGE_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QDateEdit>

class ReportsPage : public QWidget
{
    Q_OBJECT
public:
    explicit ReportsPage(QWidget* parent = nullptr);
    ~ReportsPage();

private slots:
    void onReportTypeChanged(int index);
    void onGenerateClicked();
    void onExportClicked();

private:
    void setupUI();
    void loadCustomers();
    void generateMonthlyReport();
    void generateCustomerReport();
    void generateTopProductsReport();
    void updateSummary(double total, int count);

    // Filter controls
    QComboBox* m_reportTypeCombo;
    QComboBox* m_customerCombo;
    QLabel* m_customerLabel;
    QDateEdit* m_fromDate;
    QDateEdit* m_toDate;
    QLabel* m_fromLabel;
    QLabel* m_toLabel;
    QPushButton* m_generateBtn;
    QPushButton* m_exportBtn;

    // Summary cards
    QLabel* m_totalLabel;
    QLabel* m_countLabel;
    QLabel* m_avgLabel;

    // Results table
    QTableView* m_tableView;
    QStandardItemModel* m_tableModel;

    int m_currentReportType = 0;
};

#endif#pragma once
