#pragma once

#include <Matcha/Services/IExpansionPlugin.h>

namespace nyancad {

class NyanMeshVerificationRepairPlugin : public matcha::fw::IExpansionPlugin {
public:
    [[nodiscard]] auto Id() const -> std::string_view override;
    [[nodiscard]] auto Start(matcha::fw::Shell& shell) -> matcha::fw::Expected<void> override;
    void Stop() override;
};

} // namespace nyancad
