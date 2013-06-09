#include "kuaipanoauth.h"
#include "kuaipanhttp.h"
#include <QtNetwork>
#include <QJsonDocument>
#include <QFile>
#include <QSqlQuery>

using namespace Kuaipan;

OAuth::OAuth(QString consumerKey, QString consumerSecret)
{
    _mConsumerKey = consumerKey;
    _mConsumerSecret = consumerSecret;
    _configPath =  getenv("HOME");
    _configPath += "/.kuaipan/";
    _sqlite = QSqlDatabase::addDatabase("QSQLITE");
    _sqlite.setDatabaseName(_configPath + "config.db");
    _sqlite.open();
    _query = new QSqlQuery(_sqlite);
    _query->exec("CREATE TABLE IF NOT EXISTS `config` (`key` VARCHAR(64) UNIQUE PRIMARY KEY, `value` VARCHAR(255))");

    _refreshRemoteSyncTime();
    _refreshLocalSyncTime();
}

bool OAuth::_refreshRemoteSyncTime()
{
    _query->exec("SELECT `value` FROM `config` WHERE `key` = 'lastsyncremotetime'");
    if(_query->next()){
        _lastSyncRemoteTime = _query->value(0).toString().toUInt();
    }else{
        _setRemoteSyncTime(0);
    }
    return true;
}

bool OAuth::_setRemoteSyncTime(uint time)
{
    _lastSyncRemoteTime = time;
    _query->exec("DELETE FROM `config` WHERE `key` = 'lastsyncremotetime'");
    _query->exec("INSERT INTO `config`(`key`,`value`) VALUES('lastsyncremotetime','" + QString::number(time) + "')");
    return true;
}

bool OAuth::_refreshLocalSyncTime()
{
    _query->exec("SELECT `value` FROM `config` WHERE `key` = 'lastsynclocaltime'");
    if(_query->next()){
        _lastSyncLocalTime = _query->value(0).toString().toUInt();
    }else{
        _setLocalSyncTime(0);
    }
    return true;
}

bool OAuth::_setLocalSyncTime(uint time)
{
    _lastSyncLocalTime = time;
    _query->exec("DELETE FROM `config` WHERE `key` = 'lastsynclocaltime'");
    _query->exec("INSERT INTO `config`(`key`,`value`) VALUES('lastsynclocaltime','" + QString::number(time) + "')");
    return true;
}

OAuth::~OAuth()
{
    _query->clear();
    delete _query;
    _sqlite.close();
}

QString OAuth::authorize()
{
    QMap<QString,QVariant> obj;
    QMap<QString,QString> params  = _getBaseParams();
    obj = _sendRequest("GET","https://openapi.kuaipan.cn/open/requestToken",params);

    _mOAuthToken = obj["oauth_token"].toString();
    _mOAuthTokenSecret = obj["oauth_token_secret"].toString();
    obj.clear();
    params.clear();

    return "https://www.kuaipan.cn/api.php?ac=open&op=authorise&oauth_token="+_mOAuthToken;
}

bool OAuth::accessToken(QString oauthVerifier)
{
    QMap<QString,QVariant> obj;
    QMap<QString,QString> params  = _getBaseParams();
    params["oauth_verifier"] = oauthVerifier;
    obj = _sendRequest("GET","https://openapi.kuaipan.cn/open/accessToken",params);
    _mOAuthToken = obj["oauth_token"].toString();
    _mOAuthTokenSecret = obj["oauth_token_secret"].toString();
    params.clear();
    if(obj["user_id"].toString().length()>0){
        obj.clear();
        return true;
    }else{
        return false;
    }
}

void OAuth::saveOAuthToken()
{
    _query->exec("DELETE FROM `config` WHERE `key` = 'oauth'");
    _query->exec("INSERT INTO `config`(`key`,`value`) VALUES('oauth','" + _mOAuthToken +":"+_mOAuthTokenSecret+ "')");
}

bool OAuth::readOAuthToken()
{
    _query->exec("SELECT `value` FROM `config` WHERE `key` = 'oauth'");
    if(_query->next()){
        QString oauth = _query->value(0).toString();
        _mOAuthToken = oauth.left(oauth.indexOf(':'));
        _mOAuthTokenSecret = oauth.right(oauth.length() - oauth.indexOf(':') - 1);
        QMap<QString,QVariant>user = account_info();
        if(user["user_name"].toString().length()>0){
            user.clear();
            return true;
        }
        user.clear();
    }
    _mOAuthToken = "";
    _mOAuthTokenSecret = "";
    return false;
}

QMap<QString,QVariant> OAuth::account_info()
{
    QMap<QString,QVariant> obj;
    QMap<QString,QString> params  = _getBaseParams();
    obj = _sendRequest("GET","http://openapi.kuaipan.cn/1/account_info",params);
    params.clear();
    return obj;
}

QMap<QString,QVariant> OAuth::_sendRequest(QString method,QString url, QMap<QString,QString> params)
{
    method = method.toUpper();
    params["oauth_signature"] = _makeSignature(method,url,params);
    Http http;
    if(method=="GET"){
        QString json = http.get(url,params);
        return QJsonDocument::fromJson(json.toUtf8()).toVariant().toMap();
    }else{
        QString json = http.post(url,params);
        return QJsonDocument::fromJson(json.toUtf8()).toVariant().toMap();
    }
}


bool OAuth::_downloadFile(QString url, QMap<QString,QString> params, QString path)
{
    params["oauth_signature"] = _makeSignature("GET",url,params);
    Http http;
    return http.downloadGet(url,params,path);
}

bool OAuth::_makeDir(QString path)
{
    QMap<QString,QString> params = _getBaseParams();
    params["root"] = "kuaipan";
    params["path"] = path;
    _sendRequest("GET","http://openapi.kuaipan.cn/1/fileops/create_folder",params);
    params.clear();
    return true;
}

bool OAuth::_uploadFile(QString path)
{
    QMap<QString,QString> params = _getBaseParams();
    QMap<QString,QVariant> obj = _sendRequest("GET","http://api-content.dfs.kuaipan.cn/1/fileops/upload_locate",params);
    QString url = obj["url"].toString() + "/1/fileops/upload_file";
    Http http;
    params["overwrite"] = "True";
    params["root"] = "kuaipan";
    params["path"] = path;
    params["oauth_signature"] = _makeSignature("POST",url,params);

    return http.uploadPost(url,params,_configPath+"kuaipan/"+path);
}

bool OAuth::_deleteFile(QString path)
{
    QMap<QString,QString> params = _getBaseParams();
    params["root"]="kuaipan";
    params["path"]=path;
    _sendRequest("GET","http://openapi.kuaipan.cn/1/fileops/delete",params);
    return true;
}

QString OAuth::_makeSignature(QString method, QString baseUrl, QMap<QString,QString> params)
{
    QStringList list;
    QMap<QString,QString>::iterator it;
    for(it=params.begin();it!=params.end();++it){
        list<< QUrl::toPercentEncoding(it.key()) + "=" + QUrl::toPercentEncoding(it.value());
    }
    list.sort();
    QString baseString = method.toUpper()+"&"+ QUrl::toPercentEncoding(baseUrl)+"&"+ QUrl::toPercentEncoding(list.join('&'));
    QString key = (_mConsumerSecret+"&"+_mOAuthTokenSecret);
    return _hmacSha1(key.toUtf8(),baseString.toUtf8());
}

QString OAuth::_hmacSha1(QByteArray key, QByteArray baseString)
{
    int blockSize = 64; // HMAC-SHA-1 block size, defined in SHA-1 standard
    if (key.length() > blockSize) { // if key is longer than block size (64), reduce key length with SHA-1 compression
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }

    QByteArray innerPadding(blockSize, char(0x36)); // initialize inner padding with char "6"
    QByteArray outerPadding(blockSize, char(0x5c)); // initialize outer padding with char "\"
    // ascii characters 0x36 ("6") and 0x5c ("\") are selected because they have large
    // Hamming distance (http://en.wikipedia.org/wiki/Hamming_distance)

    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.at(i); // XOR operation between every byte in key and innerpadding, of key length
        outerPadding[i] = outerPadding[i] ^ key.at(i); // XOR operation between every byte in key and outerpadding, of key length
    }

    // result = hash ( outerPadding CONCAT hash ( innerPadding CONCAT baseString ) ).toBase64
    QByteArray total = outerPadding;
    QByteArray part = innerPadding;
    part.append(baseString);
    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
    return hashed.toBase64();
}

QMap<QString,QString> OAuth::_getBaseParams()
{
    QMap<QString,QString> params;
    params["oauth_timestamp"] = QString::number(time(NULL));
    params["oauth_version"] = "1.0";
    params["oauth_consumer_key"] = _mConsumerKey;
    params["oauth_nonce"] = QString::number(qrand());
    if(_mOAuthToken.length()){
        params["oauth_token"] = _mOAuthToken;
    }
    params["oauth_signature_method"] = "HMAC-SHA1";
    return params;
}

uint OAuth::_syncFilesDownload(QString path)
{
    uint lastFileTime = 0;
    QMap<QString,QVariant> fileList;
    QMap<QString,QString> params = _getBaseParams();
    QFileInfo dirInfo(_configPath+"kuaipan"+path);
    QString encodePath = "";
    if(path.length()>0){
        encodePath = "/" + QString(QUrl::toPercentEncoding(path));
    }else{
        QDir rootPath(this->_configPath+"kuaipan");
        if(!rootPath.exists()){
            rootPath.mkdir(this->_configPath+"kuaipan");
        }
    }
    fileList = _sendRequest("GET","http://openapi.kuaipan.cn/1/metadata/kuaipan"+encodePath,params);
    params.clear();
    int total = fileList["files_total"].toInt();
    if(total){
        QVariantList files = fileList["files"].toList();
        QVariantMap obj;
        QFile *fp;
        for(int i = 0;i<total;i++){
            obj = files[i].toMap();
            if(lastFileTime<QDateTime::fromString(obj["modify_time"].toString(),"yyyy-MM-dd hh:mm:ss").toTime_t()){
                lastFileTime = QDateTime::fromString(obj["modify_time"].toString(),"yyyy-MM-dd hh:mm:ss").toTime_t();
            }
            if(!obj["is_deleted"].toBool()){
                if(obj["type"].toString()=="folder"){
                    QDir dir(this->_configPath+"kuaipan/"+path+ "/"+obj["name"].toString());
                    if(dir.exists()){
                       this->_syncFilesDownload(path+ "/"+obj["name"].toString());
                    }else{
                        if(_lastSyncRemoteTime<QDateTime::fromString(obj["modify_time"].toString(),"yyyy-MM-dd hh:mm:ss").toTime_t()){
                            dir.mkdir(this->_configPath+"kuaipan/"+path + "/" +obj["name"].toString());
                            uint folderLastTime = this->_syncFilesDownload(path+ "/"+obj["name"].toString());
                            if(lastFileTime<folderLastTime){
                                lastFileTime = folderLastTime;
                            }
                        }else{
                            _deleteFile(path+ "/"+obj["name"].toString());
                        }
                    }
                }else{
                    QMap<QString,QString> downloadParams = _getBaseParams();
                    downloadParams["root"] = "kuaipan";
                    downloadParams["path"] = path + "/" + obj["name"].toString();

                    fp = new QFile(_configPath+"kuaipan/"+downloadParams["path"]);
                    if(fp->exists()){
                        fp->open(QFile::ReadOnly);
                        QString sha1 = QCryptographicHash::hash(fp->readAll(),QCryptographicHash::Sha1).toHex();
                        fp->close();
                        delete fp;
                        if(sha1 != obj["sha1"].toString()){
                            QFileInfo fi(_configPath+"kuaipan/"+downloadParams["path"]);
                            if(fi.lastModified().toTime_t()<QDateTime::fromString(obj["modify_time"].toString(),"yyyy-MM-dd hh:mm:ss").toTime_t()){
                                QFile::remove(_configPath+"kuaipan/"+downloadParams["path"]);
                                _downloadFile("http://api-content.dfs.kuaipan.cn/1/fileops/download_file",downloadParams,_configPath+"kuaipan/"+downloadParams["path"]);
                            }else{
                                _uploadFile(downloadParams["path"]);
                            }
                        }
                    }else{
                        if(_lastSyncRemoteTime<QDateTime::fromString(obj["modify_time"].toString(),"yyyy-MM-dd hh:mm:ss").toTime_t()){
                            _downloadFile("http://api-content.dfs.kuaipan.cn/1/fileops/download_file",downloadParams,_configPath+"kuaipan/"+downloadParams["path"]);
                        }else{
                            _deleteFile(downloadParams["path"]);
                        }
                    }
                    downloadParams.clear();
                }
            }
            obj.clear();
        }
        files.clear();
    }
    fileList.clear();
    return lastFileTime;
}

bool OAuth::_syncFilesUpload(QString path)
{
    QDir dir(_configPath+"kuaipan/"+path);
    QStringList filelist = dir.entryList();
    QString fileName;
    int localTotal = filelist.length();
    QFileInfo dirInfo(_configPath+"kuaipan/"+path);

    QMap<QString,QString> params = _getBaseParams();
    QString encodePath = "";
    if(path.length()>0){
        encodePath = "/" + QString(QUrl::toPercentEncoding(path));
        path = "/" + path;
    }
    QMap<QString,QVariant> remoteFileList = _sendRequest("GET","http://openapi.kuaipan.cn/1/metadata/kuaipan"+encodePath,params);
    params.clear();
    int remoteTotal = remoteFileList["files_total"].toInt();
    QVariantList remoteFiles = remoteFileList["files"].toList();


    for(int j=2;j<localTotal;j++){
        fileName = filelist[j];
        QFileInfo fi(_configPath + "kuaipan/" + path + "/" + fileName);
        if(fi.isDir()){
            int i;
            for(i = 0;i<remoteTotal;i++){
                QVariantMap obj = remoteFiles[i].toMap();
                if(obj["type"].toString()=="folder" && obj["name"].toString() == fileName){
                    break;
                }
            }
            if(remoteTotal==0 || i == remoteTotal){
                if(fi.lastModified().toTime_t()>_lastSyncLocalTime){
                    _makeDir(path+"/"+fileName);
                    _syncFilesUpload(path+"/"+fileName);
                }else{
                    QDir dir(_configPath+"kuaipan/"+path +"/"+fileName);
                    dir.removeRecursively();
                }
            }else{
                _syncFilesUpload(path+"/"+fileName);
            }
        }else{
            int i;
            for(i = 0;i<remoteTotal;i++){
                QVariantMap obj = remoteFiles[i].toMap();
                if(obj["type"].toString()=="file" && obj["name"].toString() == fileName){
                    break;
                }
            }
            if(remoteTotal==0 || i == remoteTotal){
                if(fi.lastModified().toTime_t()>_lastSyncLocalTime){
                    _uploadFile(path+"/"+fileName);
                }else{
                    QFile::remove(_configPath+"kuaipan/"+path +"/"+fileName);
                }
            }
        }
    }
    filelist.clear();
    return true;
}

bool OAuth::SyncFiles(QString path)
{
    uint lastFileTime = _syncFilesDownload(path);
    _syncFilesUpload(path);
    _setRemoteSyncTime(lastFileTime);
    _setLocalSyncTime(time(NULL));
    return true;
}
