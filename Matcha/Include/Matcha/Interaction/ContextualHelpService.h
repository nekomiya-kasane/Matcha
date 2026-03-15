#pragma once

/**
 * @file ContextualHelpService.h
 * @brief Contextual help service: coach marks, walkthrough steps, guided tours.
 *
 * Provides a structured way to define and execute multi-step guided tours:
 * - WalkthroughStep: target element ID, title, body, anchor position
 * - Walkthrough: ordered sequence of steps with completion tracking
 * - ContextualHelpService: registers walkthroughs, manages active tour state
 *
 * This is a Foundation-layer component with zero Qt dependency.
 * The actual overlay rendering is handled by the Widget layer.
 *
 * @see Matcha_Design_System_Specification.md (Contextual Help patterns)
 */

#include <Matcha/Core/Macros.h>

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw {

// ============================================================================
// CoachMarkAnchor
// ============================================================================

/**
 * @enum CoachMarkAnchor
 * @brief Where the coach mark tooltip anchors relative to the target element.
 */
enum class CoachMarkAnchor : uint8_t {
    Top    = 0,
    Bottom = 1,
    Left   = 2,
    Right  = 3,
    Auto   = 4,   ///< System chooses best position based on available space
};

// ============================================================================
// WalkthroughStep
// ============================================================================

/**
 * @struct WalkthroughStep
 * @brief A single step in a guided walkthrough.
 */
struct WalkthroughStep {
    std::string       targetId;    ///< ID of the UI element to highlight
    std::string       title;
    std::string       body;        ///< Rich-text body content
    CoachMarkAnchor   anchor = CoachMarkAnchor::Auto;
    bool              optional = false; ///< If true, step can be skipped if target not found
};

// ============================================================================
// Walkthrough
// ============================================================================

/**
 * @struct Walkthrough
 * @brief A named sequence of walkthrough steps.
 */
struct Walkthrough {
    std::string                  id;
    std::string                  name;         ///< Human-readable name
    std::string                  description;
    std::vector<WalkthroughStep> steps;
};

// ============================================================================
// WalkthroughState
// ============================================================================

/**
 * @enum WalkthroughState
 * @brief Current state of a walkthrough execution.
 */
enum class WalkthroughState : uint8_t {
    Idle       = 0,
    Active     = 1,
    Paused     = 2,
    Completed  = 3,
    Dismissed  = 4,
};

// ============================================================================
// ContextualHelpService
// ============================================================================

/**
 * @class ContextualHelpService
 * @brief Manages walkthrough registration and tour execution.
 *
 * Usage:
 * @code
 *   ContextualHelpService help;
 *   help.RegisterWalkthrough({
 *       .id = "first_run",
 *       .name = "Getting Started",
 *       .steps = {
 *           {.targetId = "toolbar", .title = "Toolbar", .body = "Main actions here."},
 *           {.targetId = "tree", .title = "Model Tree", .body = "Browse your model."},
 *           {.targetId = "viewport", .title = "3D View", .body = "Interact with geometry."},
 *       },
 *   });
 *
 *   help.Start("first_run");
 *   // UI layer reads CurrentStep() and renders coach mark overlay
 *   help.Next();  // advance to step 2
 *   help.Next();  // advance to step 3
 *   help.Next();  // completes the walkthrough
 * @endcode
 */
class MATCHA_EXPORT ContextualHelpService {
public:
    using StepChangedCallback = std::function<void(int /*stepIndex*/, const WalkthroughStep&)>;
    using StateChangedCallback = std::function<void(WalkthroughState)>;

    ContextualHelpService() = default;

    // ====================================================================
    // Registration
    // ====================================================================

    /**
     * @brief Register a walkthrough. Returns false if ID already exists.
     */
    auto RegisterWalkthrough(Walkthrough wt) -> bool;

    /**
     * @brief Unregister a walkthrough by ID.
     */
    auto UnregisterWalkthrough(std::string_view id) -> bool;

    /**
     * @brief Find a registered walkthrough by ID.
     */
    [[nodiscard]] auto FindWalkthrough(std::string_view id) const -> const Walkthrough*;

    [[nodiscard]] auto WalkthroughCount() const -> int {
        return static_cast<int>(_walkthroughs.size());
    }

    // ====================================================================
    // Tour execution
    // ====================================================================

    /**
     * @brief Start a walkthrough by ID. Returns false if not found or already active.
     */
    auto Start(std::string_view walkthroughId) -> bool;

    /**
     * @brief Advance to the next step. Completes if at last step.
     */
    void Next();

    /**
     * @brief Go back to the previous step. No-op if at first step.
     */
    void Previous();

    /**
     * @brief Skip to a specific step index.
     */
    auto GoToStep(int index) -> bool;

    /**
     * @brief Pause the current walkthrough (keeps state, hides overlay).
     */
    void Pause();

    /**
     * @brief Resume a paused walkthrough.
     */
    void Resume();

    /**
     * @brief Dismiss (cancel) the current walkthrough.
     */
    void Dismiss();

    // ====================================================================
    // Query
    // ====================================================================

    [[nodiscard]] auto State() const -> WalkthroughState { return _state; }
    [[nodiscard]] auto ActiveWalkthroughId() const -> const std::string& { return _activeId; }
    [[nodiscard]] auto CurrentStepIndex() const -> int { return _currentStep; }
    [[nodiscard]] auto TotalSteps() const -> int;

    /**
     * @brief Get the current step, or nullptr if no walkthrough is active.
     */
    [[nodiscard]] auto CurrentStep() const -> const WalkthroughStep*;

    /**
     * @brief Progress as fraction [0.0, 1.0].
     */
    [[nodiscard]] auto Progress() const -> double;

    /**
     * @brief Check if a walkthrough has been completed (tracked by ID).
     */
    [[nodiscard]] auto IsCompleted(std::string_view walkthroughId) const -> bool;

    // ====================================================================
    // Observers
    // ====================================================================

    void OnStepChanged(StepChangedCallback cb);
    void OnStateChanged(StateChangedCallback cb);

private:
    void SetState(WalkthroughState s);
    auto ActiveWalkthrough() const -> const Walkthrough*;

    std::vector<Walkthrough>    _walkthroughs;
    std::vector<std::string>    _completedIds;
    std::string                 _activeId;
    int                         _currentStep = 0;
    WalkthroughState            _state       = WalkthroughState::Idle;
    StepChangedCallback         _stepCallback;
    StateChangedCallback        _stateCallback;
};

} // namespace matcha::fw
