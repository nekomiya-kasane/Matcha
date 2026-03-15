# Matcha 架构完善方案 (Plan 2)

> **前提**: `UiNode : CommandNode` 继承关系保持不变。  
> **目标**: 针对 2026 前沿评审中发现的 7 项改进点，设计正向完善方案。  
> **原则**: 渐进引入、向后兼容、每步可独立交付和测试。

---

## 目录

| # | 改进项 | 严重度 | 所在 Phase |
|---|--------|--------|-----------|
| P2-1 | [Notification RTTI: dynamic_cast → static_cast fast-path](#p2-1-notification-rtti-优化) | 🟡 中 | A |
| P2-3 | [NotificationCallback: std::function → std::move_only_function](#p2-3-notificationcallback-类型升级) | 🟡 中 | A |
| P2-4 | [Widget() 返回类型收窄 — 降低 Qt 泄漏面](#p2-4-widget-返回类型收窄) | 🟡 中 | B |
| P2-6 | [Reactive Property Binding 系统](#p2-6-reactive-property-binding-系统) | 🔴 高 | C |
| P2-7 | [声明式 UI 描述层 (Matcha DSL)](#p2-7-声明式-ui-描述层) | 🔴 高 | D |

### Phase 依赖关系

```
Phase A (内部优化, 零 API 破坏)
  ├── P2-1  Notification RTTI 优化
  └── P2-3  move_only_function
        │
Phase B (API 微调, 向后兼容)
  └── P2-4  Widget() 收窄
        │
Phase C (新能力, 增量引入)
  └── P2-6  Reactive Property Binding
        │
Phase D (新层, 可选)
  └── P2-7  声明式 DSL
```

---

## P2-1: Notification RTTI 优化

### 现状

`AddAnalyseNotificationCB<T>` 中先用 `T{}.ClassName()` 做 string 过滤注册，dispatch 时再用 `dynamic_cast<T*>` 做向下转型：

```cpp
// CommandNode.h:322-332
template <typename T>
    requires std::derived_from<T, Notification>
auto AddAnalyseNotificationCB(CommandNode* publisher,
                               std::function<void(T&)> callback) -> CallbackId
{
    return Subscribe(publisher, T{}.ClassName(),
        [cb = std::move(callback)](EventNode& /*sender*/, Notification& notif) {
            if (auto* typed = notif.As<T>()) {   // ← dynamic_cast
                cb(*typed);
            }
        });
}
```

### 问题

- Subscribe 已经用 `ClassName()` string 匹配做了精确过滤
- `DispatchCallbacks` 只对 `notificationClassName` 匹配的 callback 触发
- 因此到达 `As<T>()` 时，类型**已被保证**（除非存在 ClassName 碰撞）
- `dynamic_cast` 在此处是冗余的安全检查，每次触发都有 RTTI 开销

### 方案

**在 `AddAnalyseNotificationCB` 的 lambda 中替换 `dynamic_cast` 为 `static_cast`**，前提是 ClassName 唯一性由框架保证。

#### 1. 添加 `UnsafeAs<T>()` 方法（Notification 基类）

```cpp
/// @brief Unchecked downcast. Caller MUST guarantee the runtime type.
/// Used by framework internals where ClassName filtering already guarantees type.
template <typename T>
    requires std::derived_from<T, Notification>
[[nodiscard]] auto UnsafeAs() -> T* {
    return static_cast<T*>(this);
}
```

#### 2. 修改 `AddAnalyseNotificationCB` lambda

```cpp
return Subscribe(publisher, T{}.ClassName(),
    [cb = std::move(callback)](EventNode& /*sender*/, Notification& notif) {
        cb(*notif.UnsafeAs<T>());  // static_cast, ClassName already matched
    });
```

#### 3. Debug 断言

在 `UnsafeAs` 中添加 debug 模式下的 `dynamic_cast` 断言：

```cpp
template <typename T>
    requires std::derived_from<T, Notification>
[[nodiscard]] auto UnsafeAs() -> T* {
    assert(dynamic_cast<T*>(this) != nullptr && "UnsafeAs: ClassName filter mismatch");
    return static_cast<T*>(this);
}
```

Release 模式下 assert 被移除，零开销。

#### 4. ClassName 唯一性保障

由框架约定保证：每个 Notification 子类的 `ClassName()` 返回值必须全局唯一。Debug 模式下 `UnsafeAs<T>()` 的 `dynamic_cast` 断言会捕获任何违规。

### 影响范围

| 文件 | 改动 |
|------|------|
| `Notification.h` | 添加 `UnsafeAs<T>()` |
| `CommandNode.h` | `AddAnalyseNotificationCB` lambda 改用 `UnsafeAs` |

### 测试

- 现有全部 1145 tests 不应有任何行为变化
- 新增：ClassName 碰撞检测测试

---

## P2-3: NotificationCallback 类型升级

### 现状

```cpp
using NotificationCallback = std::function<void(EventNode& sender, Notification& notif)>;
```

`std::function` 对超过 SBO 容量（通常 24-32 bytes）的 lambda 触发堆分配。EventNode 的 `_callbacks` vector 中每个 entry 都持有一个。

### 问题

- `AddAnalyseNotificationCB` 的 lambda 捕获了 `std::function<void(T&)>`（本身可能已经 SBO 溢出），外面再包一层 `std::function`，几乎必然堆分配
- callback 语义上是 move-only（从不拷贝），`std::function` 要求可拷贝是不必要的约束

### 方案

```cpp
// C++23 std::move_only_function
using NotificationCallback = std::move_only_function<void(EventNode& sender, Notification& notif)>;
```

### 兼容性

| 编译器 | 支持 |
|--------|------|
| Clang 21 | ✅ (since Clang 16) |
| MSVC 17.x | ✅ (since 17.2) |
| GCC 14 | ✅ (since 12) |

### 影响范围

| 文件 | 改动 |
|------|------|
| `EventNode.h` | `using` 声明改为 `std::move_only_function` |
| `EventNode.cpp` | 无（move 语义已就位） |
| `ScopedSubscription` 构造函数 | 参数类型跟随变化 |

### 注意事项

- `std::move_only_function` 不可拷贝 → `CallbackEntry` 不可拷贝 → `_callbacks` vector 的 `erase` 操作需要 move 而非 copy（已满足，`std::vector<T>::erase` 对 move-only 类型合法）
- 外部代码如果有拷贝 `NotificationCallback` 的需求将编译失败——但根据现有代码审查，无此情况

### 测试

- 全部现有测试应通过（callback 语义不变）

---

## P2-4: Widget() 返回类型收窄

### 现状

```cpp
// UiNode.h:318
[[nodiscard]] virtual auto Widget() -> QWidget* { return nullptr; }
```

fw 层的 `UiNode` 基类直接暴露 `QWidget*`，任何拿到 `UiNode*` 的代码都能绕过抽象层操作 Qt。

### 设计约束

- **不能删除 Widget()**：内部 framework 代码大量依赖（如 `ReparentTo`、`ResolveParentWidget`、container 布局同步）
- **不能改返回 opaque handle**：改动面太大，且 Qt widget 树确实需要 `QWidget*`
- **需要限制外部可见性**

### 方案：分层可见性

#### 1. UiNode::Widget() 改为 `protected`

```cpp
class MATCHA_EXPORT UiNode : public matcha::CommandNode {
public:
    // ... (public API 不含 Widget())

protected:
    /// @brief Return the underlying QWidget. Framework-internal.
    /// Business layer should use WidgetNode typed accessors instead.
    [[nodiscard]] virtual auto Widget() -> QWidget* { return nullptr; }

    friend class Application;       // needs Widget() for window setup
    friend class WorkspaceFrame;    // needs Widget() for layout sync
    friend class FocusManager;      // needs Widget() for focus routing
    // ... (enumerate all internal consumers)
};
```

#### 2. WidgetNode 提供受控访问

```cpp
class MATCHA_EXPORT WidgetNode : public UiNode {
public:
    // Typed public accessor — still returns QWidget* but documented as
    // "for integration only, prefer SetXxx/GetXxx property APIs"
    [[nodiscard]] auto Widget() -> QWidget* override;

    // NEW: Opaque handle for C ABI / cross-boundary use
    [[nodiscard]] auto NativeHandle() -> void*;
};
```

#### 3. 过渡策略

- Phase 1: 添加 `[[deprecated("Use WidgetNode typed accessors")]]` 到 `UiNode::Widget()`
- Phase 2: 将 `UiNode::Widget()` 改为 `protected`，用 friend 列表覆盖内部使用者
- Phase 3: 审查所有 `UiNode::Widget()` 调用点，迁移到 typed accessor 或 friend

### 影响范围

需要逐步审查所有 `Widget()` 调用点。初步 grep 估算：

| 层 | 调用点 | 处理方式 |
|----|--------|---------|
| UiNode 内部 | `ReparentTo`, `ResolveParentWidget` | protected 访问，无需改 |
| Container nodes | `AddNode` 中同步 Qt widget tree | protected 访问，无需改 |
| Application/Shell | 窗口构建 | friend |
| WidgetNode 子类 | `CreateWidget` + 属性转发 | override 保持 public |
| Demo/Test | 部分直接 `->Widget()` | 迁移到 typed API |
| C API | `NyanCApi.cpp` | 通过 `NativeHandle()` |

### 测试

- 编译通过即验证（类型系统约束）

---

## P2-6: Reactive Property Binding 系统

### 现状

WidgetNode 属性全部命令式单向推送：

```cpp
lineEditNode->SetText("hello");
// model 变化后必须手动再次调用 SetText
```

无 reactive binding，无双向同步，无依赖追踪。

### 2026 前沿对标

| 框架 | 机制 | 特点 |
|------|------|------|
| SwiftUI | `@State`, `@Binding` | 编译期类型安全，自动 diff |
| Compose | `mutableStateOf()` | 运行时快照隔离 |
| Qt QML | Property bindings | 表达式重求值 |
| Slint | `.slint` property bindings | 声明式，编译期检查 |

### 设计方案

#### 核心类型：`Observable<T>`

位于 Foundation 层（零 Qt 依赖）。

```cpp
namespace matcha::fw {

/// @brief Reactive observable value with change notification.
/// Thread model: single-thread (GUI thread), no locking.
template <typename T>
class Observable {
public:
    using Callback = std::move_only_function<void(const T& oldVal, const T& newVal)>;

    explicit Observable(T initial = {}) : _value(std::move(initial)) {}

    /// @brief Get current value.
    [[nodiscard]] auto Get() const -> const T& { return _value; }

    /// @brief Implicit conversion for read access.
    [[nodiscard]] operator const T&() const { return _value; }

    /// @brief Set value. Fires observers only if value changed.
    void Set(T newValue) {
        if (_value == newValue) return;
        T old = std::exchange(_value, std::move(newValue));
        for (auto& [id, cb] : _observers) {
            cb(old, _value);
        }
    }

    /// @brief Observe changes. Returns handle for removal.
    [[nodiscard]] auto Observe(Callback cb) -> uint64_t {
        auto id = _nextId++;
        _observers.emplace_back(id, std::move(cb));
        return id;
    }

    /// @brief Remove observer by handle.
    void Unobserve(uint64_t handle) {
        std::erase_if(_observers, [handle](auto& p) { return p.first == handle; });
    }

private:
    T _value;
    std::vector<std::pair<uint64_t, Callback>> _observers;
    uint64_t _nextId = 1;
};

} // namespace matcha::fw
```

#### Binding 辅助：`PropertyBinding`

RAII guard，连接 `Observable<T>` 到 WidgetNode setter。

```cpp
namespace matcha::fw {

/// @brief RAII binding from Observable<T> to a setter function.
template <typename T>
class PropertyBinding {
public:
    PropertyBinding() = default;

    PropertyBinding(Observable<T>& source, std::move_only_function<void(const T&)> setter)
        : _source(&source)
        , _setter(std::move(setter))
    {
        // Initial sync
        _setter(source.Get());
        // Observe future changes
        _handle = source.Observe([this](const T&, const T& newVal) {
            _setter(newVal);
        });
    }

    ~PropertyBinding() {
        if (_source) _source->Unobserve(_handle);
    }

    // Move-only
    PropertyBinding(PropertyBinding&& o) noexcept { Swap(o); }
    auto operator=(PropertyBinding&& o) noexcept -> PropertyBinding& {
        if (this != &o) { PropertyBinding tmp(std::move(o)); Swap(tmp); }
        return *this;
    }

private:
    void Swap(PropertyBinding& o) noexcept {
        std::swap(_source, o._source);
        std::swap(_setter, o._setter);
        std::swap(_handle, o._handle);
    }

    Observable<T>* _source = nullptr;
    std::move_only_function<void(const T&)> _setter;
    uint64_t _handle = 0;
};

} // namespace matcha::fw
```

#### WidgetNode 层 Bind API

在具体 Node 子类（如 `LineEditNode`）上提供 typed bind 方法：

```cpp
class LineEditNode : public WidgetNode {
public:
    // Existing imperative API (preserved)
    void SetText(std::string_view text);

    // NEW: Reactive binding API
    void BindText(Observable<std::string>& source) {
        _textBinding = PropertyBinding<std::string>(source,
            [this](const std::string& val) { SetText(val); });
    }

    // NEW: Two-way binding (widget edits push back to Observable)
    void BindTextTwoWay(Observable<std::string>& source) {
        BindText(source);
        // Subscribe to TextChanged notification from self
        _textChangeSub = ScopedSubscription(*this, this, "TextChanged",
            [&source](EventNode&, Notification& notif) {
                if (auto* tc = notif.As<TextChanged>()) {
                    source.Set(std::string(tc->Text()));
                }
            });
    }

private:
    PropertyBinding<std::string> _textBinding;
    ScopedSubscription _textChangeSub;
};
```

#### 使用示例（业务层）

```cpp
// Business layer ViewModel
struct DocumentSettings {
    Observable<std::string> title{"Untitled"};
    Observable<bool> readOnly{false};
    Observable<int> fontSize{12};
};

// Wiring
DocumentSettings settings;
titleEdit->BindTextTwoWay(settings.title);
readOnlyToggle->BindChecked(settings.readOnly);
fontSizeSpinner->BindValueTwoWay(settings.fontSize);

// Now any changes to settings.title automatically update the widget,
// and user edits in the widget automatically update settings.title.
settings.title.Set("My Document"); // widget auto-updates
```

### 实施分步

| 步骤 | 内容 | 新文件 |
|------|------|--------|
| C-1 | `Observable<T>` + 单元测试 | `Include/Matcha/Foundation/Observable.h` |
| C-2 | `PropertyBinding<T>` + 单元测试 | `Include/Matcha/Foundation/PropertyBinding.h` |
| C-3 | `LineEditNode::BindText/BindTextTwoWay` 作为参考实现 | 修改 `LineEditNode.h/cpp` |
| C-4 | 扩展到其他 WidgetNode 子类 | 逐个添加 `BindXxx` |
| C-5 | `Computed<T>` 派生值（依赖追踪） | `Include/Matcha/Foundation/Computed.h` |

### 设计决策记录

| 决策 | 选择 | 理由 |
|------|------|------|
| 依赖追踪方式 | 显式 Observe/Unobserve | 避免隐式全局状态（如 Compose 的 snapshot system），与 CATIA command tree 显式订阅模型一致 |
| 线程模型 | 单线程（GUI thread） | 与 Matcha 整体模型一致，不引入锁竞争 |
| 变更检测 | `operator==` | 简单可预测，不引入 deep-diff 复杂度 |
| 双向绑定 | 通过 Notification 回路 | 复用已有 command tree 通信，不引入新传播机制 |
| 循环防护 | Set() 内部 `_dispatching` flag | 防止 A→B→A 无限循环 |

### 与现有 Notification 系统的关系

Observable 是**局部 binding**（一个 source → 一个 widget property），Notification 是**树级广播**（一个 sender → 所有 ancestor subscribers）。两者互补：

```
Observable<T>  ──bind──>  WidgetNode.SetXxx()     (local, 1:1)
WidgetNode     ──notif──> CommandTree propagation   (global, 1:N)
```

双向绑定通过 Notification → Observable 回路闭合。

---

## P2-7: 声明式 UI 描述层

### 现状

所有 UI 命令式构建：

```cpp
auto* btn = new NyanPushButton(parent);
btn->SetText("Save");
btn->SetIcon(fw::icons::Save);
layout->addWidget(btn);
```

### 设计方案：编译期 C++23 DSL

**不引入外部 DSL 文件（.qml / .slint）**，而是利用 C++23 的 designated initializers + CTAD + operator overloading 实现 in-language DSL。

#### 蓝图描述

```cpp
namespace matcha::dsl {

/// @brief Blueprint for a UI subtree. Evaluated at runtime but type-checked at compile time.
struct Blueprint {
    NodeType type;
    std::string id;
    std::vector<std::pair<std::string, std::string>> properties;  // key-value
    std::vector<Blueprint> children;
};

/// @brief Compile-time verified node descriptor.
template <FixedString TypeName>
struct Node {
    static constexpr NodeType kType = ParseNodeType(TypeName.view());

    std::string id;
    std::vector<std::pair<std::string, std::string>> props;
    std::vector<Blueprint> children;

    auto operator()(auto&&... childNodes) const -> Blueprint {
        Blueprint bp{kType, id, props, {}};
        (bp.children.push_back(std::forward<decltype(childNodes)>(childNodes).Build()), ...);
        return bp;
    }

    auto Build() const -> Blueprint { return {kType, id, props, {}}; }
};

/// @brief Property setter (fluent API).
template <FixedString TypeName>
auto operator|(Node<TypeName> node, std::pair<std::string, std::string> prop) -> Node<TypeName> {
    node.props.push_back(std::move(prop));
    return node;
}

// Convenience factory
template <FixedString TypeName>
constexpr auto N(std::string id = {}) -> Node<TypeName> {
    return {.id = std::move(id)};
}

} // namespace matcha::dsl
```

#### 使用示例

```cpp
using namespace matcha::dsl;

auto settingsPanel = N<"Container">("settings-panel")(
    N<"Label">("title") | Prop("text", "Settings"),
    N<"LineEdit">("name-input") | Prop("placeholder", "Enter name"),
    N<"CheckBox">("dark-mode") | Prop("text", "Dark Mode") | Prop("checked", "true"),
    N<"Container">("buttons")(
        N<"PushButton">("ok-btn") | Prop("text", "OK"),
        N<"PushButton">("cancel-btn") | Prop("text", "Cancel")
    )
);

// Materialize into live UiNode tree
auto* root = Materialize(settingsPanel, parentNode);
```

#### Materializer

```cpp
/// @brief Materialize a Blueprint into a live UiNode subtree.
/// @param bp The blueprint to materialize.
/// @param parent The parent UiNode to attach to.
/// @return Non-owning pointer to the root of the materialized subtree.
auto Materialize(const Blueprint& bp, UiNode* parent) -> UiNode*;
```

`Materialize` 内部使用 factory registry（`NodeType → factory function`）创建节点，应用 properties，递归处理 children。

### 分步实施

| 步骤 | 内容 | 新文件 |
|------|------|--------|
| D-1 | `Blueprint` + `Node<>` 类型 | `Include/Matcha/Foundation/Blueprint.h` |
| D-2 | `Materialize()` 引擎 | `Source/UiNodes/Core/Materializer.cpp` |
| D-3 | Property applicator registry | `Source/UiNodes/Core/PropertyApplicator.cpp` |
| D-4 | Reactive binding 集成（P2-6 依赖） | Blueprint 中支持 `BindProp("text", observable)` |
| D-5 | Hot reload | 从 JSON 反序列化 Blueprint |

### 设计决策记录

| 决策 | 选择 | 理由 |
|------|------|------|
| DSL 载体 | C++ in-language（非外部文件） | 保持单一语言，编译期类型检查，无 code-gen 工具链 |
| NodeType 验证 | `consteval ParseNodeType` | 编译期拒绝未知类型名，零运行时开销 |
| Property 类型 | `string→string` | 最大灵活性，applicator 负责解析。可在 Phase 2 引入 typed property |
| 与 UiNodeQuery 关系 | Blueprint 创建树，Query 查询树 | 互补，非替代 |

---

## 实施时间线

| Phase | 内容 | 预估工时 | 前置依赖 |
|-------|------|---------|---------|
| **A** | P2-1 + P2-3 | 1 天 | 无 |
| **B** | P2-4 | 1-2 天 | 无（可与 A 并行） |
| **C** | P2-6 | 3-5 天 | 无（可与 A/B 并行） |
| **D** | P2-7 | 5-8 天 | P2-6 (for binding integration) |

**总计: 10-16 天**

---

## 风险与缓解

| 风险 | 缓解措施 |
|------|---------|
| P2-3 move_only_function 对外部代码破坏 | 审查所有 Subscribe 调用点，确认无拷贝 callback 的用例 |
| P2-4 Widget() protected 破坏大量调用点 | 分步迁移（deprecated → protected），friend 列表覆盖内部 |
| P2-6 Observable 循环更新 | Set() 内部 `_dispatching` reentrancy guard |
| P2-7 Blueprint property 类型不安全 | Phase 2 引入 typed property variant；Phase 1 先用 string |
