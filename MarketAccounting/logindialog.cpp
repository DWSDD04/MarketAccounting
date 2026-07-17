#include "logindialog.h"
#include "dbmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("MarketPro - Login"));
    setFixedSize(420, 320);
    setLayoutDirection(Qt::RightToLeft);
    setupUI();
}

void LoginDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    QLabel* title = new QLabel("MarketPro");
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: #1a3a5c;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    QLabel* subtitle = new QLabel(tr("Accounting System"));
    subtitle->setStyleSheet("font-size: 13px; color: #6c757d; margin-bottom: 10px;");
    subtitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitle);

    // Form
    QGridLayout* formLayout = new QGridLayout();
    formLayout->setSpacing(12);

    auto addLabel = [&](const QString& text, int row) {
        QLabel* lbl = new QLabel(text);
        lbl->setStyleSheet("color: #1a3a5c; font-size: 13px; font-weight: 600;");
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        formLayout->addWidget(lbl, row, 0);
        };

    addLabel(tr("Username:"), 0);
    m_userEdit = new QLineEdit(this);
    m_userEdit->setPlaceholderText(tr("Enter username"));
    m_userEdit->setStyleSheet(
        "QLineEdit { background-color: white; border: 1px solid #9DB9D2; "
        "border-radius: 3px; padding: 8px 12px; font-size: 13px; color: #1a3a5c; }"
        "QLineEdit:focus { border: 2px solid #1a3a5c; }"
    );
    m_userEdit->setMinimumHeight(34);
    formLayout->addWidget(m_userEdit, 0, 1);

    addLabel(tr("Password:"), 1);
    m_passEdit = new QLineEdit(this);
    m_passEdit->setPlaceholderText(tr("Enter password"));
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setStyleSheet(m_userEdit->styleSheet());
    m_passEdit->setMinimumHeight(34);
    formLayout->addWidget(m_passEdit, 1, 1);

    mainLayout->addLayout(formLayout);

    // Status
    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet("color: #dc3545; font-size: 12px;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QString btnStyle =
        "QPushButton { "
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #1a3a5c, stop:1 #0f3460); "
        "  border: none; "
        "  border-radius: 4px; "
        "  padding: 10px 32px; "
        "  color: white; "
        "  font-weight: 600; "
        "  font-size: 14px; "
        "}"
        "QPushButton:hover { background-color: #0f2a4a; }"
        "QPushButton:pressed { background-color: #0a1f38; }";

    m_loginBtn = new QPushButton(tr("Login"));
    m_loginBtn->setStyleSheet(btnStyle);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setDefault(true);

    m_cancelBtn = new QPushButton(tr("Exit"));
    m_cancelBtn->setStyleSheet(
        "QPushButton { background-color: #e9ecef; border: 1px solid #dee2e6; "
        "border-radius: 4px; padding: 10px 24px; color: #495057; font-weight: 600; }"
        "QPushButton:hover { background-color: #dee2e6; }"
    );

    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_loginBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    mainLayout->addStretch();

    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &LoginDialog::onCancelClicked);
    connect(m_passEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
}

bool LoginDialog::authenticate(const QString& username, const QString& password)
{
    if (username.isEmpty() || password.isEmpty()) return false;

    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    QSqlQuery query;
    query.prepare("SELECT id, username, full_name, role, is_active FROM users "
        "WHERE username = :user AND password_hash = :hash");
    query.bindValue(":user", username);
    query.bindValue(":hash", QString(hash));

    if (!query.exec() || !query.next()) return false;

    m_user.id = query.value("id").toInt();
    m_user.username = query.value("username").toString();
    m_user.fullName = query.value("full_name").toString();
    m_user.role = query.value("role").toString();
    m_user.isActive = query.value("is_active").toBool();

    return m_user.isActive;
}

void LoginDialog::onLoginClicked()
{
    QString user = m_userEdit->text().trimmed();
    QString pass = m_passEdit->text();

    if (authenticate(user, pass)) {
        accept();
    }
    else {
        m_statusLabel->setText(tr("Invalid username or password."));
        m_passEdit->clear();
        m_passEdit->setFocus();
    }
}

void LoginDialog::onCancelClicked()
{
    reject();
}