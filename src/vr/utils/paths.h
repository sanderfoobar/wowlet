#pragma once
#include <QDebug>
#include <string>
#include <optional>
#include <experimental/optional>

namespace paths
{
using std::string;

string binaryDirectory();
string binaryDirectoryFindFile( const string& fileName );
string settingsDirectory();
string verifyIconFilePath( const string& filename );

} // namespace paths
