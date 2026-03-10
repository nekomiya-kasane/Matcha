#include <Matcha/Widgets/Menu/NyanFileDialog.h>

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHash>
#include <QStandardPaths>

namespace matcha::gui {

namespace {

// Thread-local storage for last directories per dialog type
QHash<QString, QString>& LastDirectories() {
    static QHash<QString, QString> dirs;
    return dirs;
}

constexpr const char* kOpenKey = "open";
constexpr const char* kSaveKey = "save";
constexpr const char* kDirKey  = "directory";

} // namespace

auto NyanFileDialog::GetLastDirectory(const QString& key) -> QString
{
    auto& dirs = LastDirectories();
    if (dirs.contains(key) && QDir(dirs[key]).exists()) {
        return dirs[key];
    }
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

void NyanFileDialog::SetLastDirectory(const QString& key, const QString& dir)
{
    QFileInfo info(dir);
    QString dirPath = info.isDir() ? dir : info.absolutePath();
    LastDirectories()[key] = dirPath;
}

auto NyanFileDialog::GetOpenFileName(
    QWidget* parent,
    const QString& caption,
    const QString& dir,
    const QString& filter,
    QString* selectedFilter
) -> std::optional<QString>
{
    QString initialDir = dir.isEmpty() ? GetLastDirectory(kOpenKey) : dir;

    QString result = QFileDialog::getOpenFileName(
        parent,
        caption.isEmpty() ? QObject::tr("Open File") : caption,
        initialDir,
        filter,
        selectedFilter
    );

    if (result.isEmpty()) {
        return std::nullopt;
    }

    SetLastDirectory(kOpenKey, result);
    return result;
}

auto NyanFileDialog::GetSaveFileName(
    QWidget* parent,
    const QString& caption,
    const QString& dir,
    const QString& filter,
    QString* selectedFilter
) -> std::optional<QString>
{
    QString initialDir = dir.isEmpty() ? GetLastDirectory(kSaveKey) : dir;

    QString result = QFileDialog::getSaveFileName(
        parent,
        caption.isEmpty() ? QObject::tr("Save File") : caption,
        initialDir,
        filter,
        selectedFilter
    );

    if (result.isEmpty()) {
        return std::nullopt;
    }

    SetLastDirectory(kSaveKey, result);
    return result;
}

auto NyanFileDialog::GetExistingDirectory(
    QWidget* parent,
    const QString& caption,
    const QString& dir
) -> std::optional<QString>
{
    QString initialDir = dir.isEmpty() ? GetLastDirectory(kDirKey) : dir;

    QString result = QFileDialog::getExistingDirectory(
        parent,
        caption.isEmpty() ? QObject::tr("Select Directory") : caption,
        initialDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (result.isEmpty()) {
        return std::nullopt;
    }

    SetLastDirectory(kDirKey, result);
    return result;
}

auto NyanFileDialog::GetOpenFileNames(
    QWidget* parent,
    const QString& caption,
    const QString& dir,
    const QString& filter,
    QString* selectedFilter
) -> QStringList
{
    QString initialDir = dir.isEmpty() ? GetLastDirectory(kOpenKey) : dir;

    QStringList result = QFileDialog::getOpenFileNames(
        parent,
        caption.isEmpty() ? QObject::tr("Open Files") : caption,
        initialDir,
        filter,
        selectedFilter
    );

    if (!result.isEmpty()) {
        SetLastDirectory(kOpenKey, result.first());
    }

    return result;
}

void NyanFileDialog::ClearLastDirectories()
{
    LastDirectories().clear();
}

} // namespace matcha::gui
