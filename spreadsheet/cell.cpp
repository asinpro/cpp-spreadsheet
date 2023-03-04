#include "cell.h"
#include "sheet.h"
#include "common.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <optional>
#include <variant>
#include <sstream>

class Cell::Impl {
public:
    virtual ~Impl() = default;

    virtual std::string GetText() const = 0;
    virtual Value GetValue() const = 0;

    virtual std::vector<Position> GetReferencedCells() const {
        static const std::vector<Position> empty;
        return empty;
    }
};

class Cell::EmptyImpl : public Impl {
public:
    std::string GetText() const {
        return "";
    }

    Value GetValue() const {
        return 0.0;
    };
};

class Cell::TextImpl : public Impl {
    public:
        TextImpl(std::string text) : text_(std::move(text)) {}

        std::string GetText() const {
            return text_;
        };

        Value GetValue() const {
            if (text_[0] == ESCAPE_SIGN) {
                return text_.substr(1);
            }

            std::istringstream istr(text_);
            double value;

            try{
                istr >> value;
                if (istr.eof()) {
                    return value;
                }
            } catch (...) {}

            return text_;
        }

    private:
        std::string text_;
};

class Cell::FormulaImpl : public Impl {
    public:
        FormulaImpl(Sheet& sheet, std::string text) :
            sheet_(sheet),
            formula_(ParseFormula(text)) {
        }

        std::string GetText() const override {
            return FORMULA_SIGN + formula_->GetExpression();
        };

        Value GetValue() const override {
            auto value = formula_->Evaluate(sheet_);
            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);
            }

            return std::get<FormulaError>(value);
        }

        std::vector<Position> GetReferencedCells() const override {
            return formula_->GetReferencedCells();
        }

    private:
        Sheet& sheet_;
        std::unique_ptr<FormulaInterface> formula_;
};

Cell::Cell(Sheet& sheet) : sheet_(sheet) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (impl_ && text == impl_->GetText()) {
        return;
    }

    auto new_impl = CreateImpl(text);

    if (HasCircularDependency(this, new_impl->GetReferencedCells())) {
        throw CircularDependencyException("Cyrcular dependency: " + text);
    }

    if (impl_) {
        for (const auto& pos : impl_->GetReferencedCells()) {
            auto* cell = GetOrCreateCell(pos);
            cell->dependent_cells_.erase(this);
        }
    }

    for (const auto& pos : new_impl->GetReferencedCells()) {
        auto* cell = GetOrCreateCell(pos);
        cell->dependent_cells_.insert(this);
    }

    impl_ = std::move(new_impl);

    InvalidateCache();
}

std::unique_ptr<Cell::Impl> Cell::CreateImpl(std::string text) const {
    if (text.empty()) {
        return std::make_unique<EmptyImpl>();
    } else if (text[0] == FORMULA_SIGN && text.size() > 1) {
        return std::make_unique<FormulaImpl>(sheet_, text.substr(1));
    } else {
        return std::make_unique<TextImpl>(std::move(text));
    }
}

bool Cell::HasCircularDependency(const CellInterface* target_cell, const std::vector<Position>& referenced_cells) {
    for (const auto& pos : referenced_cells) {
        auto* cell = GetOrCreateCell(pos);

        if (target_cell == cell
        || cell->HasCircularDependency(target_cell, cell->GetReferencedCells())) {
            return true;
        }
    }

    return false;
}

Cell* Cell::GetOrCreateCell(Position pos) {
    if (sheet_.GetCell(pos) == nullptr) {
        sheet_.SetCell(pos, "");
    }

    return dynamic_cast<Cell*>(sheet_.GetCell(pos));
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    if (!cached_value_) {
        cached_value_ = impl_->GetValue();
    }

    return *cached_value_;
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return false;
}

void Cell::InvalidateCache() const {
    cached_value_ = std::nullopt;

    for (const auto* cell : dependent_cells_) {
        if (cell != nullptr) {
            dynamic_cast<const Cell*>(cell)->InvalidateCache();
        }
    }
}
