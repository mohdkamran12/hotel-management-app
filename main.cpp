#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class Room {
private:
    int roomNumber;
    double nightlyRate;
    bool available;

protected:
    string roomType;

public:
    Room(int number, double rate, const string &type, bool isAvailable = true)
        : roomNumber(number), nightlyRate(rate), available(isAvailable), roomType(type) {}

    virtual ~Room() = default;

    int getRoomNumber() const { return roomNumber; }
    double getNightlyRate() const { return nightlyRate; }
    string getRoomType() const { return roomType; }
    bool isAvailable() const { return available; }

    void setAvailable(bool status) { available = status; }
};

class SingleRoom : public Room {
public:
    explicit SingleRoom(int number, bool isAvailable = true)
        : Room(number, 50.0, "Single", isAvailable) {}
};

class DoubleRoom : public Room {
public:
    explicit DoubleRoom(int number, bool isAvailable = true)
        : Room(number, 85.0, "Double", isAvailable) {}
};

class DeluxeRoom : public Room {
public:
    explicit DeluxeRoom(int number, bool isAvailable = true)
        : Room(number, 140.0, "Deluxe", isAvailable) {}
};

class Customer {
private:
    int bookingId;
    string name;
    string phone;
    int roomNumber;
    string roomType;
    int checkInDay;
    int checkOutDay;
    double totalFee;
    bool active;

public:
    Customer(int id, const string &customerName, const string &customerPhone, int roomNum,
             const string &type, int checkIn, int checkOut = -1, double fee = 0.0,
             bool isActive = true)
        : bookingId(id), name(customerName), phone(customerPhone), roomNumber(roomNum),
          roomType(type), checkInDay(checkIn), checkOutDay(checkOut), totalFee(fee),
          active(isActive) {}

    int getBookingId() const { return bookingId; }
    string getName() const { return name; }
    string getPhone() const { return phone; }
    int getRoomNumber() const { return roomNumber; }
    string getRoomType() const { return roomType; }
    int getCheckInDay() const { return checkInDay; }
    int getCheckOutDay() const { return checkOutDay; }
    double getTotalFee() const { return totalFee; }
    bool isActive() const { return active; }

    void closeBooking(int checkout, double fee) {
        checkOutDay = checkout;
        totalFee = fee;
        active = false;
    }

    string toStorageLine() const {
        ostringstream out;
        out << bookingId << "|" << name << "|" << phone << "|" << roomNumber << "|"
            << roomType << "|" << checkInDay << "|" << checkOutDay << "|" << fixed
            << setprecision(2) << totalFee << "|" << (active ? 1 : 0);
        return out.str();
    }

    static bool parseFromStorageLine(const string &line, Customer &customer) {
        vector<string> fields;
        string token;
        stringstream ss(line);

        while (getline(ss, token, '|')) {
            fields.push_back(token);
        }

        if (fields.size() != 9) {
            return false;
        }

        try {
            int id = stoi(fields[0]);
            string customerName = fields[1];
            string customerPhone = fields[2];
            int roomNum = stoi(fields[3]);
            string type = fields[4];
            int checkIn = stoi(fields[5]);
            int checkOut = stoi(fields[6]);
            double fee = stod(fields[7]);
            bool isActive = (stoi(fields[8]) == 1);

            customer = Customer(id, customerName, customerPhone, roomNum, type, checkIn,
                                checkOut, fee, isActive);
            return true;
        } catch (...) {
            return false;
        }
    }
};

class Hotel {
private:
    vector<unique_ptr<Room>> rooms;
    vector<Customer> customers;
    double totalRevenue;
    int nextBookingId;

    const string roomsFile = "rooms.txt";
    const string customersFile = "customers.txt";
    const string revenueFile = "revenue.txt";

    static string trim(const string &value) {
        size_t start = value.find_first_not_of(" \t\n\r");
        if (start == string::npos) {
            return "";
        }

        size_t end = value.find_last_not_of(" \t\n\r");
        return value.substr(start, end - start + 1);
    }

    Room *findRoomByNumber(int roomNumber) {
        for (const auto &room : rooms) {
            if (room->getRoomNumber() == roomNumber) {
                return room.get();
            }
        }
        return nullptr;
    }

    const Room *findRoomByNumber(int roomNumber) const {
        for (const auto &room : rooms) {
            if (room->getRoomNumber() == roomNumber) {
                return room.get();
            }
        }
        return nullptr;
    }

    vector<Room *> getAvailableRoomsByType(const string &type) {
        vector<Room *> availableRooms;
        for (const auto &room : rooms) {
            if (room->isAvailable() && room->getRoomType() == type) {
                availableRooms.push_back(room.get());
            }
        }
        return availableRooms;
    }

    void seedDefaultRooms() {
        rooms.clear();

        for (int i = 101; i <= 105; ++i) {
            rooms.push_back(make_unique<SingleRoom>(i));
        }

        for (int i = 201; i <= 205; ++i) {
            rooms.push_back(make_unique<DoubleRoom>(i));
        }

        for (int i = 301; i <= 305; ++i) {
            rooms.push_back(make_unique<DeluxeRoom>(i));
        }

        saveRooms();
    }

    unique_ptr<Room> createRoomByType(const string &type, int roomNumber, bool isAvailable) {
        if (type == "Single") {
            return make_unique<SingleRoom>(roomNumber, isAvailable);
        }

        if (type == "Double") {
            return make_unique<DoubleRoom>(roomNumber, isAvailable);
        }

        return make_unique<DeluxeRoom>(roomNumber, isAvailable);
    }

    void loadRooms() {
        ifstream in(roomsFile);
        if (!in.is_open()) {
            seedDefaultRooms();
            return;
        }

        rooms.clear();
        string line;

        while (getline(in, line)) {
            if (trim(line).empty()) {
                continue;
            }

            stringstream ss(line);
            string numberStr;
            string type;
            string rateStr;
            string availableStr;

            if (!getline(ss, numberStr, '|') || !getline(ss, type, '|') ||
                !getline(ss, rateStr, '|') || !getline(ss, availableStr, '|')) {
                continue;
            }

            try {
                int roomNumber = stoi(numberStr);
                bool isAvailable = (stoi(availableStr) == 1);
                rooms.push_back(createRoomByType(type, roomNumber, isAvailable));
            } catch (...) {
            }
        }

        if (rooms.empty()) {
            seedDefaultRooms();
        }
    }

    void saveRooms() const {
        ofstream out(roomsFile, ios::trunc);
        for (const auto &room : rooms) {
            out << room->getRoomNumber() << "|" << room->getRoomType() << "|"
                << fixed << setprecision(2) << room->getNightlyRate() << "|"
                << (room->isAvailable() ? 1 : 0) << "\n";
        }
    }

    void loadCustomers() {
        customers.clear();
        nextBookingId = 1;

        ifstream in(customersFile);
        if (!in.is_open()) {
            return;
        }

        string line;
        while (getline(in, line)) {
            if (trim(line).empty()) {
                continue;
            }

            Customer parsed(0, "", "", 0, "", 0);
            if (Customer::parseFromStorageLine(line, parsed)) {
                customers.push_back(parsed);
                nextBookingId = max(nextBookingId, parsed.getBookingId() + 1);
            }
        }
    }

    void saveCustomers() const {
        ofstream out(customersFile, ios::trunc);
        for (const auto &customer : customers) {
            out << customer.toStorageLine() << "\n";
        }
    }

    void loadRevenue() {
        totalRevenue = 0.0;

        ifstream in(revenueFile);
        if (!in.is_open()) {
            return;
        }

        in >> totalRevenue;
        if (in.fail()) {
            totalRevenue = 0.0;
        }
    }

    void saveRevenue() const {
        ofstream out(revenueFile, ios::trunc);
        out << fixed << setprecision(2) << totalRevenue;
    }

    static int readInt(const string &prompt) {
        while (true) {
            cout << prompt;
            int value;
            cin >> value;

            if (!cin.fail()) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }

            cout << "Invalid number. Please try again.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    static string readLine(const string &prompt) {
        cout << prompt;
        string value;
        getline(cin, value);
        return trim(value);
    }

    static string mapChoiceToRoomType(int choice) {
        if (choice == 1) {
            return "Single";
        }
        if (choice == 2) {
            return "Double";
        }
        return "Deluxe";
    }

    void printBookingHeader() const {
        cout << left << setw(10) << "ID" << setw(20) << "Name" << setw(15) << "Phone"
             << setw(10) << "Room" << setw(10) << "Type" << setw(10) << "In-Day"
             << setw(10) << "Out-Day" << setw(12) << "Fee" << setw(10) << "Status"
             << "\n";
        cout << string(107, '-') << "\n";
    }

public:
    Hotel() : totalRevenue(0.0), nextBookingId(1) {
        loadRooms();
        loadCustomers();
        loadRevenue();

        // Ensure room availability matches active bookings at startup.
        for (auto &room : rooms) {
            room->setAvailable(true);
        }

        for (const auto &customer : customers) {
            if (customer.isActive()) {
                Room *room = findRoomByNumber(customer.getRoomNumber());
                if (room != nullptr) {
                    room->setAvailable(false);
                }
            }
        }

        saveRooms();
    }

    void showRoomAvailability() const {
        cout << "\n=== Room Availability ===\n";
        cout << left << setw(12) << "Room No" << setw(12) << "Type" << setw(12)
             << "Rate/Night" << "Status\n";
        cout << string(46, '-') << "\n";

        for (const auto &room : rooms) {
            cout << left << setw(12) << room->getRoomNumber() << setw(12)
                 << room->getRoomType() << setw(12) << fixed << setprecision(2)
                 << room->getNightlyRate()
                 << (room->isAvailable() ? "Available" : "Occupied") << "\n";
        }
    }

    void bookRoom() {
        cout << "\n=== Book A Room ===\n";
        cout << "1. Single\n2. Double\n3. Deluxe\n";

        int roomChoice = readInt("Select room type: ");
        if (roomChoice < 1 || roomChoice > 3) {
            cout << "Invalid room type selection.\n";
            return;
        }

        string roomType = mapChoiceToRoomType(roomChoice);
        vector<Room *> availableRooms = getAvailableRoomsByType(roomType);

        if (availableRooms.empty()) {
            cout << "No " << roomType << " rooms are currently available.\n";
            return;
        }

        cout << "\nAvailable " << roomType << " rooms:\n";
        for (const Room *room : availableRooms) {
            cout << "- Room " << room->getRoomNumber() << " (Rate: $" << fixed
                 << setprecision(2) << room->getNightlyRate() << "/night)\n";
        }

        int selectedRoomNumber = readInt("Enter preferred room number from list above: ");
        Room *selectedRoom = nullptr;

        for (Room *room : availableRooms) {
            if (room->getRoomNumber() == selectedRoomNumber) {
                selectedRoom = room;
                break;
            }
        }

        if (selectedRoom == nullptr) {
            cout << "Selected room is invalid or unavailable.\n";
            return;
        }

        string customerName = readLine("Enter customer name: ");
        string customerPhone = readLine("Enter phone number: ");
        int checkInDay = readInt("Enter check-in day number (e.g. 1, 2, 3...): ");

        if (customerName.empty() || customerPhone.empty() || checkInDay < 1) {
            cout << "Invalid booking details.\n";
            return;
        }

        selectedRoom->setAvailable(false);
        customers.emplace_back(nextBookingId, customerName, customerPhone,
                               selectedRoom->getRoomNumber(), selectedRoom->getRoomType(),
                               checkInDay);

        cout << "\nBooking successful! Booking ID: " << nextBookingId << "\n";
        cout << "Assigned Room: " << selectedRoom->getRoomNumber() << " ("
             << selectedRoom->getRoomType() << ")\n";

        ++nextBookingId;
        saveRooms();
        saveCustomers();
    }

    void checkOut() {
        cout << "\n=== Customer Check-Out ===\n";

        vector<int> activeIndices;
        for (size_t i = 0; i < customers.size(); ++i) {
            if (customers[i].isActive()) {
                activeIndices.push_back(static_cast<int>(i));
            }
        }

        if (activeIndices.empty()) {
            cout << "No active bookings found.\n";
            return;
        }

        printBookingHeader();
        for (int index : activeIndices) {
            const Customer &customer = customers[index];
            cout << left << setw(10) << customer.getBookingId() << setw(20)
                 << customer.getName() << setw(15) << customer.getPhone() << setw(10)
                 << customer.getRoomNumber() << setw(10) << customer.getRoomType()
                 << setw(10) << customer.getCheckInDay() << setw(10) << "-" << setw(12)
                 << "-"
                 << "Active\n";
        }

        int bookingId = readInt("Enter booking ID for check-out: ");
        int checkoutDay = readInt("Enter check-out day number: ");

        for (Customer &customer : customers) {
            if (customer.getBookingId() == bookingId && customer.isActive()) {
                int nights = checkoutDay - customer.getCheckInDay();
                if (nights <= 0) {
                    cout << "Check-out day must be greater than check-in day.\n";
                    return;
                }

                Room *room = findRoomByNumber(customer.getRoomNumber());
                if (room == nullptr) {
                    cout << "Room record not found for this booking.\n";
                    return;
                }

                double fee = nights * room->getNightlyRate();
                customer.closeBooking(checkoutDay, fee);
                room->setAvailable(true);
                totalRevenue += fee;

                cout << "\nCheck-out completed.\n";
                cout << "Nights Stayed: " << nights << "\n";
                cout << "Total Fee: $" << fixed << setprecision(2) << fee << "\n";

                saveRooms();
                saveCustomers();
                saveRevenue();
                return;
            }
        }

        cout << "Active booking not found for ID " << bookingId << ".\n";
    }

    void searchBookings() const {
        cout << "\n=== Search Booking ===\n";
        cout << "1. Search by Booking ID\n2. Search by Customer Name\n";

        int choice = readInt("Select search option: ");
        bool found = false;

        if (choice == 1) {
            int id = readInt("Enter booking ID: ");
            printBookingHeader();

            for (const Customer &customer : customers) {
                if (customer.getBookingId() == id) {
                    cout << left << setw(10) << customer.getBookingId() << setw(20)
                         << customer.getName() << setw(15) << customer.getPhone()
                         << setw(10) << customer.getRoomNumber() << setw(10)
                         << customer.getRoomType() << setw(10) << customer.getCheckInDay()
                         << setw(10)
                         << (customer.getCheckOutDay() < 0
                                 ? string("-")
                                 : to_string(customer.getCheckOutDay()))
                         << setw(12) << fixed << setprecision(2) << customer.getTotalFee()
                         << (customer.isActive() ? "Active" : "Closed") << "\n";
                    found = true;
                    break;
                }
            }
        } else if (choice == 2) {
            string nameQuery = readLine("Enter customer name (partial allowed): ");
            string loweredQuery = nameQuery;
            transform(loweredQuery.begin(), loweredQuery.end(), loweredQuery.begin(),
                      ::tolower);

            printBookingHeader();
            for (const Customer &customer : customers) {
                string loweredName = customer.getName();
                transform(loweredName.begin(), loweredName.end(), loweredName.begin(),
                          ::tolower);

                if (loweredName.find(loweredQuery) != string::npos) {
                    cout << left << setw(10) << customer.getBookingId() << setw(20)
                         << customer.getName() << setw(15) << customer.getPhone()
                         << setw(10) << customer.getRoomNumber() << setw(10)
                         << customer.getRoomType() << setw(10) << customer.getCheckInDay()
                         << setw(10)
                         << (customer.getCheckOutDay() < 0
                                 ? string("-")
                                 : to_string(customer.getCheckOutDay()))
                         << setw(12) << fixed << setprecision(2) << customer.getTotalFee()
                         << (customer.isActive() ? "Active" : "Closed") << "\n";
                    found = true;
                }
            }
        } else {
            cout << "Invalid search option.\n";
            return;
        }

        if (!found) {
            cout << "No matching booking found.\n";
        }
    }

    void adminPanel() const {
        cout << "\n=== Admin Panel ===\n";
        string password = readLine("Enter admin password: ");

        if (password != "admin123") {
            cout << "Access denied.\n";
            return;
        }

        while (true) {
            cout << "\n--- Admin Menu ---\n";
            cout << "1. View all bookings\n";
            cout << "2. View active bookings\n";
            cout << "3. View room availability\n";
            cout << "4. View total revenue\n";
            cout << "5. Back to main menu\n";

            int choice = readInt("Select option: ");

            if (choice == 1) {
                if (customers.empty()) {
                    cout << "No bookings available.\n";
                    continue;
                }

                printBookingHeader();
                for (const Customer &customer : customers) {
                    cout << left << setw(10) << customer.getBookingId() << setw(20)
                         << customer.getName() << setw(15) << customer.getPhone()
                         << setw(10) << customer.getRoomNumber() << setw(10)
                         << customer.getRoomType() << setw(10)
                         << customer.getCheckInDay() << setw(10)
                         << (customer.getCheckOutDay() < 0
                                 ? string("-")
                                 : to_string(customer.getCheckOutDay()))
                         << setw(12) << fixed << setprecision(2)
                         << customer.getTotalFee()
                         << (customer.isActive() ? "Active" : "Closed") << "\n";
                }
            } else if (choice == 2) {
                bool anyActive = false;
                printBookingHeader();

                for (const Customer &customer : customers) {
                    if (customer.isActive()) {
                        anyActive = true;
                        cout << left << setw(10) << customer.getBookingId() << setw(20)
                             << customer.getName() << setw(15) << customer.getPhone()
                             << setw(10) << customer.getRoomNumber() << setw(10)
                             << customer.getRoomType() << setw(10)
                             << customer.getCheckInDay() << setw(10) << "-" << setw(12)
                             << "-"
                             << "Active\n";
                    }
                }

                if (!anyActive) {
                    cout << "No active bookings found.\n";
                }
            } else if (choice == 3) {
                showRoomAvailability();
            } else if (choice == 4) {
                cout << "Total Revenue: $" << fixed << setprecision(2) << totalRevenue
                     << "\n";
            } else if (choice == 5) {
                return;
            } else {
                cout << "Invalid option.\n";
            }
        }
    }
};

int main() {
    Hotel hotel;

    while (true) {
        cout << "\n====== Hotel Management System ======\n";
        cout << "1. Book Room\n";
        cout << "2. Check-Out Customer\n";
        cout << "3. Search Booking\n";
        cout << "4. Show Room Availability\n";
        cout << "5. Admin Panel\n";
        cout << "6. Exit\n";

        int choice;
        cout << "Enter your choice: ";
        cin >> choice;

        if (cin.fail()) {
            cout << "Invalid input. Please enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            hotel.bookRoom();
        } else if (choice == 2) {
            hotel.checkOut();
        } else if (choice == 3) {
            hotel.searchBookings();
        } else if (choice == 4) {
            hotel.showRoomAvailability();
        } else if (choice == 5) {
            hotel.adminPanel();
        } else if (choice == 6) {
            cout << "Thank you for using Hotel Management System.\n";
            break;
        } else {
            cout << "Invalid option. Please select from 1 to 6.\n";
        }
    }

    return 0;
}
