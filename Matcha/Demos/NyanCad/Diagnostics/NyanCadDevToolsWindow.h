#pragma once

/**
 * @file NyanCadDevToolsWindow.h
 * @brief Unified DevTools window: Notification Log + UI Inspector in tabs.
 *
 * Merges NyanCadDebugWindow and NyanCadDiagnosticsWindow into one
 * FloatingWindowNode with a QTabWidget.
 */

#include "Matcha/UiNodes/Shell/FloatingWindowNode.h"

#include <QHash>

class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QTableWidget;
class QPushButton;

namespace matcha::fw {
class Application;
class PlainTextEditNode;
class UiNode;
} // namespace matcha::fw

namespace nyancad {

class LayoutBoundsOverlay;
class WidgetPickerFilter;

/**
 * @brief Unified DevTools floating window.
 *
 * Tab 0: Notification Log (captures all notifications propagating past Shell)
 * Tab 1: UI Inspector (UiNode tree + property panel + layout bounds + widget picker)
 */
class NyanCadDevToolsWindow : public matcha::fw::FloatingWindowNode {

public:
    NyanCadDevToolsWindow();
    ~NyanCadDevToolsWindow() override;

    NyanCadDevToolsWindow(const NyanCadDevToolsWindow&) = delete;
    NyanCadDevToolsWindow& operator=(const NyanCadDevToolsWindow&) = delete;
    NyanCadDevToolsWindow(NyanCadDevToolsWindow&&) = delete;
    NyanCadDevToolsWindow& operator=(NyanCadDevToolsWindow&&) = delete;

    /// @brief Bind to application: sets this as Shell's parent for notifications,
    ///        and provides UiNode tree access for the inspector tab.
    void Bind(matcha::fw::Application& app);

    /// @brief Append a notification log entry.
    void LogNotification(std::string_view senderId, std::string_view className);

protected:
    void BuildContent(QWidget* contentParent, QVBoxLayout* layout) override;

    [[nodiscard]] auto AnalyseNotification(matcha::CommandNode* sender,
                                           matcha::Notification& notif) -> matcha::PropagationMode override;

private:
    // -- Tab builders --
    auto BuildNotificationTab(QWidget* tabParent) -> QWidget*;
    auto BuildInspectorTab(QWidget* tabParent) -> QWidget*;

    // -- Inspector helpers --
    void RefreshTree();
    void PopulateNode(QTreeWidgetItem* parentItem, matcha::fw::UiNode* node);
    void OnTreeItemSelected();
    void ShowPropertiesForNode(matcha::fw::UiNode* node);
    void ShowPropertiesForWidget(QWidget* widget);
    void OnWidgetPicked(QWidget* widget);
    void ToggleBounds();
    void TogglePicker();

    matcha::fw::Application* _app = nullptr;

    // -- Notification tab --
    matcha::fw::PlainTextEditNode* _logNode = nullptr;
    int _notifCount = 0;

    // -- Inspector tab --
    QTreeWidget*  _treeWidget = nullptr;
    QTableWidget* _propTable  = nullptr;
    QPushButton*  _boundsBtn  = nullptr;
    QPushButton*  _pickerBtn  = nullptr;

    LayoutBoundsOverlay* _overlay = nullptr;
    WidgetPickerFilter*  _picker  = nullptr;

    QHash<QTreeWidgetItem*, matcha::fw::UiNode*> _itemToNode;
};

} // namespace nyancad
