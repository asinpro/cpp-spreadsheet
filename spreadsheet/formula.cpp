#include "formula.h"

#include "FormulaAST.h"
#include "common.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <sstream>
#include <variant>
#include <vector>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) :
        ast_(ParseFormulaAST(expression)),
        referenced_cells_(ast_.GetCells().begin(), ast_.GetCells().end()){
        referenced_cells_.erase(
            std::unique(referenced_cells_.begin(), referenced_cells_.end()),
            referenced_cells_.end()
        );
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute([&sheet](Position pos){
                const auto* cell = sheet.GetCell(pos);
                if (cell == nullptr) {
                    return 0.0;
                }

                auto value = cell->GetValue();
                if (std::holds_alternative<double>(value)) {
                    return std::get<double>(value);
                } else if (std::holds_alternative<FormulaError>(value)) {
                    throw std::get<FormulaError>(value);
                }

                throw FormulaError(FormulaError::Category::Value);

            });
        } catch(const FormulaError& e) {
            return e;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream os;
        ast_.PrintFormula(os);
        return os.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        return referenced_cells_;
    }

private:
    FormulaAST ast_;
    std::vector<Position> referenced_cells_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try{
        return std::make_unique<Formula>(std::move(expression));
    } catch (const FormulaException& e) {
        throw e;
    } catch (...) {
        throw FormulaException("Formula parsing error: " + expression);
    }
}
