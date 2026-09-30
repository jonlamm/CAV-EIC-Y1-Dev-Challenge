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

    if (this->score != lastPrintedScore) {
        std::cout << "SCORE: " << this->score << std::endl;
        lastPrintedScore = this->score;
    }

    int antNumber=1;

    std::vector<Coord> explorationDirections={
        Coord(-1,0), //ant1 north
        Coord(-1,1), //ant2 northeast
        //and so on
        Coord(0,1),
        Coord(1,1),
        Coord(1,0),
        Coord(1,-1),
        Coord(0,-1),
        Coord(-1,-1)
    };

    for (Ant &ant : this->ants) {
        int currentAntNumber=antNumber;
        antNumber++;

        Coord explorationDirection=explorationDirections[currentAntNumber-1];
        if (ant.carryingFood) {
            ant.returnHome(this->terrainMap, this->foodMap);
            std::cout << "Ant "<<currentAntNumber<<" returned to ("<< ant.position.first << ", "<< ant.position.second << ")"<< std::endl;
            continue;
        }

        std::vector<Coord>visibleFood=ant.foodScan(this->foodMap);

        if (!visibleFood.empty()) {
            bool safeFoodFound=false;
            Coord destination=ant.position;
            int lowestRoundTripCost=ant.energy+1;

            for (Coord foodLocation:visibleFood){
                std::vector<Coord> pathToFood=shortestPath(this->terrainMap, ant.position,foodLocation);

                std::vector<Coord> pathHome=shortestPath(this->terrainMap,foodLocation,ant.homeCoord);

                int roundTripCost=calculatePathCost(this->terrainMap,pathToFood)+calculatePathCost(this->terrainMap,pathHome);

                if (roundTripCost<=ant.energy && roundTripCost<lowestRoundTripCost) {
                    lowestRoundTripCost=roundTripCost;
                    destination=foodLocation;
                    safeFoodFound=true;
                }
            }

            if (safeFoodFound) {
                ant.move(this->terrainMap, destination,this->foodMap);

                std::cout << "Ant "<<currentAntNumber<<" moved to ("<< ant.position.first << ", "<< ant.position.second << ")"<< " | carrying food: "<< ant.carryingFood<< " | energy: "<< ant.energy<< std::endl;
                continue;
            }
        }
        int explorationDistance=ant.foodRadius+1;

        int targetRow= ant.position.first+explorationDirection.first*explorationDistance;

        int targetColumn=ant.position.second+explorationDirection.second*explorationDistance;

        targetRow=std::clamp(targetRow,0,static_cast<int>(this->terrainMap.size())-1);

        targetColumn=std::clamp(targetColumn,0,static_cast<int>(this->terrainMap[0].size())-1);

        Coord explorationTarget=Coord(targetRow,targetColumn);

        std::vector<Coord> pathToExplorationTarget=shortestPath(this->terrainMap,ant.position,explorationTarget);
        std::vector<Coord> pathFromTargetHome = shortestPath(this->terrainMap,explorationTarget,ant.homeCoord);
        int explorationRoundTripCost=calculatePathCost(this->terrainMap,pathToExplorationTarget)+calculatePathCost(this->terrainMap,pathFromTargetHome);
        bool targetIsNew=explorationTarget != ant.position;

        if (targetIsNew && explorationRoundTripCost<=ant.energy){
            ant.move(this->terrainMap,explorationTarget,this->foodMap);

            std::cout << "Ant " << currentAntNumber<< " explored to ("<< ant.position.first << ", "<< ant.position.second << ")"<< " | carrying food: "<< ant.carryingFood<< " | energy: "<< ant.energy<< std::endl;

        }
        else if (ant.position != ant.homeCoord) {
            ant.returnHome(this->terrainMap,this->foodMap);
            std::cout << "Ant " << currentAntNumber<< " stopped exploring and returned to ("<< ant.position.first << ", "<< ant.position.second << ")"<< " | energy: "<< ant.energy<< std::endl;
        }
    }

}

/** You may insert any custom functions below **/
