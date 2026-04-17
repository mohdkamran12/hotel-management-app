# Hotel Management System (C++ OOP + File Handling)

Console-based hotel management system built with classes, inheritance, encapsulation, and persistent file storage.

## Features

- Room booking and allocation:
  - Single
  - Double
  - Deluxe
- Customer check-in and check-out
- Fee calculation based on stay duration
- File handling for persistent records
- Admin panel for:
  - All bookings
  - Active bookings
  - Room availability
  - Total revenue
- Bonus features:
  - Search by booking ID
  - Search by customer name
  - Room availability status view

## OOP Design

- `Room` base class (encapsulated room data)
- Derived classes:
  - `SingleRoom`
  - `DoubleRoom`
  - `DeluxeRoom`
- `Customer` class (booking record model)
- `Hotel` class (business logic, file operations, menus)

## Data Files

These files are created automatically in the project folder:

- `rooms.txt` - room inventory and availability
- `customers.txt` - booking/check-in/check-out records
- `revenue.txt` - total accumulated revenue

## Build and Run

### Option 1: MinGW g++

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o hotel_app.exe
.\hotel_app.exe
```

### Option 2: MSVC (Developer Command Prompt)

```powershell
cl /EHsc /std:c++17 main.cpp
main.exe
```

## Notes

- Admin password is: `admin123`
- Day values are numeric (e.g. check-in day `3`, check-out day `7`)
- Stay nights are computed as: `checkOutDay - checkInDay`
