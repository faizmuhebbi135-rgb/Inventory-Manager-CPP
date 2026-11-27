#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Product {
    int id;
    string name;
    int quantity;
    double price;
};

const string DATA_FILE = "inventory.txt";

string readLine(const string& prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return value;
}

string readRequiredText(const string& prompt) {
    while (true) {
        string value = readLine(prompt);

        if (value.find_first_not_of(" \t") != string::npos) {
            // Keep the file format safe: the vertical bar separates fields.
            replace(value.begin(), value.end(), '|', '/');
            return value;
        }

        cout << "This field cannot be empty. Try again.\n";
    }
}

int readInt(const string& prompt, int minimum) {
    while (true) {
        string input = readLine(prompt);
        istringstream stream(input);
        int value;
        char extra;

        if ((stream >> value) && !(stream >> extra) && value >= minimum) {
            return value;
        }

        cout << "Please enter a whole number greater than or equal to "
             << minimum << ".\n";
    }
}

double readDouble(const string& prompt, double minimum) {
    while (true) {
        string input = readLine(prompt);
        istringstream stream(input);
        double value;
        char extra;

        if ((stream >> value) && !(stream >> extra) && value >= minimum) {
            return value;
        }

        cout << "Please enter a valid number greater than or equal to "
             << minimum << ".\n";
    }
}

string toLower(string value) {
    transform(value.begin(), value.end(), value.begin(),
              [](unsigned char ch) {
                  return static_cast<char>(tolower(ch));
              });
    return value;
}

vector<Product> loadInventory() {
    vector<Product> products;
    ifstream file(DATA_FILE);
    string line;

    while (getline(file, line)) {
        istringstream row(line);
        string idText, name, quantityText, priceText;

        if (!getline(row, idText, '|') ||
            !getline(row, name, '|') ||
            !getline(row, quantityText, '|') ||
            !getline(row, priceText)) {
            continue;
        }

        try {
            Product product;
            product.id = stoi(idText);
            product.name = name;
            product.quantity = stoi(quantityText);
            product.price = stod(priceText);

            if (product.id > 0 && product.quantity >= 0 && product.price >= 0) {
                products.push_back(product);
            }
        } catch (const exception&) {
            // Ignore malformed records instead of stopping the whole program.
        }
    }

    return products;
}

bool saveInventory(const vector<Product>& products) {
    ofstream file(DATA_FILE, ios::trunc);

    if (!file) {
        return false;
    }

    file << fixed << setprecision(2);

    for (const Product& product : products) {
        file << product.id << '|'
             << product.name << '|'
             << product.quantity << '|'
             << product.price << '\n';
    }

    return file.good();
}

int getNextId(const vector<Product>& products) {
    int nextId = 1;

    for (const Product& product : products) {
        nextId = max(nextId, product.id + 1);
    }

    return nextId;
}

void printProducts(const vector<Product>& products) {
    if (products.empty()) {
        cout << "Inventory is empty. Add your first product.\n";
        return;
    }

    cout << '\n'
         << left << setw(7) << "ID"
         << setw(25) << "PRODUCT"
         << right << setw(10) << "STOCK"
         << setw(14) << "PRICE" << '\n';
    cout << string(56, '-') << '\n';

    double totalValue = 0.0;

    for (const Product& product : products) {
        cout << left << setw(7) << product.id
             << setw(25) << product.name.substr(0, 23)
             << right << setw(10) << product.quantity
             << setw(14) << fixed << setprecision(2) << product.price
             << '\n';

        totalValue += product.quantity * product.price;
    }

    cout << string(56, '-') << '\n';
    cout << "Products: " << products.size()
         << " | Total stock value: " << fixed << setprecision(2)
         << totalValue << "\n";
}

Product* findProductById(vector<Product>& products, int id) {
    for (Product& product : products) {
        if (product.id == id) {
            return &product;
        }
    }
    return nullptr;
}

void addProduct(vector<Product>& products) {
    cout << "\n--- ADD PRODUCT ---\n";

    Product product;
    product.id = getNextId(products);
    product.name = readRequiredText("Product name: ");
    product.quantity = readInt("Starting quantity: ", 0);
    product.price = readDouble("Unit price: ", 0.0);

    products.push_back(product);

    if (saveInventory(products)) {
        cout << "Product added. Assigned ID: " << product.id << "\n";
    } else {
        products.pop_back();
        cout << "Error: Could not save inventory.\n";
    }
}

void searchProducts(const vector<Product>& products) {
    cout << "\n--- SEARCH PRODUCTS ---\n";
    string query = toLower(readRequiredText("Enter a product name: "));
    bool found = false;

    for (const Product& product : products) {
        if (toLower(product.name).find(toLower(query)) != string::npos) {
            vector<Product> oneProduct{product};
            printProducts(oneProduct);
            found = true;
        }
    }

    if (!found) {
        cout << "No matching product found.\n";
    }
}

void updateStock(vector<Product>& products) {
    cout << "\n--- UPDATE STOCK ---\n";

    if (products.empty()) {
        cout << "Inventory is empty.\n";
        return;
    }

    printProducts(products);
    int id = readInt("Enter product ID: ", 1);
    Product* product = findProductById(products, id);

    if (product == nullptr) {
        cout << "Product ID not found.\n";
        return;
    }

    cout << "Selected: " << product->name
         << " | Current stock: " << product->quantity << '\n';

    int newQuantity = readInt("Enter new stock quantity: ", 0);
    int oldQuantity = product->quantity;
    product->quantity = newQuantity;

    if (saveInventory(products)) {
        cout << "Stock updated successfully.\n";
    } else {
        product->quantity = oldQuantity;
        cout << "Error: Could not save inventory.\n";
    }
}

void deleteProduct(vector<Product>& products) {
    cout << "\n--- DELETE PRODUCT ---\n";

    if (products.empty()) {
        cout << "Inventory is empty.\n";
        return;
    }

    printProducts(products);
    int id = readInt("Enter product ID to delete: ", 1);

    auto found = find_if(products.begin(), products.end(),
                         [id](const Product& product) {
                             return product.id == id;
                         });

    if (found == products.end()) {
        cout << "Product ID not found.\n";
        return;
    }

    cout << "Delete '" << found->name << "'? Type YES to confirm: ";
    string confirmation;
    getline(cin, confirmation);

    if (confirmation != "YES") {
        cout << "Deletion cancelled.\n";
        return;
    }

    Product removed = *found;
    size_t position = static_cast<size_t>(found - products.begin());
    products.erase(found);

    if (saveInventory(products)) {
        cout << "Product deleted successfully.\n";
    } else {
        products.insert(products.begin() + static_cast<long>(position), removed);
        cout << "Error: Could not save inventory.\n";
    }
}

void showLowStock(const vector<Product>& products) {
    cout << "\n--- LOW STOCK REPORT ---\n";
    int threshold = readInt("Show products with stock at or below: ", 0);
    vector<Product> lowStock;

    for (const Product& product : products) {
        if (product.quantity <= threshold) {
            lowStock.push_back(product);
        }
    }

    if (lowStock.empty()) {
        cout << "No products are at or below this stock level.\n";
    } else {
        printProducts(lowStock);
    }
}

void showMenu() {
    cout << "\n========== INVENTORY MANAGER ==========\n"
         << "1. Add product\n"
         << "2. View inventory\n"
         << "3. Search product\n"
         << "4. Update stock\n"
         << "5. Delete product\n"
         << "6. Low stock report\n"
         << "0. Save and exit\n"
         << "=======================================\n";
}

int main() {
    vector<Product> products = loadInventory();
    cout << "Welcome to DariTech Inventory Manager!\n";
    cout << products.size() << " product(s) loaded from " << DATA_FILE << ".\n";

    while (true) {
        showMenu();
        string choice = readLine("Choose an option: ");

        if (choice == "1") {
            addProduct(products);
        } else if (choice == "2") {
            cout << "\n--- CURRENT INVENTORY ---\n";
            printProducts(products);
        } else if (choice == "3") {
            searchProducts(products);
        } else if (choice == "4") {
            updateStock(products);
        } else if (choice == "5") {
            deleteProduct(products);
        } else if (choice == "6") {
            showLowStock(products);
        } else if (choice == "0") {
            cout << "Goodbye! Your inventory is saved in "
                 << DATA_FILE << ".\n";
            break;
        } else {
            cout << "Invalid option. Please choose 0-6.\n";
        }
    }

    return 0;
}
