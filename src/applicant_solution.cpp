//
// Created by dusan on 9/15/26.
//

#include "../include/antworld.h"


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
    for (Ant &ant : this->ants) {
        if (ant.carryingFood) {
            ant.returnHome(this->terrainMap, this->foodMap);
            continue;
        }

        std::vector<Coord>visibleFood=ant.foodScan(this->foodMap);

        if (!visibleFood.empty()) {
            Coord destination=visibleFood[0];

            ant.move(this->terrainMap,destination,this->foodMap);
        }
    }

}

/** You may insert any custom functions below **/
