#include "Matcha/UiNodes/Core/BaseObject.h"

namespace matcha {

// Root MetaClass -- parent is nullptr
const MetaClass BaseObject::s_metaClass{.name="BaseObject", .parent=nullptr};

BaseObject::~BaseObject() = default;

} // namespace matcha
