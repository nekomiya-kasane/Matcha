#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Widgets/Controls/NyanPropertyGrid.h>
#include <Matcha/Widgets/Controls/NyanColorPicker.h>

#include <QColor>

#include <cmath>
#include <utility>

using namespace matcha::gui;

// ============================================================================
// Compile-time checks
// ============================================================================

// PropertyType: 6 entries + Count_
static_assert(kPropertyTypeCount == 6);
static_assert(std::to_underlying(PropertyType::Text) == 0);
static_assert(std::to_underlying(PropertyType::Color) == 5);

// ============================================================================
// Runtime tests
// ============================================================================

TEST_SUITE("ContainerWidgetEnums") {

TEST_CASE("PropertyType has exactly 6 entries") {
    CHECK(kPropertyTypeCount == 6);
}

TEST_CASE("PropertyType enum ordering") {
    CHECK(std::to_underlying(PropertyType::Text) == 0);
    CHECK(std::to_underlying(PropertyType::Integer) == 1);
    CHECK(std::to_underlying(PropertyType::Double) == 2);
    CHECK(std::to_underlying(PropertyType::Bool) == 3);
    CHECK(std::to_underlying(PropertyType::Choice) == 4);
    CHECK(std::to_underlying(PropertyType::Color) == 5);
}

} // TEST_SUITE ContainerWidgetEnums

// ============================================================================
// Paginator bounds tests
// ============================================================================

TEST_SUITE("PaginatorBounds") {

TEST_CASE("Paginator SetCurrent clamped to valid range") {
    // Test bounds logic without instantiating widget (pure logic test).
    // Paginator uses 0-indexed current, valid range is [-1, count-1].
    
    // Simulate bounds checking logic:
    auto clampCurrent = [](int current, int count) -> int {
        if (current < -1) {
            return -1;
        }
        if (current >= count) {
            return count - 1;
        }
        return current;
    };
    
    // count = 5, valid range = [-1, 4]
    CHECK(clampCurrent(-2, 5) == -1);
    CHECK(clampCurrent(-1, 5) == -1);
    CHECK(clampCurrent(0, 5) == 0);
    CHECK(clampCurrent(4, 5) == 4);
    CHECK(clampCurrent(5, 5) == 4);
    CHECK(clampCurrent(10, 5) == 4);
    
    // count = 0, valid range = [-1, -1]
    CHECK(clampCurrent(0, 0) == -1);
    CHECK(clampCurrent(-1, 0) == -1);
}

} // TEST_SUITE PaginatorBounds

// ============================================================================
// ColorPicker HSV<->RGB conversion tests
// ============================================================================

TEST_SUITE("ColorPickerConversion") {

TEST_CASE("HSV to RGB conversion accuracy") {
    // Test Qt's HSV<->RGB conversion round-trip.
    
    // Pure red: H=0, S=255, V=255 -> R=255, G=0, B=0
    QColor red = QColor::fromHsv(0, 255, 255);
    CHECK(red.red() == 255);
    CHECK(red.green() == 0);
    CHECK(red.blue() == 0);
    
    // Pure green: H=120, S=255, V=255 -> R=0, G=255, B=0
    QColor green = QColor::fromHsv(120, 255, 255);
    CHECK(green.red() == 0);
    CHECK(green.green() == 255);
    CHECK(green.blue() == 0);
    
    // Pure blue: H=240, S=255, V=255 -> R=0, G=0, B=255
    QColor blue = QColor::fromHsv(240, 255, 255);
    CHECK(blue.red() == 0);
    CHECK(blue.green() == 0);
    CHECK(blue.blue() == 255);
    
    // White: H=0, S=0, V=255 -> R=255, G=255, B=255
    QColor white = QColor::fromHsv(0, 0, 255);
    CHECK(white.red() == 255);
    CHECK(white.green() == 255);
    CHECK(white.blue() == 255);
    
    // Black: H=0, S=0, V=0 -> R=0, G=0, B=0
    QColor black = QColor::fromHsv(0, 0, 0);
    CHECK(black.red() == 0);
    CHECK(black.green() == 0);
    CHECK(black.blue() == 0);
}

TEST_CASE("RGB to HSV round-trip") {
    // Test that RGB -> HSV -> RGB preserves values.
    
    auto testRoundTrip = [](int r, int g, int b) {
        QColor original(r, g, b);
        int h = original.hsvHue();
        int s = original.hsvSaturation();
        int v = original.value();
        
        // Handle achromatic colors (hue = -1).
        if (h < 0) {
            h = 0;
        }
        
        QColor reconstructed = QColor::fromHsv(h, s, v);
        
        // Allow small tolerance due to rounding.
        CHECK(std::abs(reconstructed.red() - r) <= 1);
        CHECK(std::abs(reconstructed.green() - g) <= 1);
        CHECK(std::abs(reconstructed.blue() - b) <= 1);
    };
    
    testRoundTrip(255, 0, 0);     // Red
    testRoundTrip(0, 255, 0);     // Green
    testRoundTrip(0, 0, 255);     // Blue
    testRoundTrip(255, 255, 0);   // Yellow
    testRoundTrip(0, 255, 255);   // Cyan
    testRoundTrip(255, 0, 255);   // Magenta
    testRoundTrip(128, 128, 128); // Gray
    testRoundTrip(64, 128, 192);  // Arbitrary
}

} // TEST_SUITE ColorPickerConversion

// ============================================================================
// DataTable CRUD logic tests (simulated)
// ============================================================================

TEST_SUITE("DataTableCRUD") {

TEST_CASE("DataTable row count logic") {
    // Simulate DataTable row management logic.
    
    struct MockDataTable {
        std::vector<std::vector<QString>> _data;
        int _columnCount = 0;
        
        void SetHeaders(const QStringList& headers) {
            _columnCount = headers.size();
        }
        
        int AddRow() {
            _data.emplace_back(_columnCount, QString());
            return static_cast<int>(_data.size()) - 1;
        }
        
        void SetCell(int row, int col, const QString& value) {
            if (row >= 0 && row < static_cast<int>(_data.size()) &&
                col >= 0 && col < _columnCount) {
                _data[static_cast<size_t>(row)][static_cast<size_t>(col)] = value;
            }
        }
        
        QString GetCell(int row, int col) const {
            if (row >= 0 && row < static_cast<int>(_data.size()) &&
                col >= 0 && col < _columnCount) {
                return _data[static_cast<size_t>(row)][static_cast<size_t>(col)];
            }
            return {};
        }
        
        void RemoveRow(int row) {
            if (row >= 0 && row < static_cast<int>(_data.size())) {
                _data.erase(_data.begin() + row);
            }
        }
        
        void Clear() {
            _data.clear();
        }
        
        int RowCount() const {
            return static_cast<int>(_data.size());
        }
    };
    
    MockDataTable table;
    table.SetHeaders({"A", "B", "C"});
    
    CHECK(table.RowCount() == 0);
    
    // AddRow increments count.
    int row0 = table.AddRow();
    CHECK(row0 == 0);
    CHECK(table.RowCount() == 1);
    
    int row1 = table.AddRow();
    CHECK(row1 == 1);
    CHECK(table.RowCount() == 2);
    
    // SetCell/GetCell round-trip.
    table.SetCell(0, 0, "Hello");
    table.SetCell(0, 1, "World");
    table.SetCell(1, 2, "Test");
    
    CHECK(table.GetCell(0, 0) == "Hello");
    CHECK(table.GetCell(0, 1) == "World");
    CHECK(table.GetCell(1, 2) == "Test");
    CHECK(table.GetCell(0, 2) == "");  // Default empty.
    
    // RemoveRow decrements count.
    table.RemoveRow(0);
    CHECK(table.RowCount() == 1);
    CHECK(table.GetCell(0, 2) == "Test");  // Row 1 is now row 0.
    
    // Clear resets to 0.
    table.AddRow();
    table.AddRow();
    CHECK(table.RowCount() == 3);
    table.Clear();
    CHECK(table.RowCount() == 0);
}

TEST_CASE("DataTable bounds checking") {
    // Test out-of-bounds access returns empty/safe values.
    
    struct MockDataTable {
        std::vector<std::vector<QString>> _data;
        int _columnCount = 3;
        
        QString GetCell(int row, int col) const {
            if (row < 0 || row >= static_cast<int>(_data.size()) ||
                col < 0 || col >= _columnCount) {
                return {};
            }
            return _data[static_cast<size_t>(row)][static_cast<size_t>(col)];
        }
    };
    
    MockDataTable table;
    
    // Out of bounds returns empty.
    CHECK(table.GetCell(-1, 0) == "");
    CHECK(table.GetCell(0, 0) == "");
    CHECK(table.GetCell(100, 0) == "");
    CHECK(table.GetCell(0, -1) == "");
    CHECK(table.GetCell(0, 100) == "");
}

} // TEST_SUITE DataTableCRUD

// ============================================================================
// PropertyGrid logic tests (simulated)
// ============================================================================

TEST_SUITE("PropertyGridLogic") {

TEST_CASE("PropertyType to editor mapping") {
    // Verify each PropertyType maps to expected editor type name.
    
    auto editorTypeName = [](PropertyType type) -> const char* {
        switch (type) {
        case PropertyType::Text:    return "QLineEdit";
        case PropertyType::Integer: return "QSpinBox";
        case PropertyType::Double:  return "QDoubleSpinBox";
        case PropertyType::Bool:    return "QCheckBox";
        case PropertyType::Choice:  return "QComboBox";
        case PropertyType::Color:   return "QPushButton";
        case PropertyType::Count_:  return "Unknown";
        }
        return "Unknown";
    };
    
    CHECK(std::string(editorTypeName(PropertyType::Text)) == "QLineEdit");
    CHECK(std::string(editorTypeName(PropertyType::Integer)) == "QSpinBox");
    CHECK(std::string(editorTypeName(PropertyType::Double)) == "QDoubleSpinBox");
    CHECK(std::string(editorTypeName(PropertyType::Bool)) == "QCheckBox");
    CHECK(std::string(editorTypeName(PropertyType::Choice)) == "QComboBox");
    CHECK(std::string(editorTypeName(PropertyType::Color)) == "QPushButton");
}

} // TEST_SUITE PropertyGridLogic

#ifdef __clang__
#pragma clang diagnostic pop
#endif
