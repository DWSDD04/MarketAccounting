#include "dbmanager.h"
#include <QDebug>

DbManager* DbManager::m_instance = nullptr;

DbManager::DbManager(QObject* parent) : QObject(parent) {}

DbManager::~DbManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

DbManager* DbManager::instance() {
    if (!m_instance) {
        m_instance = new DbManager();
    }
    return m_instance;
}

bool DbManager::connect(const QString& host, const QString& dbName,
    const QString& user, const QString& password, int port) {
    m_db = QSqlDatabase::addDatabase("QMYSQL");
    m_db.setHostName(host);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);
    m_db.setPort(port);
    m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1");

    m_connected = m_db.open();

    if (!m_connected) {
        qDebug() << "DB Error:" << m_db.lastError().text();
    }
    else {
        QSqlQuery query("SET NAMES 'utf8mb4'");
    }
    return m_connected;
}

bool DbManager::isConnected() const {
    return m_connected && m_db.isOpen();
}

QSqlDatabase DbManager::database() const {
    return m_db;
}

bool DbManager::executeQuery(const QString& query) {
    QSqlQuery q(query);
    return q.exec();
}

QSqlError DbManager::lastError() const {
    return m_db.lastError();
}

bool DbManager::transaction() {
    return m_db.transaction();
}

bool DbManager::commit() {
    return m_db.commit();
}

bool DbManager::rollback() {
    return m_db.rollback();
}