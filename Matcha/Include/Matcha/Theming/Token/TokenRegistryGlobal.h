#pragma once

/**
 * @file TokenRegistryGlobal.h
 * @brief fw 层的全局 ITokenRegistry 访问器。
 *
 * 由 Application 在启动时设置一次。ContainerNode 和其他 fw 层类使用它来
 * 将 SpaceToken 解析为像素值，而无需依赖 matcha::gui 层。
 *
 * @see COCAUI_Design_System_Specification.md 第 6 章 IThemeService 接口
 */

#include <Matcha/Core/Macros.h>

namespace matcha::fw {

  class ITokenRegistry;
  /**
   * @brief 设置全局令牌注册表实例。
   * @param registry 指向 ITokenRegistry 的指针（必须在所有使用者之前设置，且生命周期长于使用者）。
   */
  MATCHA_EXPORT void SetGlobalTokenRegistry(ITokenRegistry* registry);
  /**
   * @brief 获取全局令牌注册表实例。
   * @return 指向 ITokenRegistry 的指针，如果尚未设置则返回 nullptr。
   */
  [[nodiscard]] MATCHA_EXPORT auto GetGlobalTokenRegistry() -> ITokenRegistry*;

}  // namespace matcha::fw
