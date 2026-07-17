#pragma once
#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "user.h"

class QLineEdit;
class QPushButton;
class QLabel;

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent = nullptr);
    User authenticatedUser() const { return m_user; }

private slots:
    void onLoginClicked();
    void onCancelClicked();

private:
    void setupUI();
    bool authenticate(const QString& username, const QString& password);

    QLineEdit* m_userEdit = nullptr;
    QLineEdit* m_passEdit = nullptr;
    QPushButton* m_loginBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;
    QLabel* m_statusLabel = nullptr;
    User m_user;
};

#endif