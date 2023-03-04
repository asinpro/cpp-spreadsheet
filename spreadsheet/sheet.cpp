#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>

using namespace std::literals;

namespace {

  std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
      std::visit(
          [&](const auto& x) {
              output << x;
          },
          value);
      return output;
  }

} // namespace

Sheet::Sheet() :
    table_(Position::MAX_ROWS) {
}

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid Position"s);
    }

    if (pos.col >= static_cast<int>(table_[pos.row].size())) {
        table_[pos.row].resize(pos.col + 1);
    }

    if (GetCell(pos) == nullptr) {
        table_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }

    dynamic_cast<Cell*>(GetCell(pos))->Set(text);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid Position"s);
    }

    if (pos.col < static_cast<int>(table_[pos.row].size())) {
        return table_[pos.row][pos.col].get();
    }

    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid Position"s);
    }

    if (pos.col < static_cast<int>(table_[pos.row].size())) {
        table_[pos.row][pos.col] = nullptr;
    }
}

Size Sheet::GetPrintableSize() const {
    int rows = -1;
    int cols = -1;

    for (auto row_it = table_.begin(); row_it < table_.end(); ++row_it) {
        auto col_it = std::find_if(row_it->rbegin(), row_it->rend(), [](const auto& cell){
            return cell != nullptr;
        });

        if (col_it != row_it->rend()) {
            int distance = row_it->size() - (col_it - row_it->rbegin());

            cols = std::max(cols, distance - 1);
            rows = row_it - table_.begin();
        }
    }

    return {rows + 1, cols + 1};
}

void Sheet::PrintValues(std::ostream& output) const {
    PrintCells(output, [](const Cell& cell){
        return cell.GetValue();
    });
}

void Sheet::PrintTexts(std::ostream& output) const {
    PrintCells(output, [](const Cell& cell){
        return cell.GetText();
    });
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
