#pragma once
#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>

class DashboardPage : public QWidget
{
    Q_OBJECT
public:
    explicit DashboardPage(QWidget* parent = nullptr);
    ~DashboardPage();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();
    void loadStats();
    void loadRecentTransactions();
    void loadLowStock();
    void loadMonthlyChartData();

    // Stat cards
    struct StatCard {
        QString title;
        QString value;
        QString subtitle;
        QString accentColor;
    };
    QVector<StatCard> m_stats;

    // Chart data
    QVector<QPair<QString, double>> m_chartData;
    double m_chartMax = 1.0;

    // Widgets
    QTableView* m_recentTable = nullptr;
    QStandardItemModel* m_recentModel = nullptr;

    QTableView* m_stockTable = nullptr;
    QStandardItemModel* m_stockModel = nullptr;

    QWidget* m_chartWidget = nullptr;
};

#endif