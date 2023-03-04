#pragma once

#include "common.h"
#include "formula.h"
#include <memory>
#include <string>
#include <optional>

class Impl {
public:
    virtual ~Impl() = default;

    virtual std::string GetText() const = 0;
    virtual Cell::Value GetValue() const = 0;

    virtual std::vector<Position> GetReferencedCells() const {
        return {};
    }
};

class EmptyImpl : public Impl {
public:
    std::string GetText() const override;
    Cell::Value GetValue() const override;
};

class TextImpl : public Impl {
    public:
        TextImpl(std::string text) :
            text_(std::move(text)) {
        }

        std::string GetText() const override;
        Cell::Value GetValue() const override;

    private:
        std::string text_;
};

class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string text) :
            formula_(ParseFormula(text)) {
        }

        std::string GetText() const override;
        Cell::Value GetValue() const override;

        std::vector<Position> GetReferencedCells() const override {
            return formula_->GetReferencedCells();
        }

    private:
        std::unique_ptr<FormulaInterface> formula_;
};

class Cell : public CellInterface {
public:
    Cell(SheetInterface* sheet) :
        sheet_(sheet) {
    }

    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

private:
    static bool HasCircularDependency(const Cell& self, const std::vector<Position>& Cells);

    void InvalidateCache() const;

    void AddReferenceTo(Position position) ;
    void RemoveReferenceTo(Position position) ;

    SheetInterface* sheet_;
    std::unique_ptr<class Impl> impl_;
    std::vector<Position> dependent_cells_;
    mutable std::optional<Value> cached_;
};
