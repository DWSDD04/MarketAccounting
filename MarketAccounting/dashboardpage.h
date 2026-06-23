#pragma once
#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>

class DashboardPage : public QWidget
{
    Q_OBJECT
public:
    explicit DashboardPage(QWidget* parent = nullptr);
};

#endif