#pragma once

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <vector>

class Sheet : public SheetInterface {
public:
    using Row = std::vector<std::unique_ptr<Cell>>;
    using Table = std::vector<Row>;

    Sheet();
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    template<typename Function>
    void PrintCells(std::ostream& output, Function getCellValue) const;

    Table table_;
};

template<typename Function>
void Sheet::PrintCells(std::ostream& output, Function getCellValue) const {
    auto size = GetPrintableSize();

    for (int row = 0; row < size.rows; ++row) {
        bool is_first = true;

        for (int col = 0; col < size.cols; ++col) {
            if (!is_first) {
                output << '\t';
            }
            is_first = false;

            if (col < static_cast<int>(table_[row].size())) {
                if (const auto& cell = table_[row][col]) {
                    output << getCellValue(*cell);
                }
            }
        }
        output << '\n';
    }
}
