#pragma once
#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class DbManager : public QObject
{
    Q_OBJECT
public:
    static DbManager* instance();

    bool connect(const QString& host, const QString& dbName,
        const QString& user, const QString& password, int port = 3306);
    bool isConnected() const;
    QSqlDatabase database() const;
    bool executeQuery(const QString& query);
    QSqlError lastError() const;

    bool transaction();
    bool commit();
    bool rollback();

private:
    explicit DbManager(QObject* parent = nullptr);
    ~DbManager();
    DbManager(const DbManager&) = delete;
    DbManager& operator=(const DbManager&) = delete;

    static DbManager* m_instance;
    QSqlDatabase m_db;
    bool m_connected = false;
};

#endif