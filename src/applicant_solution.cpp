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
            return zoneTravelCosts[firstZoneIndex]>zoneTravelCosts[secondZoneIndex];});
    };

    static std::vector<int> assignedZoneByAnt;
    static std::vector<bool> zoneCompleted;

    if (assignedZoneByAnt.empty()) {
        assignedZoneByAnt=std::vector<int>(this->ants.size(), -1);

        zoneCompleted=std::vector<bool>(searchZones.size(), false);
    }

    for (int antIndex = 0;
         antIndex < static_cast<int>(this->ants.size());
         antIndex++) {
        Ant &currentAnt = this->ants[antIndex];

        bool antIsReadyAtHome =
            currentAnt.position == currentAnt.homeCoord &&
            !currentAnt.carryingFood;

        if (antIsReadyAtHome) {
            assignedZoneByAnt[antIndex] = -1;
        }
    }
    std::vector<int> availableAntOrder;

    for (int antIndex=0;
        antIndex<static_cast<int>(this->ants.size());
        antIndex++) {

        bool antHasNoZone=assignedZoneByAnt[antIndex]==-1;

        bool antIsHome=this->ants[antIndex].position==this->ants[antIndex].homeCoord;

        bool antIsNotCarryingFood=!this->ants[antIndex].carryingFood;

        if (antHasNoZone&&antIsHome&&antIsNotCarryingFood) {
            availableAntOrder.push_back(antIndex);
        }
    }

    std::sort(availableAntOrder.begin(),
        availableAntOrder.end(),
        [this](int firstAntIndex,int secondAntIndex) {
            return this->ants[firstAntIndex].energy>this->ants[secondAntIndex].energy;
        });

    for (int availableAntIndex:availableAntOrder) {
        for (int candidateZoneIndex:zoneOrder) {
            if (zoneCompleted[candidateZoneIndex]) {
                continue;
            }
            bool zoneAlreadyAssigned=false;

            for (int currentAssignment:assignedZoneByAnt) {
                if (currentAssignment==candidateZoneIndex) {
                    zoneAlreadyAssigned=true;
                    break;
                }
            }
            if (zoneAlreadyAssigned) {
                continue;
            }
            int requiredRoundTripEnergy=zoneTravelCosts[candidateZoneIndex]*2;

            if (requiredRoundTripEnergy<this->ants[availableAntIndex].energy) {
                assignedZoneByAnt[availableAntIndex]=candidateZoneIndex;

                Coord newZone=searchZones[candidateZoneIndex];

                std::cout << "Reassigned Ant "<< availableAntIndex + 1<< " with remaining energy "<< this->ants[availableAntIndex].energy<< " to zone ("<< newZone.first << ", "<< newZone.second << ")"<< std::endl;

                break;
            }
        }
    }


    int antNumber=1;

//    std::vector<Coord> explorationDirections={
//        Coord(-1,0), //ant1 north
//        Coord(-1,1), //ant2 northeast
//        //and so on
//        Coord(0,1),
//        Coord(1,1),
//        Coord(1,0),
//        Coord(1,-1),
//        Coord(0,-1),
//        Coord(-1,-1)
//    };

    for (Ant &ant : this->ants) {
        int currentAntNumber=antNumber;
        antNumber++;

        //Coord explorationDirection=explorationDirections[currentAntNumber-1];
        if (ant.carryingFood) {
            ant.returnHome(this->terrainMap, this->foodMap);
            std::cout << "Ant "<<currentAntNumber<<" returned to ("<< ant.position.first << ", "<< ant.position.second << ")"<< std::endl;
            continue;
        }

        std::vector<Coord>visibleFood=ant.foodScan(this->foodMap);

        for (Coord discoveredFood : visibleFood) {
            bool foodAlreadyKnown = false;

            for (Coord knownFood : knownFoodLocations) {
                if (knownFood == discoveredFood) {
                    foodAlreadyKnown = true;
                    break;
                }
            }

            if (!foodAlreadyKnown) {
                knownFoodLocations.push_back(discoveredFood);
            }
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
                         static_cast<int>(knownFoodLocations.size());
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

                std::cout << "Ant " << currentAntNumber
                          << " moved to known food at ("
                          << ant.position.first << ", "
                          << ant.position.second << ")"
                          << " | carrying food: "
                          << ant.carryingFood
                          << " | energy: "
                          << ant.energy
                          << std::endl;

                continue;
            }
        }
        /*   OLD CODE FOR DIRECTIONS VERSION
        int explorationDistance=ant.foodRadius+1;

        int targetRow= ant.position.first+explorationDirection.first*explorationDistance;

        int targetColumn=ant.position.second+explorationDirection.second*explorationDistance;

        targetRow=std::clamp(targetRow,0,static_cast<int>(this->terrainMap.size())-1);

        targetColumn=std::clamp(targetColumn,0,static_cast<int>(this->terrainMap[0].size())-1);

        Coord explorationTarget=Coord(targetRow,targetColumn);*/

        int antIndex=currentAntNumber-1;

        int assignedZoneIndex=assignedZoneByAnt[antIndex];

        if (assignedZoneIndex==-1) {
            if (ant.position!=ant.homeCoord) {
                ant.returnHome(this->terrainMap, this->foodMap);
            }
            continue;
/*
            for (int candidateZoneIndex:zoneOrder) {
                if (zoneCompleted[candidateZoneIndex]) {
                    continue;
                }

                bool zoneAlreadyAssigned=false;

                for (int currentAssignment:assignedZoneByAnt) {
                    if (currentAssignment==candidateZoneIndex) {
                        zoneAlreadyAssigned=true;
                        break;

                    }
                }

                if (zoneAlreadyAssigned) {
                    continue;
                }

                int requiredRoundTripEnergy=zoneTravelCosts[candidateZoneIndex]*2;

                if (requiredRoundTripEnergy<=ant.energy) {
                    assignedZoneByAnt[antIndex]=candidateZoneIndex;

                    assignedZoneIndex=candidateZoneIndex;
                    Coord newZone=searchZones[candidateZoneIndex];

                    std::cout<<"Reassigned Ant "<< currentAntNumber<< " to zone ("<< newZone.first << ", "<< newZone.second << ")"<< std::endl;
                    break;
                }
            }

            if (assignedZoneIndex==-1) {
                continue;
            }*/
        }

        Coord explorationTarget=searchZones[assignedZoneIndex];

        if (ant.position==explorationTarget && visibleFood.empty()) {
            zoneCompleted[assignedZoneIndex]=true;
            assignedZoneByAnt[antIndex]=-1;

            std::cout<<"ant "<<currentAntNumber<< " completed zone ("<< explorationTarget.first << ", "<< explorationTarget.second << ")"<< std::endl;

            if (ant.position!=ant.homeCoord) {
                ant.returnHome(this->terrainMap,this->foodMap);
                std::cout <<"Ant "<<currentAntNumber<<" returned home after completing its zone"<<std::endl;
            }
            continue;
        }

        std::vector<Coord> pathToExplorationTarget=shortestPath(this->terrainMap,ant.position,explorationTarget);
        std::vector<Coord> pathFromTargetHome = shortestPath(this->terrainMap,explorationTarget,ant.homeCoord);
        int explorationRoundTripCost=calculatePathCost(this->terrainMap,pathToExplorationTarget)+calculatePathCost(this->terrainMap,pathFromTargetHome);
        bool targetIsNew=explorationTarget != ant.position;

        if (targetIsNew && explorationRoundTripCost<ant.energy){
            ant.move(this->terrainMap,explorationTarget,this->foodMap);

            std::cout << "Ant " << currentAntNumber<< " explored to ("<< ant.position.first << ", "<< ant.position.second << ")"<< " | carrying food: "<< ant.carryingFood<< " | energy: "<< ant.energy<< std::endl;
        }
        else if (targetIsNew) {
            assignedZoneByAnt[antIndex]=-1;
            std::cout << "Ant " << currentAntNumber<< " released zone ("<< explorationTarget.first << ", "<< explorationTarget.second<< ") because it no longer has enough energy"<< std::endl;

            if (ant.position != ant.homeCoord) {
                ant.returnHome(this->terrainMap,this->foodMap);
            }
            continue;
        }


        else if (ant.position != ant.homeCoord) {
            ant.returnHome(this->terrainMap,this->foodMap);
            std::cout << "Ant " << currentAntNumber<< " stopped exploring and returned to ("<< ant.position.first << ", "<< ant.position.second << ")"<< " | energy: "<< ant.energy<< std::endl;
        }
    }

}

/** You may insert any custom functions below **/
