/**
 * @file InteractionTimingRegistry.cpp
 * @brief Implementation of InteractionTimingRegistry (§8.7).
 */

#include <Matcha/Animation/InteractionTimingRegistry.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

namespace matcha::fw {

using Ms = std::chrono::milliseconds;

// ============================================================================
// Spec §8.7.1 default values
// ============================================================================

static constexpr std::array<Ms, kTimingTokenCount> kDefaults = {{
    Ms{200},    // HoverDelay
    Ms{500},    // TooltipDelay
    Ms{100},    // TooltipDismissDelay
    Ms{500},    // LongPressThreshold
    Ms{400},    // DoubleClickWindow
    Ms{300},    // DebounceSearch
    Ms{100},    // DebounceResize
    Ms{30000},  // AutoSaveInterval
    Ms{60000},  // IdleTimeout
    Ms{500},    // RepeatKeyInitial
    Ms{33},     // RepeatKeyInterval
    Ms{150},    // DragInitDelay
    Ms{5000},   // ToastDismissTimeout
    Ms{200},    // MenuOpenDelay
    Ms{300},    // MenuCloseDelay
}};

// ============================================================================
// Construction
// ============================================================================

InteractionTimingRegistry::InteractionTimingRegistry()
    : _values(kDefaults)
{
}

// ============================================================================
// Access
// ============================================================================

auto InteractionTimingRegistry::Get(TimingToken token) const -> Milliseconds
{
    const auto idx = static_cast<std::size_t>(token);
    if (idx >= kTimingTokenCount) {
        return Ms{0};
    }
    return _values[idx];
}

auto InteractionTimingRegistry::GetMs(TimingToken token) const -> int
{
    return static_cast<int>(Get(token).count());
}

auto InteractionTimingRegistry::DefaultValue(TimingToken token) -> Milliseconds
{
    const auto idx = static_cast<std::size_t>(token);
    if (idx >= kTimingTokenCount) {
        return Ms{0};
    }
    return kDefaults[idx];
}

// ============================================================================
// Mutation
// ============================================================================

void InteractionTimingRegistry::Set(TimingToken token, Milliseconds value)
{
    const auto idx = static_cast<std::size_t>(token);
    if (idx < kTimingTokenCount) {
        _values[idx] = value;
    }
}

void InteractionTimingRegistry::ResetToDefault(TimingToken token)
{
    const auto idx = static_cast<std::size_t>(token);
    if (idx < kTimingTokenCount) {
        _values[idx] = kDefaults[idx];
    }
}

void InteractionTimingRegistry::ResetAll()
{
    _values = kDefaults;
}

// ============================================================================
// Platform integration (§8.7.2)
// ============================================================================

auto InteractionTimingRegistry::ApplyPlatformOverrides() -> int
{
    int updated = 0;

#ifdef _WIN32
    // DoubleClickWindow — GetDoubleClickTime()
    {
        const UINT dcTime = ::GetDoubleClickTime();
        if (dcTime > 0) {
            _values[static_cast<std::size_t>(TimingToken::DoubleClickWindow)] = Ms{dcTime};
            ++updated;
        }
    }

    // HoverDelay — SPI_GETMOUSEHOVERTIME
    {
        UINT hoverTime = 0;
        if (::SystemParametersInfoW(SPI_GETMOUSEHOVERTIME, 0, &hoverTime, 0) && hoverTime > 0) {
            _values[static_cast<std::size_t>(TimingToken::HoverDelay)] = Ms{hoverTime};
            ++updated;
        }
    }

    // RepeatKeyInitial — SPI_GETKEYBOARDDELAY
    // Returns 0..3 representing 250ms..1000ms in 250ms increments
    {
        UINT keyDelay = 0;
        if (::SystemParametersInfoW(SPI_GETKEYBOARDDELAY, 0, &keyDelay, 0)) {
            const int delayMs = 250 + static_cast<int>(keyDelay) * 250;
            _values[static_cast<std::size_t>(TimingToken::RepeatKeyInitial)] = Ms{delayMs};
            ++updated;
        }
    }

    // RepeatKeyInterval — SPI_GETKEYBOARDSPEED
    // Returns 0..31 representing ~2.5 to ~30 repeats/s
    // Formula: interval = 1000 / (2.5 + speed * (30.0 - 2.5) / 31.0)
    {
        UINT keySpeed = 0;
        if (::SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0, &keySpeed, 0)) {
            const double rps = 2.5 + static_cast<double>(keySpeed) * 27.5 / 31.0;
            const int intervalMs = static_cast<int>(1000.0 / rps);
            _values[static_cast<std::size_t>(TimingToken::RepeatKeyInterval)] = Ms{intervalMs};
            ++updated;
        }
    }
#endif

    return updated;
}

} // namespace matcha::fw
