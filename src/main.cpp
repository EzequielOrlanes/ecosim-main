#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>
#include <thread>
#include <algorithm>

using namespace std;
//flags to enable type of use at threads depends on case.
const bool ThreadParallelFlag = true;
const bool ThreadSequentialFlag = false;
//using mutex to control acess to resource.
std::mutex mtx;

static const uint32_t NUM_ROWS = 15;
// Constants Initial Values
const uint32_t INITIAL_ENERGY_HERBIVORE = 100;
const uint32_t INITIAL_PLANT_AGE = 0;

// Constants
const uint32_t PLANT_MAXIMUM_AGE = 10;
const uint32_t HERBIVORE_MAXIMUM_AGE = 50;
const uint32_t CARNIVORE_MAXIMUM_AGE = 80;
const uint32_t MAXIMUM_ENERGY = 200;
const uint32_t THRESHOLD_ENERGY_FOR_REPRODUCTION = 20;
// Probabilities
const double PLANT_REPRODUCTION_PROBABILITY = 0.2;
const double HERBIVORE_REPRODUCTION_PROBABILITY = 0.075;
const double CARNIVORE_REPRODUCTION_PROBABILITY = 0.025;
const double HERBIVORE_MOVE_PROBABILITY = 0.7;
const double HERBIVORE_EAT_PROBABILITY = 0.9;
const double CARNIVORE_MOVE_PROBABILITY = 0.5;
const double CARNIVORE_EAT_PROBABILITY = 1.0;

// Type definitions
enum entity_type_t {
    empty,
    plant,
    herbivore,
    carnivore
};

struct pos_t {
    uint32_t i;
    uint32_t j;
};

struct entity_t {
    entity_type_t type;
    int32_t energy;
    int32_t age;
};

// Auxiliary code to convert the entity_type_t enum to a string
NLOHMANN_JSON_SERIALIZE_ENUM(entity_type_t, {
{empty, " "},
{plant, "P"},
{herbivore, "H"},
{carnivore, "C"},
})

// Auxiliary code to convert the entity_t struct to a JSON object
namespace nlohman {
    void to_json(nlohmann::json &j, const entity_t &e) {
        j = nlohmann::json{{"type", e.type}, {"energy", e.energy}, {"age", e.age}};
    }
}
//That's create a random number each time that's called.
std::mt19937 createRandomGenerator() {
  static random_device rd;
  return mt19937(rd());  
}

//Cheking when age timing end and return true if this timing of life ending.
bool check_age(entity_t &current_entity){
    if(current_entity.type == plant && current_entity.age >= PLANT_MAXIMUM_AGE) return true;
    if(current_entity.type == herbivore && current_entity.age >= HERBIVORE_MAXIMUM_AGE) return true;
    if (current_entity.type == carnivore && current_entity.age >= CARNIVORE_MAXIMUM_AGE) return true;
}

bool limits_grid(uint32_t adjacent_i, uint32_t adjacent_j){
      if (adjacent_i >= 0 && adjacent_i < NUM_ROWS && adjacent_j >= 0 && adjacent_j < NUM_ROWS) return true; 
}

void growing_plant(uint32_t i, uint32_t j,
                std::vector<std::vector<entity_t>> &updated_grid){
                if(ThreadParallelFlag) std::lock_guard<std::mutex> lock(mtx);
if ((rand() / (double)RAND_MAX) < PLANT_REPRODUCTION_PROBABILITY) {
                        // Calcula as posições das células vizinhas
                        std::vector<pos_t> adjacent_cells = {
                            {i - 1, j}, // UP
                            {i + 1, j}, // DOWN
                            {i, j - 1}, // LEFT
                            {i, j + 1}  // RIGHT
                        };
                        // Embaralha aleatoriamente as posições das células vizinhas
                        std::random_shuffle(adjacent_cells.begin(), adjacent_cells.end());
                        for (const pos_t &adjacent_pos : adjacent_cells) {
                            uint32_t adjacent_i = adjacent_pos.i;
                            uint32_t adjacent_j = adjacent_pos.j;
                            // Verifica se a célula vizinha está dentro dos limites do grid
                            if (limits_grid(adjacent_i, adjacent_j)) {
                                entity_t &target_entity = updated_grid[adjacent_i][adjacent_j];
                                // Verifica se a célula vizinha está vazia (empty)
                                if (target_entity.type == entity_type_t::empty) {
                                    // Cria uma nova planta na célula vizinha vazia
                                    target_entity.type = plant;
                                    target_entity.energy = 0; // A energia da planta pode ser mantida como 0
                                    target_entity.age = 0;    // A idade da planta é reiniciada
                                    break; // O crescimento da planta ocorreu com sucesso
                                }
                            }
                        }
                    }
}
void herbiviral_movement(uint32_t i, uint32_t j, std::vector<std::vector<entity_t>> &updated_grid,
entity_t &updated_entity,entity_t &current_entity){
    if(ThreadParallelFlag) std::lock_guard<std::mutex> lock(mtx);
    if ((rand() / (double)RAND_MAX) < HERBIVORE_MOVE_PROBABILITY) {
                            // Calcula as posições das células vizinhas 
                            std::vector<pos_t> adjacent_cells = {
                                {i - 1, j}, // UP
                                {i + 1, j}, // DOWN
                                {i, j - 1}, // LEFT
                                {i, j + 1}  // RIGHT
                            };
                            // Embaralha aleatoriamente as posições das células vizinhas
                            random_shuffle(adjacent_cells.begin(), adjacent_cells.end());
                            for (const pos_t &adjacent_pos : adjacent_cells) {
                                uint32_t adjacent_i = adjacent_pos.i;
                                uint32_t adjacent_j = adjacent_pos.j;
                                // Verifica se a célula vizinha está dentro dos limites do grid
                                if (limits_grid(adjacent_i, adjacent_j)) {
                                    entity_t &target_entity = updated_grid[adjacent_i][adjacent_j];
                                    // Verifica se a célula vizinha está vazia e não contém um carnívoro
                                    if (target_entity.type == entity_type_t::empty && target_entity.type != entity_type_t::carnivore) {
                                        // Move o herbívoro para a célula vizinha
                                        updated_entity.type = entity_type_t::empty;
                                        updated_entity.energy = 0; // Custo de energia pelo movimento do herbírovo
                                        target_entity.type = herbivore;
                                        target_entity.energy = current_entity.energy - 5;
                                        target_entity.age = current_entity.age + 1;
                                        break; // O herbívoro moveu-se com sucesso
                                    }
                                }
                            }
                        }

}
void herbiviral_eating(uint32_t i, uint32_t j, std::vector<std::vector<entity_t>> &updated_grid,
entity_t &updated_entity,entity_t &current_entity){
    if(ThreadParallelFlag) std::lock_guard<std::mutex> lock(mtx);
    if ((rand() / (double)RAND_MAX) < HERBIVORE_EAT_PROBABILITY) {
                            // Calcula as posições das células vizinhas (acima, abaixo, esquerda, direita)
                            std::vector<pos_t> adjacent_cells = {
                                {i - 1, j}, // UP
                                {i + 1, j}, // DOWN
                                {i, j - 1}, // LEFT
                                {i, j + 1}  // RIGHT
                            };
                            // Verifica se alguma célula adjacente contém uma planta
                            for (const pos_t &adjacent_pos : adjacent_cells) {
                                uint32_t adjacent_i = adjacent_pos.i;
                                uint32_t adjacent_j = adjacent_pos.j;
                                // Verifica se a célula vizinha está dentro dos limites do grid
                                if (limits_grid(adjacent_i, adjacent_j)) {
                                    entity_t &target_entity = updated_grid[adjacent_i][adjacent_j];
                                    // Verifica se a célula adjacente contém uma planta
                                    if (target_entity.type == plant) {
                                        // O herbívoro come a planta
                                        updated_entity.energy += 30;
                                        current_entity.energy += 30; // Ganho de energia ao comer uma planta
                                        target_entity.type = entity_type_t::empty; // A planta é removida
                                        target_entity.energy = 0;   // A célula fica vazia
                                        break; // O herbívoro comeu com sucesso
                                    }
                                }
                            }
                        }
}

void herbiviral_energy_reproduction(uint32_t i, uint32_t j, std::vector<std::vector<entity_t>> &updated_grid,
entity_t &updated_entity,entity_t &current_entity){
      if(ThreadParallelFlag) std::lock_guard<std::mutex> lock(mtx);
      if (current_entity.energy > THRESHOLD_ENERGY_FOR_REPRODUCTION && 
                            (rand() / (double)RAND_MAX) < HERBIVORE_REPRODUCTION_PROBABILITY) {
                            // Verifica se a energia do herbívoro é suficiente para reprodução
                            if (current_entity.energy >= 10) {
                                // Calcula as posições das células vizinhas (acima, abaixo, esquerda, direita)
                                std::vector<pos_t> adjacent_cells = {
                                    {i - 1, j}, // UP
                                    {i + 1, j}, // DOWN
                                    {i, j - 1}, // LEFT
                                    {i, j + 1}  // RIGHT
                                };
                                // Embaralha aleatoriamente as posições das células vizinhas
                                std::random_shuffle(adjacent_cells.begin(), adjacent_cells.end());
                                for (const pos_t &adjacent_pos : adjacent_cells) {
                                    uint32_t adjacent_i = adjacent_pos.i;
                                    uint32_t adjacent_j = adjacent_pos.j;
                                    // Verifica se a célula vizinha está dentro dos limites do grid
                                    if (limits_grid(adjacent_i, adjacent_j)) {
                                        entity_t &target_entity = updated_grid[adjacent_i][adjacent_j];
                                        // Verifica se a célula vizinha está vazia (empty)
                                        if (target_entity.type == entity_type_t::empty) {
                                            // O herbívoro se reproduz
                                            updated_entity.energy -= 10;
                                            current_entity.energy -= 10; // Custo de energia da reprodução
                                            target_entity.type = herbivore;
                                            target_entity.energy = 20;  // Energia inicial da prole
                                            target_entity.age = 0;      // Idade da prole começa em 0
                                            break; // A reprodução do herbívoro ocorreu com sucesso
                                        }
                                    }
                                }
                            }
                        }
}
void carnivore_movement(uint32_t i, uint32_t j, std::vector<std::vector<entity_t>> &updated_grid,
entity_t &updated_entity,entity_t &current_entity){
    if(ThreadParallelFlag) std::lock_guard<std::mutex> lock(mtx);
    if ((rand() / (double)RAND_MAX) < CARNIVORE_MOVE_PROBABILITY) {
                        // Calcula as posições das células vizinhas (acima, abaixo, esquerda, direita)
                        std::vector<pos_t> adjacent_cells = {
                            {i - 1, j}, // UP
                            {i + 1, j}, // DOWN
                            {i, j - 1}, // LEFT
                            {i, j + 1}  // RIGHT
                        };
                        // Embaralha aleatoriamente as posições das células vizinhas
                        std::random_shuffle(adjacent_cells.begin(), adjacent_cells.end());
                        for (const pos_t &adjacent_pos : adjacent_cells) {
                            uint32_t adjacent_i = adjacent_pos.i;
                            uint32_t adjacent_j = adjacent_pos.j;
                            // Verifica se a célula vizinha está dentro dos limites do grid
                            if (limits_grid(adjacent_i, adjacent_j)) {
                                entity_t &target_entity = updated_grid[adjacent_i][adjacent_j];
                                // Move o carnívoro para a célula vizinha
                                updated_entity.type = entity_type_t::empty;
                                updated_entity.energy = 0; // Custo de energia pelo movimento
                                target_entity.type = carnivore;
                                target_entity.energy = current_entity.energy - 5;
                                target_entity.age = current_entity.age + 1;
                                break; // O carnívoro moveu-se com sucesso
                            }
                        }
                    }}

void carnivore_eating(uint32_t i, uint32_t j, std::vector<std::vector<entity_t>> &updated_grid,
entity_t &updated_entity,entity_t &current_entity){
    if(ThreadParallelFlag) std::lock_guard<std::mutex> lock(mtx);
    // Verifica se alguma célula adjacente contém um herbívoro
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            uint32_t adjacent_i = i + dx;
            uint32_t adjacent_j = j + dy;
            // Verifica se a célula vizinha está dentro dos limites do grid
            if (limits_grid(adjacent_i, adjacent_j)) {
                entity_t &target_entity = updated_grid[adjacent_i][adjacent_j];
                    // Verifica se a célula adjacente contém um herbívoro
                    if (target_entity.type == herbivore) {
                    // O carnívoro come o herbívoro
                    updated_entity.energy += 20;
                    current_entity.energy += 20; // Ganho de energia ao comer um herbívoro
                    target_entity.type = entity_type_t::empty;  // O herbívoro é removido
                    target_entity.energy = 0;    // A célula fica vazia
                    }
            }
        }
    }
}

void carnivore_energy_reproduction(uint32_t i, uint32_t j, std::vector<std::vector<entity_t>> &updated_grid,
entity_t &updated_entity,entity_t &current_entity){ 
    if(ThreadParallelFlag) std::lock_guard<std::mutex> lock(mtx);
    if (current_entity.energy > THRESHOLD_ENERGY_FOR_REPRODUCTION && 
                        (rand() / (double)RAND_MAX) < CARNIVORE_REPRODUCTION_PROBABILITY) {
                        // Verifica se a energia do carnívoro é suficiente para reprodução
                            if (current_entity.energy >= 10) {
                            // Calcula as posições das células vizinhas (acima, abaixo, esquerda, direita)
                            std::vector<pos_t> adjacent_cells = {
                                {i - 1, j}, // UP
                                {i + 1, j}, // DOWN
                                {i, j - 1}, // LEFT
                                {i, j + 1}  // RIGHT
                            };
                            // Embaralha aleatoriamente as posições das células vizinhas
                            std::random_shuffle(adjacent_cells.begin(), adjacent_cells.end());
                            for (const pos_t &adjacent_pos : adjacent_cells) {
                                uint32_t adjacent_i = adjacent_pos.i;
                                uint32_t adjacent_j = adjacent_pos.j;
                                // Verifica se a célula vizinha está dentro dos limites do grid
                                if (limits_grid(adjacent_i, adjacent_j)) {
                                    entity_t &target_entity = updated_grid[adjacent_i][adjacent_j];
                                    // Verifica se a célula vizinha está vazia (empty)
                                    if (target_entity.type == entity_type_t::empty) {
                                        // O carnívoro se reproduz
                                        updated_entity.energy -= 10;
                                        current_entity.energy -= 10; // Custo de energia da reprodução
                                        target_entity.type = carnivore;
                                        target_entity.energy = 20;  // Energia inicial da prole
                                        target_entity.age = 0;      // Idade da prole começa em 0
                                        break; // A reprodução do carnívoro ocorreu com sucesso
                                    }
                                }
                            }
                        }
                    }
}
// Grid that contains the entities
static std::vector<std::vector<entity_t>> entity_grid;
// static std::vector<std::vector<entity_t>> updated_grid = entity_grid ;
int main() {  
    crow::SimpleApp app;
    // Endpoint to serve the HTML page
    CROW_ROUTE(app, "/")
    ([](crow::request &, crow::response &res) {
        // Return the HTML content here
        res.set_static_file_info_unsafe("../public/index.html");
        res.end(); });
    CROW_ROUTE(app, "/start-simulation")
        .methods("POST"_method)([](crow::request &req, crow::response &res) { 
        // Parse the JSON request body
        nlohmann::json request_body = nlohmann::json::parse(req.body);
       // Validate the request body 
        uint32_t total_entities = (uint32_t)request_body["plants"] + (uint32_t)request_body["herbivores"] + (uint32_t)request_body["carnivores"];
    if (total_entities > NUM_ROWS * NUM_ROWS) {
        res.code = 400;
        res.body = "Too many entities";
        res.end();
        return;
    }
    // Clear the entity grid
    entity_grid.clear();
    entity_grid.assign(NUM_ROWS, vector<entity_t>(NUM_ROWS, { entity_type_t::empty, 0, 0}));
    // Create the entities
    // <YOUR CODE HERE>
        int row, col;
        auto numRandom = createRandomGenerator();
        for( int i = 0; i < (uint32_t)request_body["plants"]; i++) {   
            std::uniform_int_distribution<> dis(0, 14);
            row = dis(numRandom);
            col = dis(numRandom);
            while(!entity_grid[row][col].type == entity_type_t::empty){
                row = dis(numRandom);
                col = dis(numRandom);
            }
            entity_grid[row][col].type = plant;
            entity_grid[row][col].age = 0; 
        }
        for( int i = 0; i < (uint32_t)request_body["herbivores"]; i++){
            std::uniform_int_distribution<> dis(0, 14);
            row = dis(numRandom);
            col = dis(numRandom);
            while(!entity_grid[row][col].type == entity_type_t::empty){
                row = dis(numRandom);
                col = dis(numRandom);
            }
            entity_grid[row][col].type = herbivore;
            entity_grid[row][col].age = 0;
            entity_grid[row][col].energy = 100; 
        }
        for( int i = 0; i < (uint32_t)request_body["carnivores"]; i++){
            std::uniform_int_distribution<> dis(0, 14);
            row = dis(numRandom);
            col = dis(numRandom);
            while(!entity_grid[row][col].type == entity_type_t::empty){
                row = dis(numRandom);
                col = dis(numRandom);
            }
            entity_grid[row][col].type = carnivore;
            entity_grid[row][col].age = 0;
            entity_grid[row][col].energy = 100;
        }
        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        res.body = json_grid.dump();
        res.end(); });
        // Endpoint to process HTTP GET requests for the next simulation iteration
        CROW_ROUTE(app, "/next-iteration")
        .methods("GET"_method)([](){
        // Simulate the next iteration
        // Iterate over the entity grid and simulate the behaviour of each entity
        // <YOUR CODE HERE>
        // Return the JSON representation of the entity grid
    std::vector<std::vector<entity_t>> updated_grid = entity_grid;
        for (uint32_t i = 0; i < NUM_ROWS; ++i) {
            for (uint32_t j = 0; j < NUM_ROWS; ++j) {
                entity_t &current_entity = entity_grid[i][j];
                entity_t &updated_entity = updated_grid[i][j]; 
                // Skiping all empty cells
                if (current_entity.type == entity_type_t::empty) continue;
                // Update the age
                updated_entity.age++;
                //Cheking if age to entities and killer when getting the age.
                if(check_age(current_entity)){
                     updated_entity.type = entity_type_t::empty;
                     updated_entity.energy = 0;
                }
                if(current_entity.energy <= 0){
                    updated_entity.type = entity_type_t::empty;
                    updated_entity.energy = 0;
                    updated_entity.age = 0;
                }
                else {
                // Implement growth and additional requirements to plants
                if (current_entity.type == plant) growing_plant(i,j, updated_grid);
                // Implement movementing to herbivores
                if (current_entity.type == herbivore) herbiviral_movement(i, j, updated_grid, updated_entity, current_entity );
                // Implement eating for herbivores
                if (current_entity.type == herbivore) herbiviral_eating(i, j,updated_grid, updated_entity, current_entity);
                // Implement reproduction and energy update for herbivores
                if (current_entity.type == herbivore) herbiviral_energy_reproduction(i,j,updated_grid, updated_entity, current_entity);
                }
                // Implement movement for carnivores
                if (current_entity.type == carnivore) carnivore_movement(i, j,updated_grid, updated_entity, current_entity);
                // Implement eating for carnivores
                if (current_entity.type == carnivore) carnivore_eating(i, j,updated_grid, updated_entity, current_entity);
                // Implement reproduction and energy update for carnivores
                if (current_entity.type == carnivore) carnivore_energy_reproduction(i,j,updated_grid, updated_entity, current_entity);
            }
        }

    if(ThreadParallelFlag){
    //threads
    thread new_plants_thread(growing_plant);
    thread new_herbivores_movements(herbiviral_movement);
    thread new_herbiviral_eating(herbiviral_eating);
    thread new_herbiviral_energy_reproduction(herbiviral_energy_reproduction);
    thread new_carnivore_movement(carnivore_movement);
    thread new_carnivore_eating(carnivore_eating);
    thread new_carnivore_energy_reproduction(carnivore_energy_reproduction);
    //join
    new_plants_thread.join();
    new_herbivores_movements.join();
    new_herbiviral_eating.join();
    new_herbiviral_energy_reproduction.join();
    new_carnivore_movement.join();
    new_carnivore_eating.join();
    new_carnivore_energy_reproduction.join();
    }

    if(ThreadSequentialFlag){
    thread new_plants_thread(growing_plant);
    new_plants_thread.join();
    thread new_herbivores_movements(herbiviral_movement);
    new_herbivores_movements.join();
    thread new_herbiviral_eating(herbiviral_eating);
    new_herbiviral_eating.join();
    thread new_herbiviral_energy_reproduction(herbiviral_energy_reproduction);
    new_herbiviral_energy_reproduction.join();
    thread new_carnivore_movement(carnivore_movement);
    new_carnivore_movement.join();
    thread new_carnivore_eating(carnivore_eating);
    new_carnivore_eating.join();
    thread new_carnivore_energy_reproduction(carnivore_energy_reproduction);
    new_carnivore_energy_reproduction.join();
    }
    // Update the entity grid with the updated entities
    entity_grid = updated_grid;
    nlohmann::json json_grid = entity_grid; 
    return json_grid.dump(); });
    app.port(8080).run();
    return 0;
}