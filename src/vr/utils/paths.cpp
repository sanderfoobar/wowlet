#include "paths.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <QString>
#include <QFileInfo>
#include <QDir>

namespace paths
{
string binaryDirectory()
{
    const auto path = QCoreApplication::applicationDirPath();
    if ( path == "" ) {
        qCritical() << "Could not find binary directory.";
        return "";
    }

    return path.toStdString() + "/../";  // @ TODO: removeme
}

string binaryDirectoryFindFile( const string& fileName ) {
    const auto path = binaryDirectory();
    if (path.empty()) {
        return "";
    }

    const auto filePath = QDir( QString::fromStdString( path ) + '/'
                                + QString::fromStdString( fileName ) );
    QFileInfo file( filePath.path() );
    if (!file.exists())
    {
        qCritical() << "Could not find file '" << fileName.c_str()
                     << "' in binary directory.";
        return "";
    }

    return QDir::toNativeSeparators( file.filePath() ).toStdString();
}

string settingsDirectory() {
    const auto path = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
    if (path == "") {
        qCritical() << "Could not find settings directory.";
        return "";
    }
    return path.toStdString();
}

string verifyIconFilePath( const string& filename ) {
    const string notifIconPath = paths::binaryDirectoryFindFile( filename );
    if (notifIconPath.empty()) {
        qCritical() << "Could not find icon " << filename.c_str() << "\"";
    }

    return notifIconPath;
}

} // namespace paths
