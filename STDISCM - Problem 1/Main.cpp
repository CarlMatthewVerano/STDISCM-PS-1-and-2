#include "GraphConfig.h"
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>


static void edgesFormatter(const std::string& edgeToFormat, bool isLast) {
	size_t spacePos = edgeToFormat.find(' ');
	std::string node1 = edgeToFormat.substr(0, spacePos);
	std::string node2 = edgeToFormat.substr(spacePos + 1);

	std::cout << "(" << node1 << "," << node2 << ")";
	if (!isLast) {
		std::cout << ", ";
	}
	else {
		std::cout << std::endl;
	}
}

//void printGraph(std::map<std::string, std::vector<std::string>> adjList) {
//	for (const auto& pair : adjList) {
//		std::cout << pair.first << ": ";
//		for (const auto& vertex : pair.second) {
//			std::cout << vertex << " -> ";
//		}
//		std::cout << "NULL" << std::endl;
//	}
//
//}

void printNodes(std::map<std::string, std::vector<std::string>> adjList) {
	for (const auto& pair : adjList) {
		std::cout << pair.first << " ";
	}
	std::cout << std::endl;
}

void printEdges(std::map<std::string, std::vector<std::string>> adjList) {
	for (const auto& pair : adjList) {
		for (const auto& vertex : pair.second) {
			edgesFormatter(pair.first + " " + vertex, pair.first == adjList.rbegin()->first && vertex == pair.second.back());
		}
	}
}

void checkNodeExistInGraph(std::map<std::string, std::vector<std::string>> adjList, const std::string& query) {
	std::string nodeQuery = query.substr(5);

	bool nodeExists = false;
	for (auto i = adjList.begin(); i != adjList.end(); i++) {
		if (i->first == nodeQuery) {
			nodeExists = true;
			break; // Exit the loop early if the node is found
		}
	}
	if (nodeExists) {
		std::cout << "Node " << nodeQuery << " is in the graph" << std::endl;
	}
	else {
		std::cout << "Node " << nodeQuery << " is not in the graph" << std::endl;
	}
}

void nodeSearcher(const std::map<std::string, std::vector<std::string>>& part, const std::string& nodeQuery, bool& nodeExists/*, const std::string& runner*/) {
	for (const auto& pair : part) {
		if (pair.first == nodeQuery) {
			nodeExists = true;
			return; // Exit the thread early if the node is found
		}
	}
}


void checkNodeExistInGraphParallel(std::map<std::string, std::vector<std::string>> adjList, const std::string& query) {
	std::string nodeQuery = query.substr(5);

	bool nodeExists = false;

	std::vector<std::thread> workerThreads;
	int numThreads = 8;

	size_t totalSize = adjList.size();
	size_t baseSize = totalSize / numThreads;
	size_t remainder = totalSize % numThreads;

	auto it = adjList.begin();

	for (int i = 0; i < numThreads; i++) {
		std::map<std::string, std::vector<std::string>> part;
		size_t partSize = baseSize + (i < remainder ? 1 : 0);

		for (size_t j = 0; j < partSize; j++) {
			part.insert(*it);
			it++;
		}

		//workerThreads.emplace_back(nodeSearcher, part, nodeQuery, std::ref(nodeExists), "T-" + std::to_string(i));
		workerThreads.emplace_back(nodeSearcher, part, nodeQuery, std::ref(nodeExists));
	}

	for (auto& t : workerThreads) {
		t.join();
	}
	if (nodeExists) {
		std::cout << "Node " << nodeQuery << " is in the graph" << std::endl;
	}
	else {
		std::cout << "Node " << nodeQuery << " is not in the graph" << std::endl;
	}
}

void checkEdgeExistInGraph(std::map<std::string, std::vector<std::string>> adjList, const std::string& query) {
	std::string edgeQuery = query.substr(5);
	size_t spacePos = edgeQuery.find(' ');

	if (spacePos == std::string::npos) {
		std::cout << "INVALID EDGE QUERY" << std::endl;
		return;
	}

	std::string src = edgeQuery.substr(0, spacePos);
	std::string dest = edgeQuery.substr(spacePos + 1);

	bool found = false;

	if (adjList.find(src) != adjList.end()) {
		if (std::find(adjList[src].begin(), adjList[src].end(), dest) != adjList[src].end()) {
			std::cout << "Edge (" << src << "," << dest << ") is in the graph" << std::endl;
			found = true;
		}
	}

	if (!found) {
		std::cout << "Edge (" << src << "," << dest << ") is not in the graph" << std::endl;
	}
}

static bool dfsUtil(const std::string& src, const std::string& dest, std::unordered_set<std::string>& visited, std::vector<std::string>& path, std::map<std::string, std::vector<std::string>> adjList) {
	visited.insert(src);
	path.push_back(src);

	if (src == dest) {
		return true;
	}

	for (const std::string& neighbor : adjList[src]) {
		if (visited.find(neighbor) == visited.end()) {
			if (dfsUtil(neighbor, dest, visited, path, adjList)) {
				return true;
			}
		}
	}

	path.pop_back();
	return false;
}

static bool findPathDFS(const std::string& src, const std::string& dest, std::vector<std::string>& path, std::map<std::string, std::vector<std::string>> adjList) {
	std::unordered_set<std::string> visited;
	return dfsUtil(src, dest, visited, path, adjList);
}

static void checkPathExistInGraph(std::map<std::string, std::vector<std::string>> adjList, const std::string& query) {
	std::string pathQuery = query.substr(5);
	size_t spacePos = pathQuery.find(' ');

	if (spacePos == std::string::npos) {
		std::cout << "INVALID PATH QUERY" << std::endl;
		return;
	}

	std::string src = pathQuery.substr(0, spacePos);
	std::string dest = pathQuery.substr(spacePos + 1);

	std::vector<std::string> path;

	if (findPathDFS(src, dest, path, adjList)) {
		std::cout << "Path from " << src << " to " << dest << ": ";
		for (const std::string& node : path) {
			std::cout << node << " ";
		}
		std::cout << std::endl;
	}
	else {
		std::cout << "No path found from " << src << " to " << dest << std::endl;
	}
}


static void queries(std::map<std::string, std::vector<std::string>> adjList) {
	std::string query;
	while (true)
	{
		std::cout << "Graph Query: ";
		getline(std::cin, query);

		std::cout << "Response: ";

		if (query == "exit") {
			break;
		}
		else if (query == "nodes") {
			printNodes(adjList);
		}
		//else if (query.find("node ") == 0) {
		//	checkNodeExistInGraph(adjList, "MAIN", query);
		//}
		else if (query == "edges") {
			printEdges(adjList);
		}
		else if (query.find("edge ") == 0) {
			checkEdgeExistInGraph(adjList, query);
		}
		else if (query.find("path ") == 0) {
			checkPathExistInGraph(adjList, query);
		}
		else if (query.find("node ") == 0) {
			//demo02(adjList.size(), adjList, query);
			std::cout << std::endl;
			/*checkNodeExistInGraph(adjList, query);*/
			checkNodeExistInGraphParallel(adjList, query);
		}
		else {
			std::cout << "INVALID QUERY" << std::endl;
		}

	}
}


int main()
{
	GraphConfig graphConfig("graphFile2.txt");
	graphConfig.graphFileReader();
	queries(graphConfig.adjList);
}