#pragma once

/**
 * @file MetaClass.h
 * @brief Static meta-class system for runtime type identification.
 *
 * Matcha equivalent of CATMetaClass / CATDeclareClass / CATImplementClass.
 * Provides a compile-time-registered, linked-list-style class hierarchy that
 * enables string-based RTTI (IsAKindOf) without relying on dynamic_cast for
 * type-name queries.
 *
 * @par Design
 * Each class in the BaseObject hierarchy has exactly one static MetaClass
 * instance, created via MATCHA_IMPLEMENT_CLASS(). The MetaClass stores:
 *  - The class name (string_view, compile-time constant)
 *  - A pointer to the parent class's MetaClass (nullptr for BaseObject)
 *
 * IsAKindOf(name) walks the MetaClass parent chain — O(depth), no RTTI,
 * no dynamic_cast, no hash maps.
 *
 * @par Usage
 * In the class declaration (header):
 * @code
 *   class MATCHA_EXPORT MyNode : public CommandNode {
 *       MATCHA_DECLARE_CLASS
 *   public:
 *       // ...
 *   };
 * @endcode
 *
 * In the implementation file (.cpp):
 * @code
 *   MATCHA_IMPLEMENT_CLASS(MyNode, CommandNode)
 * @endcode
 *
 * @par Event Bridge Pattern (Business-Layer Wrapper)
 * Matcha is a pure UI framework — it does NOT contain business logic.
 * Business-layer wrappers (deriving from CommandNode) wrap Qt widgets and
 * are responsible for:
 *  1. Connecting to all business-relevant Qt signals on their wrapped widget
 *  2. Converting each Qt signal into a typed Notification subclass
 *  3. Dispatching the Notification via CommandNode::SendNotification() so it
 *     propagates up the command tree to other business modules
 *
 * This decouples the UI framework from business semantics — the framework
 * provides the tree structure and notification transport, while the business
 * layer defines what events mean and who handles them.
 */

#include "Matcha/Core/Macros.h"

#include <string_view>

namespace matcha {

/**
 * @brief Static meta-class descriptor — one per class in the hierarchy.
 *
 * Equivalent to CATMetaClass in 3DEXPERIENCE. Each MetaClass instance is
 * a static singleton created by MATCHA_IMPLEMENT_CLASS(). The parent pointer
 * forms a singly-linked chain from leaf to root (BaseObject, whose parent
 * is nullptr).
 */
struct MATCHA_EXPORT MetaClass {
    std::string_view name;        ///< Class name (e.g., "CommandNode")
    const MetaClass* parent;      ///< Parent class MetaClass (nullptr for root)

    /**
     * @brief Walk the parent chain to check if this class is, or derives from,
     *        a class with the given name.
     * @param queryName The class name to search for.
     * @return true if this MetaClass or any ancestor has name == queryName.
     */
    [[nodiscard]] auto IsAKindOf(std::string_view queryName) const -> bool {
        for (const auto* mc = this; mc != nullptr; mc = mc->parent) {
            if (mc->name == queryName) {
                return true;
            }
        }
        return false;
    }
};

} // namespace matcha

// --------------------------------------------------------------------------- //
// Macros
// --------------------------------------------------------------------------- //

/**
 * @brief Declare MetaClass support inside a class body.
 *
 * Place this macro in the class declaration (typically right after the opening
 * brace, before public:). It declares:
 *  - A static MetaClass instance (s_metaClass)
 *  - A static accessor GetStaticMetaClass()
 *  - A virtual override GetMetaClass()
 *  - ClassName() and IsAKindOf() overrides that delegate to MetaClass
 *
 * Equivalent to CATDeclareClass.
 */
#define MATCHA_DECLARE_CLASS                                                     \
public:                                                                          \
    static const matcha::MetaClass s_metaClass;                                  \
    [[nodiscard]] static auto GetStaticMetaClass() -> const matcha::MetaClass* { \
        return &s_metaClass;                                                     \
    }                                                                            \
    [[nodiscard]] auto GetMetaClass() const -> const matcha::MetaClass* override {\
        return &s_metaClass;                                                     \
    }                                                                            \
    [[nodiscard]] auto ClassName() const -> std::string_view override {           \
        return s_metaClass.name;                                                 \
    }                                                                            \
    [[nodiscard]] auto IsAKindOf(std::string_view _name) const -> bool override {\
        return s_metaClass.IsAKindOf(_name);                                     \
    }                                                                            \
private:

/**
 * @brief Define the static MetaClass instance in a .cpp file.
 *
 * @param ClassName  The class being registered (e.g., CommandNode).
 * @param ParentClass The direct parent class (e.g., EventNode).
 *
 * Equivalent to CATImplementClass.
 *
 * @code
 *   MATCHA_IMPLEMENT_CLASS(CommandNode, EventNode)
 * @endcode
 */
#define MATCHA_IMPLEMENT_CLASS(Class, ParentClass) \
    const matcha::MetaClass Class::s_metaClass{#Class, ParentClass::GetStaticMetaClass()};
