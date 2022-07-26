#include "route_planner.h"
#include <algorithm>

RoutePlanner::RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y): m_Model(model) {
    // Convert inputs to percentage:
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;

    start_node = &m_Model.FindClosestNode(start_x, start_y);
    end_node = &m_Model.FindClosestNode(end_x, end_y);
}


// The distance to the end_node is the h value.
// Node objects have a distance method to determine the distance to another node.

float RoutePlanner::CalculateHValue(RouteModel::Node const *node) {
    return node->distance(*end_node);
}


// Use the FindNeighbors() method of the current_node to populate current_node.neighbors vector with all the neighbors.
// For each node in current_node.neighbors, set the parent, the h_value, the g_value.
// Use CalculateHValue below to implement the h-Value calculation.
// For each node in current_node.neighbors, add the neighbor to open_list and set the node's visited attribute to true.

void RoutePlanner::AddNeighbors(RouteModel::Node *current_node) {
    current_node->FindNeighbors();

    for (RouteModel::Node *neighbor : current_node->neighbors) {
        // set parent, g_value (distance from start), h_value (heuristic/estimate; distance to end)
        neighbor->parent = current_node;
        neighbor->g_value = current_node->g_value + current_node->distance(*neighbor);
        neighbor->h_value = CalculateHValue(neighbor);
        // add to open_list to process this neighbor and mark as visited
        open_list.push_back(neighbor);
        neighbor->visited = true;
    }
}


// Sort the open_list according to the sum of the h value and g value.
// Create a pointer to the node in the list with the lowest sum.
// Remove that node from the open_list.
// Return the pointer.

bool CompareNodes(RouteModel::Node *node1,RouteModel::Node *node2){
    return (node1->g_value + node1->h_value) < (node2->g_value + node2->h_value);
}

RouteModel::Node *RoutePlanner::NextNode() {
    std::sort(open_list.begin(), open_list.end(), CompareNodes);
    RouteModel::Node *next_node = open_list.front();
    open_list.erase(open_list.begin());
    return next_node;
}

// This method should take the current (final) node as an argument and iteratively follow the
// chain of parents of nodes until the starting node is found.
// For each node in the chain, add the distance from the node to its parent to the distance variable.
// The returned vector should be in the correct order: the start node should be the first element
// of the vector, the end node should be the last element.

std::vector<RouteModel::Node> RoutePlanner::ConstructFinalPath(RouteModel::Node *current_node) {
    std::vector<RouteModel::Node> path_found;
    distance = 0.0f;

    while (current_node -> parent != nullptr) {
        path_found.push_back(*current_node);
        distance += current_node->distance(*current_node->parent);
        current_node = current_node->parent;
    }

    path_found.push_back(*current_node);
    std::reverse(std::begin(path_found),std::end(path_found));
    distance *= m_Model.MetricScale();

    return path_found;

}


// The A* Search algorithm.
// Use the AddNeighbors method to add all the neighbors of the current node to the open_list.
// Use the NextNode() method to sort the open_list and return the next node.
// When the search has reached the end_node, use the ConstructFinalPath method to return the final path that was found.
// Store the final path in the m_Model.path attribute before the method exits. This path will then be displayed on the map tile.

void RoutePlanner::AStarSearch() {
    RouteModel::Node *current_node = nullptr;
    start_node->visited = true;
    open_list.push_back(start_node);

    while(!open_list.empty()) {
        current_node = NextNode();
        if (current_node->distance(*end_node) == 0) {
            m_Model.path = ConstructFinalPath(current_node);
            return;
        }
        AddNeighbors(current_node);
    }

    return;

}