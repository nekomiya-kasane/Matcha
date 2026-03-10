/**
 * @file ViewportFrame.cpp
 * @brief Implementation of the ViewportFrame composite widget.
 */

#include <Matcha/Widgets/Shell/ViewportFrame.h>

#include <Matcha/Widgets/Core/DropZoneOverlay.h>
#include <Matcha/Widgets/Shell/ViewportHeaderBar.h>
#include <Matcha/Widgets/Shell/ViewportWidget.h>

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QVBoxLayout>

namespace matcha::gui {

ViewportFrame::ViewportFrame(fw::ViewportId vpId, fw::ViewportWidget* vpWidget,
                             QWidget* parent)
    : QWidget(parent)
    , _vpId(vpId)
    , _vpWidget(vpWidget)
{
    // Header bar
    _header = new ViewportHeaderBar(vpId, this);

    // Layout: header on top, viewport widget fills the rest
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(_header);

    if (_vpWidget != nullptr) {
        _vpWidget->setParent(this);
        layout->addWidget(_vpWidget, 1); // stretch factor 1
    }

    // Drop zone overlay (stacked on top of everything, hidden by default)
    _overlay = new DropZoneOverlay(this);
    _overlay->hide();

    // Accept drops for viewport DnD
    setAcceptDrops(true);

    // Forward header signals
    connect(_header, &ViewportHeaderBar::dragStarted,
            this, &ViewportFrame::dragStarted);
    connect(_header, &ViewportHeaderBar::dragEnded,
            this, &ViewportFrame::dragEnded);
    connect(_header, &ViewportHeaderBar::closeRequested,
            this, &ViewportFrame::closeRequested);
    connect(_header, &ViewportHeaderBar::maximizeToggled,
            this, &ViewportFrame::maximizeToggled);
    connect(_header, &ViewportHeaderBar::splitHRequested,
            this, &ViewportFrame::splitHRequested);
    connect(_header, &ViewportHeaderBar::splitVRequested,
            this, &ViewportFrame::splitVRequested);
}

ViewportFrame::~ViewportFrame()
{
    // Detach the inner ViewportWidget -- its lifetime is managed by the
    // Viewport UiNode, not by this frame widget.
    if (_vpWidget != nullptr) {
        _vpWidget->setParent(nullptr);
    }
}

void ViewportFrame::SetLabel(const QString& text)
{
    if (_header != nullptr) {
        _header->SetLabel(text);
    }
}

void ViewportFrame::ShowOverlay()
{
    if (_overlay != nullptr) {
        _overlay->setGeometry(rect());
        _overlay->raise();
        _overlay->show();
    }
}

void ViewportFrame::HideOverlay()
{
    if (_overlay != nullptr) {
        _overlay->ClearZone();
        _overlay->hide();
    }
}

void ViewportFrame::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // Keep overlay sized to match frame
    if (_overlay != nullptr && _overlay->isVisible()) {
        _overlay->setGeometry(rect());
    }
}

static const QString kMimeType = QStringLiteral("application/x-matcha-viewport");

void ViewportFrame::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat(kMimeType)) {
        // Don't accept drops on yourself
        auto sourceId = event->mimeData()->data(kMimeType).toULongLong();
        if (fw::ViewportId::From(sourceId) == _vpId) {
            event->ignore();
            return;
        }
        ShowOverlay();
        event->acceptProposedAction();
    }
}

void ViewportFrame::dragMoveEvent(QDragMoveEvent* event)
{
    if (_overlay == nullptr || !event->mimeData()->hasFormat(kMimeType)) {
        return;
    }
    auto zone = _overlay->ZoneAtPoint(event->position().toPoint());
    _overlay->SetActiveZone(zone);
    event->acceptProposedAction();
}

void ViewportFrame::dragLeaveEvent(QDragLeaveEvent* /*event*/)
{
    HideOverlay();
}

void ViewportFrame::dropEvent(QDropEvent* event)
{
    HideOverlay();

    if (!event->mimeData()->hasFormat(kMimeType)) {
        return;
    }

    auto sourceIdVal = event->mimeData()->data(kMimeType).toULongLong();
    auto sourceId = fw::ViewportId::From(sourceIdVal);
    if (sourceId == _vpId) {
        return;
    }

    auto zone = _overlay->ZoneAtPoint(event->position().toPoint());
    emit viewportDropped(sourceId, _vpId, static_cast<int>(zone));
    event->acceptProposedAction();
}

} // namespace matcha::gui
