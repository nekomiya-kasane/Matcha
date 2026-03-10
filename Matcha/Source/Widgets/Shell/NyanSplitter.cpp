/**
 * @file NyanSplitter.cpp
 * @brief Implementation of NyanSplitter themed splitter with handle styling.
 */

#include <Matcha/Widgets/Shell/NyanSplitter.h>

#include <QPainter>
#include <QSplitterHandle>

namespace matcha::gui {

// ============================================================================
// Themed splitter handle
// ============================================================================

namespace {

class ThemedSplitterHandle : public QSplitterHandle {
public:
    ThemedSplitterHandle(Qt::Orientation orientation, QSplitter* parent)
        : QSplitterHandle(orientation, parent)
    {
        setMouseTracking(true);
    }

protected:
    void paintEvent(QPaintEvent* /*event*/) override
    {
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing);

        const auto istate = underMouse() ? InteractionState::Hovered : InteractionState::Normal;
        const auto style = GetThemeService().Resolve(WidgetKind::Splitter, 0, istate);
        p.fillRect(rect(), style.border);
    }
};

} // anonymous namespace

// ============================================================================
// Construction
// ============================================================================

NyanSplitter::NyanSplitter(QWidget* parent)
    : QSplitter(Qt::Horizontal, parent)
    , ThemeAware(WidgetKind::Splitter)
{
    ApplyHandleStyle();
}

NyanSplitter::NyanSplitter(Qt::Orientation orientation, QWidget* parent)
    : QSplitter(orientation, parent)
    , ThemeAware(WidgetKind::Splitter)
{
    ApplyHandleStyle();
}

NyanSplitter::~NyanSplitter() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanSplitter::SetCollapseButtonVisible(bool visible)
{
    _collapseButtonVisible = visible;
    update();
}

auto NyanSplitter::CollapseButtonVisible() const -> bool
{
    return _collapseButtonVisible;
}

void NyanSplitter::CollapsePanel(int index)
{
    _savedSizes = sizes();
    QList<int> newSizes = sizes();
    if (index >= 0 && index < newSizes.size()) {
        newSizes[index] = 0;
        setSizes(newSizes);
    }
}

void NyanSplitter::RestorePanel(int index)
{
    if (!_savedSizes.isEmpty() && index >= 0 && index < _savedSizes.size()) {
        setSizes(_savedSizes);
        _savedSizes.clear();
    }
}

// ============================================================================
// Theme
// ============================================================================

void NyanSplitter::OnThemeChanged()
{
    ApplyHandleStyle();
    update();
}

auto NyanSplitter::createHandle() -> QSplitterHandle*
{
    return new ThemedSplitterHandle(orientation(), this);
}

void NyanSplitter::ApplyHandleStyle()
{
    setHandleWidth(kHandleWidth);
}

} // namespace matcha::gui
