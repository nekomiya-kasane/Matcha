#pragma once

/**
 * @file NyanFileDialog.h
 * @brief Themed QFileDialog wrapper with static methods.
 *
 * NyanFileDialog provides:
 * - Static methods for file open/save operations
 * - Themed styling matching application theme
 * - Last directory memory per dialog type
 *
 * @par Usage
 * @code
 * auto file = NyanFileDialog::GetOpenFileName(parent, "Open File", "", "Text Files (*.txt)");
 * if (file) {
 *     // Use *file
 * }
 * @endcode
 *
 * @see NyanDialog for custom dialogs.
 */

#include <Matcha/Core/Macros.h>

#include <QString>

#include <optional>

class QWidget;

namespace matcha::gui {

/**
 * @brief Themed QFileDialog wrapper with static methods.
 *
 * All methods are static and remember the last directory per dialog type.
 */
class MATCHA_EXPORT NyanFileDialog {
public:
    NyanFileDialog() = delete;

    /**
     * @brief Show an open file dialog.
     * @param parent Parent widget.
     * @param caption Dialog title.
     * @param dir Initial directory (empty = last used or home).
     * @param filter File filter (e.g., "Text Files (*.txt);;All Files (*)").
     * @param selectedFilter Output: selected filter (optional).
     * @return Selected file path, or empty optional if cancelled.
     */
    [[nodiscard]] static auto GetOpenFileName(
        QWidget* parent = nullptr,
        const QString& caption = QString(),
        const QString& dir = QString(),
        const QString& filter = QString(),
        QString* selectedFilter = nullptr
    ) -> std::optional<QString>;

    /**
     * @brief Show a save file dialog.
     * @param parent Parent widget.
     * @param caption Dialog title.
     * @param dir Initial directory (empty = last used or home).
     * @param filter File filter (e.g., "Text Files (*.txt);;All Files (*)").
     * @param selectedFilter Output: selected filter (optional).
     * @return Selected file path, or empty optional if cancelled.
     */
    [[nodiscard]] static auto GetSaveFileName(
        QWidget* parent = nullptr,
        const QString& caption = QString(),
        const QString& dir = QString(),
        const QString& filter = QString(),
        QString* selectedFilter = nullptr
    ) -> std::optional<QString>;

    /**
     * @brief Show a directory selection dialog.
     * @param parent Parent widget.
     * @param caption Dialog title.
     * @param dir Initial directory (empty = last used or home).
     * @return Selected directory path, or empty optional if cancelled.
     */
    [[nodiscard]] static auto GetExistingDirectory(
        QWidget* parent = nullptr,
        const QString& caption = QString(),
        const QString& dir = QString()
    ) -> std::optional<QString>;

    /**
     * @brief Show an open files dialog (multiple selection).
     * @param parent Parent widget.
     * @param caption Dialog title.
     * @param dir Initial directory (empty = last used or home).
     * @param filter File filter.
     * @param selectedFilter Output: selected filter (optional).
     * @return List of selected file paths, or empty list if cancelled.
     */
    [[nodiscard]] static auto GetOpenFileNames(
        QWidget* parent = nullptr,
        const QString& caption = QString(),
        const QString& dir = QString(),
        const QString& filter = QString(),
        QString* selectedFilter = nullptr
    ) -> QStringList;

    /**
     * @brief Clear the last directory memory.
     */
    static void ClearLastDirectories();

private:
    [[nodiscard]] static auto GetLastDirectory(const QString& key) -> QString;
    static void SetLastDirectory(const QString& key, const QString& dir);
};

} // namespace matcha::gui
