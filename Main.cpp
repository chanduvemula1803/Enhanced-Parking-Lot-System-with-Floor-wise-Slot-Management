#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <memory>
#include <iomanip>

enum class VehicleType { CAR, BIKE, TRUCK };
enum class SpotType { COMPACT, LARGE, HANDICAPPED, ELECTRIC };

class Vehicle {
public:
    Vehicle(std::string licensePlate, VehicleType type) 
        : licensePlate(licensePlate), type(type) {}
    
    std::string getLicensePlate() const { return licensePlate; }
    VehicleType getType() const { return type; }

private:
    std::string licensePlate;
    VehicleType type;
};

class ParkingSpot {
public:
    ParkingSpot(std::string spotId, SpotType spotType) 
        : spotId(spotId), spotType(spotType), isOccupied(false) {}

    bool isAvailable() const { return !isOccupied; }
    void assignVehicle(std::shared_ptr<Vehicle> vehicle) {
        this->vehicle = vehicle;
        isOccupied = true;
    }
    void removeVehicle() {
        vehicle = nullptr;
        isOccupied = false;
    }

    std::string getSpotId() const { return spotId; }
    SpotType getSpotType() const { return spotType; }
    std::shared_ptr<Vehicle> getVehicle() const { return vehicle; }

private:
    std::string spotId;
    SpotType spotType;
    bool isOccupied;
    std::shared_ptr<Vehicle> vehicle;
};

class Floor {
public:
    Floor(int floorNumber) : floorNumber(floorNumber) {
        // Create spots 1A to 1Z, 2A to 2Z, etc.
        for (char c = 'A'; c <= 'Z'; c++) {
            std::string spotId = std::to_string(floorNumber) + c;
            // Alternate between COMPACT and LARGE spots
            SpotType type = (c % 2 == 0) ? SpotType::COMPACT : SpotType::LARGE;
            spots.push_back(std::make_shared<ParkingSpot>(spotId, type));
        }
    }

    void displayAvailableSpots() const {
        std::cout << "Floor " << floorNumber << " available spots:\n";
        for (const auto& spot : spots) {
            if (spot->isAvailable()) {
                std::cout << spot->getSpotId() << " (" 
                          << spotTypeToString(spot->getSpotType()) << ")\t";
            }
        }
        std::cout << "\n\n";
    }

    std::shared_ptr<ParkingSpot> findAvailableSpot(VehicleType vehicleType) {
        for (auto& spot : spots) {
            if (spot->isAvailable()) {
                if ((vehicleType == VehicleType::CAR && spot->getSpotType() == SpotType::COMPACT) ||
                    (vehicleType == VehicleType::TRUCK && spot->getSpotType() == SpotType::LARGE) ||
                    (vehicleType == VehicleType::BIKE)) {
                    return spot;
                }
            }
        }
        return nullptr;
    }

private:
    int floorNumber;
    std::vector<std::shared_ptr<ParkingSpot>> spots;

    std::string spotTypeToString(SpotType type) const {
        switch(type) {
            case SpotType::COMPACT: return "COMPACT";
            case SpotType::LARGE: return "LARGE";
            case SpotType::HANDICAPPED: return "HANDICAPPED";
            case SpotType::ELECTRIC: return "ELECTRIC";
            default: return "UNKNOWN";
        }
    }
};

class Ticket {
public:
    Ticket(std::shared_ptr<Vehicle> vehicle, std::shared_ptr<ParkingSpot> spot)
        : ticketId(generateId()), vehicle(vehicle), assignedSpot(spot),
          entryTime(std::chrono::system_clock::now()) {}

    std::string getTicketId() const { return ticketId; }
    std::shared_ptr<Vehicle> getVehicle() const { return vehicle; }
    std::shared_ptr<ParkingSpot> getAssignedSpot() const { return assignedSpot; }
    std::chrono::system_clock::time_point getEntryTime() const { return entryTime; }

private:
    std::string ticketId;
    std::shared_ptr<Vehicle> vehicle;
    std::shared_ptr<ParkingSpot> assignedSpot;
    std::chrono::system_clock::time_point entryTime;

    static std::string generateId() {
        static int counter = 0;
        return "T" + std::to_string(++counter);
    }
};

class ParkingLot {
public:
    static ParkingLot& getInstance() {
        static ParkingLot instance;
        return instance;
    }

    void initializeFloors(int numFloors) {
        for (int i = 1; i <= numFloors; i++) {
            floors.push_back(std::make_shared<Floor>(i));
        }
    }

    void displayAllAvailableSpots() {
        for (const auto& floor : floors) {
            floor->displayAvailableSpots();
        }
    }

    std::shared_ptr<Ticket> parkVehicle(std::shared_ptr<Vehicle> vehicle) {
        for (auto& floor : floors) {
            auto spot = floor->findAvailableSpot(vehicle->getType());
            if (spot) {
                spot->assignVehicle(vehicle);
                auto ticket = std::make_shared<Ticket>(vehicle, spot);
                tickets[ticket->getTicketId()] = ticket;
                return ticket;
            }
        }
        return nullptr;
    }

    double unparkVehicle(std::string ticketId) {
        if (tickets.find(ticketId) == tickets.end()) return -1;

        auto ticket = tickets[ticketId];
        auto spot = ticket->getAssignedSpot();
        spot->removeVehicle();

        auto duration = std::chrono::system_clock::now() - ticket->getEntryTime();
        auto hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
        double fee = hours * 10;

        tickets.erase(ticketId);
        return fee;
    }

private:
    ParkingLot() = default;
    std::vector<std::shared_ptr<Floor>> floors;
    std::map<std::string, std::shared_ptr<Ticket>> tickets;
};

int main() {
    auto& parkingLot = ParkingLot::getInstance();

    // Initialize with 3 floors (1, 2, 3)
    parkingLot.initializeFloors(3);

    // Display available spots before parking
    std::cout << "Initial available spots:\n";
    parkingLot.displayAllAvailableSpots();

    // Park a car
    auto car = std::make_shared<Vehicle>("ABC123", VehicleType::CAR);
    auto ticket = parkingLot.parkVehicle(car);

    if (ticket) {
        std::cout << "\nVehicle parked at spot: " << ticket->getAssignedSpot()->getSpotId() 
                  << "\nTicket ID: " << ticket->getTicketId() << std::endl;
        
        // Display available spots after parking
        // std::cout << "\nAvailable spots after parking:\n";
        // parkingLot.displayAllAvailableSpots();

        // Unpark after some time
        double fee = parkingLot.unparkVehicle(ticket->getTicketId());
        std::cout << "\nUnparking vehicle. Fee: $" << fee << std::endl;

        // Display available spots after unparking
        // std::cout << "\nAvailable spots after unparking:\n";
        // parkingLot.displayAllAvailableSpots();
    } else {
        std::cout << "No available spot!" << std::endl;
    }

    return 0;
}