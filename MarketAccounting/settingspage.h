#pragma once
#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>

class StyledLineEdit;
class QPushButton;
class QLabel;

class SettingsPage : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsPage(QWidget* parent = nullptr);
    ~SettingsPage();

private slots:
    void onSaveClicked();
    void onReloadClicked();

private:
    void setupUI();
    void loadSettings();

    StyledLineEdit* m_companyNameEdit = nullptr;
    StyledLineEdit* m_exchangeRateEdit = nullptr;
    StyledLineEdit* m_vatPercentEdit = nullptr;
    StyledLineEdit* m_receiptHeaderEdit = nullptr;
    StyledLineEdit* m_receiptFooterEdit = nullptr;
    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_reloadBtn = nullptr;
    QLabel* m_statusLabel = nullptr;
};

#endif