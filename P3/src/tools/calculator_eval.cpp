#include "tools/calculator.h"
#include <stack>
#include <vector>
#include <string>

namespace {

int get_precedence_rank(CalcToken::Kind op_kind) {
    if (op_kind == CalcToken::Kind::Times || op_kind == CalcToken::Kind::Divide) {
        return 2;
    }
    if (op_kind == CalcToken::Kind::Plus || op_kind == CalcToken::Kind::Minus) {
        return 1;
    }
    return 0;
}

bool evaluate_single_operation(std::stack<double>& value_stack, CalcToken::Kind op_kind, std::string* error_msg) {
    if (value_stack.size() < 2) {
        *error_msg = "missing operand";
        return false;
    }

    double right_val = value_stack.top();
    value_stack.pop();
    
    double left_val = value_stack.top();
    value_stack.pop();
    
    double computed_val = 0.0;

    switch (op_kind) {
        case CalcToken::Kind::Plus:
            computed_val = left_val + right_val;
            break;
        case CalcToken::Kind::Minus:
            computed_val = left_val - right_val;
            break;
        case CalcToken::Kind::Times:
            computed_val = left_val * right_val;
            break;
        case CalcToken::Kind::Divide:
            if (right_val == 0.0) {
                *error_msg = "division by zero";
                return false;
            }
            computed_val = left_val / right_val;
            break;
        default:
            *error_msg = "unknown operator";
            return false;
    }

    value_stack.push(computed_val);
    return true;
}

}

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* result, std::string* error) {
    if (tokens.empty()) {
        *error = "empty expression";
        return false;
    }

    std::stack<double> value_stack;
    std::stack<CalcToken::Kind> operator_stack;

    for (const auto& token : tokens) {
        if (token.kind == CalcToken::Kind::Number) {
            value_stack.push(token.value);
        } 
        else if (token.kind == CalcToken::Kind::LParen) {
            operator_stack.push(token.kind);
        } 
        else if (token.kind == CalcToken::Kind::RParen) {
            while (!operator_stack.empty() && operator_stack.top() != CalcToken::Kind::LParen) {
                CalcToken::Kind pending_op = operator_stack.top();
                operator_stack.pop();
                
                if (!evaluate_single_operation(value_stack, pending_op, error)) {
                    return false;
                }
            }

            if (operator_stack.empty()) {
                *error = "mismatched parentheses";
                return false;
            }
            operator_stack.pop();
        } 
        else {
            int current_rank = get_precedence_rank(token.kind);
            if (current_rank == 0) {
                *error = "unknown operator";
                return false;
            }

            while (!operator_stack.empty() &&
                   operator_stack.top() != CalcToken::Kind::LParen &&
                   get_precedence_rank(operator_stack.top()) >= current_rank) {
                
                CalcToken::Kind stack_op = operator_stack.top();
                operator_stack.pop();
                
                if (!evaluate_single_operation(value_stack, stack_op, error)) {
                    return false;
                }
            }
            operator_stack.push(token.kind);
        }
    }

    while (!operator_stack.empty()) {
        CalcToken::Kind remaining_op = operator_stack.top();
        operator_stack.pop();

        if (remaining_op == CalcToken::Kind::LParen) {
            *error = "mismatched parentheses";
            return false;
        }
        
        if (!evaluate_single_operation(value_stack, remaining_op, error)) {
            return false;
        }
    }

    if (value_stack.size() != 1) {
        *error = "invalid expression";
        return false;
    }

    *result = value_stack.top();
    error->clear();
    return true;
}