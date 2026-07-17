#include "dashboardpage.h"
#include "dbmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPainter>
#include <QPaintEvent>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QTimer>

// ─── Simple Chart Widget ───
class MiniBarChart : public QWidget {
    Q_OBJECT  // ← ADDED THIS
public:
    explicit MiniBarChart(QWidget* parent = nullptr) : QWidget(parent) {}
    QVector<QPair<QString, double>> data;
    double maxValue = 1.0;

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        int padding = 20;
        int barAreaW = width() - padding * 2;
        int barAreaH = height() - padding * 2 - 20;
        int n = data.size();
        if (n == 0) return;

        int gap = 8;
        int barW = (barAreaW - gap * (n - 1)) / n;

        for (int i = 0; i < n; ++i) {
            double ratio = qMin(1.0, data[i].second / maxValue);
            int barH = int(barAreaH * ratio);
            int x = padding + i * (barW + gap);
            int y = height() - padding - 20 - barH;

            // Bar shadow
            p.fillRect(x + 2, y + 2, barW, barH, QColor("#d0dce8"));

            // Bar gradient
            QLinearGradient grad(x, y, x, y + barH);
            grad.setColorAt(0, QColor("#1a3a5c"));
            grad.setColorAt(1, QColor("#4a7fb5"));
            p.fillRect(x, y, barW, barH, grad);

            // Value label
            p.setPen(QColor("#1a3a5c"));
            p.setFont(QFont("Segoe UI", 8, QFont::Bold));
            QString val = QString::number(data[i].second, 'f', 0);
            p.drawText(x, y - 14, barW, 12, Qt::AlignCenter, val);

            // Month label
            p.setPen(QColor("#6c757d"));
            p.setFont(QFont("Segoe UI", 8));
            p.drawText(x, height() - padding - 18, barW, 16, Qt::AlignCenter, data[i].first);
        }
    }
};

// ─── DashboardPage ───
DashboardPage::DashboardPage(QWidget* parent) : QWidget(parent)
{
    setupUI();
    loadStats();
    loadRecentTransactions();
    loadLowStock();
    loadMonthlyChartData();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        loadStats();
        loadRecentTransactions();
        loadLowStock();
        loadMonthlyChartData();
        });
    timer->start(30000);
}

DashboardPage::~DashboardPage() {}

void DashboardPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    QLabel* title = new QLabel(tr("Dashboard Overview"));
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #1a3a5c; margin-bottom: 4px;");
    mainLayout->addWidget(title);

    QLabel* subtitle = new QLabel(tr("Real-time overview of your market performance"));
    subtitle->setStyleSheet("font-size: 13px; color: #6c757d; margin-bottom: 10px;");
    mainLayout->addWidget(subtitle);

    // ─── Stat Cards Row ───
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(15);

    auto createCard = [&](const QString& titleText, const QString& accent) -> QFrame* {
        QFrame* card = new QFrame(this);
        card->setStyleSheet(QString(
            "QFrame { "
            " background-color: #FFFFFF; "
            " border: 1px solid #9DB9D2; "
            " border-radius: 6px; "
            " border-left: 4px solid %1; "
            "}"
        ).arg(accent));
        card->setMinimumHeight(110);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QVBoxLayout* v = new QVBoxLayout(card);
        v->setContentsMargins(16, 14, 16, 14);
        v->setSpacing(6);

        QLabel* lblTitle = new QLabel(titleText);
        lblTitle->setStyleSheet("font-size: 12px; color: #6c757d; font-weight: 600;");
        v->addWidget(lblTitle);

        QLabel* lblValue = new QLabel("0");
        lblValue->setStyleSheet("font-size: 26px; font-weight: bold; color: #1a3a5c;");
        lblValue->setObjectName("value_" + titleText);
        v->addWidget(lblValue);

        QLabel* lblSub = new QLabel("");
        lblSub->setStyleSheet("font-size: 11px; color: #6c757d;");
        lblSub->setObjectName("sub_" + titleText);
        v->addWidget(lblSub);

        return card;
        };

    cardsLayout->addWidget(createCard(tr("Today's Sales"), "#0f3460"));
    cardsLayout->addWidget(createCard(tr("This Month"), "#16213e"));
    cardsLayout->addWidget(createCard(tr("Active Customers"), "#1a3a5c"));
    cardsLayout->addWidget(createCard(tr("Low Stock Items"), "#dc3545"));
    mainLayout->addLayout(cardsLayout);

    // ─── Middle Row: Chart + Alerts ───
    QHBoxLayout* midLayout = new QHBoxLayout();
    midLayout->setSpacing(15);

    QFrame* chartFrame = new QFrame(this);
    chartFrame->setStyleSheet(
        "QFrame { background-color: #FFFFFF; border: 1px solid #9DB9D2; border-radius: 6px; }"
    );
    QVBoxLayout* chartLayout = new QVBoxLayout(chartFrame);
    chartLayout->setContentsMargins(16, 16, 16, 16);

    QLabel* chartTitle = new QLabel(tr("Revenue Trend (Last 6 Months)"));
    chartTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1a3a5c; margin-bottom: 8px;");
    chartLayout->addWidget(chartTitle);

    m_chartWidget = new MiniBarChart(this);
    m_chartWidget->setMinimumHeight(220);
    chartLayout->addWidget(m_chartWidget, 1);

    midLayout->addWidget(chartFrame, 2);

    QFrame* alertFrame = new QFrame(this);
    alertFrame->setStyleSheet(chartFrame->styleSheet());
    QVBoxLayout* alertLayout = new QVBoxLayout(alertFrame);
    alertLayout->setContentsMargins(16, 16, 16, 16);

    QLabel* alertTitle = new QLabel(tr("Low Stock Alerts"));
    alertTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1a3a5c; margin-bottom: 8px;");
    alertLayout->addWidget(alertTitle);

    m_stockTable = new QTableView(this);
    m_stockModel = new QStandardItemModel(this);
    m_stockModel->setHorizontalHeaderLabels({ tr("Product"), tr("Stock"), tr("Min") });
    m_stockTable->setModel(m_stockModel);
    m_stockTable->setStyleSheet(
        "QTableView { background-color: #FFFFFF; border: none; gridline-color: #E8F0F8; "
        "selection-background-color: #B8D4E8; selection-color: #000000; }"
        "QHeaderView::section { background-color: #F0F5FA; padding: 6px; border: none; "
        "font-weight: 600; color: #1a3a5c; font-size: 11px; }"
        "QTableView::item { padding: 5px; font-size: 12px; }"
        "QTableView::item:alternate { background-color: #F5F9FC; }"
    );
    m_stockTable->horizontalHeader()->setStretchLastSection(true);
    m_stockTable->setAlternatingRowColors(true);
    m_stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_stockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_stockTable->verticalHeader()->setVisible(false);
    alertLayout->addWidget(m_stockTable, 1);

    midLayout->addWidget(alertFrame, 1);
    mainLayout->addLayout(midLayout, 2);

    // ─── Bottom: Recent Transactions ───
    QFrame* recentFrame = new QFrame(this);
    recentFrame->setStyleSheet(chartFrame->styleSheet());
    QVBoxLayout* recentLayout = new QVBoxLayout(recentFrame);
    recentLayout->setContentsMargins(16, 16, 16, 16);

    QLabel* recentTitle = new QLabel(tr("Recent Transactions"));
    recentTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1a3a5c; margin-bottom: 8px;");
    recentLayout->addWidget(recentTitle);

    m_recentTable = new QTableView(this);
    m_recentModel = new QStandardItemModel(this);
    m_recentModel->setHorizontalHeaderLabels({
        tr("ID"), tr("Ref"), tr("Type"), tr("Amount"), tr("Discount"), tr("Net"), tr("Date")
        });
    m_recentTable->setModel(m_recentModel);
    m_recentTable->setStyleSheet(
        "QTableView { background-color: #FFFFFF; border: none; gridline-color: #D6E6F0; "
        "selection-background-color: #B8D4E8; selection-color: #000000; }"
        "QHeaderView::section { background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #F0F5FA, stop:1 #D6E6F0); padding: 8px; border: 1px solid #9DB9D2; "
        "border-left: none; font-weight: 600; color: #1a3a5c; }"
        "QTableView::item { padding: 6px; }"
        "QTableView::item:alternate { background-color: #F5F9FC; }"
    );
    m_recentTable->horizontalHeader()->setStretchLastSection(true);
    m_recentTable->setAlternatingRowColors(true);
    m_recentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recentTable->hideColumn(0);
    m_recentTable->verticalHeader()->setVisible(false);
    recentLayout->addWidget(m_recentTable, 1);

    mainLayout->addWidget(recentFrame, 3);
}

void DashboardPage::loadStats()
{
    auto findVal = [&](const QString& name) -> QLabel* {
        return findChild<QLabel*>("value_" + name);
        };
    auto findSub = [&](const QString& name) -> QLabel* {
        return findChild<QLabel*>("sub_" + name);
        };

    QSqlQuery q1;
    q1.prepare("SELECT COALESCE(SUM(net_amount), 0) as total, COUNT(*) as cnt "
        "FROM transactions WHERE type = 'sale' AND DATE(created_at) = CURDATE()");
    if (q1.exec() && q1.next()) {
        findVal(tr("Today's Sales"))->setText(QString::number(q1.value("total").toDouble(), 'f', 2));
        findSub(tr("Today's Sales"))->setText(tr("%1 transactions").arg(q1.value("cnt").toInt()));
    }

    QSqlQuery q2;
    q2.prepare("SELECT COALESCE(SUM(net_amount), 0) as total, COUNT(*) as cnt "
        "FROM transactions WHERE type = 'sale' "
        "AND MONTH(created_at) = MONTH(CURDATE()) AND YEAR(created_at) = YEAR(CURDATE())");
    if (q2.exec() && q2.next()) {
        findVal(tr("This Month"))->setText(QString::number(q2.value("total").toDouble(), 'f', 2));
        findSub(tr("This Month"))->setText(tr("%1 transactions").arg(q2.value("cnt").toInt()));
    }

    QSqlQuery q3;
    q3.exec("SELECT COUNT(*) as cnt FROM customers WHERE is_active = 1");
    if (q3.next()) {
        findVal(tr("Active Customers"))->setText(q3.value("cnt").toString());
        findSub(tr("Active Customers"))->setText(tr("Total registered customers"));
    }

    QSqlQuery q4;
    q4.exec("SELECT COUNT(*) as cnt FROM products WHERE quantity <= min_stock AND is_active = 1");
    if (q4.next()) {
        int low = q4.value("cnt").toInt();
        findVal(tr("Low Stock Items"))->setText(QString::number(low));
        findSub(tr("Low Stock Items"))->setText(low > 0 ? tr("Requires attention") : tr("All stock healthy"));
    }
}

void DashboardPage::loadRecentTransactions()
{
    m_recentModel->removeRows(0, m_recentModel->rowCount());

    QSqlQuery query(
        "SELECT id, reference_number, type, total_amount, discount, net_amount, created_at "
        "FROM transactions ORDER BY created_at DESC LIMIT 10"
    );

    int row = 0;
    while (query.next()) {
        m_recentModel->insertRow(row);
        m_recentModel->setItem(row, 0, new QStandardItem(query.value("id").toString()));
        m_recentModel->setItem(row, 1, new QStandardItem(query.value("reference_number").toString()));
        m_recentModel->setItem(row, 2, new QStandardItem(query.value("type").toString()));
        m_recentModel->setItem(row, 3, new QStandardItem(QString::number(query.value("total_amount").toDouble(), 'f', 2)));
        m_recentModel->setItem(row, 4, new QStandardItem(QString::number(query.value("discount").toDouble(), 'f', 2)));
        m_recentModel->setItem(row, 5, new QStandardItem(QString::number(query.value("net_amount").toDouble(), 'f', 2)));
        m_recentModel->setItem(row, 6, new QStandardItem(query.value("created_at").toDateTime().toString("yyyy-MM-dd hh:mm")));
        row++;
    }

    m_recentTable->resizeColumnsToContents();
}

void DashboardPage::loadLowStock()
{
    m_stockModel->removeRows(0, m_stockModel->rowCount());

    QSqlQuery query(
        "SELECT name, quantity, min_stock FROM products "
        "WHERE quantity <= min_stock AND is_active = 1 ORDER BY quantity ASC LIMIT 8"
    );

    int row = 0;
    while (query.next()) {
        m_stockModel->insertRow(row);
        m_stockModel->setItem(row, 0, new QStandardItem(query.value("name").toString()));
        m_stockModel->setItem(row, 1, new QStandardItem(query.value("quantity").toString()));
        m_stockModel->setItem(row, 2, new QStandardItem(query.value("min_stock").toString()));
        row++;
    }

    m_stockTable->resizeColumnsToContents();
}

void DashboardPage::loadMonthlyChartData()
{
    m_chartData.clear();
    m_chartMax = 1.0;

    QSqlQuery query(
        "SELECT DATE_FORMAT(created_at, '%Y-%m') as month, COALESCE(SUM(net_amount), 0) as total "
        "FROM transactions WHERE type = 'sale' "
        "AND created_at >= DATE_SUB(CURDATE(), INTERVAL 5 MONTH) "
        "GROUP BY month ORDER BY month"
    );

    while (query.next()) {
        QString month = query.value("month").toString();
        double total = query.value("total").toDouble();
        QDate d = QDate::fromString(month + "-01", "yyyy-MM-dd");
        QString label = d.toString("MMM");
        m_chartData.append({ label, total });
        if (total > m_chartMax) m_chartMax = total;
    }

    if (m_chartData.size() < 6) {
        for (int i = m_chartData.size(); i < 6; ++i) {
            m_chartData.prepend({ "-", 0.0 });
        }
    }

    MiniBarChart* chart = qobject_cast<MiniBarChart*>(m_chartWidget);
    if (chart) {
        chart->data = m_chartData;
        chart->maxValue = m_chartMax * 1.1;
        chart->update();
    }
}

void DashboardPage::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    QLinearGradient grad(0, 0, 0, height());
    grad.setColorAt(0, QColor("#f5f8fb"));
    grad.setColorAt(1, QColor("#e8f0f8"));
    p.fillRect(rect(), grad);
    QWidget::paintEvent(event);
}