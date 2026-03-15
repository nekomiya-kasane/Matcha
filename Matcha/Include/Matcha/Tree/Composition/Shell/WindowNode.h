#pragma once

/**
 * @file WindowNode.h
 * @brief Top-level window abstraction -- owns a QMainWindow via Pimpl.
 *
 * WindowNode represents a top-level OS window in the UiNode tree.
 * Each WindowNode owns a QMainWindow internally and provides the
 * window chrome (title bar, action bar, status bar, central area).
 *
 * WindowKind determines behavior:
 *  - Main: The primary application window. One per application.
 *  - Floating: A detached tab window (child of Main, always-on-top).
 *  - Detached: Reserved for future use.
 *
 * @note WindowNode is a child of Shell in the UiNode tree.
 * @note Qt types are hidden behind Pimpl -- zero Qt in public header.
 * @see docs/02_Architecture_Design.md section 2.5.4
 */

#include "Matcha/Core/StrongId.h"
#include "Matcha/Core/Types.h"
#include "Matcha/Tree/UiNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include <string>
#include <string_view>

class QWidget;

namespace matcha::gui {
class UpdateGuard;
} // namespace matcha::gui

namespace matcha::fw {

class DocumentToolBarNode;
class LogoButtonNode;
class StatusBarNode;
class TitleBarNode;
class WorkspaceFrame;

/**
 * @brief Window kind -- determines window behavior and Qt flags.
 */
enum class WindowKind : uint8_t {
    Main,      ///< Primary application window (one per app).
    Floating,  ///< Detached tab window (child of Main, always-on-top).
    Detached,  ///< Reserved for future use.
};

/**
 * @brief Top-level window UiNode -- owns a QMainWindow via Pimpl.
 *
 * Created by Application::CreateWindow(). Do not construct directly.
 */
class MATCHA_EXPORT WindowNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification {
        using CloseRequested = matcha::fw::CloseRequested;
    };

    /**
     * @brief Construct a WindowNode.
     * @param id Window identifier (unique per application).
     * @param windowId Typed WindowId for this window.
     * @param kind Window kind (Main/Floating/Detached).
     */
    WindowNode(std::string id, WindowId windowId, WindowKind kind);
    ~WindowNode() override;

    WindowNode(const WindowNode&) = delete;
    auto operator=(const WindowNode&) -> WindowNode& = delete;
    WindowNode(WindowNode&&) = delete;
    auto operator=(WindowNode&&) -> WindowNode& = delete;

    // -- Identity --

    /// @brief Typed WindowId for this window.
    [[nodiscard]] auto Id() const -> WindowId { return _windowId; }

    /// @brief Window kind (Main/Floating/Detached).
    [[nodiscard]] auto Kind() const -> WindowKind { return _kind; }

    // -- Window operations --

    /// @brief Set the window title.
    void SetTitle(std::string_view title);

    /// @brief Get the current window title.
    [[nodiscard]] auto Title() const -> std::string;

    /// @brief Show the window.
    void Show();

    /// @brief Hide the window.
    void Hide();

    /// @brief Check if the window is visible.
    [[nodiscard]] auto IsVisible() const -> bool;

    /// @brief Close the window (dispatches CloseRequested notification).
    void Close();

    /// @brief Minimize the window.
    void Minimize();

    /// @brief Maximize the window (or restore if already maximized).
    void Maximize();

    // -- Size constraints --

    /// @brief Set the minimum window size.
    void SetMinimumSize(int w, int h);

    /// @brief Resize the window.
    void Resize(int w, int h);

    /// @brief Move the window to a screen position.
    void Move(int x, int y);

    /// @brief Framework hard-floor minimum size: 952x536 (16:9).
    static constexpr int kMinWidth  = 952;
    static constexpr int kMinHeight = 536;

    // -- UiNode child access --

    /// @brief Get the TitleBarNode child (Main or Floating title bar).
    [[nodiscard]] auto GetTitleBar() -> observer_ptr<TitleBarNode>;

    /// @brief Get the DocumentToolBarNode child (module combo + doc tabs + global buttons).
    [[nodiscard]] auto GetDocumentToolBar() -> observer_ptr<DocumentToolBarNode>;

    /// @brief Get the LogoButtonNode child (clickable logo spanning two rows).
    [[nodiscard]] auto GetLogoButton() -> observer_ptr<LogoButtonNode>;

    /// @brief Get the WorkspaceFrame child (owns ActionBar, DocumentArea, ControlBar).
    [[nodiscard]] auto GetWorkspaceFrame() -> observer_ptr<WorkspaceFrame>;

    /// @brief Get the StatusBarNode child.
    [[nodiscard]] auto GetStatusBarNode() -> observer_ptr<StatusBarNode>;

    // -- Widget access (minimal, needed for layout wiring) --

    /// @brief Get the central content area widget (WorkspaceFrame host).
    [[nodiscard]] auto ContentArea() -> QWidget*;

    /// @brief Get the underlying QWidget (QMainWindow).
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Freeze updates --

    /// @brief Freeze window painting. Returns RAII guard.
    [[nodiscard]] auto FreezeUpdates() -> gui::UpdateGuard;

    // -- Internal --

    /// @brief Build the QMainWindow and child widgets.
    /// @internal Called by Application. Not part of public API.
    virtual void BuildWindow(QWidget* parent);

    /// @brief Check if the window has been built.
    [[nodiscard]] auto IsBuilt() const -> bool;

    /// @brief Mark the window as requesting close (called by close event filter).
    void MarkCloseRequested();

    /// @brief Check if close was requested.
    [[nodiscard]] auto IsCloseRequested() const -> bool;

protected:
    WindowId _windowId;
    WindowKind _kind;
    QWidget* _mainWindow = nullptr;
    QWidget* _centralArea = nullptr;
    std::string _title;
    bool _closeRequested = false;
};

} // namespace matcha::fw
