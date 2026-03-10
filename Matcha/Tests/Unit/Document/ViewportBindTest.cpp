#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "Matcha/Services/IViewportRenderer.h"
#include "Matcha/UiNodes/Document/Viewport.h"

using matcha::fw::Expected;
using matcha::fw::InputEvent;
using matcha::fw::IViewportRenderer;
using matcha::fw::Viewport;
using matcha::fw::ViewportId;

// ============================================================================
// MockRenderer -- minimal IViewportRenderer for testing
// ============================================================================

namespace {

class MockRenderer : public IViewportRenderer {
public:
    int attachCount  = 0;
    int detachCount  = 0;
    int renderCount  = 0;
    int resizeCount  = 0;
    int dprCount     = 0;
    int visCount     = 0;
    int handleCount  = 0;
    int inputCount   = 0;
    ViewportId lastVpId {};
    bool ready = true;

    auto OnAttach(ViewportId vpId, void* /*nativeHandle*/,
                  int /*width*/, int /*height*/, double /*dpr*/) -> Expected<void> override {
        ++attachCount;
        lastVpId = vpId;
        return {};
    }
    void OnDetach(ViewportId vpId) override {
        ++detachCount;
        lastVpId = vpId;
    }
    auto OnRenderFrame(ViewportId /*vpId*/) -> Expected<void> override {
        ++renderCount;
        return {};
    }
    [[nodiscard]] auto IsReady() const -> bool override { return ready; }
    void OnResize(ViewportId /*vpId*/, int /*w*/, int /*h*/, double /*dpr*/) override { ++resizeCount; }
    void OnDprChanged(ViewportId /*vpId*/, double /*dpr*/) override { ++dprCount; }
    void OnVisibilityChanged(ViewportId /*vpId*/, bool /*visible*/) override { ++visCount; }
    void OnNativeHandleChanged(ViewportId /*vpId*/, void* /*h*/,
                               int /*w*/, int /*h2*/, double /*dpr*/) override { ++handleCount; }
    void OnInputEvent(ViewportId /*vpId*/, const InputEvent& /*event*/) override { ++inputCount; }
};

} // anonymous namespace

TEST_CASE("Viewport: bind sets renderer and calls OnAttach") {
    Viewport vp("test-vp", ViewportId::From(1));
    MockRenderer renderer;

    vp.BindRenderer(&renderer);
    CHECK(renderer.attachCount == 1);
    CHECK(renderer.lastVpId == ViewportId::From(1));
}

TEST_CASE("Viewport: unbind calls OnDetach") {
    Viewport vp("test-vp", ViewportId::From(2));
    MockRenderer renderer;

    vp.BindRenderer(&renderer);
    vp.UnbindRenderer();
    CHECK(renderer.detachCount == 1);
}

TEST_CASE("Viewport: double-bind detaches first renderer") {
    Viewport vp("test-vp", ViewportId::From(3));
    MockRenderer r1;
    MockRenderer r2;

    vp.BindRenderer(&r1);
    CHECK(r1.attachCount == 1);

    vp.BindRenderer(&r2);
    CHECK(r1.detachCount == 1);
    CHECK(r2.attachCount == 1);
}

TEST_CASE("Viewport: RequestFrame sets dirty flag") {
    Viewport vp("test-vp", ViewportId::From(4));

    CHECK_FALSE(vp.IsDirty());
    vp.RequestFrame();
    CHECK(vp.IsDirty());
}

TEST_CASE("Viewport: ClearDirty resets dirty flag") {
    Viewport vp("test-vp", ViewportId::From(5));

    vp.RequestFrame();
    CHECK(vp.IsDirty());

    vp.ClearDirty();
    CHECK_FALSE(vp.IsDirty());
}

TEST_CASE("Viewport: destruction detaches renderer") {
    MockRenderer renderer;
    {
        Viewport vp("test-vp", ViewportId::From(6));
        vp.BindRenderer(&renderer);
    }
    CHECK(renderer.detachCount == 1);
}
