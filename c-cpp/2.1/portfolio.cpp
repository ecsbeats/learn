#include <iostream>
#include <string>
#include <vector>
#include <memory> // Required for std::unique_ptr, std::make_unique
#include <utility> // Required for std::move

// 1. The 'Stock' Class
// A simple struct with a constructor and a helpful destructor to show when it's cleaned up.
struct Stock {
    std::string ticker;
    double price;

    Stock(std::string t, double p) : ticker(std::move(t)), price(p) {
        std::cout << "  [+] Stock " << ticker << " created.\n";
    }

    ~Stock() {
        std::cout << "  [-] Stock " << ticker << " destroyed.\n";
    }
};

// ---

// 2. The 'Portfolio' Class
// This class demonstrates the "Rule of Zero". It manages a complex resource
// (a collection of heap-allocated objects) without needing a single line of
// manual memory management code (no new, delete, or explicit destructor).
class Portfolio {
private:
    // The portfolio uniquely owns its stocks. std::vector<std::unique_ptr<Stock>>
    // is the perfect tool for this. It expresses "a collection of objects that
    // I, and only I, own."
    std::vector<std::unique_ptr<Stock>> stocks_;

public:
    // No need to write a destructor, copy constructor, or move constructor.
    // The compiler-generated defaults will correctly handle the `stocks_` vector.
    // When the Portfolio is destroyed, the vector's destructor will be called,
    // which in turn calls the destructor for every unique_ptr, which then
    // deletes the Stock object it owns.

    // 3. Functionality: `add_stock`
    // We pass the unique_ptr by value. This forces the caller to explicitly
    // give up ownership via std::move.
    void add_stock(std::unique_ptr<Stock> stock) {
        // We then move the stock from the function parameter into our vector.
        stocks_.push_back(std::move(stock));
    }

    // 3. Functionality: `display_portfolio`
    void display_portfolio() const {
        std::cout << "\n--- Portfolio Holdings ---\n";
        if (stocks_.empty()) {
            std::cout << "Portfolio is empty.\n";
        } else {
            for (const auto& stock_ptr : stocks_) {
                std::cout << "  Ticker: " << stock_ptr->ticker
                          << ", Price: $" << stock_ptr->price << "\n";
            }
        }
        std::cout << "--------------------------\n";
    }
};

// ---

// 4. `main()` to demonstrate usage
int main() {
    std::cout << "Entering main scope...\n";
    
    // We use a local scope to clearly see when the Portfolio is destroyed.
    {
        // Create the Portfolio object.
        Portfolio my_portfolio;

        // Create several Stock objects on the heap using std::make_unique.
        // This is the modern, exception-safe way to create heap objects.
        auto stock1 = std::make_unique<Stock>("AAPL", 172.25);
        auto stock2 = std::make_unique<Stock>("GOOG", 135.50);
        auto stock3 = std::make_unique<Stock>("MSFT", 330.10);

        // Add stocks to the portfolio. We must use std::move to transfer
        // ownership from the local unique_ptr variables to the portfolio.
        // After this, stock1, stock2, and stock3 will be nullptr.
        my_portfolio.add_stock(std::move(stock1));
        my_portfolio.add_stock(std::move(stock2));
        my_portfolio.add_stock(std::move(stock3));

        // Display the portfolio's contents.
        my_portfolio.display_portfolio();

        std::cout << "\nLeaving portfolio scope. Destructors should be called automatically...\n";
    } // `my_portfolio` goes out of scope here. RAII in action!

    std::cout << "Exited main scope. All resources have been cleaned up.\n";
    return 0;
}
