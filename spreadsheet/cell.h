#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <memory>
#include <unordered_set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
        Cell(Sheet& sheet);
        ~Cell();

        void Set(std::string text);
        void Clear();

        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

        bool IsReferenced() const;

private:
        bool HasCircularDependency(const CellInterface* cell, const std::vector<Position>& referenced_cells);
        Cell* GetOrCreateCell(Position pos);

        class Impl;
        class EmptyImpl;
        class TextImpl;
        class FormulaImpl;

        std::unique_ptr<Impl> CreateImpl(std::string text) const;

        void InvalidateCache() const;

        std::unique_ptr<Impl> impl_;

        Sheet& sheet_;

        mutable std::optional<Value> cached_value_;
        std::unordered_set<Cell*> dependent_cells_;
};
