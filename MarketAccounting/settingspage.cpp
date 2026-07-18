#include "settingspage.h"
#include "styledlineedit.h"
#include "dbmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QGroupBox>

SettingsPage::SettingsPage(QWidget* parent) : QWidget(parent)
{
    setupUI();
    loadSettings();
}

SettingsPage::~SettingsPage() {}

void SettingsPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    QString btnStyle =
        "QPushButton { "
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #F0F5FA, stop:1 #D6E6F0); "
        "  border: 1px solid #9DB9D2; "
        "  border-radius: 3px; "
        "  padding: 8px 24px; "
        "  color: #1a3a5c; "
        "  font-weight: 600; "
        "}"
        "QPushButton:hover { background-color: #C8DDE8; }"
        "QPushButton:pressed { background-color: #B8D4E8; }";

    QLabel* title = new QLabel(tr("System Settings"));
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #1a3a5c; margin-bottom: 8px;");
    mainLayout->addWidget(title);

    QGroupBox* formBox = new QGroupBox(tr("General Settings"), this);
    formBox->setStyleSheet(
        "QGroupBox { "
        "  background-color: #E8F0F8; "
        "  border: 1px solid #9DB9D2; "
        "  border-radius: 4px; "
        "  margin-top: 12px; "
        "  font-weight: bold; "
        "  color: #1a3a5c; "
        "}"
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  left: 10px; "
        "  padding: 0 5px; "
        "}"
    );
    QGridLayout* grid = new QGridLayout(formBox);
    grid->setSpacing(10);
    grid->setContentsMargins(20, 20, 20, 20);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(3, 1);

    auto addLabel = [&](const QString& text, int row, int col) {
        QLabel* lbl = new QLabel(text);
        lbl->setStyleSheet("color: #1a3a5c; font-size: 13px; font-weight: 600;");
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        grid->addWidget(lbl, row, col);
        };

    m_companyNameEdit = new StyledLineEdit(this);
    addLabel(tr("Company Name:"), 0, 2);
    grid->addWidget(m_companyNameEdit, 0, 1, 1, 3);

    m_exchangeRateEdit = new StyledLineEdit(this);
    m_exchangeRateEdit->setPlaceholderText("89500");
    addLabel(tr("USD to LBP Rate:"), 1, 2);
    grid->addWidget(m_exchangeRateEdit, 1, 1);

    m_vatPercentEdit = new StyledLineEdit(this);
    m_vatPercentEdit->setPlaceholderText("0.00");
    addLabel(tr("VAT %:"), 1, 0);
    grid->addWidget(m_vatPercentEdit, 1, 3);

    m_receiptHeaderEdit = new StyledLineEdit(this);
    addLabel(tr("Receipt Header:"), 2, 2);
    grid->addWidget(m_receiptHeaderEdit, 2, 1, 1, 3);

    m_receiptFooterEdit = new StyledLineEdit(this);
    addLabel(tr("Receipt Footer:"), 3, 2);
    grid->addWidget(m_receiptFooterEdit, 3, 1, 1, 3);

    mainLayout->addWidget(formBox);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_saveBtn = new QPushButton(tr("Save Settings"));
    m_saveBtn->setStyleSheet(btnStyle);
    m_reloadBtn = new QPushButton(tr("Reload"));
    m_reloadBtn->setStyleSheet(btnStyle);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_reloadBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet("color: #28a745; font-size: 13px;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addStretch();

    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsPage::onSaveClicked);
    connect(m_reloadBtn, &QPushButton::clicked, this, &SettingsPage::onReloadClicked);
}

void SettingsPage::loadSettings()
{
    QSqlQuery query("SELECT setting_key, setting_value FROM settings");
    while (query.next()) {
        QString key = query.value("setting_key").toString();
        QString val = query.value("setting_value").toString();
        if (key == "company_name") m_companyNameEdit->setText(val);
        else if (key == "usd_to_lbp_rate") m_exchangeRateEdit->setText(val);
        else if (key == "vat_percent") m_vatPercentEdit->setText(val);
        else if (key == "receipt_header") m_receiptHeaderEdit->setText(val);
        else if (key == "receipt_footer") m_receiptFooterEdit->setText(val);
    }
}

void SettingsPage::onSaveClicked()
{
    auto updateSetting = [&](const QString& key, const QString& val) {
        QSqlQuery q;
        q.prepare("INSERT INTO settings (setting_key, setting_value) VALUES (:k, :v) "
            "ON DUPLICATE KEY UPDATE setting_value = :v");
        q.bindValue(":k", key);
        q.bindValue(":v", val);
        return q.exec();
        };

    bool ok = true;
    ok &= updateSetting("company_name", m_companyNameEdit->text().trimmed());
    ok &= updateSetting("usd_to_lbp_rate", m_exchangeRateEdit->text().trimmed());
    ok &= updateSetting("vat_percent", m_vatPercentEdit->text().trimmed());
    ok &= updateSetting("receipt_header", m_receiptHeaderEdit->text().trimmed());
    ok &= updateSetting("receipt_footer", m_receiptFooterEdit->text().trimmed());

    if (ok) {
        m_statusLabel->setText(tr("Settings saved successfully!"));
        m_statusLabel->setStyleSheet("color: #28a745; font-size: 13px;");
    }
    else {
        m_statusLabel->setText(tr("Error saving settings."));
        m_statusLabel->setStyleSheet("color: #dc3545; font-size: 13px;");
    }
}

void SettingsPage::onReloadClicked()
{
    loadSettings();
    m_statusLabel->setText(tr("Settings reloaded."));
    m_statusLabel->setStyleSheet("color: #1a3a5c; font-size: 13px;");
}