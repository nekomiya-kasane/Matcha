#pragma once

#include <Matcha/Services/IViewportRenderer.h>

#include <vector>

namespace matcha::test {

class MockRenderer : public matcha::fw::IViewportRenderer {
public:
    int attachCount  = 0;
    int detachCount  = 0;
    int renderCount  = 0;
    int resizeCount  = 0;
    int dprCount     = 0;
    int visCount     = 0;
    int handleCount  = 0;
    int inputCount   = 0;
    bool ready       = true;

    matcha::fw::ViewportId lastVpId {};
    int lastWidth  = 0;
    int lastHeight = 0;
    double lastDpr = 0.0;
    bool lastVisible = false;
    std::vector<matcha::fw::InputEvent> receivedEvents;

    auto OnAttach(matcha::fw::ViewportId vpId, void* /*nativeHandle*/,
                  int width, int height, double dpr) -> matcha::fw::Expected<void> override {
        ++attachCount;
        lastVpId = vpId;
        lastWidth = width;
        lastHeight = height;
        lastDpr = dpr;
        return {};
    }

    void OnDetach(matcha::fw::ViewportId vpId) override {
        ++detachCount;
        lastVpId = vpId;
    }

    auto OnRenderFrame(matcha::fw::ViewportId /*vpId*/) -> matcha::fw::Expected<void> override {
        ++renderCount;
        return {};
    }

    [[nodiscard]] auto IsReady() const -> bool override { return ready; }

    void OnResize(matcha::fw::ViewportId /*vpId*/, int w, int h, double dpr) override {
        ++resizeCount;
        lastWidth = w;
        lastHeight = h;
        lastDpr = dpr;
    }

    void OnDprChanged(matcha::fw::ViewportId /*vpId*/, double dpr) override {
        ++dprCount;
        lastDpr = dpr;
    }

    void OnVisibilityChanged(matcha::fw::ViewportId /*vpId*/, bool visible) override {
        ++visCount;
        lastVisible = visible;
    }

    void OnNativeHandleChanged(matcha::fw::ViewportId /*vpId*/, void* /*h*/,
                               int w, int h2, double dpr) override {
        ++handleCount;
        lastWidth = w;
        lastHeight = h2;
        lastDpr = dpr;
    }

    void OnInputEvent(matcha::fw::ViewportId /*vpId*/, const matcha::fw::InputEvent& event) override {
        ++inputCount;
        receivedEvents.push_back(event);
    }
};

} // namespace matcha::test
