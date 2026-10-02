//
// Created by dusan on 9/15/26.
//

#include "../include/antworld.h"
#include <iostream>
#include <algorithm>

/** @brief this is where you as the applicant will make use of the above functions to develop your solution.
 * here are some existing examples of how calling these functions works to help get you started!
 */
void AntWorld::forage() {
    // std::vector<Coord> visibleFood = this->ants[0].foodScan(this->foodMap);
    //
    // Coord desiredDestination = Coord(5, 5);
    // Coord finalPos = this->ants[0].move(this->terrainMap, desiredDestination, this->foodMap);
    // bool destCheck = (desiredDestination == finalPos);
    //
    // this->ants[0].dropPheromone(this->pheromoneMap);
    //
    // this->ants[0].erasePheromone(this->pheromoneMap);
    //
    // this->ants[0].returnHome(this->terrainMap, this->foodMap);
    static int lastPrintedScore = -1;
    static std::vector<Coord> knownFoodLocations;
    static std::vector<Coord> scoutDiscoveries;
    static int activeScoutIndex = -1;
    static int activeZoneIndex = -1;

    if (this->score != lastPrintedScore) {
        std::cout << "SCORE: " << this->score << std::endl;
        lastPrintedScore = this->score;
    }

    int mapRows=static_cast<int>(this->terrainMap.size());
    int mapColumns=static_cast<int>(this->terrainMap[0].size());

    std::vector<int> zoneRows={
        mapRows/4, mapRows/2,(3*mapRows)/4
    };

    std::vector<int> zoneColumns={
        mapColumns/4, mapColumns/2,(3*mapColumns)/4
    };

    std::vector<Coord> searchZones;

    for (int zoneRow:zoneRows) {
        for (int zoneColumn:zoneColumns) {
            searchZones.emplace_back(zoneRow,zoneColumn);
        };
    };

    static std::vector<int> zoneTravelCosts;

    if (zoneTravelCosts.empty()) {
        for (Coord zone:searchZones) {
            std::vector<Coord> pathFromHome=shortestPath(this->terrainMap,this->homeCoordinates,zone);
            int oneWayCost=calculatePathCost(this->terrainMap,pathFromHome);
            zoneTravelCosts.push_back(oneWayCost);
        };
    };

    static std::vector<int> zoneOrder;

    if (zoneOrder.empty()) {
        for (int zoneIndex=0;
            zoneIndex<static_cast<int>(searchZones.size());
            zoneIndex++) {zoneOrder.push_back(zoneIndex);}

        std::sort(zoneOrder.begin(),zoneOrder.end(),[](int firstZoneIndex,int secondZoneIndex){
            return zoneTravelCosts[firstZoneIndex]<zoneTravelCosts[secondZoneIndex];});
    };

    static std::vector<int> assignedZoneByAnt;
    static std::vector<bool> zoneCompleted;

    if (assignedZoneByAnt.empty()) {
        assignedZoneByAnt=std::vector<int>(this->ants.size(), -1);

        zoneCompleted=std::vector<bool>(searchZones.size(), false);
    }

    bool allAntsReadyAtHome = true;

    for (int antIndex = 0;
         antIndex < static_cast<int>(this->ants.size());
         antIndex++) {

        bool antIsHome =
            this->ants[antIndex].position ==
            this->ants[antIndex].homeCoord;

        bool antIsCarryingFood =
            this->ants[antIndex].carryingFood;

        if (!antIsHome || antIsCarryingFood) {
            allAntsReadyAtHome = false;
            break;
        }
    }

    if (allAntsReadyAtHome &&
        !knownFoodLocations.empty()) {

        bool anyReportedFoodReachable = false;

        for (int antIndex = 0;
             antIndex < static_cast<int>(this->ants.size());
             antIndex++) {

            Ant &candidateCollector =
                this->ants[antIndex];

            for (Coord reportedFood : knownFoodLocations) {
                std::vector<Coord> pathToFood =
                    shortestPath(
                        this->terrainMap,
                        candidateCollector.homeCoord,
                        reportedFood
                    );

                std::vector<Coord> pathHome =
                    shortestPath(
                        this->terrainMap,
                        reportedFood,
                        candidateCollector.homeCoord
                    );

                int requiredEnergy =
                    calculatePathCost(
                        this->terrainMap,
                        pathToFood
                    )
                    +
                    calculatePathCost(
                        this->terrainMap,
                        pathHome
                    );

                if (requiredEnergy <
                    candidateCollector.energy) {

                    anyReportedFoodReachable = true;
                    break;
                }
            }

            if (anyReportedFoodReachable) {
                break;
            }
        }

        if (!anyReportedFoodReachable) {
            std::cout << "Abandoning "
                      << knownFoodLocations.size()
                      << " unreachable reported food locations"
                      << std::endl;

            knownFoodLocations.clear();
        }
    }

    bool colonyReadyToScout =
        activeScoutIndex == -1 &&
        knownFoodLocations.empty() &&
        allAntsReadyAtHome;

    if (colonyReadyToScout) {
        int nextZoneIndex = -1;

        for (int candidateZoneIndex : zoneOrder) {
            if (!zoneCompleted[candidateZoneIndex]) {
                nextZoneIndex = candidateZoneIndex;
                break;
            }
        }

        if (nextZoneIndex != -1) {
            int requiredScoutEnergy =
                zoneTravelCosts[nextZoneIndex] * 2;

            int selectedScoutIndex = -1;

            for (int antIndex = 0;
                 antIndex < static_cast<int>(this->ants.size());
                 antIndex++) {

                bool antCanScout =
                    requiredScoutEnergy <
                    this->ants[antIndex].energy;

                bool antUsesLessEnergy =
                    selectedScoutIndex == -1 ||
                    this->ants[antIndex].energy <
                    this->ants[selectedScoutIndex].energy;

                if (antCanScout && antUsesLessEnergy) {
                    selectedScoutIndex = antIndex;
                }
            }

            if (selectedScoutIndex != -1) {
                activeScoutIndex = selectedScoutIndex;
                activeZoneIndex = nextZoneIndex;

                assignedZoneByAnt[activeScoutIndex] =
                    activeZoneIndex;

                scoutDiscoveries.clear();

                Coord scoutZone =
                    searchZones[activeZoneIndex];

                std::cout << "Selected Ant "<< activeScoutIndex + 1<< " with energy "<< this->ants[activeScoutIndex].energy<< " to scout zone ("<< scoutZone.first << ", "<< scoutZone.second << ")"<< std::endl;
            }
        }
    }

    std::vector<int> actionOrder;

    for (int antIndex = 0;
         antIndex < static_cast<int>(this->ants.size());
         antIndex++) {
        actionOrder.push_back(antIndex);
    }

    std::sort(
        actionOrder.begin(),
        actionOrder.end(),
        [this](int firstAntIndex, int secondAntIndex) {
            return this->ants[firstAntIndex].energy <
                   this->ants[secondAntIndex].energy;
        }
    );

    for (int antIndex : actionOrder) {

        Ant &ant = this->ants[antIndex];
        int currentAntNumber = antIndex + 1;

        bool antIsActiveScout =
            antIndex == activeScoutIndex;

        if (antIsActiveScout) {
            Coord scoutTarget =
                searchZones[activeZoneIndex];

            if (ant.position != scoutTarget) {
                ant.move(
                    this->terrainMap,
                    scoutTarget,
                    this->foodMap
                );

                std::cout << "Scout Ant "
                          << currentAntNumber
                          << " travelled to zone ("
                          << scoutTarget.first << ", "
                          << scoutTarget.second << ")"
                          << " | energy: "
                          << ant.energy
                          << std::endl;

                continue;
            }

            std::vector<Coord> scoutVisibleFood =
                ant.foodScan(this->foodMap);

            for (Coord discoveredFood : scoutVisibleFood) {
                bool discoveryAlreadyStored = false;

                for (Coord storedDiscovery : scoutDiscoveries) {
                    if (storedDiscovery == discoveredFood) {
                        discoveryAlreadyStored = true;
                        break;
                    }
                }

                if (!discoveryAlreadyStored) {
                    scoutDiscoveries.push_back(discoveredFood);
                }
            }

            zoneCompleted[activeZoneIndex] = true;

            ant.returnHome(
                this->terrainMap,
                this->foodMap
            );

            if (ant.position == ant.homeCoord) {
                for (Coord reportedFood : scoutDiscoveries) {
                    bool foodAlreadyReported = false;

                    for (Coord knownFood : knownFoodLocations) {
                        if (knownFood == reportedFood) {
                            foodAlreadyReported = true;
                            break;
                        }
                    }

                    if (!foodAlreadyReported) {
                        knownFoodLocations.push_back(reportedFood);
                    }
                }

                std::cout << "Scout Ant "
                          << currentAntNumber
                          << " returned home and reported "
                          << scoutDiscoveries.size()
                          << " food locations"
                          << std::endl;

                assignedZoneByAnt[antIndex] = -1;
                activeScoutIndex = -1;
                activeZoneIndex = -1;
                scoutDiscoveries.clear();
            }

            continue;
        }

        if (ant.carryingFood) {
            ant.returnHome(
                this->terrainMap,
                this->foodMap
            );

            std::cout << "Collector Ant "
                      << currentAntNumber
                      << " returned home"
                      << std::endl;

            continue;
        }

        if (!knownFoodLocations.empty()) {
            bool safeFoodFound = false;
            Coord destination = ant.position;
            int lowestRoundTripCost = ant.energy + 1;

            for (Coord knownFoodLocation : knownFoodLocations) {
                std::vector<Coord> pathToFood =
                    shortestPath(
                        this->terrainMap,
                        ant.position,
                        knownFoodLocation
                    );

                std::vector<Coord> pathHome =
                    shortestPath(
                        this->terrainMap,
                        knownFoodLocation,
                        ant.homeCoord
                    );

                int roundTripCost =
                    calculatePathCost(
                        this->terrainMap,
                        pathToFood
                    )
                    +
                    calculatePathCost(
                        this->terrainMap,
                        pathHome
                    );

                if (roundTripCost < ant.energy &&
                    roundTripCost < lowestRoundTripCost) {

                    lowestRoundTripCost = roundTripCost;
                    destination = knownFoodLocation;
                    safeFoodFound = true;
                }
            }

            if (safeFoodFound) {
                Coord finalPosition =
                    ant.move(
                        this->terrainMap,
                        destination,
                        this->foodMap
                    );

                if (finalPosition == destination) {
                    for (int knownFoodIndex = 0;
                         knownFoodIndex <
                         static_cast<int>(
                             knownFoodLocations.size()
                         );
                         knownFoodIndex++) {

                        if (knownFoodLocations[knownFoodIndex] ==
                            destination) {

                            knownFoodLocations.erase(
                                knownFoodLocations.begin() +
                                knownFoodIndex
                            );

                            break;
                        }
                    }
                }

                std::cout << "Collector Ant "
                          << currentAntNumber
                          << " collected reported food at ("
                          << ant.position.first << ", "
                          << ant.position.second << ")"
                          << " | energy: "
                          << ant.energy
                          << std::endl;

                continue;
            }
        }

        if (ant.position != ant.homeCoord) {
            ant.returnHome(
                this->terrainMap,
                this->foodMap
            );
        }
    }

}

/** You may insert any custom functions below **/