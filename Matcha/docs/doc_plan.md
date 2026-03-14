# Matcha Design System Specification — 补全计划

---

## 1. Part I 七柱结构评估

### 1.1 提案

将 Part I (Design Language) 划分为 7 个 Section，允许四层目录：

```
# Part I: Design Language
## I.1 Tokens
### 1.1 Color System
#### 1.1.1 Neutral Scale
```

7 个 Section: **Tokens · Typography · Style · Widgets · Layout · Interaction · Motion**

### 1.2 评估

| 维度 | 评价 | 说明 |
|------|:----:|------|
| **行业对标** | ✅ | Material Design 3 = Color / Typography / Layout / Interaction / Motion / Components；Carbon = Tokens / Typography / Layout / Components / Patterns；Apple HIG = Foundations / Patterns / Components / Technologies。Matcha 的 7 柱与行业主流高度一致 |
| **认知分组** | ✅ | 7±2 规则内（Miller's Law），每柱对应一个设计师的独立认知域 |
| **自包含性** | ✅ | 每柱可独立阅读——设计师可以只看 Typography 或只看 Motion |
| **阅读顺序** | ✅ | 自然渐进：原语(Tokens → Typography) → 系统(Style) → 目录(Widgets) → 行为(Layout → Interaction → Motion) |
| **4 层目录** | ✅ | 当前 3 层导致 Ch 10 (46 widgets) 过于扁平；4 层允许 Section → Chapter → Sub-section → Detail |

### 1.3 需要调整的细节

| 问题 | 决策 | 理由 |
|------|------|------|
| Typography 从 Tokens 独立是否合理？ | **独立** | Typography 有独有关注点（字体回退、CJK、平台选择、字号阶梯、行高比例），Material/Apple/Carbon 均独立一节 |
| Color 在 Tokens 还是独立？ | **在 Tokens 内** | Color 虽然复杂（75 tokens），但其关注点（色相、对比度、语义映射）本质是 token 生成规则，不像 Typography 有字体加载等外部依赖 |
| Icon/Cursor 归属？ | **Tokens 内** | Icon 是视觉原语（URI 体系、设计语言、色彩继承模型），Cursor 是状态→形状映射——都是 token 级别的原语定义 |
| Accessibility/i18n 归属？ | **Interaction 内** | 键盘导航、Mnemonic、Focus 管理、屏幕阅读器——本质是交互行为的子集。对比度要求交叉引用 Tokens |
| Style 是否独立？ | **独立** | Style 是 Tokens → Widgets 的桥梁（WidgetStyleSheet、Variant×State、ResolvedStyle），定义"如何将 token 组合为组件视觉"。这是设计系统的核心抽象，值得独立一节 |
| Loading/Empty/Error 状态归属？ | **Layout 内** | 这些是页面级状态模板，影响布局区域内容的替换，与 Layout 的 Composition Templates 自然关联 |

### 1.4 最终结论

**7 柱结构是好主意**，调整后的完整 Part I 结构见 §2。

---

## 2. 完整文档结构

### Part 0: Motivation & Theoretical Foundations（不变）

- Ch 0: The Problem of UI Redesign under Architectural Inertia

### Part I: Design Language Specification

> 设计师的完整表达。定义设计系统的全部视觉、交互、行为规格。
> 粒度 = 当前 Ch 2-5 水平（含 token 值、struct 定义、数学公式）。
> 可交叉引用后续 Parts 的实现架构。

#### I.1 Foundations & Principles

> 提纲挈领：设计哲学、架构分层、术语表。

| § | 标题 | 来源 | 变更 |
|---|------|------|------|
| 1.1 | Document Scope & Audience | 原 §1.1 | 不变 |
| 1.2 | Core Design Principles | 原 §1.2 | 不变 |
| 1.3 | Design Language Principles | 原 §1.3 | 不变 |
| 1.4 | Three-Layer Token Architecture | 原 §1.4 | 不变 |
| 1.5 | Architecture Layer Diagram | 原 §1.5 | 不变 |
| 1.6 | Terminology Glossary | 原 §1.6 | 不变 |

#### I.2 Tokens

> 原语层：颜色、空间、图标、光标。

| § | 标题 | 来源 | 变更 |
|---|------|------|------|
| 2.1 | Color System | 原 Ch 2 全部 | 不变 |
| 2.2 | Spatial System | 原 Ch 4 全部 | 不变 |
| 2.3 | Icon Design Language | 原 Ch 16 (§16.1-16.6, §16.8) | **移入 Part I**；resolution pipeline 交叉引用 Part II |
| 2.4 | Cursor Design | 原 Ch 17 全部 | **移入 Part I** |

#### I.3 Typography

> 字体体系独立一节，含字族选择、字号阶梯、平台回退、CJK。

| § | 标题 | 来源 | 变更 |
|---|------|------|------|
| 3.1-3.7 | Typography System | 原 Ch 3 全部 | 不变 |

#### I.4 Style

> Token → Widget 的桥梁：声明式样式架构、变体模式、状态矩阵。

| § | 标题 | 来源 | 变更 |
|---|------|------|------|
| 4.1 | Declarative Style Architecture | 原 Ch 9 (§9.1-9.9) | **移入 Part I**（设计定义部分）；Resolve() 实现 → Part III |
| 4.2 | Standard Variant Patterns | 原 Ch 9 (§9.9) | 不变 |
| 4.3 | WidgetKind Registry | 原 Ch 11 (§11.1-11.3) | **移入 Part I**（enum 定义 + mapping 表）；Override 机制 → Part III |

#### I.5 Widgets

> 46 个组件的完整视觉和行为规格。

| § | 标题 | 来源 | 变更 |
|---|------|------|------|
| 5.1-5.46 | Per-Widget Component Spec | 原 Ch 10 全部 | **深化** 🆕：+Anatomy +FSM +Keyboard +Usage +Math per widget |

每个 widget 的标准 12 节模板：

```
Synopsis · Anatomy 🆕 · Theme Properties · Variant×State Matrix
Interaction FSM 🆕 · Notification Catalog · UiNode API
Animation · Mathematical Model · Keyboard Contract 🆕
Accessibility · Usage Guidelines 🆕
```

- 25 个交互控件：完整 12 节
- 21 个静态/容器控件：省略 FSM（标注 "Stateless"）和 Keyboard（标注 "Not focusable"），保留 Anatomy + Usage

#### I.6 Layout

> 布局算法、页面组合模板、响应式规则。

| § | 标题 | 来源 | 变更 |
|---|------|------|------|
| 6.1 | Layout Algorithm Specification 🆕 | 新增 | HBox/VBox/Grid/Form 分配算法、flex、overflow、spacing |
| 6.2 | Composition Templates 🆕 | 新增 | Shell Layout / Property Panel / Dialog Templates (5 types) |
| 6.3 | Responsive Rules 🆕 | 新增 | 窗口缩小优先级矩阵、collapse/auto-hide 阈值、minimum viable size |
| 6.4 | Loading / Empty / Error State Templates 🆕 | 新增 | skeleton/spinner/empty/error 模板、per-region 状态矩阵 |

#### I.7 Interaction

> 跨组件交互模式、无障碍设计规则、国际化设计。

| § | 标题 | 来源 | 变更 |
|---|------|------|------|
| 7.1 | Selection Model 🆕 | 新增 | FSM、Ctrl/Shift 语义、anchor/focus、rubber-band、跨视图同步 |
| 7.2 | Form Validation 🆕 | 新增 | Rule 类型、trigger timing、error display、dirty/pristine FSM |
| 7.3 | Scroll & Virtualization 🆕 | 新增 | 动量衰减、overscroll、snap、virtual scrolling、scroll-to-focus |
| 7.4 | Popup Positioning 🆕 | 新增 | 12-anchor、flip/shift/resize、maxHeight、arrow、screen-edge |
| 7.5 | Text Overflow & Truncation 🆕 | 新增 | per-widget elide、multi-line clamp、truncation-tooltip |
| 7.6 | Context Menu Composition 🆕 | 新增 | 多源合并规则、ordering、separator auto-insert |
| 7.7 | Notification Stacking 🆕 | 新增 | 堆叠方向/间距/maxVisible、队列、priority |
| 7.8 | Drag & Drop Design 🆕 | 新增 | 通用 DnD FSM、preview spec、drop zone 视觉 |
| 7.9 | Accessibility Design | 原 Ch 18 (§18.1-18.2, §18.4, §18.6-18.11) | **移入 Part I** +shortcut mgmt 🆕 +HiDPI 🆕 |
| 7.10 | Internationalization Design | 原 Ch 19 全部 | **移入 Part I** +per-widget RTL rules 🆕 |

#### I.8 Motion

> 时间语言：时长、缓动、弹簧、编排、手势动效。

| § | 标题 | 来源 | 变更 |
|---|------|------|------|
| 8.1 | Duration & Easing Tokens | 原 Ch 5 (§5.1-5.2) | 不变 |
| 8.2 | Spring Dynamics | 原 Ch 5 (§5.3) | 不变 |
| 8.3 | TransitionDef | 原 Ch 5 (§5.4) | 不变 |
| 8.4 | Interaction Timing Tokens 🆕 | 新增 | tooltip=500ms, submenu=200ms, search=250ms, longPress=500ms |
| 8.5 | Popup Lifecycle FSM 🆕 | 新增 | 通用 popup open/close/reposition FSM |
| 8.6 | Drag Gesture Timing 🆕 | 新增 | manhattan distance, time threshold, preview 规格 |
| 8.7 | Choreography 🆕 | 新增 | stagger 策略、关联动画、中断决策树 |
| 8.8 | Gesture-Driven Motion 🆕 | 新增 | 拖拽弹簧跟随、释放吸附/回弹 |
| 8.9 | Reduced Motion Design | 原 Ch 5 (§5.5-5.6) | +per-animation 替代行为表 🆕 |

---

### Part II: Theme Engine（实现 Part I 的 token 解析 + icon pipeline）

| Ch | 标题 | 来源 | 变更 |
|:--:|------|------|------|
| 9 | IThemeService Interface | 原 Ch 6 | 不变 |
| 10 | JSON Theme Configuration | 原 Ch 7 | 不变 |
| 11 | NyanTheme Implementation | 原 Ch 8 | +§11.14 Theme Transition 🆕 |
| 12 | Icon Resolution Pipeline | 原 Ch 16 (§16.7, §16.9-16.10) | 实现 Part I §2.3 的 icon 设计 |

### Part III: Component Style Architecture（实现 Part I §4 的样式解析）

| Ch | 标题 | 来源 | 变更 |
|:--:|------|------|------|
| 13 | Resolve() & BuildDefaultVariants() | 原 Ch 9 (§9.7-9.12) + Ch 8 (§8.4-8.5, §8.10-8.11) | 合并样式解析实现 |
| 14 | Component Override Mechanism | 原 Ch 11 (§11.4-11.8) | 不变 |

### Part IV: Animation Engine（实现 Part I §8 的动效物理）

| Ch | 标题 | 来源 | 变更 |
|:--:|------|------|------|
| 15 | Animation Architecture | 原 Ch 12 | 不变 |
| 16 | Spring Physics Implementation | 原 Ch 13 | 不变 |
| 17 | Widget Animation Integration | 原 Ch 14 | 不变 |

### Part V: Accessibility & i18n Infrastructure（实现 Part I §7.9-7.10）

| Ch | 标题 | 来源 | 变更 |
|:--:|------|------|------|
| 18 | Focus Management | 原 Ch 18 (§18.3) | 不变 |
| 19 | A11yAudit | 原 Ch 18 (§18.5) | 不变 |
| 20 | Focus Trap & Screen Reader | 原 Ch 18 (§18.8-18.10) | 不变 |
| 21 | Mnemonic System Architecture | 原 Ch 18 (§18.11.18) | 不变 |

### Part VI: Dynamic Injection

| Ch | 标题 | 来源 |
|:--:|------|------|
| 22 | Dynamic Token Extension | 原 Ch 20 |
| 23 | Custom Theme Registration | 原 Ch 21 |
| 24 | C ABI | 原 Ch 22 |

### Part VII: Testing & Validation

| Ch | 标题 | 来源 |
|:--:|------|------|
| 25 | Test Infrastructure | 原 Ch 23 |
| 26 | JSON Validation Pipeline | 原 Ch 24 |
| 27 | Design Token Consistency | 原 Ch 25 |

### Part VIII: UI Architecture

| Ch | 标题 | 来源 | 变更 |
|:--:|------|------|------|
| 28 | UiNode Tree Architecture | 原 Ch 26 | +§28.9 Generic DnD Protocol 🆕 |
| 29 | Multi-Window & Floating Tab | 原 Ch 27 | +§29.6 Window Management 🆕 |
| 30 | Viewport System | 原 Ch 28 | 不变 |
| 31 | ActionBar Drag & Dock | 原 Ch 29 | 不变 |

### Part IX: Roadmap

| Ch | 标题 | 来源 |
|:--:|------|------|
| 32 | Implementation Roadmap | 原 Ch 30 |

### Appendices A-E

- Appendix B: +Undo/Redo System 🆕

---

## 3. Part I 的 4 层目录示意

```
# Part I: Design Language Specification
                                                    ← Level 1: Part
## I.1 Foundations & Principles
## I.2 Tokens
                                                    ← Level 2: Section
### 2.1 Color System
### 2.2 Spatial System
### 2.3 Icon Design Language
### 2.4 Cursor Design
                                                    ← Level 3: Chapter
#### 2.1.1 Color Token Vocabulary Overview
#### 2.1.2 Neutral Scale
#### 2.1.3 Semantic Hue Scales
#### 2.1.4 Special Purpose Tokens
#### 2.1.5 Tonal Palette Generation Algorithm
                                                    ← Level 4: Sub-section

## I.5 Widgets

### 5.1 PushButton
#### Synopsis
#### Anatomy 🆕
#### Theme-Customizable Properties
#### Variant & State Matrix
#### Interaction FSM 🆕
#### Notification Catalog
#### UiNode Public API
#### Animation Specification
#### Mathematical Model
#### Keyboard Contract 🆕
#### Accessibility Contract
#### Usage Guidelines 🆕

### 5.2 ToolButton
...
```

---

## 4. 新增内容总量

| Part I Section | 新增 §数 | ~行数 | 类型 |
|----------------|:--------:|------:|------|
| I.2 Tokens (icon/cursor 移入) | 0 新 | ~100 改 | 重组 |
| I.4 Style (declarative style 移入) | 0 新 | ~50 改 | 重组 |
| I.5 Widgets (深化 46 widgets) | ~150 子节 | ~2000 | 🆕 |
| I.6 Layout (全新) | 4 | ~750 | 🆕 |
| I.7 Interaction (10 sections, 含移入+新增) | 8 新 | ~1700 | 🆕 + 重组 |
| I.8 Motion (扩展+新增) | 5 新 | ~500 | 🆕 |
| **Part I 小计** | | **~5100** | |
| Parts II-VIII 扩展 | 6 | ~600 | 🆕 |
| 重编号 + TOC | — | ~300 | 改 |
| **总计** | | **~6000** | |

最终 spec: ~9400 → **~15400 行**，32 章 + 附录。

---

## 5. 执行顺序

| Step | 操作 |
|:----:|------|
| 1 | 全文重组+重编号：Part I 7 柱结构，移入 Icon/Cursor/A11y/i18n/Style，后续 Parts 重排 |
| 2 | Part I §6 Layout：4 sections 全量写入 |
| 3 | Part I §7 Interaction：8 新 sections 写入 |
| 4 | Part I §8 Motion：5 新 sections 写入 |
| 5 | Part I §5 Widgets：逐 widget 深化（Anatomy/FSM/Keyboard/Usage/Math） |
| 6 | Parts II-VIII 各章扩展（Theme Transition, DnD, Window Mgmt, Undo/Redo） |
| 7 | TOC 最终更新 |
| 8 | 质量检查 |

---

## 6. 质量检查

- [ ] 所有新增标题含 `🆕`
- [ ] 4 层目录深度一致
- [ ] Review 1 的 21 个 Gap 全覆盖
- [ ] Review 2 七层模型每层 ★★★
- [ ] 46 widget 12 节模板完整
- [ ] FSM 用 mermaid，公式用 LaTeX
- [ ] 全文编号连续，TOC 一致
- [ ] Part I 内部交叉引用指向后续 Parts 正确编号
- [ ] 无 TODO/TBD/placeholder

---

## 7. 结构与完备性审计（对照 review.md + HCI 理论）

### 7.1 抽象层级评估

**行业基准**（各主流设计系统的 Part I 等价结构）：

| 设计系统 | 顶层分类 | 层数 | 定位 |
|---------|---------|:---:|------|
| **Material Design 3** | Foundations: Accessibility · Color · Content · Interaction · Layout · Typography · Shape · Motion | 3 | 设计师手册 |
| **Apple HIG** | Foundations: Accessibility · App Icons · Branding · Color · Dark Mode · Icons · Images · Layout · Materials · Motion · SF Symbols · Typography | 3 | 设计师+开发者 |
| **Carbon (IBM)** | Foundations: Color · Grid · Icons · Motion · Spacing · Themes · Typography | 3 | 设计师手册 |
| **Ant Design v5** | Design Values · Global Styles (Color · Font · Icon · Motion) · Patterns | 3 | 设计师手册 |
| **Matcha (doc_plan)** | I.1 Foundations · I.2 Tokens · I.3 Typography · I.4 Style · I.5 Widgets · I.6 Layout · I.7 Interaction · I.8 Motion | 4 | 设计师完整表达 |

**结论**：Matcha 的 7 柱 + Foundations 结构在行业中偏向 **最完整的一端**。Material Design 3 有 8 个 Foundations 子类，但不含 Widgets（Components 是独立的顶层类别）。Matcha 将 Widgets 纳入 Part I 是一个**正确的差异化决策**——因为目标是"唯一信息源重新实现"，而非"指导已有组件库的使用"。

**4 层目录深度**在行业中罕见（多数为 3 层），但考虑到 Matcha 的 46 个 widget 各有 12 节子规格，4 层是必要的结构决策。

### 7.2 哲学思考检查

对照 HCI 经典理论，检查 doc_plan 的 7 柱是否覆盖了设计系统的哲学基础：

| HCI 理论 | 核心概念 | doc_plan 覆盖 | 评估 |
|---------|---------|--------------|------|
| **Norman 的 7 个行动阶段** | Gulf of Execution / Gulf of Evaluation | I.7 Interaction 中的 FSM + feedback 模式 | ✅ FSM 定义了 execution 路径；但 **evaluation feedback（操作结果的视觉/听觉反馈）** 未被独立定义 |
| **Nielsen 10 启发式** | Visibility of system status / Match between system and real world / User control / Consistency / Error prevention / Recognition / Flexibility / Aesthetic / Help recover / Help & documentation | 分散在各柱中 | 🟡 缺少 **系统状态可见性**（§6.4 Loading/Empty/Error 部分覆盖）和 **帮助与文档**（完全未涉及 contextual help / onboarding） |
| **Gestalt 原则** | Proximity · Similarity · Continuity · Closure · Figure-ground · Common region | I.4 Style 的 variant pattern 隐含了 similarity/proximity | 🟡 未显式定义 **视觉分组规则**（什么时候用 border vs spacing vs background 区分组） |
| **Fitts' Law** | 目标距离/大小与交互时间关系 | I.2 Tokens 的 SizeToken + I.5 Widgets 的 hit-test geometry | ✅ 通过 minHeight=32px 和 touch target 要求间接覆盖 |
| **Hick's Law** | 选项数量与决策时间关系 | 未显式提及 | 🔴 缺少关于 **menu item 上限、option 数量阈值**（如 ComboBox 超过 N 项应切换到搜索模式）的设计指导 |
| **Miller's Law** | 工作记忆 7±2 限制 | §1.2 中提到了 7 柱认知分组 | 🟡 未在 widget 层面应用（如 tab 最大可见数、breadcrumb 深度限制） |
| **Affordance 理论** | 物理/感知/认知 affordance | I.5 Widgets 的 cursor 映射 + interaction state visual | 🟡 缺少关于 **signifier 设计规则**（什么视觉线索暗示可点击/可拖拽/可编辑） |

### 7.3 review.md Gap 覆盖验证

逐条检查 review.md 的 21 个 Gap 在 doc_plan 中的覆盖：

| Gap | 主题 | doc_plan 覆盖位置 | 状态 |
|-----|------|------------------|:---:|
| G1 | 布局算法 | I.6 §6.1 | ✅ |
| G2 | 滚动物理 | I.7 §7.3 | ✅ |
| G3 | 表单验证 | I.7 §7.2 | ✅ |
| G4 | 弹出层定位 | I.7 §7.4 | ✅ |
| G5 | Selection Model | I.7 §7.1 | ✅ |
| G6 | Loading/Empty/Error | I.6 §6.4 | ✅ |
| G7 | Keyboard Shortcut | I.7 §7.9 (a11y) | ✅ |
| G8 | Text Overflow | I.7 §7.5 | ✅ |
| G9 | Dialog 生命周期 | I.5 Dialog widget FSM 🆕 | ✅ |
| G10 | 泛型 DnD | I.7 §7.8 | ✅ |
| G11 | Toast 堆叠 | I.7 §7.7 | ✅ |
| G12 | Hover/Debounce | I.8 §8.4 | ✅ |
| G13 | High-DPI | I.7 §7.9 (a11y) | ✅ |
| G14 | 主题切换过渡 | Part II §11.14 | ✅ |
| G15 | Context Menu 组合 | I.7 §7.6 | ✅ |
| G16 | Undo/Redo | Appendix B | ✅ |
| G17 | 窗口管理 | Part VIII §29.6 | ✅ |
| G18 | Error 边界 | I.6 §6.4 | ✅ |
| G19 | CollapsibleSection 嵌套 | I.5 widget 深化 | ✅ |
| G20 | DataTable 高级交互 | I.5 widget 深化 | ✅ |
| G21 | PropertyGrid 编辑器 | I.5 widget 深化 | ✅ |

**21/21 全覆盖** ✅

### 7.4 发现的新缺失（doc_plan 未覆盖但 HCI 理论要求）

| # | 缺失主题 | 理论来源 | 应放置位置 | 严重度 |
|---|---------|---------|-----------|:-----:|
| **N1** | **Feedback & System Status** | Nielsen #1: 系统应始终让用户知道正在发生什么 | I.7 Interaction 新增 §7.11 | 🔴 |
| | 内容：操作反馈分级（即时视觉 → toast → dialog）、进行中状态指示（spinner/progress 选择规则）、成功/失败反馈时机（操作完成后 ≤100ms 给出视觉确认） | | | |
| **N2** | **Visual Grouping Rules（视觉分组）** | Gestalt proximity/similarity/common region | I.4 Style 新增 §4.4 | 🟡 |
| | 内容：何时用 border vs spacing vs background 区分组；分组间距阶梯（组内 4-8px，组间 16-24px，区域间 32px+）；嵌套分组规则 | | | |
| **N3** | **Signifier Design（可供性提示）** | Norman affordance + signifier | I.7 Interaction 新增 §7.12 | 🟡 |
| | 内容：可点击元素的视觉提示（cursor change + hover state + underline for links）；可拖拽元素的提示（drag handle icon / grab cursor）；可编辑元素的提示（pencil icon / border style change on hover） | | | |
| **N4** | **Cognitive Load Thresholds** | Hick's Law + Miller's Law | I.7 Interaction 新增 §7.13 | 🟡 |
| | 内容：ComboBox 选项 > 10 应启用搜索；Tab 可见数量上限（建议 ≤ 7，overflow → "more" 按钮）；Menu 项数上限（建议 ≤ 12 per level）；Breadcrumb 深度上限（≤ 5 级）；Dialog 内控件数上限（> 15 → 分页/分步 Wizard） | | | |
| **N5** | **Contextual Help / Onboarding** | Nielsen #10: Help & Documentation | I.6 Layout 新增 §6.5 或 I.7 新增 | 🟢 |
| | 内容：Tooltip vs Popover help 的选择规则；First-run tour 模式定义（步骤式 spotlight + dismissible）；上下文帮助链接样式（icon + text-link） | | | |
| **N6** | **Error Prevention（操作前）** | Nielsen #5: Error Prevention | I.7 §7.2 Form Validation 扩展 | 🟡 |
| | 内容：危险操作确认模式（destructive action → Confirm dialog with explicit typing）；不可逆操作的 2-step 确认；input constraint 预防（disable invalid submit button） | | | |
| **N7** | **Content / Writing Guidelines** | NN/g Design Systems 101: Style Guide 应含 tone of voice | I.1 Foundations 新增 §1.7 | 🟢 |
| | 内容：UI 文案风格指南（句式、大小写、标点、术语一致性）；Error message 模板（"[What happened]. [What to do]."）；Button label 规则（动词开头，如 "Save" not "OK"）；Placeholder text 规则 | | | |

### 7.5 结构调整建议

基于上述分析，对 doc_plan 的 7 柱结构提出以下修正：

| 修正 | 内容 | 影响 |
|------|------|------|
| **I.1 +§1.7 Content & Writing Guidelines 🆕** | UI 文案规范、error message 模板、button label 规则 | ~80 行 |
| **I.4 +§4.4 Visual Grouping Rules 🆕** | Gestalt-based 分组间距阶梯、border vs spacing vs bg 决策树 | ~60 行 |
| **I.7 +§7.11 Feedback & System Status 🆕** | 操作反馈分级、进行中状态、成功/失败反馈时机 | ~120 行 |
| **I.7 +§7.12 Signifier Design 🆕** | 可点击/可拖拽/可编辑的视觉提示规则 | ~80 行 |
| **I.7 +§7.13 Cognitive Load Thresholds 🆕** | 各控件的选项/项数/深度上限推荐值 | ~60 行 |
| **I.7 §7.2 扩展 Error Prevention** | 危险操作确认模式、不可逆 2-step、input constraint | ~40 行 |

**新增总量**：~440 行，更新后总量 ~6440 行，最终 spec ~15840 行。

### 7.6 与 review.md 七层模型的最终对照

补充 N1-N7 后的覆盖度：

| Layer | 目标 ★★★ | doc_plan 达到 | 补充后 |
|-------|:--------:|:----------:|:------:|
| L1 Primitives | ★★★ | ★★★ | ★★★ |
| L2 Tokens | ★★★ | ★★★ | ★★★ |
| L3 Patterns | ★★★ | ★★★ | ★★★ |
| L4 Components | ★★★ | ★★★ | ★★★ |
| L5 Compositions | ★★★ | ★★★ | ★★★ |
| L6 Interaction Contracts | ★★★ | ★★☆ | ★★★ (N1/N3/N4 补足) |
| L7 Motion Choreography | ★★★ | ★★★ | ★★★ |

### 7.7 总结

doc_plan 的 7 柱结构**总体合适**，与 Material Design 3 / Apple HIG 的 Foundations 分类高度对齐，4 层目录解决了 46 widget 的扁平化问题。

**主要不足**是 HCI 理论中三个重要维度未显式覆盖：

1. **Feedback 分级**（Nielsen #1）—— 操作结果反馈的完整设计规则
2. **Signifier 规范**（Norman affordance）—— 可供性的视觉提示体系
3. **认知负荷阈值**（Hick/Miller）—— 控件选项数、层级深度的上限指导

这三个补充（+N1-N7，~440 行）使 doc_plan 从"覆盖已有实现的所有行为"提升到"覆盖 HCI 理论要求的所有设计决策"，达到 **UI 开发人员仅凭 spec 文档从零实现 UI** 的完备性标准。
