#ifndef KUAIPANOAUTH_H
#define KUAIPANOAUTH_H

#include <qstring.h>
#include <QMap>
#include <QObject>
#include <QtNetwork>
#include <QSqlDatabase>

namespace Kuaipan{
    class OAuth
    {
    private:
        QString _mConsumerKey;
        QString _mConsumerSecret;
        QString _mOAuthToken;
        QString _mOAuthTokenSecret;

        QString _configPath;
        QSqlDatabase _sqlite;
        QSqlQuery *_query;
        uint _lastSyncLocalTime;
        uint _lastSyncRemoteTime;
    public:
        OAuth(QString consumerKey, QString consumerSecret);
        ~OAuth();
        QString authorize();
        bool accessToken(QString oauthVerifier);
        void saveOAuthToken();
        bool readOAuthToken();

        QMap<QString,QVariant> account_info();

        bool SyncFiles(QString path);
        uint _syncFilesDownload(QString path);
        bool _syncFilesUpload(QString path);
    private:
        QMap<QString,QVariant> _sendRequest(QString method,QString url, QMap<QString,QString> params);
        bool _downloadFile(QString url, QMap<QString,QString> params, QString path);
        bool _uploadFile(QString path);
        bool _deleteFile(QString path);
        bool _makeDir(QString path);
        QString _makeSignature(QString method, QString baseUrl, QMap<QString,QString> params);
        QString _hmacSha1(QByteArray key, QByteArray baseString);
        QMap<QString,QString> _getBaseParams();

        bool _setRemoteSyncTime(uint time);
        bool _refreshRemoteSyncTime();
        bool _setLocalSyncTime(uint time);
        bool _refreshLocalSyncTime();
    };
}
#endif // KUAIPANOAUTH_H
